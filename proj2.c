#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<semaphore.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<sys/wait.h>
//Author : Pavel Stepanov ( xstepa77 )

sem_t *mutex;                                  //semaphore for critical section of urednik
sem_t *output;                                 //semaphore for output to file (fprintf)
sem_t *sem_queue1;                             //semaphore that signals for zakaznik in queue 1 (fronta 1) that urednik is free
sem_t *sem_queue2;                             //semaphore that signals for zakaznik in queue 2 (fronta 2) that urednik is free
sem_t *sem_queue3;                             //semaphore that signals for zakaznik in queue 3 (fronta 3) that urednik is free
sem_t *zakaznik_coming1;                       //semaphore that signals for urednik that zakaznik is coming to him from queue 1 (fronta 1)
sem_t *zakaznik_coming2;                       //semaphore that signals for urednik that zakaznik is coming to him from queue 2 (fronta 2)
sem_t *zakaznik_coming3;                       //semaphore that signals for urednik that zakaznik is coming to him from queue 3 (fronta 3)

int *queue1 = 0;                               //number of zakaznik in queue 1 (fronta 1)
int *queue2 = 0;                               //number of zakaznik in queue 2 (fronta 2)
int *queue3 = 0;                               //number of zakaznik in queue 3 (fronta 3)

FILE *file;                                    //file for output (proj2.out)

int NZ, NU, TZ, TU, F;                         //arguments from command line
int *output_counter = 0;                       //counter of lines in output file
int *posta_open = 0;                           //counter of opened posta (if it is 0, posta is closed, if it is 1, posta is opened)

void urednik(int id);                          //declaration of function urednik
void zakaznik(int id);                         //declaration of function zakaznik
void init_sem();                               //declaration of function that initializes semaphores
void destroy_sem();                            //declaration of function that destroys semaphores
int randomforfronta();                         //declaration of function that returns random number for fronta (ONLY FOR UREDNIK)
int randomforsleep(int value, int arg);        //declaration of function that returns random number for usleep



int randomforsleep(int value, int arg){        //function that returns random number for usleep
    if(value == 1 && arg == 5) { 
        srand(time(NULL) * getpid());          //if F = 1, then return 0 or 1 
        int rndm = 1000 *(rand() % 2 );
        return rndm;
    }
    if(value == 0){                            //if argument is 0, return 0
        return 0;                   
    }
    else if(arg != 5){                         //if it is not F, then return random number from 0 to value
        srand(time(NULL) * getpid());
        int rndm = (1000 *(rand() % value + 1) );
        return rndm;
    }
    else{                                      // if it is F, then return random number from value/2 to value
        srand(time(NULL) * getpid());
        int rndm = (1000 *(rand() % (value / 2) + value / 2));
        return rndm;
    }
}


int randomforfronta(){                         //function that optimizes random number for fronta
    int random;
    srand(time(NULL) * getpid());              //seed for random number
    if((*queue1) > 0 && (*queue2) > 0 && (*queue3) > 0){ //if all queues are not empty, then return random number 1 or 3
        random = (rand() % 3) + 1;
        return random;
    }
    else if((*queue1) > 0 && (*queue2) > 0){   //if queue 1 and queue 2 are not empty, then return random number 1 or 2
        random = (rand() % 2) + 1;
        return random;
    }
    else if((*queue2) > 0 && (*queue3) > 0){   //if queue 2 and queue 3 are not empty, then return random number 2 or 3
        random = (rand() % 2) + 2;
        return random;
    }
    else if((*queue1) > 0 && (*queue3) > 0){   //if queue 1 and queue 3 are not empty, then return random number 1 or 3
        random = (rand() % 2) + 1;
        if(random == 1){
            return random;
        }
        else{
            return random + 2;
        }
    }
    else if ((*queue1) > 0) return 1;          //if only queue 1 is not empty, then return 1
    else if ((*queue2) > 0) return 2;          //if only queue 2 is not empty, then return 2
    else if ((*queue3) > 0) return 3;          //if only queue 3 is not empty, then return 3
    else{                                      //in any other case, return random number from 1 to 3
        random = (rand() % 3) + 1;
        return random;
    }
}


