#include <numeric>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>
#include "./utils.cpp"
#include <cstring>
#include <fstream>

using namespace std;

int totalSum = 0;    //Acumulador compartilhado
atomic_flag lock_stream = ATOMIC_FLAG_INIT;  

void acquire(atomic_flag *lock)
{
    while (lock->test_and_set());
}

void release(atomic_flag *lock)
{
    lock->clear();
}

vector<char> populateVector(int size)
{   //Gerando vetores de tamanho size com números aleatróios entre [-100,100]
    vector<char> array;
    for (int i = 0; i < size; i++)
    {
        int num = (rand() % 201) - 100;  
        array.push_back((char) num);
    }
    return array;
}

void sumVector(vector<char> vectorToSum, int numThreads ){
   
    int localSum;
    for (int i=0; i<vectorToSum.size(); i++){
          localSum += (int) vectorToSum[i]; 
    }
    acquire(&lock_stream);  
    //Região crítica
    totalSum += localSum;
    release(&lock_stream);     
}


float adderWithSpinlocks(vector<char>  numbersVector, int K, int N) {

        double totalTime = 0;


        int vectorSize = N / K;   //Utilizando a sugestão do professor de dividir  N/K


        //Dividindo o vetor em subvetores
        vector<vector<char>> vectorOfChunks = splitVectorIntoChunks(numbersVector, vectorSize);
        
        vector<thread> v;

        // Começando a contar o tempo
        auto start = chrono::system_clock::now();

        for (int i = 0; i < K ; i++)
        {
            vector<char> chunk = vectorOfChunks[i];            
            thread thread_n(sumVector, chunk, i);
            v.push_back((thread &&)(thread_n));
        }

        for (int i = 0; i < K; i++)
        {
            v[i].join();
        }

        // Finalizando a contagem do tempo
        auto end = chrono::system_clock::now();

        chrono::duration<double> elapsed = end - start;

        totalTime = totalTime + elapsed.count();    

        return totalTime;  

}

float execute(int K, int N){

    int numberOfExecutions = 10;  
    double totalTimeForNumberOfExecutions = 0;
    vector<char> numbersVector = populateVector(N); //Populando o vetor de tamanho N com números no intervalo [-100,100] 

     for (int i = 1; i <= numberOfExecutions; i++)    {
         totalTimeForNumberOfExecutions+= adderWithSpinlocks(numbersVector, K, N);

     }
    printf("-----------------------------------------------------");        
    printf("Número de threads %i\n" , K);
    printf("Número de elementos no vetor inicial %i\n" , N);
    printf("Soma total %i\n" , totalSum);
    printf("Tempo médio para 10 iterações %f\n", totalTimeForNumberOfExecutions/numberOfExecutions);        
    printf("------------------------------------------------------");        

     
    return  totalTimeForNumberOfExecutions/numberOfExecutions;
}



int main(){

    ofstream resultFile;
    resultFile.open("results.csv"); 
    resultFile << "Número de Threads" << "," << "Tempo(s)" << endl;     

    for (int n = 10000000; n<= 1000000000;){
      
        for (int k = 1; k <=  256;){         
            resultFile << to_string(k)  <<  "," << to_string(execute(k, n))  << endl;          
        k = k*2;
        }        
     
        n=n*10;             
    }    

    return 0;
}