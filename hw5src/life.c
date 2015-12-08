/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "life.h"
#include "util.h"


#define NUM_OF_THREADS 4
//pthread_mutex_t mutex;
typedef struct parameter {
    char * inboard;
    char * outboard;
    int nrows;
    int ncols;
    int start_row;
    int end_row;
}parameter;

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
    char* temp = b1; \
    b1 = b2; \
    b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

void game_of_life_boundary (int row_index, char* outboard,
        char* inboard,
        const int nrows,
        const int ncols) {
    const int LDA = nrows;
    int i,j;

    for (j = 0; j < ncols; j++){
        const int jwest = mod (j-1, ncols);
        const int jeast = mod (j+1, ncols);
        const int jwLDA = jwest*LDA;
        const int jLDA = j*LDA;
        const int jeLDA = jeast*LDA;
        i = row_index;

        // dead becomes alive
        if (inboard[jLDA+i] == 0x3){
            const int inorth = mod (i-1, nrows);
            const int isouth = mod (i+1, nrows);
            outboard[jLDA+i] = outboard[jLDA+i] | (1<<4);
            outboard[jwLDA+inorth]++;
            outboard[jLDA+inorth]++;
            outboard[jeLDA+inorth]++;
            outboard[jwLDA+i]++;
            outboard[jeLDA+i]++;
            outboard[jwLDA+isouth]++;
            outboard[jLDA+isouth]++;
            outboard[jeLDA+isouth]++;
        }
        // live becomes dead
        else if (inboard[jLDA+i] >= 0x10 && (inboard[jLDA+i] <= 0x11 || inboard[jLDA+i] >= 0x14)){
            const int inorth = mod (i-1, nrows);
            const int isouth = mod (i+1, nrows);
            outboard[jLDA+i] = outboard[jLDA+i] & (~(1<<4));
            outboard[jwLDA+inorth]--;
            outboard[jLDA+inorth]--;
            outboard[jeLDA+inorth]--;
            outboard[jwLDA+i]--;
            outboard[jeLDA+i]--;
            outboard[jwLDA+isouth]--;
            outboard[jLDA+isouth]--;
            outboard[jeLDA+isouth]--;
        }
    }
    //SWAP_BOARDS( outboard, inboard );

    return;
}

void *game_of_life_thread (void * args)
{
    struct parameter* p = (struct parameter*) args;
    int i, j;

    char *inboard = p->inboard;
    char *outboard = p->outboard;
    const int nrows = p->nrows;
    const int ncols = p->ncols;
    const int start_row = p->start_row;
    const int end_row = p->end_row;
    //printf("start_row: %d; end_row: %d\n", start_row, end_row);
    //const int end_row = p->end_row;
    const int LDA = nrows;

    for (j = 0; j < ncols; j++){
        const int jwest = mod (j-1, ncols);
        const int jeast = mod (j+1, ncols);
        const int jwLDA = jwest*LDA;
        const int jLDA = j*LDA;
        const int jeLDA = jeast*LDA;
        i = start_row;

        for (i = start_row+1; i < end_row-1; i++){
            // dead becomes alive
            //if ((inboard[jLDA+i] & (1<<4)) == 0 && (inboard[jLDA+i] & 3) == 3){
            if (inboard[jLDA+i] == 0x3){
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                outboard[jLDA+i] = outboard[jLDA+i] | (1<<4);
                outboard[jwLDA+inorth]++;
                outboard[jLDA+inorth]++;
                outboard[jeLDA+inorth]++;
                outboard[jwLDA+i]++;
                outboard[jeLDA+i]++;
                outboard[jwLDA+isouth]++;
                outboard[jLDA+isouth]++;
                outboard[jeLDA+isouth]++;
            }
            // live becomes dead
            //else if ((inboard[jLDA+i] & (1<<4)) != 0 && (inboard[jLDA+i] & 7) > 3 && (inboard[jLDA+i] & 7) < 2){
            else if (inboard[jLDA+i] >= 0x10 && (inboard[jLDA+i] <= 0x11 || inboard[jLDA+i] >= 0x14)){
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                outboard[jLDA+i] = outboard[jLDA+i] & (~(1<<4));
                outboard[jwLDA+inorth]--;
                outboard[jLDA+inorth]--;
                outboard[jeLDA+inorth]--;
                outboard[jwLDA+i]--;
                outboard[jeLDA+i]--;
                outboard[jwLDA+isouth]--;
                outboard[jLDA+isouth]--;
                outboard[jeLDA+isouth]--;
            }
        }
    }
    //SWAP_BOARDS( outboard, inboard );

    /*
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!!
     */
    return NULL;
}

/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char* game_of_life (char* outboard, char* inboard, const int nrows, const int ncols, const int gens_max)
    {
        // Use sequential implementation if gameboard is less than 32*32.
        if (nrows < 32)  {
            return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);
        }

        //mutex = PTHREAD_MUTEX_INITIALIZER;
        //pthread_mutex_init(&mutex, NULL);

        // Preprocess such that the 4th bit of outboard hold the life/die data and the LSBits holds the neighbor_count.
        const int LDA = nrows;
        int curgen, i, j;
        for (j = 0; j < ncols; j++){
            const int jwest = mod (j-1, ncols);
            const int jeast = mod (j+1, ncols);
            const int jwLDA = jwest*LDA;
            const int jLDA = j*LDA;
            const int jeLDA = jeast*LDA;
            for (i = 0; i < nrows; i++){
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const char neighbor_count =
                    inboard[jwLDA+inorth] + inboard[jLDA+inorth] + inboard[jeLDA+inorth] +
                    inboard[jwLDA+i] + inboard[jeLDA+i] +
                    inboard[jwLDA+isouth] + inboard[jLDA+isouth] + inboard[jeLDA+isouth];
                // inside board[], right-most three bits are neighbor_count, 4th bit from right is 1 if it's alive.
                outboard[jLDA+i] = inboard[jLDA+i]<<4 | neighbor_count;
            }
        }

        pthread_t threads[NUM_OF_THREADS];
        struct parameter thread_p[NUM_OF_THREADS];
        int start_row_index;

        for (curgen = 0; curgen < gens_max; curgen++)
        {
            // Copy outboard to inboard.
            memmove (inboard, outboard, nrows * ncols * sizeof (char));
            start_row_index = 0;
            for (i = 0; i < NUM_OF_THREADS; i++) {
                thread_p[i].inboard      =       inboard; // All threads opperate on the same board
                thread_p[i].outboard     =       outboard;
                thread_p[i].ncols        =       ncols;   // All threads use the same overall read bounds
                thread_p[i].nrows        =       nrows;
                thread_p[i].start_row  =       start_row_index; // Defines Start segment
                thread_p[i].end_row =  start_row_index+nrows/NUM_OF_THREADS;
                game_of_life_boundary(start_row_index, outboard, inboard, nrows, ncols);
                game_of_life_boundary(thread_p[i].end_row-1, outboard, inboard, nrows, ncols);
                start_row_index += nrows/NUM_OF_THREADS;
            }

            for(i=0;i<NUM_OF_THREADS;i++) {
                pthread_create(&threads[i], NULL, game_of_life_thread, (void*) &thread_p[i]);
            }

            for (i=0; i<NUM_OF_THREADS; i++){
                pthread_join(threads[i],NULL);
            }
        }

        // Postprocess output so that it only shows dead or alive
        for (j = 0; j < ncols; j++){
            const int jLDA = j*LDA;
            for (i = 0; i < nrows; i++){
                outboard[jLDA+i] = outboard[jLDA+i] >> 4;
            }
        }

        return outboard;
    }
