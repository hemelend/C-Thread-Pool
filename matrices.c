#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "thpool.h"

#define RED "\033[0;31m"
#define NC "\033[0m"
#define NUMBER_THREADS 8
#define NUMBER_MATRICES 40
#define MAX_MATRIX_SIZE 5
#define MIN_MATRIX_SIZE 2

typedef struct matrix_sst {
    int rows;
    int columns;
    int *data;
} matrix_sst;


typedef struct matrix_cst {
    matrix_sst matrix_1;
    matrix_sst matrix_2;
    matrix_sst matrix_3;
} matrix_cst;


pthread_mutex_t mutexMatrix;

threadpool thpool_init(int theards);
matrix_sst initMatrix(int rows, int cols);
matrix_cst initComplexMatrix(matrix_cst matrix);
void resetMatrix(int rows, int cols, matrix_sst *matrix);
void setComplexMatrix(matrix_cst *ptrComplexMatrix);
void closeComplexMatrix(matrix_cst *matrix);
void closeMatrix(matrix_sst *matrix);
void printMatrix(matrix_cst *matrix);
void matrix_fprint(matrix_sst *matrix, FILE *stream);
void print(matrix_sst *matrix);
int getDataFromMatrix(matrix_sst *matrix, int i, int j);


matrix_sst initMatrix(int rows, int cols) {
    if (rows < 0 && cols < 0) {
        fprintf(stderr, RED "[ERROR]" NC ": Matrix struct without size.\n");

        exit(EXIT_FAILURE);
    }

    matrix_sst matrix;

    matrix.columns    = cols;
    matrix.rows       = rows;
    matrix.data       = malloc(cols * rows * sizeof(int));

    if (matrix.data == NULL) {
        fprintf(stderr, RED "[ERROR]" NC ": Unable to allocate the matrix data memory.\n");
        exit(EXIT_FAILURE);
    }

    return matrix;
}

void setMatrix(int rows, int cols, matrix_sst *matrix)
{
    if (matrix->data == NULL) {
        fprintf(stderr, RED "[ERROR]" NC ": Unable to set the matrix data memory.\n");
        exit(EXIT_FAILURE);
    }
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            *(matrix->data + row * matrix->rows + col) = rand() % 9 + 1;
        }
    }
}

void resetMatrix(int rows, int cols, matrix_sst *matrix)
{
    if (matrix->data == NULL) {
        fprintf(stderr, RED "[ERROR]" NC ": Unable to set the matrix data memory.\n");
        exit(EXIT_FAILURE);
    }
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            *(matrix->data + row * matrix->rows + col) = 0;
        }
    }
}

void closeMatrix(matrix_sst *matrix) 
{
    free(matrix->data);
}

void matrix_fprint(matrix_sst *matrix, FILE *stream)
{
    fprintf(stream, "<matrix>\n");
    for (size_t i = 0; i < matrix->rows; i++)
    {
        for (size_t j = 0; j < matrix->columns; j++)
        {
            fprintf(stream, "%d ", getDataFromMatrix(matrix, i, j));
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "</matrix>\n");
}

void print(matrix_sst *matrix)
{
    matrix_fprint(matrix, stdout);
}

int getDataFromMatrix(matrix_sst *matrix, int i, int j)
{
    return *(matrix->data + i * matrix->rows + j);
}

void printMatrix(matrix_cst *matrix)
{
    print(&matrix->matrix_1);
    print(&matrix->matrix_2);
    print(&matrix->matrix_3);
    
}

void closeComplexMatrix(matrix_cst *matrix)
{
    closeMatrix(&matrix->matrix_1);
    closeMatrix(&matrix->matrix_2);
    closeMatrix(&matrix->matrix_3);
    // free(matrix);
}

matrix_cst initComplexMatrix(matrix_cst matrix)
{
    int matrix_size = rand() % (MAX_MATRIX_SIZE - MIN_MATRIX_SIZE + 1) + MIN_MATRIX_SIZE;
    matrix_sst ptrMatrix1;
    matrix_sst ptrMatrix2;
    matrix_sst ptrMatrix3;
    ptrMatrix1 = initMatrix(matrix_size, matrix_size);
    resetMatrix(matrix_size, matrix_size, &ptrMatrix1);
    ptrMatrix2 = initMatrix(matrix_size, matrix_size);
    resetMatrix(matrix_size, matrix_size, &ptrMatrix2);
    ptrMatrix3 = initMatrix(matrix_size, matrix_size);
    resetMatrix(matrix_size, matrix_size, &ptrMatrix3);

    matrix.matrix_1 = ptrMatrix1;
    matrix.matrix_2 = ptrMatrix2;
    matrix.matrix_3 = ptrMatrix3;

    return matrix;
}

void setComplexMatrix(matrix_cst *ptrComplexMatrix)
{
    setMatrix(ptrComplexMatrix->matrix_1.columns, ptrComplexMatrix->matrix_1.rows, &ptrComplexMatrix->matrix_1);
    setMatrix(ptrComplexMatrix->matrix_2.columns, ptrComplexMatrix->matrix_2.rows, &ptrComplexMatrix->matrix_2);
    // setMatrix(ptrComplexMatrix->matrix_3.columns, ptrComplexMatrix->matrix_3.rows, &ptrComplexMatrix->matrix_3);
}

void process_tasks(void* arg)
{
    matrix_cst ptrComplexMatrix = *(matrix_cst*) arg;

    ptrComplexMatrix = initComplexMatrix(ptrComplexMatrix);

    setComplexMatrix(&ptrComplexMatrix);

    pthread_mutex_lock(&mutexMatrix);
    printf("Thread #%u working on matrix\n", (int)pthread_self());
    printMatrix(&ptrComplexMatrix);
    pthread_mutex_unlock(&mutexMatrix);

    closeComplexMatrix(&ptrComplexMatrix);
}

int main(int argc, char* argv[]) 
{
    srand(time(NULL));
    // initialize fuelMutex
    pthread_mutex_init(&mutexMatrix, NULL);
        
    // step 1 -> create the complex matrix    -> DONE
    // step 2 -> print the each simple matrix -> DONE
    // step 3 -> average simple matrix
    // step 4 -> add two simple matrix
    // step 5 -> transpose simple matrix
    // step 6 -> create identity matrix
    // step 7 -> swap rows and swap cols

    // create the thpool
    puts("Making threadpool with 8 threads");
    threadpool thpool = thpool_init(NUMBER_THREADS);

    puts("Adding 40 tasks to threadpool");
	int i;
	for (i=0; i<40; i++){
		matrix_cst* matrix = malloc(sizeof(matrix_cst));
		thpool_add_work(thpool, process_tasks, matrix);
	};

    // wait the thpool
    thpool_wait(thpool);
    // destroy the thpool
	puts("Killing threadpool");
	thpool_destroy(thpool);

    pthread_mutex_destroy(&mutexMatrix);
    return EXIT_SUCCESS;
}