void urednik(int id){   //PROCESS UREDNIK
    //beginning of process.
    bool CLRK = true;
    sem_wait(output); 
    fprintf(file, "%d: U %d: started\n", ++(*output_counter), id);
    sem_post(output);
    while(CLRK == true){
        sem_wait(mutex);                     //waits until other urednik make his choice of queue
        int fronta = randomforfronta();      //random number for fronta
        if (fronta == 1 && ((*queue1) > 0 )){   //if queue1 is not empty
            *queue1 = (*queue1) - 1;            //decrement number of zakaznik in queue1, because he is going to be served
            sem_post(mutex);                    //post mutex for other uredniks
            sem_post(sem_queue1);               //send signal to zakazniks that they can come to urednik
            sem_wait(zakaznik_coming1);         //waits for a signal from zakaznik that he is coming to urednik
            //service of zakaznik
            sem_wait(output); 
            fprintf(file, "%d: U %d: serving a service of type 1\n", ++(*output_counter), id);
            sem_post(output);
            //finish of service
            usleep(randomforsleep(11, 0));      //sleep for random time from 0 to 10
            sem_wait(output);
            fprintf(file, "%d: U %d: service finished\n", ++(*output_counter), id);
            sem_post(output);
        }
        else if(fronta == 2 && ((*queue2) > 0)){ //if queue2 is not empty
            *queue2 = (*queue2) - 1;            //decrement number of zakaznik in queue2, because he is going to be served
            sem_post(mutex);                    //post mutex for other uredniks
            sem_post(sem_queue2);               //send signal to zakazniks that they can come to urednik
            sem_wait(zakaznik_coming2);         //waits for a signal from zakaznik that he is coming to urednik
            //service of zakaznik
            sem_wait(output);
            fprintf(file, "%d: U %d: serving a service of type 2\n", ++(*output_counter), id);
            sem_post(output);
            //finish of service
            usleep(randomforsleep(11, 0));      //sleep for random time from 0 to 10
            sem_wait(output);
            fprintf(file, "%d: U %d: service finished\n", ++(*output_counter), id);
            sem_post(output);
        }
        else if(fronta == 3 && ((*queue3) > 0)){
            *queue3 = (*queue3) - 1;          //decrement number of zakaznik in queue3, because he is going to be served
            sem_post(mutex);                  //post mutex for other uredniks
            sem_post(sem_queue3);             //send signal to zakazniks that they can come to urednik
            sem_wait(zakaznik_coming3);       //waits for a signal from zakaznik that he is coming to urednik
            //service of zakaznik
            sem_wait(output);                 
            fprintf(file, "%d: U %d: serving a service of type 3\n", ++(*output_counter), id);
            sem_post(output);
            //finish of service
            usleep(randomforsleep(11, 0));    //sleep for random time from 0 to 10
            sem_wait(output);
            fprintf(file, "%d: U %d: service finished\n", ++(*output_counter), id);
            sem_post(output);

        }
        else if((*queue1) <= 0 && (*queue2) <= 0 && (*queue3) <= 0 && (*posta_open) == 1){
            //if all queues are empty and posta is open, then urednik is going to take break
            sem_post(mutex);                  //post mutex for other uredniks
            sem_wait(output);
            fprintf(file, "%d: U %d: taking break\n", ++(*output_counter), id);
            sem_post(output);

            usleep(randomforsleep(TU, 4));    //sleep for random time from 0 to TU
            sem_wait(output);
            fprintf(file, "%d: U %d: break finished\n", ++(*output_counter), id);
            sem_post(output);
        }
        else if((*posta_open) == 0 && (*queue1) <= 0 && (*queue2) <= 0 && (*queue3) <= 0){
            //if all queues are empty and posta is closed, then urednik is going home
            sem_post(mutex);                  //post mutex for other uredniks
            sem_wait(output);
            fprintf(file, "%d: U %d: going home\n", ++(*output_counter), id);
            sem_post(output);
            CLRK = false;                     //cycle of urednik is finished
        }
        else{
            sem_post(mutex);                  //post mutex for other uredniks in any other case
        }
    }
    if(file) fclose(file);
    exit(0);
}

