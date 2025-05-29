# Processador Sismico em C

Este projeto implementa um simulador básico de um sistema de processamento de dados sismológicos usando processos e pipes no Linux.


## Descrição

O programa simula dois processos:

- **Estação Sismológica Remota (ESR):** gera dados sismológicos binários (16 bits) e envia para o processamento.
- **Centro de Processamento Geofísico (CPG):** recebe os dados da ESR, converte e exibe em diferentes formatos numéricos (binário, octal, decimal, hexadecimal).

O fluxo utiliza comunicação via pipe entre processos pai e filho (fork).


## Como usar

### Compilação

gcc -o processador_sismico processador_sismico.c

### Execução

./processador_sismico <FORMATO_SAIDA> <NUM_LEITURAS> <TEMPO_LIMITE>

- `FORMATO_SAIDA`: formato para exibir o dado sismológico recebido. Valores válidos:
  - `2`   - binário
  - `8`   - octal
  - `10`  - decimal
  - `16`  - hexadecimal
- `NUM_LEITURAS`: número de leituras sismológicas a serem feitas (inteiro positivo).
- `TEMPO_LIMITE`: tempo máximo (em segundos) para execução do programa.

### Exemplo

./processador_sismico 10 5 30

Executa 5 leituras, exibindo os dados em decimal, com limite de 30 segundos.

---

## Funcionamento interno

- O programa gera uma sequência de leituras (de 1 até `NUM_LEITURAS`).
- Para cada leitura:
  - Cria um pipe para comunicação.
  - O processo pai gera um processo filho via `fork()`.
  - O processo filho (CPG) lê o dado do pipe e imprime no formato solicitado.
  - O processo pai (ESR) gera e envia um dado binário sismológico para o filho via pipe.
  - O pai espera o filho terminar antes de iniciar a próxima leitura.
- Caso o tempo limite (`TEMPO_LIMITE`) seja atingido ou o usuário pressione Ctrl+C, o programa é encerrado com mensagens apropriadas.

---

## Sinais tratados

- `SIGINT` (Ctrl+C): termina o programa imediatamente, matando o processo filho.
- `SIGALRM`: termina o programa se o tempo limite for ultrapassado.


---

Se quiser contribuir, abra uma issue ou pull request!

