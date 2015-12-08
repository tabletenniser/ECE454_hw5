/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include <string.h>
#include "life.h"
#include "util.h"

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

/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{

    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
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
    //SWAP_BOARDS( outboard, inboard );

    for (curgen = 0; curgen < gens_max; curgen++)
    {
    	// Copy inboard to outboard to ensure that the outboards are same.
    	//memmove (outboard, inboard, nrows * ncols * sizeof (char));
    	// Copy outboard to inboard.
    	memmove (inboard, outboard, nrows * ncols * sizeof (char));
    	for (j = 0; j < ncols; j++){
            const int jwest = mod (j-1, ncols);
            const int jeast = mod (j+1, ncols);
            const int jwLDA = jwest*LDA;
            const int jLDA = j*LDA;
            const int jeLDA = jeast*LDA;            
        	for (i = 0; i < nrows; i++){
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
                /*
                const char neighbor_count = 
                	inboard[jwLDA+inorth] + inboard[jLDA+inorth] + inboard[jeLDA+inorth] +
                	inboard[jwLDA+i] + inboard[jeLDA+i] +
                	inboard[jwLDA+isouth] + inboard[jLDA+isouth] + inboard[jeLDA+isouth];
                	
                outboard[jLDA+i] = alivep (neighbor_count, inboard[jLDA+i]);
                */
            }
        }
        //SWAP_BOARDS( outboard, inboard );
    }
    //SWAP_BOARDS( outboard, inboard );
    // Format output so that it only shows dead or alive
    for (j = 0; j < ncols; j++){
    	const int jLDA = j*LDA;
    	for (i = 0; i < nrows; i++){
        	outboard[jLDA+i] = outboard[jLDA+i] >> 4;
      	}
    }
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
    return outboard;
}
