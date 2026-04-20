// Desenvolvido por: Victor da Silva Goncalves
// Trabalho N1 - Algoritmos 2
// Professor: Alessandro Joao Brassanini
// Instituicao: UniAvan - Sistemas de Informacao
// Data: Abril de 2026

/*
  JOGO DO LABIRINTO 10x10 - VERSAO MELHORADA

  LEGENDA DO MAPA:
    @  = Posicao atual do jogador
    X  = Parede (nao pode atravessar)
    O  = Saida (objetivo do jogo)
    * = Armadilha/Buraco (penalidade severa de pontos)
    .  = Caminho livre

  CONTROLES:
    W = Mover para CIMA
    S = Mover para BAIXO
    A = Mover para ESQUERDA
    D = Mover para DIREITA
    Q = Sair do jogo

  SISTEMA DE PONTUACAO:
    - Pontuacao inicial: 1000 pontos
    - Cada passo valido: -5 pontos
    - Bater na parede:   -50 pontos
    - Cair em armadilha: -150 pontos

  SOBRE AS CORES (ANSI Escape Codes):
    Este codigo usa sequencias de escape ANSI para colorir o terminal.
    Sao suportadas nativamente em Linux, Mac e no terminal moderno do
    Windows (Windows 10+, PowerShell, Git Bash, WSL).
    Formato: printf("\033[CODIGOm texto \033[0m")
      \033[ = inicio da sequencia de escape
      m     = fim do codigo de cor
      \033[0m = reset (volta a cor padrao)

  SOBRE OS SONS:
    O caractere \a (BEL) envia um sinal sonoro ao terminal.
    Em Linux/Mac toca o beep do sistema. No Windows pode piscar
    a barra de tarefas.
*/

/* INCLUDES */
#include <stdio.h>   /* printf, scanf - entrada e saida padrao             */
#include <stdlib.h>  /* system() - limpar tela                             */
#include <ctype.h>   /* toupper() - converter letra para maiuscula         */
#include <locale.h>  /* setlocale() - suporte a caracteres especiais       */


/*
   BLOCO 1: CONSTANTES DO JOGO COMO "const int"
   Usamos "const int" em vez de #define.
   "const int" cria uma variavel global de verdade, com tipo definido e
   espaco na memoria, protegida contra modificacao pelo programa.
*/
const int TAMANHO_LABIRINTO = 10;  /* Dimensao do labirinto (linhas e colunas) */
const int PONTOS_INICIAIS   = 1000;/* Pontuacao com que o jogador comeca       */
const int CUSTO_PASSO       = 5;   /* Desconto por cada passo valido dado      */
const int CUSTO_PAREDE      = 50;  /* Desconto ao bater em uma parede          */
const int CUSTO_BURACO      = 150; /* Desconto ao cair em uma armadilha        */

/* Codigos das celulas da matriz - identificam o tipo de cada casa           */
const int LIVRE  =  0; /* Caminho livre: jogador pode passar livremente     */
const int PAREDE =  1; /* Parede: bloqueia o movimento completamente        */
const int SAIDA  = -1; /* Saida: objetivo do jogo, encerra ao chegar aqui   */
const int BURACO = -2; /* Armadilha: jogador passa, mas perde muitos pontos */

/*
   BLOCO 2: CONSTANTES DE COR - SEQUENCIAS DE ESCAPE ANSI
   Em vez de usar SetConsoleTextAttribute() do Windows, usamos sequencias
   de escape ANSI. Sao strings que o terminal interpreta como comandos.
*/
const char* COR_RESET   = "\033[0m";  /* Volta a cor padrao do terminal    */
const char* COR_JOGADOR = "\033[92m"; /* Verde brilhante - para o @        */
const char* COR_PAREDE  = "\033[90m"; /* Cinza escuro - para o X           */
const char* COR_SAIDA   = "\033[93m"; /* Amarelo brilhante - para o O      */
const char* COR_BURACO  = "\033[91m"; /* Vermelho brilhante - para o * */
const char* COR_TITULO  = "\033[96m"; /* Ciano brilhante - cabecalho       */
const char* COR_HUD     = "\033[95m"; /* Magenta - informacoes do jogador  */
const char* COR_VITORIA = "\033[92m"; /* Verde brilhante - tela de vitoria */
const char* COR_DERROTA = "\033[91m"; /* Vermelho - tela de game over      */
const char* COR_AVISO   = "\033[93m"; /* Amarelo - mensagens de eventos    */

