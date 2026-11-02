// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

int len(char* arr){
    // calcula la longitud de un string sin incluir '\0'
    
    int cont = 0; // Contador final para el tamano del string

    while (arr[cont] != '\0') {
        cont++;
    }

    return cont;
}

char* strDuplicate(char* src) {
    
    int cont = len(src);

    char* copia = (char*)malloc(sizeof(char) * (cont+1));

    if (copia == NULL) return NULL;

    for (int i = 0; i<cont; i++){
        copia[i] = src[i];
    }
    copia[cont] = '\0';

    return copia;
}

int strCompare(char* s1, char* s2) {
  
    // Recorrer ambos mientras tengan caracteres
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0'){
        if (s1[i] < s2[i]) return 1;
        if (s1[i] > s2[i]) return -1;
       i++;
    }

    // Si los dos terminaron al mismo tiempo, son iguales
    if (s1[i] == '\0' && s2[i] == '\0') {
        return 0;
    } else if (s1[i] == '\0'){          // Si alguno termino el ciclo antes
        return 1;
    } else {
        return -1;
    }
}



char* strConcatenate(char* src1, char* src2) {
    // Contadores para el tamano de cada string y solicitar memoria para el nuevo

    int size1 = len(src1);
    int size2 = len(src2); 
    
    char* nuevo = (char*)malloc(sizeof(char) * (size1 + size2 + 1)); // los tamanos de los dos strings + '\0'
    if (nuevo == NULL) return NULL;

    int i = 0;
    while (i < size1){ // recorre el primer string
        nuevo[i] = src1[i];
        i++;
    }
    free(src1); // Libera primer string

    int j = 0;
    while (j < size2){ // a partir de i (que queda en la ultima posicion de src1), recorre el segundo string
        nuevo[i] = src2[j];
        i++;
        j++;
    }
    free(src2); // Libera elsegundo

    nuevo[size1+size2] = '\0';

    return nuevo;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

void cellsEn0(int cells[]){
    // Funcion auxiliar para inicializar todas las celdas de una fila en 0
    for (int i = 0; i < GRID_COLS; i++){
        cells[i] = 0;
    }
}

GameBoard* gameBoardNew() {
    int contF = 0;

    GameBoard* board = (GameBoard*)malloc(sizeof(GameBoard)); // Pide memoria para el tablero
    if (board == NULL) return NULL;

    RowNode* fila = (RowNode*)malloc(sizeof(RowNode)); // Pide memoria para el primer nodo
    if (fila == NULL){
        free(board);
        return NULL;
    }
    contF++;
    fila->next = NULL;
    cellsEn0(fila->cells); // Inicializa la primera fila en 0
    RowNode* actual = fila;

    for (int i = 1; i < 20; i++){
        RowNode* nuevo = (RowNode*)malloc(sizeof(RowNode)); // Pide memoria para el siguiente nodo
        if (nuevo == NULL){
            for (int F = 0; F < contF; F++){
                RowNode* sig = fila->next;
                free(fila);
                fila = sig; // Recorre desde la primera fila hasta la ultima que fue contada
            }
            free(board);
            return NULL;
        } // Cada vez que se crea un nodo nuevo revisa si le asigno un espacio de memoria, sino libera todo

        cellsEn0(nuevo->cells);
        nuevo->next = NULL;
        contF++;
        actual->next = nuevo; 
        actual = nuevo;
    }

    board->first_row = fila; // Asigna el primer nodo creado
    board->queue_head = NULL;
    board->queue_tail = NULL;
    board->first_particle = NULL;
    
    board->current_piece.active = 0; // pieza inactiva
    
    board->score = 0;
    board->tick_timer = 0;
    
    return board;
}


void gameBoardDelete(GameBoard* board) {
    
    if (board == NULL) return; // Caso base

    RowNode* actual = board->first_row;
    RowNode* siguiente = actual;
    while (actual != NULL){
        siguiente = siguiente -> next;
        free (actual);
        actual = siguiente;
    }

    PieceQueueNode* cuello = board->queue_head;
    PieceQueueNode* cabeza = cuello;
    while (cuello != NULL){
        cabeza = cabeza ->next;
        free(cuello);
        cuello = cabeza;
    }

    ParticleNode*  particula = board->first_particle;
    ParticleNode* particula2 = particula;
    while (particula != NULL){
        particula2 = particula2->next;
        free(particula);
        particula = particula2;
    }

    free(board);
}

void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    ParticleNode* nuevo = (ParticleNode*)malloc(sizeof(ParticleNode));
    nuevo->x = x;
    nuevo->y = y;
    nuevo->vel_x = ((rand() % 100) / 10.0f) - 5.0f; // datos de juego_base
    nuevo->vel_y = ((rand() % 50) / -10.0f) - 2.0f;
    nuevo->color = color;
    nuevo->life_timer = 30 + (rand() % 30);
    nuevo->next = board->first_particle;
    board->first_particle = nuevo;
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    
    RowNode* actual = board->first_row;
    RowNode* antes = NULL;

    int llena = 0; // Sera 10 si esta llena
    int eliminadas = 0;
    int fila = 0;

    while (actual != NULL){ // Ciclo para las filas
        llena = 0;
        for (int j=0; j<GRID_COLS; j++){ // Ciclo para las cells
            if (actual->cells[j] > 0){
                llena++;
            }
        }
        if (llena < 10){// Fila no llena
            antes = actual;
            actual = actual->next;
        } else { // Fila llena
            for (int i = 0 ; i< 10; i++){
                gameBoardAddParticle(board, OFFSET_X + i * CELL_SIZE + CELL_SIZE/2, OFFSET_Y + fila * CELL_SIZE + CELL_SIZE/2, actual->cells[i]);
            } // Llama a las particulas por cada celda
            if (actual == board->first_row) { // Caso primera fila llena
                RowNode* nuevo = (RowNode*)malloc(sizeof(RowNode));
                nuevo->next = actual->next;
                board->first_row = nuevo;
                antes = nuevo;
                cellsEn0(nuevo->cells);
                RowNode* aborrar = actual;
                actual=actual->next;
                free(aborrar);
            } else { // Caso ordinario
                antes->next = actual->next;
                RowNode* aborrar = actual;
                actual = actual->next; // Enlaza la fila anterior con la siguiente, eliminando la actual
                free(aborrar); // Libera la fila llena
                RowNode* nuevo = (RowNode*)malloc(sizeof(RowNode)); // Crea una fila nueva
                nuevo->next=board->first_row;
                board->first_row = nuevo;
                cellsEn0(nuevo->cells);
            }
            eliminadas++;
        }
        
        fila++;
    }
    return eliminadas;
}

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    PieceQueueNode* nuevo = (PieceQueueNode*)malloc(sizeof(PieceQueueNode));
    nuevo->type = type;
    nuevo->next = NULL;

    if (board->queue_tail == NULL) {
        board->queue_head = nuevo;
        board->queue_tail = nuevo;
    }
    else {
        board->queue_tail->next = nuevo;
        board->queue_tail = nuevo;
    }
}

