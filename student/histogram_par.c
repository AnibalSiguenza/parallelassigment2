#include "histogram.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "names.h"

int current_avaiable_chunck;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct pthread_args{
    char * buffer;
    char dummy[64];
    histogram_t histogram;
};

void * count_words_in_chunk(void * ptr){
	char current_word[20] = "";
    struct pthread_args * args=(struct pthread_args *)ptr;
	int c = 0;
    int init;
    while(args->buffer[current_avaiable_chunck]!=TERMINATOR){
        //Begining of critic section
        pthread_mutex_lock(&mutex);
        init=current_avaiable_chunck;
        current_avaiable_chunck=current_avaiable_chunck+CHUNKSIZE;
        //End of critiv section
        pthread_mutex_unlock(&mutex);
        for (int i=0; i<CHUNKSIZE; i++) {
            if(isalpha(args->buffer[i+init])&& i%CHUNKSIZE!=0){
                current_word[c++] = args->buffer[i+init];
            } else {
                current_word[c] = '\0';
                int res = getNameIndex(current_word);
                if (res != -1)
                    args->histogram[res]++;
                c = 0;
            }
	    }
    }
    return NULL;
}    
void get_histogram(char *buffer, int* histogram, int num_threads){
    current_avaiable_chunck=0;
    pthread_t * thread=(pthread_t*)malloc(num_threads*sizeof(*thread));
    struct pthread_args * arg ;
    arg = (struct pthread_args *)calloc(num_threads,sizeof(*arg));
    for(int i=0;i<num_threads;i++){
        arg[i].buffer=buffer;
        pthread_create(thread+i,NULL,&count_words_in_chunk,arg+i);
    }
    for(int i=0;i<num_threads;i++){
        pthread_join(thread[i],NULL);
        for(int j=0;j<NNAMES;j++){
            histogram[j]+=arg[i].histogram[j];
        }
    }
    free(thread);
    free(arg);
}