void zakaznik(int id){ //PROCESS ZAKAZNIK
    //beginning of process zakaznik
    sem_wait(output);
    fprintf(file, "%d: Z %d: started\n", ++(*output_counter), id);
    sem_post(output);
    usleep(randomforsleep(TZ, 3));            //sleep for random time from 0 to TZ. Then enter posta
    srand(time(NULL) * getpid());
    int fronta = (rand() % 3) + 1;            //random number from 1 to 3
    if(fronta == 1  && (*posta_open) == 1){   //goes to queue1 if posta is open
        sem_wait(output);
        *queue1 = (*queue1) + 1;              //increment number of zakaznik in queue1
        fprintf(file, "%d: Z %d: entering office for a service 1\n", ++(*output_counter), id);
        sem_post(output);
        sem_wait(sem_queue1);                 //waits for a signal from urednik that he called him

        sem_wait(output);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*output_counter), id);
        sem_post(output);
        sem_post(zakaznik_coming1);           //sends signal to urednik that he is coming to him
        usleep(randomforsleep(11, 0));        //sleep for random time from 0 to 10

        sem_wait(output);
        fprintf(file, "%d: Z %d: going home\n", ++(*output_counter), id);
        sem_post(output);
    }
    else if(fronta == 2 && (*posta_open) == 1){
        sem_wait(output);
        *queue2 = (*queue2) + 1;              //increment number of zakaznik in queue2
        fprintf(file, "%d: Z %d: entering office for a service 2\n", ++(*output_counter), id);
        sem_post(output);
        sem_wait(sem_queue2);                 //waits for a signal from urednik that he called him

        sem_wait(output);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*output_counter), id);
        sem_post(output);
        sem_post(zakaznik_coming2);           //sends signal to urednik that he is coming to him
        usleep(randomforsleep(11, 0));        //sleep for random time from 0 to 10

        sem_wait(output);
        fprintf(file, "%d: Z %d: going home\n", ++(*output_counter), id);
        sem_post(output);
    }
    else if(fronta == 3 && (*posta_open) == 1){
        sem_wait(output);
        *queue3 = (*queue3) + 1;              //increment number of zakaznik in queue3
        fprintf(file, "%d: Z %d: entering office for a service 3\n", ++(*output_counter), id);
        sem_post(output);
        sem_wait(sem_queue3);                 //waits for a signal from urednik that he called him

        sem_wait(output);
        fprintf(file, "%d: Z %d: called by office worker\n", ++(*output_counter), id);
        sem_post(output);
        sem_post(zakaznik_coming3);           //sends signal to urednik that he is coming to him
        usleep(randomforsleep(11, 0));        //sleep for random time from 0 to 10

        sem_wait(output); 
        fprintf(file, "%d: Z %d: going home\n", ++(*output_counter), id);
        sem_post(output);
    }
    else if(*posta_open == 0){               //if posta is closed, then zakaznik is going home
        sem_wait(output);
        fprintf(file, "%d: Z %d: going home\n", ++(*output_counter), id);
        sem_post(output);
    }
    if (file) fclose(file);
    exit(0);
}

