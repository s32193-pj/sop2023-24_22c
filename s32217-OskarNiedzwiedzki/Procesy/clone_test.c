#define _GNU_SOURCE /*clone*/
#include <sched.h>  /*clone*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <sys/types.h>  /*pid_t*/
#include <sys/mman.h>   /*mmap()*/
#define _ERROR_MALLOC 1
#define _ERROR_PROCESS 2

#define PROCESS_STACK_SIZE (unsigned int)(1<<16) /*1kB*/
typedef unsigned char BYTE;

struct sum_args{
	int lower, upper;
	int* out; /*this must be a pointer to shared memory*/
};

/*returns sum of the numbers from lower to upper bound*/
int sum(void*);

int main(){
	pid_t sum1_pid, sum2_pid;
	BYTE* sum_1_stack, *sum_2_stack;
	struct sum_args sum_1_args={0,99}, sum_2_args={100,200};
	int* sum_out_vals; /*[0] -> sum1 [1] -> sum2*/

	int status=0;

	printf("kurwaaaaaaaa");	
	/*create shared memory between processes for returning values, MAP_ANONYMOUS makes mapping not backed by any file, last 2 arguments are ignored*/
	/*PROT_NONE allows for any operation (read,write,exec)*/
	sum_out_vals = mmap(NULL,sizeof(int)*2,PROT_NONE,MAP_SHARED | MAP_ANONYMOUS,-1,0);

	/*make shared memory point to outputs*/
	sum_1_args.out = &sum_out_vals[0];
	sum_2_args.out = &sum_out_vals[1];

	sum_1_stack = malloc(PROCESS_STACK_SIZE);
	sum_2_stack = malloc(PROCESS_STACK_SIZE);

	if(sum_1_stack==NULL || sum_2_stack==NULL){
		printf("Memory could not be allocated using malloc()");
		return _ERROR_MALLOC;
	}

	printf("Before clone");

	sum1_pid=clone(sum,sum_1_stack+PROCESS_STACK_SIZE,SIGCHLD,&sum_1_args);
	sum2_pid=clone(sum,sum_2_stack+PROCESS_STACK_SIZE,SIGCHLD,&sum_2_args);

	if(sum1_pid < 0 || sum2_pid < 0){
		printf("Process could not be created");
		return _ERROR_PROCESS;
	}

	if(waitpid(sum1_pid,&status,0)==-1){
		printf("sum_1 waitpid error");
		return _ERROR_PROCESS;
	}

	printf("hulabula");
	printf("output after sum_1: %d\n", *(sum_1_args.out));

	printf("kurcze bele\n");

	if(waitpid(sum2_pid,&status,0)==-1){
		printf("sum_2 waitpid error");
		return _ERROR_PROCESS;
	}

	printf("output after sum_2: %d\n", *(sum_2_args.out));

	/*free memory*/
	free(sum_1_stack);
	free(sum_2_stack);
	munmap(sum_out_vals, sizeof(int)*2);
	
	return EXIT_SUCCESS;
}

int sum(void* args){
	int lower=0;
	int upper=0;
	int x =0;
	int sum=0;
	
	struct sum_args* s_args = (struct sum_args*)args;

	lower = s_args->lower;
	upper = s_args->upper;

	if(lower > upper)
		return 0;

	for(x=lower;x<=upper;x++) 
		sum+=x;

	*(s_args->out) = sum; /*save to the shared memory*/
	return 0;
}

