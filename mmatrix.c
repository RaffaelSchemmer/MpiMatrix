/* --------------------------------------------------------------------------------------

Obj do Fonte : Algoritimo paralelo de multiplicação matricial em C (Utilizando MPI)
Disciplina   : CMP 256 2013/II UFRGS PPGC (Prf. Nicolas Bruno Maillard) 
Data         : 03/12/2013
Autor        : Raffael Bottoli Schemmer

--------------------------------------------------------------------------------------- */

// Bibliotecas utilizadas
#include "mpi.h" // Mpi para comunicacao em rede entre os nós
#include <stdio.h> // Biblioteca padrão para entrada e saida.

#define SIZE 1024			/* Size of matrices */
// Constante que define o tamanho da matriz.
// Esse valor pode ser parametrizável seguindo as restrições:
// #1 - Apenas valores base 2 são aceitos Ex:(2/4/8/16/32/64) não existindo restrição de tamanho.
// #2 - As matrizes serão sempre quadráticas, no sentido que SIZE define tanto o número de linhas como de colunas.

unsigned long int A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];
// Declaração das matrizes (Parte estática (SIZE))

// Inicializa a matriz A com valores crescente (A partir de zero)
void fill_matrixA(unsigned long int m[SIZE][SIZE])
{
  static unsigned long int n=0;
  int i, j;
  for (i=0; i<SIZE; i++)
    for (j=0; j<SIZE; j++)
      m[i][j] = n++;
}

// Inicia a matriz B com valores decrescentes (A partir de SIZE*SIZE-1)
void fill_matrixB(unsigned long int m[SIZE][SIZE])
{
  static unsigned long int n=(SIZE*SIZE)-1;
  int i, j;
  for (i=0; i<SIZE; i++)
    for (j=0; j<SIZE; j++)
      m[i][j] = n--;
}

// Mostra a matriz C calculada na tela (Usada para depuração)
void print_matrix(unsigned long int m[SIZE][SIZE])
{
  int i, j = 0;
  for (i=0; i<SIZE; i++) {
    printf("\n\t| ");
    for (j=0; j<SIZE; j++)
      printf("%2lu ", m[i][j]);
    printf("|");
  }
}

int main(int argc, char *argv[])
{
  int myrank, P, from, to, i, j, k;
  MPI_Status status;
  
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);	// Ranking do nó
  MPI_Comm_size(MPI_COMM_WORLD, &P); // Número total de nós do cluster

  // Somente calcula matrizes divisíveis pelo número de processadores
  if (SIZE%P!=0) {
    if (myrank==0) printf("Matrix size not divisible by number of processors\n");
    MPI_Finalize();
    exit(-1);
  }

  // Variáveis utilizadas para separar a matriz A (SIZE) em linhas comparado ao número de threads (P).
  from = myrank * SIZE/P;
  to = (myrank+1) * SIZE/P;

  // Inicializa as matrizes A e B
  if (myrank==0) {
    fill_matrixA(A);
    fill_matrixB(B);
  }
 
  // Realiza o broadcast transmitindo a matriz B para todos os nós (Nó 0 root somente irá realizar o broadcast).
  MPI_Bcast (B, 2*(SIZE*SIZE), MPI_UNSIGNED, 0, MPI_COMM_WORLD);
  
  // Realiza a transmissão das partes (linhas) da matriz A entre todos os nós (Nó 0 root somente irá realizar o scatter).
  MPI_Scatter (A, 2*(SIZE*SIZE/P), MPI_UNSIGNED, A[from], 2*(SIZE*SIZE/P), MPI_UNSIGNED, 0, MPI_COMM_WORLD);
  
  // Realiza a computação das linhas que cada thread foi designada para processar (Todos os nós executam computação sobre a matriz A)
  for (i=from; i<to; i++) 
    for (j=0; j<SIZE; j++) {
      C[i][j]=0;
      for (k=0; k<SIZE; k++)
	C[i][j] += A[i][k]*B[k][j];
    }
  
  // Realiza a recepção das partes (linhas) da matriz C de cada um dos nós (Nó 0 root somente irá realizar o gather).
  MPI_Gather (C[from], 2*(SIZE*SIZE/P), MPI_UNSIGNED, C, 2*(SIZE*SIZE/P), MPI_UNSIGNED, 0, MPI_COMM_WORLD);
  
  // DUMP - Escreve a matriz C na tela (Descomente as linhas de código abaixo)
  /*
	if(myrank == 0)
		print_matrix(C);
  */		
  MPI_Finalize();
  return 0;
}