int main(int argc, char *argv[]) //MAIN PROCESS
{
    if (argc != 6) //check number of arguments
    {
        fprintf(stderr, "Wrong number of arguments\n");
        exit(1);
    }
    NZ = atoi(argv[1]);
    NU = atoi(argv[2]);
    TZ = atoi(argv[3]);
    TU = atoi(argv[4]);
    F  = atoi(argv[5]);
    //check if arguments are in correct range
    if (NZ < 1 || NU < 1 || TZ < 0 || TZ > 10000 || TU < 0 || TU > 100 || F < 0 || F > 10000)
    {
        fprintf(stderr, "Wrong arguments\n");
        exit(1);
    }
    file = fopen("proj2.out", "w");
    if (file == NULL) {
        fprintf(stderr, "Error with proj2.out file\n");
        exit(1);
    }
    init_sem();                      //initialize semaphores
    *posta_open = 1;                 //posta is open
    pid_t clerk_processes[NU + 1];   //array of clerk processes (used for killing them)
    for(int i = 1; i <= NU; i++){    //create NU urednik processes
        pid_t pid_ur = fork();
        if(pid_ur == 0){             //child processes
            urednik(i);
        }
        else if(pid_ur < 0){         //if fork failed
            fprintf(stderr, "Could not create Urednik process %d\n", i);
            destroy_sem();
            if(file != NULL) fclose(file);
            for(int k = 0; k < i; k++) { //kill all created processes
                kill(clerk_processes[k], SIGKILL);
            }
            exit(1);
        }
    }
    pid_t customer_processes[NZ + 1]; //array of zakaznik processes (used for killing them)
    for(int i = 1; i <= NZ; i++){     //create NZ zakaznik processes
        pid_t pid_zk = fork();
        if(pid_zk == 0){              //child processes
            zakaznik(i);
        }
        else if(pid_zk < 0){          //if fork failed
            fprintf(stderr, "Could not create Zakaznik process %d\n", i);
            destroy_sem();
            if(file != NULL) fclose(file);
            for(int k = 0; k < i; k++) { //kill all created processes
                kill(customer_processes[k], SIGKILL);
            }
            exit(1);
        }
    }
    usleep(randomforsleep(F, 5));     //sleep for random time from F/2 to F
    sem_wait(output);                 
    *posta_open = 0;                  //close posta
    fprintf(file, "%d: closing\n", ++(*output_counter));
    sem_post(output);
    while(wait(NULL) > 0);            //wait for all processes to finish
    destroy_sem();                    //destroy semaphores
    if(file != NULL) fclose(file);    //close file
    exit(0);

}

void init_sem(){ //initialize semaphores
    mutex = mmap (NULL, sizeof(mutex), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(mutex, 1, 1);
    output = mmap (NULL, sizeof(output), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(output, 1, 1);
    output_counter = mmap (NULL, sizeof(output_counter), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    posta_open = mmap (NULL, sizeof(posta_open), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    queue1 = mmap (NULL, sizeof(queue1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue2 = mmap (NULL, sizeof(queue2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    queue3 = mmap (NULL, sizeof(queue3), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_queue1 = mmap (NULL, sizeof(sem_queue1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(sem_queue1, 1, 0);
    sem_queue2 = mmap (NULL, sizeof(sem_queue2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(sem_queue2, 1, 0);
    sem_queue3 = mmap (NULL, sizeof(sem_queue3), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(sem_queue3, 1, 0);
    zakaznik_coming1 = mmap (NULL, sizeof(zakaznik_coming1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(zakaznik_coming1, 1, 0);
    zakaznik_coming2 = mmap (NULL, sizeof(zakaznik_coming2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(zakaznik_coming2, 1, 0);
    zakaznik_coming3 = mmap (NULL, sizeof(zakaznik_coming3), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(zakaznik_coming3, 1, 0);
    if(mutex == NULL || output == NULL || output_counter == NULL || posta_open == NULL || queue1 == NULL || queue2 == NULL || queue3 == NULL || sem_queue1 == NULL || sem_queue2 == NULL || sem_queue3 == NULL || zakaznik_coming1 == NULL || zakaznik_coming2 == NULL || zakaznik_coming3 == NULL){
        fprintf(stderr, "Error with semaphores\n");
        destroy_sem();
        exit(1);
    }
    setbuf(file, NULL);
}

void destroy_sem(){ //destroy semaphores
    sem_destroy(mutex);
    sem_destroy(output);
    sem_destroy(sem_queue1);
    sem_destroy(sem_queue2);
    sem_destroy(sem_queue3);
    sem_destroy(zakaznik_coming1);
    sem_destroy(zakaznik_coming2);
    sem_destroy(zakaznik_coming3);
    munmap(mutex, sizeof(mutex));
    munmap(output, sizeof(output));
    munmap(output_counter, sizeof(output_counter));
    munmap(queue1, sizeof(queue1));
    munmap(queue2, sizeof(queue2));
    munmap(queue3, sizeof(queue3));
    munmap(sem_queue1, sizeof(sem_queue1));
    munmap(sem_queue2, sizeof(sem_queue2));
    munmap(sem_queue3, sizeof(sem_queue3));
    munmap(posta_open, sizeof(posta_open));
    munmap(zakaznik_coming1, sizeof(zakaznik_coming1));
    munmap(zakaznik_coming2, sizeof(zakaznik_coming2));
    munmap(zakaznik_coming3, sizeof(zakaznik_coming3));
}