/* Codigos dos tipos de som - identificam qual efeito sonoro reproduzir     */
const int SOM_MOVIMENTO = 1; /* Beep simples ao se mover normalmente        */
const int SOM_PAREDE    = 2; /* Beep longo ao colidir com parede            */
const int SOM_BURACO    = 3; /* Multiplos beeps de alerta ao cair em buraco */
const int SOM_VITORIA   = 4; /* Beeps de comemoracao ao vencer              */

/*
   BLOCO 3: VARIAVEIS GLOBAIS DE ESTADO DO JOGO
   Cada informacao do estado do jogo e uma variavel global independente.
   Variaveis globais sao declaradas fora de qualquer funcao e podem ser
   lidas e modificadas diretamente por qualquer funcao do programa.
*/
int jogLinha   = 0;   /* Linha atual do jogador na matriz (0 a 9)          */
int jogColuna  = 0;   /* Coluna atual do jogador na matriz (0 a 9)         */
int jogPassos  = 0;   /* Contador de movimentos validos realizados         */
int jogPontos  = 0;   /* Pontuacao atual do jogador                        */
int jogJogando = 1;   /* Flag de controle: 1 = em jogo | 0 = encerrar      */
char ultimoAviso[150];/* Mensagem do ultimo evento, exibida no rodape      */

