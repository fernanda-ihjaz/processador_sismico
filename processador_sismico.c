#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

// CPG
void centro_processamento_geofisico(int* pipe_fd, int formato_saida, int num_leitura);

// ESR
void estacao_sismologica_remota(int* pipe_fd, int num_leitura);

// gera um dado sismológico (string binária de 16 bits)
char* dado_sismologico();

// recebe uma string binária de 16 bits e retorna um inteiro
unsigned int string_para_inteiro(const char* string_binaria);

// função que é executada quando é pressionado Ctrl + C 
void handler();

// função que é executada quando o TEMPO_LIMITE chega em 0
void handler_alarm();

pid_t pid;

int main(int argc, char *argv[]) 
{
    srand(time(NULL));
    
    if (argc != 4) 
    {
        printf("Uso: %s ./processador_sismico <FORMATO_SAIDA> <NUM_LEITURAS> <TEMPO_LIMITE>\n", argv[0]);
        return 1;
    }
    
    int FORMATO_SAIDA = atoi(argv[1]);
    int NUM_LEITURAS = atoi(argv[2]);
    int TEMPO_LIMITE = atoi(argv[3]);
    
    if (FORMATO_SAIDA != 2 && FORMATO_SAIDA != 8 && FORMATO_SAIDA != 10 && FORMATO_SAIDA != 16)
    {
        printf("Argumento inválido para FORMATO_SAIDA.\nArgumentos válidos:\n2 -> binário\n8 -> octal \n10 -> decimal \n16 -> hexadecimal\n");
        _exit(1);
    }

    if (NUM_LEITURAS < 0 || TEMPO_LIMITE < 0)
    {
        printf("Argumento inválido para NUM_LEITURAS ou TEMPO_LIMITE, o argumento deve ser um inteiro positivo\n");
        _exit(1);
    }

    signal(SIGINT, handler);
    alarm(TEMPO_LIMITE);

    for (int num_leitura = 1; num_leitura <= NUM_LEITURAS; num_leitura++)
    {
        // criação do link
        int pipe_fd[2];
        pipe(pipe_fd); 

        // criação do fork
        pid = fork();

        signal(SIGALRM, handler_alarm);

        if (pid < 0)
        {
            printf("Processo inválido");
            _exit(1);
        }

        if (pid == 0) // filho
        {
            // CPG
            centro_processamento_geofisico(pipe_fd, FORMATO_SAIDA, num_leitura);

            _exit(0); // finaliza o processo filho, para o pai dele saber que ele finalizou e parar de esperar
        }

        if (pid > 0) // pai
        {
            // ESR
            estacao_sismologica_remota(pipe_fd, num_leitura);

            // espera o filho
            wait(NULL);
        }
    }

    return 0;
}

void centro_processamento_geofisico(int* pipe_fd, int formato_saida, int num_leitura)
{
    close(pipe_fd[1]); // fecha canal de escrita
    char dado[17];

    if (read(pipe_fd[0], dado, 17) > 0) 
    {
        if (formato_saida == 2)
        {
            printf("\033[0;34m[CPG PID:%d] Recebido: %s -> Formato 2: %s\033[0m\n", getpid(), dado, dado);
        }

        else if (formato_saida == 8)
        {
            int octal = string_para_inteiro(dado);
            printf("\033[0;34m[CPG PID:%d] Recebido: %s -> Formato 8: %o\033[0m\n", getpid(), dado, octal);
        }

        else if (formato_saida == 10)
        {
            int decimal = string_para_inteiro(dado);
            printf("\033[0;34m[CPG PID:%d] Recebido: %s -> Formato 10: %u\033[0m\n", getpid(), dado, decimal);
        }

        else if (formato_saida == 16)
        {
            int hexa = string_para_inteiro(dado);
            printf("\033[0;34m[CPG PID:%d] Recebido: %s -> Formato 16: %X\033[0m\n", getpid(), dado, hexa);
        }
        
        dado[16] = '\0';

        fflush(stdout);
    }

    close(pipe_fd[0]); // fecha a leitura
}

void estacao_sismologica_remota(int* pipe_fd, int num_leitura)
{
    close(pipe_fd[0]); //fecha leitura

    char* dado = dado_sismologico();
    write(pipe_fd[1], dado, 17);
    
    printf("\033[0;32m[ESR PID:%d] Leitura #%d %s transmitida.\033[0m\n", getpid(), num_leitura, dado);
    free(dado);
    
    close(pipe_fd[1]); //fecha escrita
}

char* dado_sismologico()
{
    char* binario = malloc(17 * sizeof(char));
    if (binario == NULL) return NULL;

    for (int j = 0; j < 16; j++) 
    {
        binario[j] = (rand() % 2) ? '1' : '0';
    }
    binario[16] = '\0';

    return binario;
}

unsigned int string_para_inteiro(const char* string_binaria) 
{
    return (unsigned int) strtol(string_binaria, NULL, 2);
}

void handler() 
{
    printf("Análise interrompida pelo pesquisador!\n");
    if (pid > 0) {
        kill(pid, SIGKILL);
    }
    kill(getpid(), SIGKILL);
}

void handler_alarm() 
{
    printf("\n\033[0;31m[ALERTA] Processamento excedeu o tempo limite!\033[0m\n");
    if (pid > 0) {
        kill(pid, SIGKILL);
    }
    kill(getpid(), SIGKILL);
}