int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    if(board->queue_head == NULL && board->queue_tail == NULL) return 0;     //caso lista vacia
    
    *out_type = (board->queue_head)->type;                                   //extraemos el tipo de la primera pieza de la cola
    PieceQueueNode* queue_head_anterior = board->queue_head;
    board->queue_head = (board->queue_head)->next;
    free(queue_head_anterior);
    
    if(board->queue_head == NULL) board->queue_tail=NULL;                   //caso lista de un solo elemento
    return 1;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS =========

int testsFallidos = 0;

void check(int condicion, char* nombre) {
    if (condicion) {
        printf("  OK     %s\n", nombre);
    } else {
        printf("  FALLO  %s\n", nombre);
        testsFallidos++;
    }
}

// Cuenta las filas recorriendo la lista hasta NULL
int contarFilas(GameBoard* board) {
    int n = 0;
    for (RowNode* r = board->first_row; r != NULL; r = r->next) n++;
    return n;
}

// Cuenta las particulas de la lista
int contarParticulas(GameBoard* board) {
    int n = 0;
    for (ParticleNode* p = board->first_particle; p != NULL; p = p->next) n++;
    return n;
}

// Llena todas las celdas de la fila de indice `indice` con 1
void llenarFila(GameBoard* board, int indice) {
    RowNode* fila = getRowAt(board, indice);
    for (int c = 0; c < GRID_COLS; c++) fila->cells[c] = 1;
}

// Devuelve 1 si todas las celdas de la fila valen 0
int filaVacia(RowNode* fila) {
    for (int c = 0; c < GRID_COLS; c++) {
        if (fila->cells[c] != 0) return 0;
    }
    return 1;
}

// Devuelve 1 si todas las filas del tablero estan vacias
int tableroVacio(GameBoard* board) {
    for (RowNode* r = board->first_row; r != NULL; r = r->next) {
        if (!filaVacia(r)) return 0;
    }
    return 1;
}

