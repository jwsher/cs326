/* findminmax_seq.c - find the min and max values in a random array
 *
 * usage: ./findminmax <seed> <arraysize>
 *
 */
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include"measure.h"

/* a struct used to pass results to caller */
struct results {
    int min;
    int max;
};

/**
 *
The find_min_and_max() function is passed a pointer to an array of integers, subarray, and the size of the array, n.
 The array is scanned and working min and max values are updated when appropriate.  When the function is finished,
  the final min and max values are passed back to the caller using a results struct.
 */
struct results find_min_and_max(int *subarray, int n)
{
    int i, min, max;
    min = max = subarray[0];
    struct results r;
    
    for (i = 1; i < n; i++) {
        if (subarray[i] < min) {
            min = subarray[i];
        }
        if (subarray[i] > max) {
            max = subarray[i];
        }
    }
    
    r.min = min;
    r.max = max;
    return r;
}
/**
 * The main() function accepts two command-line arguments: a seed value and the size of the random array to generate.
 *  The seed value is used by the random number generator.  If you want to test with the same randomly generated array,
 *  you should use the same seed value from run to run.  To test different arrays, use different seed values.
 *   This will be useful when testing your parallel versions for correctness.
 *   For example you can invoke findminmax_seq as follows:
 *
 *   $ findminmax_seq 1 10000 1

This will use a seed value of 1 and generate an array of size 10000 with 1 process.
 *
 */
int main(int argc, char **argv)
{
    int *array;
    int arraysize = 0;
    int seed;
    int nprocs=1;

    /**
     * How much each process sorts and how much extra is left over that we should give to the last process
     */
    int *pipes;
    pid_t *sub_proc;
    int proc_share=0;
    int proc_leftover=0;
   	int offset=0;
   	int launched=0;
   	int * proc_results;

    char randomstate[8];
    struct results r;
    int i;

    /* process command line arguments */
    if (argc != 4) {
        printf("usage: ./findminmax <seed> <arraysize> <nprocs>\n");
        return 1;
    }

    seed = atoi(argv[1]);
    arraysize = atoi(argv[2]);
    nprocs = atoi(argv[3]);

    proc_share=arraysize/nprocs;
    proc_leftover=arraysize%nprocs;
    proc_results = (int *) malloc(sizeof(int) * nprocs*2);

    /* allocate array and populate with random values */
    array = (int *) malloc(sizeof(int) * arraysize);
    
    initstate(seed, randomstate, 8);

    for (i = 0; i < arraysize; i++) {
        array[i] = random();
    }
    
    /* begin computation */

    pipes=(int *) malloc(sizeof(int)*nprocs*2);
    sub_proc=(pid_t *) malloc(sizeof(pid_t)*nprocs);

   	mtf_measure_begin();
   	for(i=0 ; i < nprocs;i++) {
   		pipe(&pipes[i*2]);
   		int size=proc_share+(proc_leftover > i ? 1 : 0);
   		pid_t tmp_pid=fork();
   		if (!tmp_pid)  {
   		    int write_pipe=pipes[i*2+1];

   		    FILE *pipe=fdopen(write_pipe,"wb");
   			r = find_min_and_max(&array[offset], size);
   		    printf("min = %d, max = %d size->%d\n", r.min, r.max,size);
   			fwrite(&r,sizeof(struct results),1,pipe);
   			fclose(pipe);
   			return 0;
   		} else {
   			launched++;
   			sub_proc[i]=tmp_pid;
   		}
   	    offset+=size;
    }
   	while(launched>0) {
   		for(i=0 ; i < nprocs;i++) {
   			struct results tmp_result;
   			int status=0;
   			int pid=0;
   			if (sub_proc[i]==0) { continue; }
   			pid=waitpid(sub_proc[i],&status,0);
   			if (pid) {
   	   		    int read_pipe=pipes[i*2];
   	   		    FILE *pipe=fdopen(read_pipe,"rb");
   				launched--;
   				sub_proc[i]=0;
   	   			fread(&tmp_result,sizeof(struct results),1,pipe);
   	   		    printf("parent -> min = %d, max = %d\n", tmp_result.min, tmp_result.max);
   	   			fclose(pipe);
   	   			proc_results[i*2]=tmp_result.max;
   	   			proc_results[i*2+1]=tmp_result.min;
   			}
   		}
   	}
	r = find_min_and_max(proc_results, nprocs*2);


    mtf_measure_end();
    
    printf("Execution time: ");
    mtf_measure_print_seconds(1);

    printf("min = %d, max = %d\n", r.min, r.max);

	r = find_min_and_max(array, arraysize);

	printf("check -> min = %d, max = %d\n", r.min, r.max);

    return 0;
}
