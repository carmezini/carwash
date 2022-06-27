#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#define THREAD_NUM 5

int vagas_ocupadas = 0;
int limite_vagas = 5; //no máximo 5 vagas
sem_t cliente, funcionario, cliente_pronto, funcionario_pronto;
pthread_mutex_t mutex;

void clienteDesiste(int index) {
    printf("Cliente %d saiu da loja por que não havia mais vagas\n", index);
    pthread_exit(NULL);
}

void lavandoCarro(void) {
    sleep(rand() % 8);
    printf("funcionário lavou um carro\n");
}

void carroEmLavagem(int index) {
    sem_wait(&cliente);
    sem_wait(&funcionario);
    printf("Cliente %d posicionou carro para a lavagem.\n", index);
}

void clienteSaiLoja(int index) {
    printf("Cliente %d saiu da loja.\n", index);
    pthread_exit(NULL);
}

void* clienteEntraLoja(void* args) {
    int index = *(int*)args;
    int semValueFuncionario;
    sem_getvalue(&funcionario, &semValueFuncionario);
    if ( (semValueFuncionario == 0) && (vagas_ocupadas == limite_vagas) ) //se não tiver vagas
        clienteDesiste(index);
    else
        printf("Cliente %d entrou na loja. \n", index);
        pthread_mutex_lock(&mutex);
        vagas_ocupadas++;
        carroEmLavagem(index);
        lavandoCarro();
        clienteSaiLoja(index);
        sem_post(&funcionario);
        sem_post(&cliente);
        free(args);
        vagas_ocupadas--;
        pthread_mutex_unlock(&mutex);
        sem_post(&cliente_pronto);
        sem_post(&funcionario_pronto);
}

int main(int argc, char *argv[]) {
    pthread_t thcliente[THREAD_NUM];
    sem_init(&cliente, 0, 5);
    sem_init(&funcionario, 0, 1);
    sem_init(&cliente_pronto, 0, 1);
    sem_init(&funcionario_pronto, 0, 1);
    int i;
    for (i = 0; i < THREAD_NUM; i++) {
        int* pos = malloc(sizeof(int));
        *pos = i;
        if (pthread_create(&thcliente[i], NULL, &clienteEntraLoja, pos) != 0) {
            perror("Failed to create thread");
        }
    }

    for (i = 0; i < THREAD_NUM; i++) {
        if (pthread_join(thcliente[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    
    sem_destroy(&cliente);
    sem_destroy(&funcionario);
    sem_destroy(&cliente_pronto);
    sem_destroy(&funcionario_pronto);
}