/*
   BLOCO 4: LABIRINTO GLOBAL
   Mapa:
    0 = LIVRE | 1 = PAREDE | -1 = SAIDA | -2 = BURACO
*/
int labirinto[10][10] = {
    /* Col: 0  1  2  3  4  5  6  7  8  9  */
    /*L0*/ { 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
    /*L1*/ { 0, 1, 0, 1, 0, 1, 0, 1, 1, 0 },
    /*L2*/ { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0 },
    /*L3*/ { 1, 1, 0, 1, 1, 1, 0, 1, 0, 1 },
    /*L4*/ { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    /*L5*/ { 0, 1, 1,-2, 0, 0, 1, 1, 0, 1 },
    /*L6*/ { 0, 0, 1,-2, 1, 0, 1, 0, 0, 0 },
    /*L7*/ { 1, 0, 1, 1, 1, 0, 1, 0, 1, 0 },
    /*L8*/ { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
    /*L9*/ { 0, 1, 1, 1, 1, 1, 0, 0, 0,-1 }
};

/* PROTOTIPOS DAS FUNCOES */
void inicializarJogo(void);
void mostrarLabirinto(void);
int  validarMovimento(int novaLinha, int novaColuna);
void tocarSom(int tipoSom);
void aplicarMovimento(int novaLinha, int novaColuna);
void processarComando(char comando);
void mostrarVitoria(void);
void mostrarGameOver(void);

/*
   FUNCAO: tocarSom
   PROPOSITO: Reproduz efeitos sonoros usando o caractere de controle \a.
   PARAMETROS: tipoSom - constante que identifica qual som tocar
*/
void tocarSom(int tipoSom) {
    if (tipoSom == SOM_MOVIMENTO) {
        printf("\a");
        fflush(stdout); 
    } else if (tipoSom == SOM_PAREDE) {
        printf("\a");
        fflush(stdout);
    } else if (tipoSom == SOM_BURACO) {
        printf("\a");
        fflush(stdout);
        printf("\a");
        fflush(stdout);
        printf("\a");
        fflush(stdout);
    } else if (tipoSom == SOM_VITORIA) {
        printf("\a");
        fflush(stdout);
        printf("\a");
        fflush(stdout);
        printf("\a");
        fflush(stdout);
        printf("\a");
        fflush(stdout);
    }
}

/*
   FUNCAO: inicializarJogo
   PROPOSITO: Define o estado inicial de todas as variaveis globais do jogo.
*/
void inicializarJogo(void) {
    jogLinha   = 0;               /* Jogador inicia em (0,0)               */
    jogColuna  = 0;
    jogPassos  = 0;               /* Nenhum passo dado ainda               */
    jogPontos  = PONTOS_INICIAIS; /* 1000 pontos iniciais                  */
    jogJogando = 1;               /* Jogo ativo                            */
    sprintf(ultimoAviso, "Boa sorte! Encontre a saida [O].");
}

/*
   FUNCAO: mostrarLabirinto
   PROPOSITO: Renderiza o estado completo do jogo na tela a cada turno.
*/
void mostrarLabirinto(void) {
    int i, j; /* Variaveis de controle dos lacos - C ANSI exige declarar  */

    /* Limpa a tela: "clear" no Linux/Mac, "cls" no Windows              */
    system("clear || cls");

    /* CABECALHO DO JOGO */
    printf("%s", COR_TITULO);
    printf("==========================================================\n");
    printf("         JOGO DO LABIRINTO 10x10 - UniAvan 2026          \n");
    printf("==========================================================\n");
    printf("%s", COR_RESET);
    printf(" Controles: W(cima) A(esq) S(baixo) D(dir) | Q = Sair\n");
    printf("----------------------------------------------------------\n");

    /* HUD: DADOS DO JOGADOR EM TEMPO REAL */
    printf("%s", COR_HUD);
    printf(" Posicao : Linha %d | Coluna %d\n",
           jogLinha  + 1,
           jogColuna + 1);
    printf(" Passos  : %d\n", jogPassos);
    printf(" Pontos  : %d\n", jogPontos);
    printf("%s", COR_RESET);
    printf("----------------------------------------------------------\n");

    /* LEGENDA */
    printf(" %s@ Jogador%s  ", COR_JOGADOR, COR_RESET);
    printf("%sX Parede%s  ",   COR_PAREDE,  COR_RESET);
    printf("%sO Saida%s  ",    COR_SAIDA,   COR_RESET);
    printf("%s* Buraco%s\n",   COR_BURACO,  COR_RESET);
    printf("----------------------------------------------------------\n");

    /* RENDERIZACAO DO LABIRINTO (GRID) */
    for (i = 0; i < TAMANHO_LABIRINTO; i++) {     /* Percorre cada LINHA  */
        printf(" ");                               /* Indentacao do grid   */
        for (j = 0; j < TAMANHO_LABIRINTO; j++) { /* Percorre cada COLUNA */

            if (i == jogLinha && j == jogColuna) {
                printf("%s@ %s", COR_JOGADOR, COR_RESET);
            } else if (labirinto[i][j] == PAREDE) {
                printf("%sX %s", COR_PAREDE, COR_RESET);
            } else if (labirinto[i][j] == SAIDA) {
                printf("%sO %s", COR_SAIDA, COR_RESET);
            } else if (labirinto[i][j] == BURACO) {
                printf("%s* %s", COR_BURACO, COR_RESET);
            } else {
                printf(". ");
            }

        }
        printf("\n"); /* Quebra de linha ao terminar cada linha do grid   */
    }

    /* AVISO DO ULTIMO EVENTO */
    printf("----------------------------------------------------------\n");
    printf("%s >> %s%s\n", COR_AVISO, ultimoAviso, COR_RESET);
    printf("==========================================================\n");
}

/*
   FUNCAO: validarMovimento
   PROPOSITO: Verifica se a posicao de destino e valida.
              Retorna 1 (permitido) ou 0 (bloqueado).
   PARAMETROS:
     novaLinha  - linha de destino calculada apos o comando
     novaColuna - coluna de destino calculada apos o comando
*/
int validarMovimento(int novaLinha, int novaColuna) {
    /* Verifica se o destino esta dentro dos limites da matriz (0 a 9)   */
    if (novaLinha  < 0 || novaLinha  >= TAMANHO_LABIRINTO ||
        novaColuna < 0 || novaColuna >= TAMANHO_LABIRINTO) {
        return 0; /* Fora dos limites: MOVIMENTO BLOQUEADO               */
    }

    /* Verifica se a celula de destino e uma parede                      */
    if (labirinto[novaLinha][novaColuna] == PAREDE) {
        return 0; /* E parede: MOVIMENTO BLOQUEADO                       */
    }

    return 1; /* Nenhuma restricao: MOVIMENTO PERMITIDO                  */
}

/*
   FUNCAO: aplicarMovimento
   PROPOSITO: Atualiza as variaveis globais de estado apos um movimento
              valido. Processa consequencias: passo, buraco ou vitoria.
   PARAMETROS:
     novaLinha  - linha de destino ja validada
     novaColuna - coluna de destino ja validada
*/
void aplicarMovimento(int novaLinha, int novaColuna) {
    /* Atualiza a posicao do jogador diretamente nas variaveis globais   */
    jogLinha  = novaLinha;
    jogColuna = novaColuna;

    jogPassos++;              /* Incrementa contador de passos            */
    jogPontos -= CUSTO_PASSO; /* Desconta custo do passo (-5)             */

    /* Verifica o conteudo da celula de destino (ja atualizada acima)    */
    if (labirinto[jogLinha][jogColuna] == SAIDA) {
        tocarSom(SOM_VITORIA);
        jogJogando = 0; /* Encerra o loop principal                       */
        sprintf(ultimoAviso, "VOCE CHEGOU NA SAIDA! PARABENS!");

    } else if (labirinto[jogLinha][jogColuna] == BURACO) {
        tocarSom(SOM_BURACO);
        jogPontos -= CUSTO_BURACO; /* Penalidade extra de -150 pontos     */
        sprintf(ultimoAviso,
                "ARMADILHA! Voce caiu num buraco! -%d pontos!", CUSTO_BURACO);

        if (jogPontos <= 0) {
            jogPontos  = 0;
            jogJogando = 0; /* Game over por pontos zerados               */
        }

    } else {
        tocarSom(SOM_MOVIMENTO);
        sprintf(ultimoAviso,
                "Movimento realizado. Passos: %d | Pontos: %d",
                jogPassos, jogPontos);
    }
}

/*
   FUNCAO: processarComando
   PROPOSITO: Interpreta o caractere digitado, calcula o destino e
              delega para validarMovimento() e aplicarMovimento().
   PARAMETROS:
     comando - caractere digitado (ja em maiuscula)
*/
void processarComando(char comando) {
    int novaLinha  = jogLinha;   /* Copia da variavel global              */
    int novaColuna = jogColuna;  /* Copia da variavel global              */

    if (comando == 'W') {
        novaLinha--;  /* Cima: diminui indice de linha (0 = topo)         */
    } else if (comando == 'S') {
        novaLinha++;  /* Baixo: aumenta indice de linha                   */
    } else if (comando == 'A') {
        novaColuna--; /* Esquerda: diminui indice de coluna               */
    } else if (comando == 'D') {
        novaColuna++; /* Direita: aumenta indice de coluna                */
    } else if (comando == 'Q') {
        jogJogando = 0; /* Encerra voluntariamente                        */
        sprintf(ultimoAviso, "Voce abandonou o jogo. Ate a proxima!");
        return;
    } else {
        sprintf(ultimoAviso,
                "Comando invalido! Use W, A, S, D para mover ou Q para sair.");
        return;
    }

    /* Verifica se o destino e valido e aplica ou penaliza               */
    if (validarMovimento(novaLinha, novaColuna)) {
        aplicarMovimento(novaLinha, novaColuna);
    } else {
        tocarSom(SOM_PAREDE);
        jogPontos -= CUSTO_PAREDE; /* -50 pontos por bater na parede      */

        if (jogPontos < 0) {
            jogPontos = 0;
        }

        sprintf(ultimoAviso,
                "PAREDE! Movimento bloqueado. -%d pontos! (Total: %d)",
                CUSTO_PAREDE, jogPontos);

        if (jogPontos == 0) {
            jogJogando = 0;
        }
    }
}

/*
   FUNCAO: mostrarVitoria
   PROPOSITO: Exibe a tela final de vitoria.
*/
void mostrarVitoria(void) {
    system("clear || cls");
    printf("%s", COR_VITORIA);
    printf("\n");
    printf("  **********************************************************\n");
    printf("  * *\n");
    printf("  * PARABENS! VOCE ESCAPOU DO LABIRINTO!                 *\n");
    printf("  * *\n");
    printf("  **********************************************************\n");
    printf("\n");
    printf("%s", COR_HUD);
    printf("  Resumo da partida:\n");
    printf("  - Passos dados   : %d\n", jogPassos);
    printf("  - Pontuacao final: %d / %d\n", jogPontos, PONTOS_INICIAIS);
    printf("\n");
    printf("%s", COR_TITULO);
    printf("  Desenvolvido por: Victor da Silva Goncalves\n");
    printf("  UniAvan - Algoritmos II++ - 2026\n");
    printf("%s\n", COR_RESET);
}

/*
   FUNCAO: mostrarGameOver
   PROPOSITO: Exibe a tela final de derrota.
*/
void mostrarGameOver(void) {
    system("clear || cls");
    printf("%s", COR_DERROTA);
    printf("\n");
    printf("  **********************************************************\n");
    printf("  * *\n");
    printf("  * GAME OVER! SEM PONTOS RESTANTES!             *\n");
    printf("  * *\n");
    printf("  **********************************************************\n");
    printf("\n");
    printf("%s", COR_HUD);
    printf("  Resumo da partida:\n");
    printf("  - Passos dados   : %d\n", jogPassos);
    printf("  - Pontuacao final: 0\n");
    printf("\n");
    printf("%s", COR_TITULO);
    printf("  Tente novamente! O labirinto ainda espera por voce.\n");
    printf("%s\n", COR_RESET);
}

/*
   FUNCAO: main
   PROPOSITO: Ponto de entrada. Inicializa o jogo e executa o loop
              principal ate vitoria, derrota ou desistencia.
*/
int main(void) {
    char comando; /* Caractere lido do teclado a cada turno               */

    /* Configura o locale para suporte a caracteres em portugues         */
    setlocale(LC_ALL, "Portuguese");

    /* Inicializa as variaveis globais com os valores de inicio de jogo  */
    inicializarJogo();

    /* LOOP PRINCIPAL DO JOGO */
    while (jogJogando) {

        mostrarLabirinto(); /* Renderiza tela com HUD e grid             */

        if (!jogJogando) {
            break; /* Estado pode ter mudado durante a renderizacao      */
        }

        printf(" Digite seu movimento (W/A/S/D) ou Q para sair: ");
        scanf(" %c", &comando); /* Espaco antes de %c descarta o '\n' residual */

        comando = (char)toupper((unsigned char)comando);

        processarComando(comando);

    }

    /* TELA FINAL */
    if (labirinto[jogLinha][jogColuna] == SAIDA) {
        mostrarVitoria();
    } else if (jogPontos == 0) {
        mostrarGameOver();
    }

    printf("%s", COR_RESET); /* Restaura cor padrao do terminal          */
    return 0;
}