void correrTests() {
    // ---- strDuplicate ----
    printf("strDuplicate\n");
    char* d1 = strDuplicate("");
    check(d1 != NULL && strCompare(d1, "") == 0, "1. string vacio");
    free(d1);
    char* d2 = strDuplicate("x");
    check(d2 != NULL && strCompare(d2, "x") == 0, "2. un caracter");
    free(d2);
    char* d3 = strDuplicate("qwerty");
    check(d3 != NULL && strCompare(d3, "qwerty") == 0, "3. varios caracteres");
    free(d3);

    // ---- strCompare ----
    printf("strCompare\n");
    check(strCompare("", "") == 0, "1. dos vacios");
    check(strCompare("a", "a") == 0, "2. un caracter iguales");
    check(strCompare("a", "b") == 1, "2. un caracter: a < b");
    check(strCompare("abc", "abd") == 1, "3. abc vs abd");
    check(strCompare("abd", "abc") == -1, "3. abd vs abc (orden inverso)");
    check(strCompare("casa", "casas") == 1, "extra: el prefijo es menor");
    check(strCompare("b", "abc") == -1, "extra: b > abc aunque sea mas corto");

    // ---- strConcatenate ----
    printf("strConcatenate\n");
    char* s1 = strDuplicate("");
    char* s2 = strDuplicate("abc");
    char* c1 = strConcatenate(s1, s2);
    check(c1 != NULL && strCompare(c1, "abc") == 0, "1. vacio + 3 caracteres");
    free(c1);
    char* s3 = strDuplicate("hola ");
    char* s4 = strDuplicate("mundo");
    char* c2 = strConcatenate(s3, s4);
    check(c2 != NULL && strCompare(c2, "hola mundo") == 0, "2. dos strings de 5 caracteres");
    free(c2);

    // ---- Cola ----
    printf("Cola\n");
    GameBoard* b = gameBoardNew();
    int out = -1;
    for (int i = 0; i < 5; i++) gameBoardEnqueuePiece(b, i);

    int fifo = 1;
    for (int i = 0; i < 5; i++) {
        int ok = gameBoardDequeuePiece(b, &out);
        if (!ok || out != i) fifo = 0;
    }
    check(fifo, "5 push y 5 pop respetan el orden FIFO");

    out = -1;
    check(gameBoardDequeuePiece(b, &out) == 0, "el 6to pop devuelve 0");
    check(out == -1, "el 6to pop no modifica out_type");
    check(b->queue_head == NULL && b->queue_tail == NULL, "head y tail quedan en NULL");

    gameBoardEnqueuePiece(b, 6);
    check(b->queue_head != NULL && b->queue_head == b->queue_tail, "push posterior: head == tail");
    check(gameBoardDequeuePiece(b, &out) == 1 && out == 6, "push posterior: se puede extraer");
    check(b->queue_head == NULL && b->queue_tail == NULL, "vuelve a quedar vacia");
    gameBoardDelete(b);

    // ---- Tablero ----
    printf("Tablero\n");

    // 2. Ultima fila llena
    b = gameBoardNew();
    check(contarFilas(b) == 20, "tablero nuevo tiene 20 filas");
    RowNode* primeraAntes = b->first_row;
    llenarFila(b, GRID_ROWS - 1);
    int borradas = gameBoardCheckAndClearLines(b);
    check(borradas == 1, "fila 19 llena: devuelve 1");
    check(b->first_row != primeraAntes && filaVacia(b->first_row), "fila 19 llena: first_row es una fila nueva vacia");
    check(contarFilas(b) == 20, "fila 19 llena: sigue habiendo 20 filas");
    check(contarParticulas(b) == GRID_COLS, "fila 19 llena: una particula por celda");
    gameBoardDelete(b);

    // 3. Filas 0 y 1 llenas
    b = gameBoardNew();
    llenarFila(b, 0);
    llenarFila(b, 1);
    borradas = gameBoardCheckAndClearLines(b);
    check(borradas == 2, "filas 0 y 1 llenas: devuelve 2");
    check(contarFilas(b) == 20, "filas 0 y 1 llenas: sigue habiendo 20 filas");
    check(tableroVacio(b), "filas 0 y 1 llenas: el tablero queda vacio");
    gameBoardDelete(b);

    // Extra: una fila incompleta no se borra
    b = gameBoardNew();
    llenarFila(b, 19);
    getRowAt(b, 19)->cells[4] = 0;
    check(gameBoardCheckAndClearLines(b) == 0, "extra: fila con un hueco no se borra");
    check(contarFilas(b) == 20, "extra: fila con un hueco: 20 filas");
    gameBoardDelete(b);

    if (testsFallidos == 0) printf("\nTodos los tests pasaron\n");
    else printf("\n%d test(s) fallaron\n", testsFallidos);
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
