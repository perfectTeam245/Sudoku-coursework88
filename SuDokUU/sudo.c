#define STB_IMAGE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS 
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "libs/glfw/lib-vc2019/glfw3.lib")
#include "libs/glfw/include/GLFW/glfw3.h"
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "stb_image.h"
#define N 9
#define CNT_TEXT 21

int gridDigits[N][N] = { 0 }; // Массив для хранения цифр в клетках (0 - нет цифры)
int SolutionSudoku[N][N] = { 0 };
int CountgridDigits = 0; // Сколько в данный момент цифр на поле
int gameState = 0; // 0 - меню
int ALG_State = 0; // 0 - Backtracking; 1 - Logical

// Размеры игрового поля
float side = 1.35f;

// Глобальные переменные для хранения координат выбранной клетки
float selectedCellX = -2.0f; // Начальное значение вне сетки
float selectedCellY = -2.0f;

int ButtonEasyState = 1; // 0 - ничего, 1 - обводится в рамку
int ButtonNormalState = 0;
int ButtonHardState = 0;


enum Game_States {
    MENU = 0,
    GAME,
    SETTINGS,
    YOU_WIN,
    YOU_WIN1,
    SAVED_SUDOKU,
    COMP_MODE,
    EXIT,
    CREATE,
    GAME1,
};

enum TexturesName {
    TEXT_WICTORY = 0,
    TEXT_HOME,
    TEXT_MENU,
    TEXT_GAME,
    TEXT_SETTINGS,
    TEXT_PLAY_BUT,
    TEXT_SETTINGS_BUT,
    TEXT_SAVED_SUDOKU_BUT,
    TEXT_COMUTER_MODE_BUT,
    TEXT_EXIT_BUT,
    TEXT_ALG,
    TEXT_BACK_TRACK_BUT,
    TEXT_LOGICAL_BUT,
    TEXT_DIFFICULTY,
    TEXT_BACK_BUT,
    TEXT_COMP_MODE,
    TEXT_CREATE_BUT,
    TEXT_SOLVE_BUT,
    TEXT_DECIDE_BUT,
    TEXT_SAVE_BUT,
    TEXT_SAVEDSUDOKU
};

enum Colors {
    RED = -1,
    BLACK = 0,
    BLUE,
    PINK
};

union CellColor {
    enum Colors color;
};

union CellColor input_Digits[N][N] = { 0 };


double startTime = 0.0;
double elapsedTime = 0.0;
int gameActive = 0;


typedef struct LeaderboardEntry {
    double time;
    struct LeaderboardEntry* next;
} LeaderboardEntry;

LeaderboardEntry* head = NULL;


void startTimer() {
    startTime = glfwGetTime();
    gameActive = 1;
}

void stopTimer() {
    elapsedTime = glfwGetTime() - startTime;
    gameActive = 0;
}

double getElapsedTime() {
    if (gameActive) {
        return glfwGetTime() - startTime;
    }
    return elapsedTime;
}

// Часть кода, связанная с алгоритмом back tracking

typedef struct {
    int possible[N]; // 1 — число возможно, 0 — нет
    int count;       // Количество возможных чисел
} CellOptions;

CellOptions options[N][N];

// Функция обновления возможных значений после вставки числа
int updateOptions(int i, int j, int num, int remove) {
    int startRow = i - i % 3; // Начало строки блока 3x3
    int startCol = j - j % 3; // Начало столбца блока 3x3

    for (int k = 0; k < N; k++) { // Проверка строки
        if (gridDigits[i][k] == 0) {
            if (remove && options[i][k].possible[num - 1]) {
                options[i][k].possible[num - 1] = 0;
                options[i][k].count--;
            }
            if (options[i][k].count == 0) return 0; // Ошибка: нет кандидатов
        }
    }

    for (int k = 0; k < N; k++) { // Проверка столбца
        if (gridDigits[k][j] == 0) {
            if (remove && options[k][j].possible[num - 1]) {
                options[k][j].possible[num - 1] = 0;
                options[k][j].count--;
            }
            if (options[k][j].count == 0) return 0; // Ошибка: нет кандидатов
        }
    }

    for (int r = 0; r < 3; r++) { // Проверка блока 3x3
        for (int c = 0; c < 3; c++) {
            int ri = startRow + r;
            int cj = startCol + c;
            if (gridDigits[ri][cj] == 0) {
                if (remove && options[ri][cj].possible[num - 1]) {
                    options[ri][cj].possible[num - 1] = 0;
                    options[ri][cj].count--;
                }
                if (options[ri][cj].count == 0) return 0; // Ошибка: нет кандидатов
            }
        }
    }
    return 1;
}

// Функция инициализации возможных значений
void initOptions() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (gridDigits[i][j] == 0) { // Для пустых ячеек
                for (int num = 0; num < N; num++) {
                    options[i][j].possible[num] = 1; // Все числа возможны
                }
                options[i][j].count = N;
            }
            else { // Для заполненных ячеек
                for (int num = 0; num < N; num++) {
                    options[i][j].possible[num] = 0; // Никакие числа не возможны
                }
                options[i][j].count = 0;
            }
        }
    }
    for (int i = 0; i < N; i++) { // Обновляем на основе заполненных ячеек
        for (int j = 0; j < N; j++) {
            if (gridDigits[i][j] != 0) {
                updateOptions(i, j, gridDigits[i][j], 1);
            }
        }
    }
}

bool solveBacktracking() {
    int minCount = N + 1; // Минимальное количество кандидатов
    int minI = -1, minJ = -1; // Координаты ячейки с минимальным количеством кандидатов
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (gridDigits[i][j] == 0 && options[i][j].count < minCount) {
                minCount = options[i][j].count;
                minI = i;
                minJ = j;
            }
        }
    }
    if (minI == -1) {
        return true;
    } // Если нет пустых ячеек, судоку решено

    for (int num = 1; num <= N; num++) { // Пробуем каждое возможное число
        if (options[minI][minJ].possible[num - 1]) {
            gridDigits[minI][minJ] = num; // Устанавка числа
            input_Digits[minI][minJ].color = PINK;
            CellOptions tempOptions[N][N]; // Сохранение копии options для отката
            memcpy(tempOptions, options, sizeof(options));
            if (updateOptions(minI, minJ, num, 1)) { // Обновление кандидатов
                if (solveBacktracking()) return true; // Рекурсивное продолжение
            }
            gridDigits[minI][minJ] = 0; // Откат изменения
            input_Digits[minI][minJ].color = BLACK;
            memcpy(options, tempOptions, sizeof(options)); // Восстанавление options
        }
    }
    return false; // Нет решения
}


// Логическое Решение судоку 

bool is_safe(int row, int col, int num) {
    //ряд
    for (int j = 0; j < N; j++) {
        if (gridDigits[row][j] == num) {
            return false;
        }
    }
    //столбец
    for (int i = 0; i < N; i++) {
        if (gridDigits[i][col] == num) {
            return false;
        }
    }
    // 3 x 3 блок
    int start_row = row - row % 3;
    int start_col = col - col % 3;
    for (int r = 0; r < 3; r++) {
        for (int d = 0; d < 3; d++) {
            if (gridDigits[start_row + r][start_col + d] == num) {
                return false;
            }
        }
    }
    return true;
}


void initialize_possibilities(bool possib[N][N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (gridDigits[i][j] == 0) {
                for (int num = 1; num <= N; num++) {
                    possib[i][j][num - 1] = is_safe(i, j, num);
                }
            }
            else {
                for (int num = 0; num < N; num++) {
                    possib[i][j][num] = false;
                }
            }
        }
    }
}

// Поиск клеток с единственным возможным числом
bool find_naked_singles(bool possib[N][N][N]) {
    bool progress = false;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (gridDigits[i][j] == 0) {
                int count = 0;
                int possible_num = 0;
                for (int num = 0; num < N; num++) {
                    if (possib[i][j][num]) {
                        count++;
                        possible_num = num + 1;
                    }
                }
                if (count == 1) {
                    gridDigits[i][j] = possible_num;
                    input_Digits[i][j].color = PINK;
                    progress = true;
                    initialize_possibilities(possib);
                }
            }
        }
    }
    return progress;
}

// Поиск скрытых одиночек по рядам
bool find_hidden_singles_row(bool possib[N][N][N]) {
    bool progress = false;
    for (int i = 0; i < N; i++) {
        for (int num = 1; num <= N; num++) {
            int count = 0;
            int possible_col = -1;
            for (int j = 0; j < N; j++) {
                if (gridDigits[i][j] == 0 && possib[i][j][num - 1]) {
                    count++;
                    possible_col = j;
                }
            }
            if (count == 1) {
                gridDigits[i][possible_col] = num;
                input_Digits[i][possible_col].color = PINK;
                progress = true;
                initialize_possibilities(possib);
            }
        }
    }
    return progress;
}

// Поиск скрытых одиночек в столбцах 
bool find_hidden_singles_col(bool possib[N][N][N]) {
    bool progress = false;
    for (int j = 0; j < N; j++) {
        for (int num = 1; num <= N; num++) {
            int count = 0;
            int possible_row = -1;
            for (int i = 0; i < N; i++) {
                if (gridDigits[i][j] == 0 && possib[i][j][num - 1]) {
                    count++;
                    possible_row = i;
                }
            }
            if (count == 1) {
                gridDigits[possible_row][j] = num;
                input_Digits[possible_row][j].color = PINK;
                progress = true;
                initialize_possibilities(possib);
            }
        }
    }
    return progress;
}

// Поиск скрытых одиночек в блоках 3x3 
bool find_hidden_singles_3x3(bool possib[N][N][N]) {
    bool progress = false;
    for (int block_row = 0; block_row < 3; block_row++) {
        for (int block_col = 0; block_col < 3; block_col++) {
            for (int num = 1; num <= N; num++) {
                int count = 0;
                int possible_i = -1;
                int possible_j = -1;
                for (int i = block_row * 3; i < block_row * 3 + 3; i++) {
                    for (int j = block_col * 3; j < block_col * 3 + 3; j++) {
                        if (gridDigits[i][j] == 0 && possib[i][j][num - 1]) {
                            count++;
                            possible_i = i;
                            possible_j = j;
                        }
                    }
                }
                if (count == 1) {
                    gridDigits[possible_i][possible_j] = num;
                    input_Digits[possible_i][possible_j].color = PINK;
                    progress = true;
                    initialize_possibilities(possib);
                }
            }
        }
    }
    return progress;
}

void solve_logical() {
    bool possib[N][N][N]; // Массив возможных значений
    initialize_possibilities(possib);

    bool progress = true;
    while (progress) {
        progress = false;
        if (find_naked_singles(possib)) progress = true;
        if (find_hidden_singles_row(possib)) progress = true;
        if (find_hidden_singles_col(possib)) progress = true;
        if (find_hidden_singles_3x3(possib)) progress = true;
    }
}


// Часть кода с генерацией судоку
void Del_gridDidits() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            gridDigits[i][j] = 0;
            input_Digits[i][j].color = BLACK;
        }
    }
}

int Check_cell(int num, int i, int j) {
    // Проверка столбца
    for (int k = 0; k < N; k++) {
        if (gridDigits[k][j] == num)
            return 0;
    }
    // Проверка строки
    for (int k = 0; k < N; k++) {
        if (gridDigits[i][k] == num)
            return 0;
    }
    // Проверка блока 3 на 3
    int startRow = i - i % 3;  // Определяем начало блока 3 на 3
    int startCol = j - j % 3;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            if (gridDigits[startRow + row][startCol + col] == num) {
                return 0;  // Число уже есть в блоке 3 на 3
            }
        }
    }
    return 1;  // Числа нет, можно ставить
}

void Fill_diagonal() {
    int nums[N] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    for (int block = 0; block < N; block += 3) {
        // Перемешивание чисел
        for (int i = 0; i < N - 1; i++) {
            int j = i + rand() % (N - i);
            int temp = nums[i];
            nums[i] = nums[j];
            nums[j] = temp;
        }
        int idx = 0;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                gridDigits[block + i][block + j] = nums[idx++];
            }
        }
    }
}

int Generate_field(int cell) {

    int i = 0, j = 0, num = 0;

    if (cell == 81) {
        memmove(SolutionSudoku, gridDigits, sizeof(gridDigits));
        return 1;
    }

    i = cell / 9;
    j = cell % 9;

    if (gridDigits[i][j] != 0) {
        return Generate_field(cell + 1);
    }

    for (num = 1; num <= 9; num++) {
        if (Check_cell(num, i, j)) {
            gridDigits[i][j] = num;
            if (Generate_field(cell + 1)) {
                return 1;
            }
            gridDigits[i][j] = 0;  // Сбрасываем, если не удалось
        }
    }
    return 0;  // Не удалось найти подходящее число
}

// удаление подсказок
void Build_level(int cells_to_remove) {

    int removed = 0;

    while (removed < cells_to_remove) {
        int i = rand() % N; // Случайная строка
        int j = rand() % N; // Случайный столбец
        if (gridDigits[i][j] != 0) { // Если ячейка заполнена
            gridDigits[i][j] = 0;    // Удаляем число
            removed++;
        }
    }
}

void Countdigits() {
    CountgridDigits = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (gridDigits[i][j] != 0 && (input_Digits[i][j].color == BLUE || input_Digits[i][j].color == BLACK)) {
                CountgridDigits++;
            }
        }
    }
}

int check_wrong_num() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (input_Digits[i][j].color == RED) {
                return 1;
            }
        }
    }
    return 0;
}

// Часть кода, связанная с Графикой


// Структуры для кнопок
typedef struct {
    float x, y, width, height; // Координаты и размеры
    int hovered;
} But1;

typedef struct {
    float x, y, width, height;
    int hovered;
} But2;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButSaved;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButComp;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButExit;

typedef struct {
    float x, y, width, height;
    const char* label;
    int hovered;
} ButEasy;

typedef struct {
    float x, y, width, height;
    const char* label;
    int hovered;
} ButNormal;

typedef struct {
    float x, y, width, height;
    const char* label;
    int hovered;
} ButHard;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButBack;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButBacktrack;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButLogic;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButHome;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButCreate;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButSolve;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButDecide;

typedef struct {
    float x, y, width, height;
    int hovered;
} ButSave;

typedef struct {
    float x, y, width, height;
    const char* label;
    int hovered;
} Butsave1;


// Заполнение полей структур кнопок
But1 playButton = { -0.4f, 0.5f, 0.8f, 0.2f, 0 };
But2 setButton = { -0.4f, 0.2f, 0.8f, 0.2f, 0 };
ButSaved savedButton = { -0.4f, -0.1f, 0.8f, 0.2f, 0 };
ButComp compButton = { -0.4f, -0.4f, 0.8f, 0.2f, 0 };
ButExit exitButton = { -0.4f, -0.7f, 0.8f, 0.2f, 0 };
ButEasy easyButton = { -0.4f, 0.0f, 0.8f, 0.2f, "EASY", 0 };
ButNormal normalButton = { -0.4f, -0.3f, 0.8f, 0.2f, "NORMAL", 0 };
ButHard hardButton = { -0.4f, -0.6f, 0.8f, 0.2f, "HARD", 0 };
ButBack backButton = { -1.0f, 1.0f, 0.2f, 0.2f, 0 };
ButBacktrack backtrackButton = { -0.7f, 0.77f,  0.6f, 0.17f, 0 };
ButLogic logicButton = { 0.1f, 0.77f,  0.6f, 0.17f, 0 };
ButHome homeButton = { -0.4f, -0.5, 0.8f, 0.3f, 0 };
ButCreate createButton = { -0.5f, 0.3f, 1.0f, 0.3f, 0 };
ButSolve solveButton = { -0.53f, -0.1f, 1.01f, 0.3f, 0 };
ButDecide decideButton = { -0.3f, 1.0f, 0.6f, 0.2f, 0 };
ButSave saveButton = { -0.3f, 1.0f, 0.6f, 0.2f, 0 };
Butsave1 save1Button = { -0.7f, 1.0f, 1.4f, 0.2f, "GAME I", 0 };
// Текстуры
GLuint Textures[CNT_TEXT];

void drawTexture(float left, float right, float bottom, float top) {
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(left, bottom);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(right, bottom);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(right, top);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(left, top);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawField() {
    glLineWidth(3.0f); // Толщина линий 5 пикселей для жирных
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f); // Чёрный цвет

    float half_side = side / 2;
    int countX = 0, countY = 0;


    // Горизонтальные жирные линии
    for (float y = -half_side, x = half_side; countY <= 3; y += side / 3, countY++) {
        glVertex2f(-x, y);
        glVertex2f(x, y);
    }

    // Вертикальные жирные линии
    for (float x = -half_side, y = half_side; countX <= 3; x += side / 3, countX++) {
        glVertex2f(x, -y);
        glVertex2f(x, y);
    }
    glEnd();

    glLineWidth(1.0f); // Толщина линий 1 пиксель для тонких
    glBegin(GL_LINES);

    countY = countX = 0;

    // Горизонтальные тонкие линии
    for (float y = -half_side, x = half_side; countY <= 9; y += side / 9, countY++) {
        if (countY % 3 == 0) continue; // Пропускаем жирные линии
        glVertex2f(-x, y);
        glVertex2f(x, y);
    }

    // Вертикальные тонкие линии
    for (float x = -half_side, y = half_side; countX <= 9; x += side / 9, countX++) {
        if (countX % 3 == 0) continue; // Пропускаем жирные линии
        glVertex2f(x, -y);
        glVertex2f(x, y);
    }
    glEnd();
}

// Функция для отрисовки цифр
void drawDigit() {
    float half_side = side / 2;
    float cellSize = side / 9;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (gridDigits[i][j] == 0) continue; // Пропускаем, если нет цифры
            float x = -half_side + cellSize / 2 + j * cellSize - cellSize / 4; // Координата X клетки
            float y = half_side - cellSize / 2 - i * cellSize - cellSize / 4; // Координата Y клетки
            float segSize = 0.04f; // Размер сегмента цифры

            glLineWidth(2.2f); // Толщина линий для цифр
            glBegin(GL_LINES);


            if (input_Digits[i][j].color == BLUE) {
                glColor3f(0.0f, 0.0f, 1.0f); // Синий для введённых пользователем
            }
            else if (input_Digits[i][j].color == RED) {
                glColor3f(1.0f, 0.0f, 0.0f); // Красный - если пользователь вставил цифру не по правилам
            }
            else if (input_Digits[i][j].color == PINK) {
                glColor3f(1.0f, 0.0f, 1.0f); // Розовый - для цифр, вставленных роботом
            }
            else {
                glColor3f(0.0f, 0.0f, 0.0f); // Чёрный для сгенерированных
            }


            switch (gridDigits[i][j]) {
            case 1:
                // Вертикальная черта справа
                glVertex2f(x + segSize, y);
                glVertex2f(x + segSize, y + segSize * 2);
                break;
            case 2:
                // Верхняя горизонтальная
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x + segSize * 2, y + segSize * 2);
                // Средняя горизонтальная
                glVertex2f(x, y + segSize);
                glVertex2f(x + segSize * 2, y + segSize);
                // Нижняя горизонтальная
                glVertex2f(x, y);
                glVertex2f(x + segSize * 2, y);
                // Вертикальная слева снизу
                glVertex2f(x, y);
                glVertex2f(x, y + segSize);
                // Вертикальная справа сверху
                glVertex2f(x + segSize * 2, y + segSize * 2);
                glVertex2f(x + segSize * 2, y + segSize);
                break;
            case 3:
                // Верхняя горизонтальная
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x + segSize * 2, y + segSize * 2);
                // Средняя горизонтальная
                glVertex2f(x, y + segSize);
                glVertex2f(x + segSize * 2, y + segSize);
                // Нижняя горизонтальная
                glVertex2f(x, y);
                glVertex2f(x + segSize * 2, y);
                // Вертикальная справа
                glVertex2f(x + segSize * 2, y + segSize * 2);
                glVertex2f(x + segSize * 2, y);
                break;
            case 4:
                // Верхняя вертикальная слева
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x, y + segSize);
                // Средняя вертикальная справа
                glVertex2f(x + segSize * 2, y + segSize * 2);
                glVertex2f(x + segSize * 2, y);
                // Средняя горизонтальная
                glVertex2f(x, y + segSize);
                glVertex2f(x + segSize * 2, y + segSize);
                break;
            case 5:
                // Верхняя горизонтальная
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x + segSize * 2, y + segSize * 2);
                // Средняя горизонтальная
                glVertex2f(x, y + segSize);
                glVertex2f(x + segSize * 2, y + segSize);
                // Нижняя горизонтальная
                glVertex2f(x, y);
                glVertex2f(x + segSize * 2, y);
                // Вертикальная слева сверху
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x, y + segSize);
                // Вертикальная справа снизу
                glVertex2f(x + segSize * 2, y + segSize);
                glVertex2f(x + segSize * 2, y);
                break;
            case 6:
                // Верхняя горизонтальная
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x + segSize * 2, y + segSize * 2);
                // Средняя горизонтальная
                glVertex2f(x, y + segSize);
                glVertex2f(x + segSize * 2, y + segSize);
                // Нижняя горизонтальная
                glVertex2f(x, y);
                glVertex2f(x + segSize * 2, y);
                // Вертикальная слева
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x, y);
                // Вертикальная справа снизу
                glVertex2f(x + segSize * 2, y + segSize);
                glVertex2f(x + segSize * 2, y);
                break;
            case 7:
                // Верхняя горизонтальная
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x + segSize * 2, y + segSize * 2);
                // Вертикальная справа
                glVertex2f(x + segSize * 2, y + segSize * 2);
                glVertex2f(x + segSize * 2, y);
                break;
            case 8:
                // Верхняя горизонтальная
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x + segSize * 2, y + segSize * 2);
                // Средняя горизонтальная
                glVertex2f(x, y + segSize);
                glVertex2f(x + segSize * 2, y + segSize);
                // Нижняя горизонтальная
                glVertex2f(x, y);
                glVertex2f(x + segSize * 2, y);
                // Вертикальная слева
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x, y);
                // Вертикальная справа
                glVertex2f(x + segSize * 2, y + segSize * 2);
                glVertex2f(x + segSize * 2, y);
                break;
            case 9:
                // Верхняя горизонтальная
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x + segSize * 2, y + segSize * 2);
                // Средняя горизонтальная
                glVertex2f(x, y + segSize);
                glVertex2f(x + segSize * 2, y + segSize);
                // Нижняя горизонтальная
                glVertex2f(x, y);
                glVertex2f(x + segSize * 2, y);
                // Вертикальная слева сверху
                glVertex2f(x, y + segSize * 2);
                glVertex2f(x, y + segSize);
                // Вертикальная справа
                glVertex2f(x + segSize * 2, y + segSize * 2);
                glVertex2f(x + segSize * 2, y);
                break;
            }
            glEnd();
        }
    }
}

// Отрисовка текста 
void drawText(float x, float y, const char* text, float scale) {
    glLineWidth(2.0f); // Толщина линий для текста
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f); // Чёрный цвет

    float charWidth = 0.06f * scale; // Ширина символа
    float charHeight = 0.08f * scale; // Высота символа
    float charSpacing = 0.02f * scale; // Отступ между символами
    float startX = x;

    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        float px = startX + i * (charWidth + charSpacing); // Учитываем отступ
        float py = y;

        switch (c) {
        case 'A':
            // Верхняя горизонтальная
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight);
            // Средняя горизонтальная
            glVertex2f(px, py + charHeight / 2);
            glVertex2f(px + charWidth, py + charHeight / 2);
            // Вертикальная слева
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Вертикальная справа
            glVertex2f(px + charWidth, py + charHeight);
            glVertex2f(px + charWidth, py);
            break;
        case 'D':
            // Вертикальная слева
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Верхняя горизонтальная
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth / 2, py + charHeight);
            // Нижняя горизонтальная
            glVertex2f(px, py);
            glVertex2f(px + charWidth / 2, py);
            // Диагональ справа верхняя
            glVertex2f(px + charWidth / 2, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight / 2);
            // Диагональ справа нижняя
            glVertex2f(px + charWidth, py + charHeight / 2);
            glVertex2f(px + charWidth / 2, py);
            break;
        case 'E':
            // Вертикальная слева
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Верхняя горизонтальная
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight);
            // Средняя горизонтальная
            glVertex2f(px, py + charHeight / 2);
            glVertex2f(px + charWidth, py + charHeight / 2);
            // Нижняя горизонтальная
            glVertex2f(px, py);
            glVertex2f(px + charWidth, py);
            break;
        case 'G':
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight);
            // Нижняя горизонтальная
            glVertex2f(px, py);
            glVertex2f(px + charWidth, py);
            // Вертикальная слева
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Вертикальная справа снизу
            glVertex2f(px + charWidth, py + charHeight / 2);
            glVertex2f(px + charWidth, py);
            // Средняя горизонтальная справа
            glVertex2f(px + charWidth / 2, py + charHeight / 2);
            glVertex2f(px + charWidth, py + charHeight / 2);
            break;
        case 'H':
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Вертикальная справа
            glVertex2f(px + charWidth, py + charHeight);
            glVertex2f(px + charWidth, py);
            // Средняя горизонтальная
            glVertex2f(px, py + charHeight / 2);
            glVertex2f(px + charWidth, py + charHeight / 2);
            break;
        case 'I':
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight);
            // Нижняя горизонтальная
            glVertex2f(px, py);
            glVertex2f(px + charWidth, py);
            // Вертикальная середина
            glVertex2f(px + charWidth / 2, py + charHeight);
            glVertex2f(px + charWidth / 2, py);
            break;
        case 'L':
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Нижняя горизонтальная
            glVertex2f(px, py);
            glVertex2f(px + charWidth, py);
            break;
        case 'M':
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Вертикальная справа
            glVertex2f(px + charWidth, py + charHeight);
            glVertex2f(px + charWidth, py);
            // Диагонали
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth / 2, py + charHeight / 2);
            glVertex2f(px + charWidth / 2, py + charHeight / 2);
            glVertex2f(px + charWidth, py + charHeight);
            break;
        case 'N':
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Вертикальная справа
            glVertex2f(px + charWidth, py + charHeight);
            glVertex2f(px + charWidth, py);
            // Диагональ
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth, py);
            break;
        case 'O':
            // Верхняя горизонтальная
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight);
            // Нижняя горизонтальная
            glVertex2f(px, py);
            glVertex2f(px + charWidth, py);
            // Вертикальная слева
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Вертикальная справа
            glVertex2f(px + charWidth, py + charHeight);
            glVertex2f(px + charWidth, py);
            break;
        case 'R':
            // Вертикальная слева
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py);
            // Верхняя горизонтальная
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight);
            // Средняя горизонтальная
            glVertex2f(px, py + charHeight / 2);
            glVertex2f(px + charWidth, py + charHeight / 2);
            // Вертикальная справа сверху
            glVertex2f(px + charWidth, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight / 2);
            // Диагональ снизу
            glVertex2f(px + charWidth / 2, py + charHeight / 2);
            glVertex2f(px + charWidth, py);
            break;
        case 'S':
            // Верхняя горизонтальная
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth, py + charHeight);
            // Средняя горизонтальная
            glVertex2f(px, py + charHeight / 2);
            glVertex2f(px + charWidth, py + charHeight / 2);
            // Нижняя горизонтальная
            glVertex2f(px, py);
            glVertex2f(px + charWidth, py);
            // Вертикальная слева сверху
            glVertex2f(px, py + charHeight);
            glVertex2f(px, py + charHeight / 2);
            // Вертикальная справа снизу
            glVertex2f(px + charWidth, py + charHeight / 2);
            glVertex2f(px + charWidth, py);
            break;
        case 'Y':
            // Вертикальная середина снизу
            glVertex2f(px + charWidth / 2, py + charHeight / 2);
            glVertex2f(px + charWidth / 2, py);
            // Диагонали сверху
            glVertex2f(px, py + charHeight);
            glVertex2f(px + charWidth / 2, py + charHeight / 2);
            glVertex2f(px + charWidth, py + charHeight);
            glVertex2f(px + charWidth / 2, py + charHeight / 2);
            break;
        }
    }
    glEnd();
}

void saveGame() {
    FILE* file = NULL;
    if (fopen_s(&file, "save.txt", "w") != 0) {
        printf("Failed to save game\n");
        return;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(file, "%d ", gridDigits[i][j]);
        }
        fprintf(file, "\n");
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(file, "%d ", SolutionSudoku[i][j]);
        }
        fprintf(file, "\n");
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(file, "%d ", input_Digits[i][j].color);
        }
        fprintf(file, "\n");
    }
    fprintf(file, "%d\n", CountgridDigits);
    fclose(file);
}

void loadGame() {
    FILE* file = NULL;
    if (fopen_s(&file, "save.txt", "r") != 0) {
        printf("No saved game found\n");
        return;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fscanf_s(file, "%d ", &gridDigits[i][j]);
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fscanf_s(file, "%d ", &SolutionSudoku[i][j]);
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fscanf_s(file, "%d ", &input_Digits[i][j]);
        }
    }
    fscanf_s(file, "%d", &CountgridDigits);
    fclose(file);
}

void SavedSudoku() {

    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_SAVEDSUDOKU]);
    drawTexture(-1.0f, 1.0f, -1.0f, 1.0f);

    float left = -0.7f, right = 0.7f, bottom = 0.8f, top = 1.0f;

    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 0.7f);
    if (save1Button.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    glVertex2f(left, top);
    glVertex2f(right, top);
    glVertex2f(right, bottom);
    glVertex2f(left, bottom);
    glEnd();

    float charWidth = 0.06f;
    float charSpacing = 0.04f;
    int textLength = 4;
    float totalTextWidth = textLength * charWidth + (textLength - 1) * charSpacing;
    float textX = save1Button.x + (save1Button.width - totalTextWidth) / 2;
    float textY = save1Button.y - (save1Button.height) / 2 - 0.04;
    drawText(textX, textY, save1Button.label, 1.0f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_BACK_BUT]);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (backButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-1.0f, -0.8f, 0.8f, 1.0f);
}


void drawButtonEasy(ButEasy* but) {
    glBegin(GL_QUADS);
    if (but->hovered) {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    else {
        glColor3f(0.5f, 1.0f, 0.0f);
    }
    glVertex2f(but->x, but->y);
    glVertex2f(but->x + but->width, but->y);
    glVertex2f(but->x + but->width, but->y - but->height);
    glVertex2f(but->x, but->y - but->height);
    glEnd();

    if (ButtonEasyState == 1) {
        glLineWidth(5.0f);
        glBegin(GL_LINE_LOOP);
        glColor3f(1.0f, 0.0f, 0.0f);

        glVertex2f(but->x, but->y);
        glVertex2f(but->x + but->width, but->y);
        glVertex2f(but->x + but->width, but->y - but->height);
        glVertex2f(but->x, but->y - but->height);
        glEnd();
    }

    float charWidth = 0.06f;
    float charSpacing = 0.04f;
    int textLength = 4;
    float totalTextWidth = textLength * charWidth + (textLength - 1) * charSpacing;
    float textX = but->x + (but->width - totalTextWidth) / 2;
    float textY = but->y - (but->height) / 2 - 0.04;
    drawText(textX, textY, but->label, 1.0f);
}

void drawButtonNormal(ButNormal* but) {
    glBegin(GL_QUADS);
    if (but->hovered) {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    else {
        glColor3f(1.0f, 1.0f, 0.0f);
    }
    glVertex2f(but->x, but->y);
    glVertex2f(but->x + but->width, but->y);
    glVertex2f(but->x + but->width, but->y - but->height);
    glVertex2f(but->x, but->y - but->height);
    glEnd();

    if (ButtonNormalState == 1) {
        glLineWidth(5.0f);
        glBegin(GL_LINE_LOOP);
        glColor3f(1.0f, 0.0f, 0.0f);

        glVertex2f(but->x, but->y);
        glVertex2f(but->x + but->width, but->y);
        glVertex2f(but->x + but->width, but->y - but->height);
        glVertex2f(but->x, but->y - but->height);
        glEnd();
    }

    float charWidth = 0.06f;
    float charSpacing = 0.04f;
    int textLength = 6;
    float totalTextWidth = textLength * charWidth + (textLength - 1) * charSpacing;
    float textX = but->x + (but->width - totalTextWidth) / 2;
    float textY = but->y - (but->height) / 2 - 0.04;
    drawText(textX, textY, but->label, 1.0f);
}

void drawButtonHard(ButHard* but) {
    glBegin(GL_QUADS);
    if (but->hovered) {
        glColor3f(0.7f, 0.7f, 0.7f);
    }
    else {
        glColor3f(1.0f, 0.5f, 0.0f);
    }
    glVertex2f(but->x, but->y);
    glVertex2f(but->x + but->width, but->y);
    glVertex2f(but->x + but->width, but->y - but->height);
    glVertex2f(but->x, but->y - but->height);
    glEnd();

    if (ButtonHardState == 1) {
        glLineWidth(5.0f);
        glBegin(GL_LINE_LOOP);
        glColor3f(1.0f, 0.0f, 0.0f);

        glVertex2f(but->x, but->y);
        glVertex2f(but->x + but->width, but->y);
        glVertex2f(but->x + but->width, but->y - but->height);
        glVertex2f(but->x, but->y - but->height);
        glEnd();
    }

    float charWidth = 0.06f;
    float charSpacing = 0.04f;
    int textLength = 4;
    float totalTextWidth = textLength * charWidth + (textLength - 1) * charSpacing;
    float textX = but->x + (but->width - totalTextWidth) / 2;
    float textY = but->y - (but->height) / 2 - 0.04;
    drawText(textX, textY, but->label, 1.0f);
}


// Функция для отрисовки выбранной клетки
void drawSelectedCell() {
    float half_side = side / 2;
    if (selectedCellX >= -half_side && selectedCellX <= half_side && selectedCellY >= -half_side && selectedCellY <= half_side) {
        glBegin(GL_QUADS);
        glColor4f(0.0f, 1.0f, 0.5f, 0.5f); // цвет для выбранной клетки
        float Cell = side / 18.35;
        glVertex2f(selectedCellX - Cell, selectedCellY - Cell);
        glVertex2f(selectedCellX + Cell, selectedCellY - Cell);
        glVertex2f(selectedCellX + Cell, selectedCellY + Cell);
        glVertex2f(selectedCellX - Cell, selectedCellY + Cell);
        glEnd();
    }
}

void Init_textures() {

    int width[CNT_TEXT], height[CNT_TEXT], channels[CNT_TEXT];
    const unsigned char* ImageNames[CNT_TEXT] = { "WICTORY.png", "HOME.png",
        "MENU.png", "GAME.png", "SETTINGS.png", "Play.png", "Settings_b.png", "Saved sudoku.png",
        "Computer mode.png", "Exit.png", "Alg.png", "Backtracking.png", "Logical.png", "Difficulty.png",
        "Back.png", "ComputerMode.png", "Create.png", "Solve.png", "Decide.png", "Save.png", "SavedSudoku.png"};
    unsigned char* image;

    glGenTextures(CNT_TEXT, Textures);

    for (int i = 0; i < CNT_TEXT; i++) {

        glBindTexture(GL_TEXTURE_2D, Textures[i]);

        image = stbi_load(ImageNames[i], &width[i], &height[i], &channels[i], 0);
        if (!image) {
            puts("image load fail");
            return;
        }
        if (channels[i] == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width[i], height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels[i] == 4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width[i], height[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        stbi_image_free(image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
}

void drawDecideBut() {
    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_DECIDE_BUT]);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (decideButton.hovered == 1) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.3f, 0.3f, 0.8f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void Menu() {
    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_MENU]);
    glColor3f(1.0f, 0.0f, 1.0f);
    drawTexture(-1.0f, 1.0f, -1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_PLAY_BUT]);
    glColor3f(0.0f, 1.0f, 1.0f);
    if (playButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.4f, 0.4f, 0.3f, 0.5f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_SETTINGS_BUT]);
    glColor3f(1.0f, 0.0f, 1.0f);
    if (setButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.4f, 0.4f, 0.0f, 0.2f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_SAVED_SUDOKU_BUT]);
    glColor3f(0.0f, 1.0f, 0.0f);
    if (savedButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.4f, 0.4f, -0.3f, -0.1f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_COMUTER_MODE_BUT]);
    glColor3f(0.0f, 0.0f, 1.0f);
    if (compButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.4f, 0.4f, -0.6f, -0.4f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_EXIT_BUT]);
    glColor3f(1.0f, 0.0f, 0.0f);
    if (exitButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.4f, 0.4f, -0.9f, -0.7f);
}

void Settings() {
    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_SETTINGS]);
    glColor3f(0.8f, 0.0f, 1.0f);
    drawTexture(-1.0f, 1.0f, -1.0f, 1.0f);


    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_ALG]);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawTexture(-0.35f, 0.35f, 0.85f, 0.99f);


    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_BACK_TRACK_BUT]);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (backtrackButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }

    drawTexture(-0.7f, -0.1f, 0.6f, 0.77f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_LOGICAL_BUT]);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (logicButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }

    drawTexture(0.1f, 0.7f, 0.6f, 0.77f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_DIFFICULTY]);
    glColor3f(1.0f, 1.0f, 1.0f);

    drawTexture(-0.6f, 0.6f, 0.02f, 0.2f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_BACK_BUT]);
    glColor3f(1.0f, 1.0f, 1.0f);

    if (backButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-1.0f, -0.8f, 0.8f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void Comp_mode() {
    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_COMP_MODE]);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawTexture(-1.0f, 1.0f, -1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_CREATE_BUT]);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (createButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.5f, 0.5f, 0.0f, 0.3f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_SOLVE_BUT]);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (solveButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.53f, 0.48f, -0.4f, -0.1f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_BACK_BUT]);

    glColor3f(1.0f, 1.0f, 1.0f);
    if (backButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-1.0f, -0.8f, 0.8f, 1.0f);
}

void Wictory() {
    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_WICTORY]);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawTexture(-1.0f, 1.0f, -1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_HOME]);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (homeButton.hovered == 1) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.4f, 0.4f, -0.8f, -0.5f);
}

void addRecord(double time) {

    LeaderboardEntry* newEntry = (LeaderboardEntry*)malloc(sizeof(LeaderboardEntry));
    if (newEntry == NULL) {
        return; 
    }

    newEntry->time = time;
    newEntry->next = NULL;

    if (head == NULL || time < head->time) {
        newEntry->next = head;
        head = newEntry;
    }
    else {
        LeaderboardEntry* current = head;
        while (current->next != NULL && current->next->time < time) {
            current = current->next;
        }
        newEntry->next = current->next;
        current->next = newEntry;
    }

    int count = 0;
    LeaderboardEntry* current = head;
    while (current != NULL && count < 9) {
        count++;
        current = current->next;
    }
    if (current != NULL && current->next != NULL) {
        LeaderboardEntry* toFree = current->next;
        current->next = NULL;
        while (toFree != NULL) {
            LeaderboardEntry* temp = toFree;
            toFree = toFree->next;
            free(temp);
        }
    }
}

void loadLeaderboard() {
    FILE* file = fopen("leaderboard.txt", "r");
    if (file) {
        double time;
        while (fscanf(file, "%lf", &time) == 1) {
            addRecord(time);
        }
        fclose(file);
    }
}

void saveLeaderboard() {
    FILE* file = fopen("leaderboard.txt", "w");

    if (file) {
        LeaderboardEntry* current = head;
        while (current != NULL) {
            fprintf(file, "%.2f\n", current->time);
            current = current->next;
        }
        fclose(file);
    }
}


void handleGameEnd() {
    stopTimer();
    double gameTime = getElapsedTime();
    printf("Game completed in %.2f seconds\n", gameTime);

    int recordCount = 0;
    LeaderboardEntry* current = head;
    while (current != NULL && recordCount < 10) {
        recordCount++;
        current = current->next;
    }

    if (head == NULL || gameTime < head->time || recordCount < 10) {
        addRecord(gameTime);
        saveLeaderboard();
        printf("Record saved with time %.2f\n", gameTime);
    }
}

void displayLeaderboard() {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    int rank = 1;
    LeaderboardEntry* current = head;
    float y = 0.8f;

    while (current != NULL && rank <= 10) {
        char buffer[100];
        snprintf(buffer, 100, "%d. %.2f sec", rank, current->time);
        drawText(-0.5f, y, buffer, 0.5f);
        y -= 0.1f;
        current = current->next;
        rank++;
    }

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_BACK_BUT]);

    if (backButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }

    drawTexture(-1.0f, -0.8f, 0.8f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void freeLeaderboard() {
    while (head != NULL) {
        LeaderboardEntry* temp = head;
        if (temp == NULL) break;
        head = temp->next;
        free(temp);
    }
    head = NULL; 
}

void Game() {
    if (!gameActive) startTimer();

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_GAME]);
    glColor3f(1.0f, 0.9f, 0.7f);
    drawTexture(-1.0f, 1.0f, -1.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_BACK_BUT]);
    if (backButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-1.0f, -0.8f, 0.8f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_SAVE_BUT]);
    if (saveButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-0.3f, 0.3f, 0.8f, 1.0f);

    drawField();
    drawSelectedCell();
    drawDigit();
}

void Game1() {
    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_GAME]);
    glColor3f(1.0f, 0.9f, 0.7f);
    drawTexture(-1.0f, 1.0f, -1.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, Textures[TEXT_BACK_BUT]);

    if (backButton.hovered) {
        glColor3f(0.5f, 0.5f, 0.5f);
    }
    drawTexture(-1.0f, -0.8f, 0.8f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    drawDecideBut();
}

void Create() {
    drawDecideBut();
}

void drawFrame() {

    float left, right, bottom, top;

    left = backtrackButton.x, right = backtrackButton.x + backtrackButton.width,
        bottom = backtrackButton.y - backtrackButton.height, top = backtrackButton.y;

    if (ALG_State) {
        left = logicButton.x, right = logicButton.x + logicButton.width,
            bottom = logicButton.y - logicButton.height, top = logicButton.y;
    }

    glLineWidth(5.0f);
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0f, 0.0f, 0.0f);

    glVertex2f(left, top);
    glVertex2f(right, top);
    glVertex2f(right, bottom);
    glVertex2f(left, bottom);
    glEnd();
}

// Функция для проверки наведения на кнопку
void checkButtonHover(float mouseX, float mouseY) {
    // Сбрасываем состояние hovered для всех кнопок
    playButton.hovered = 0;
    setButton.hovered = 0;
    savedButton.hovered = 0;
    compButton.hovered = 0;
    exitButton.hovered = 0;
    easyButton.hovered = 0;
    normalButton.hovered = 0;
    hardButton.hovered = 0;
    backtrackButton.hovered = 0;
    logicButton.hovered = 0;
    backButton.hovered = 0;
    createButton.hovered = 0;
    solveButton.hovered = 0;
    decideButton.hovered = 0;
    saveButton.hovered = 0;
    homeButton.hovered = 0;
    save1Button.hovered = 0;

    if (gameState == MENU) {
        if (mouseX >= playButton.x && mouseX <= playButton.x + playButton.width &&
            mouseY >= playButton.y - playButton.height && mouseY <= playButton.y) {
            playButton.hovered = 1;
        }
        if (mouseX >= setButton.x && mouseX <= setButton.x + setButton.width &&
            mouseY >= setButton.y - setButton.height && mouseY <= setButton.y) {
            setButton.hovered = 1;
        }
        if (mouseX >= savedButton.x && mouseX <= savedButton.x + savedButton.width &&
            mouseY >= savedButton.y - savedButton.height && mouseY <= savedButton.y) {
            savedButton.hovered = 1;
        }
        if (mouseX >= compButton.x && mouseX <= compButton.x + compButton.width &&
            mouseY >= compButton.y - compButton.height && mouseY <= compButton.y) {
            compButton.hovered = 1;
        }
        if (mouseX >= exitButton.x && mouseX <= exitButton.x + exitButton.width &&
            mouseY >= exitButton.y - exitButton.height && mouseY <= exitButton.y) {
            exitButton.hovered = 1;
        }
    }

    else if (gameState == SETTINGS) {
        if (mouseX >= easyButton.x && mouseX <= easyButton.x + easyButton.width &&
            mouseY >= easyButton.y - easyButton.height && mouseY <= easyButton.y) {
            easyButton.hovered = 1;
        }
        if (mouseX >= normalButton.x && mouseX <= normalButton.x + normalButton.width &&
            mouseY >= normalButton.y - normalButton.height && mouseY <= normalButton.y) {
            normalButton.hovered = 1;
        }
        if (mouseX >= hardButton.x && mouseX <= hardButton.x + hardButton.width &&
            mouseY >= hardButton.y - hardButton.height && mouseY <= hardButton.y) {
            hardButton.hovered = 1;
        }
        if (mouseX >= backtrackButton.x && mouseX <= backtrackButton.x + backtrackButton.width &&
            mouseY >= backtrackButton.y - backButton.height && mouseY <= backtrackButton.y) {
            backtrackButton.hovered = 1;
        }
        if (mouseX >= logicButton.x && mouseX <= logicButton.x + logicButton.width &&
            mouseY >= logicButton.y - logicButton.height && mouseY <= logicButton.y) {
            logicButton.hovered = 1;
        }
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            backButton.hovered = 1;
        }
    }

    else if (gameState == GAME) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            backButton.hovered = 1;
        }
        if (mouseX >= saveButton.x && mouseX <= saveButton.x + saveButton.width &&
            mouseY >= saveButton.y - saveButton.height && mouseY <= saveButton.y) {
            saveButton.hovered = 1;
        }
    }

    else if (gameState == GAME1) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            backButton.hovered = 1;
        }
        if (mouseX >= decideButton.x && mouseX <= decideButton.x + decideButton.width &&
            mouseY >= decideButton.y - decideButton.height && mouseY <= decideButton.y) {
            decideButton.hovered = 1;
        }
    }

    else if (gameState == COMP_MODE) {
        if (mouseX >= createButton.x && mouseX <= createButton.x + createButton.width &&
            mouseY >= createButton.y - createButton.height && mouseY <= createButton.y) {
            createButton.hovered = 1;
        }
        if (mouseX >= solveButton.x && mouseX <= solveButton.x + solveButton.width &&
            mouseY >= solveButton.y - solveButton.height && mouseY <= solveButton.y) {
            solveButton.hovered = 1;
        }
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            backButton.hovered = 1;
        }
    }

    else if (gameState == CREATE) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            backButton.hovered = 1;
        }
        if (mouseX >= decideButton.x && mouseX <= decideButton.x + decideButton.width &&
            mouseY >= decideButton.y - decideButton.height && mouseY <= decideButton.y) {
            decideButton.hovered = 1;
        }
    }

    else if (gameState == YOU_WIN) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            backButton.hovered = 1;
        }
        if (mouseX >= homeButton.x && mouseX <= homeButton.x + homeButton.width &&
            mouseY >= homeButton.y - homeButton.height && mouseY <= homeButton.y) {
            homeButton.hovered = 1;
        }
    }

    else if (gameState == YOU_WIN1) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            backButton.hovered = 1;
        }
        if (mouseX >= homeButton.x && mouseX <= homeButton.x + homeButton.width &&
            mouseY >= homeButton.y - homeButton.height && mouseY <= homeButton.y) {
            homeButton.hovered = 1;
        }
    }

    else if (gameState == SAVED_SUDOKU) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            backButton.hovered = 1;
        }
        if (mouseX >= save1Button.x && mouseX <= save1Button.x + save1Button.width &&
            mouseY >= save1Button.y - save1Button.height && mouseY <= save1Button.y) {
            save1Button.hovered = 1;
        }
    }
}

void SelectCell(float mouseX, float mouseY) {
    float half_side = side / 2;
    float cellSize = side / 9;
    if (mouseX >= -half_side && mouseX <= half_side &&
        mouseY >= -half_side && mouseY <= half_side) {
        int cellX = (int)((mouseX + half_side) / cellSize);
        int cellY = (int)((mouseY + half_side) / cellSize);
        selectedCellX = -half_side + cellSize * cellX + cellSize / 2;
        selectedCellY = -half_side + cellSize * cellY + cellSize / 2;
    }
}

void Leaderboard() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    displayLeaderboard();
}

// Функция для обработки клика по кнопке
void handleButtonClick(float mouseX, float mouseY) {
    if (gameState == MENU) {
        if (mouseX >= playButton.x && mouseX <= playButton.x + playButton.width &&
            mouseY >= playButton.y - playButton.height && mouseY <= playButton.y) {
            // Переход к игровому полю
            gameState = GAME;
            // Сбрасывается поле и генерируется новое с учётом текущей сложности
            Del_gridDidits();
            Fill_diagonal();
            Generate_field(0);
            int cells_to_remove = 15; // По умолчанию EASY
            if (ButtonNormalState == 1) cells_to_remove = 22;
            else if (ButtonHardState == 1) cells_to_remove = 30;
            CountgridDigits = N * N - cells_to_remove;
            Build_level(cells_to_remove);
        }
        else if (mouseX >= setButton.x && mouseX <= setButton.x + setButton.width &&
            mouseY >= setButton.y - setButton.height && mouseY <= setButton.y) {
            gameState = SETTINGS; // Переход к настройкам
        }
        else if (mouseX >= savedButton.x && mouseX <= savedButton.x + savedButton.width &&
            mouseY >= savedButton.y - savedButton.height && mouseY <= savedButton.y) {
            gameState = SAVED_SUDOKU;
        }
        else if (mouseX >= compButton.x && mouseX <= compButton.x + compButton.width &&
            mouseY >= compButton.y - compButton.height && mouseY <= compButton.y) {
            gameState = COMP_MODE;
        }
        else if (mouseX >= exitButton.x && mouseX <= exitButton.x + exitButton.width &&
            mouseY >= exitButton.y - exitButton.height && mouseY <= exitButton.y) {
            gameState = EXIT;
        }
    }

    else if (gameState == GAME) {
        // Обработка клика по кнопке BACK
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            gameState = MENU; // Возврат в меню
        }
        if (mouseX >= saveButton.x && mouseX <= saveButton.x + saveButton.width &&
            mouseY >= saveButton.y - saveButton.height && mouseY <= saveButton.y) {
            saveGame();
        }
        // Обработка клика для выбора клетки
        SelectCell(mouseX, mouseY);
    }

    else if (gameState == GAME1) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            gameState = MENU;
        }
        else if (mouseX >= decideButton.x && mouseX <= decideButton.x + decideButton.width &&
            mouseY >= decideButton.y - decideButton.height && mouseY <= decideButton.y) {
            if (ALG_State == 1) {
                if (!check_wrong_num() && CountgridDigits >= N * N - 30)
                    solve_logical();
            }
            else {
                if (!check_wrong_num()) {
                    initOptions();
                    solveBacktracking();
                }
            }
        }

        SelectCell(mouseX, mouseY);
    }

    else if (gameState == SETTINGS) {
        int cells_to_remove = 0; // Инициализация переменной
        bool levelChanged = false; // Флаг для отслеживания изменения уровня
        if (mouseX >= easyButton.x && mouseX <= easyButton.x + easyButton.width &&
            mouseY >= easyButton.y - easyButton.height && mouseY <= easyButton.y) {
            if (ButtonEasyState != 1) { // Проверяем, изменился ли уровень
                ButtonEasyState = 1;
                ButtonNormalState = 0;
                ButtonHardState = 0;
                cells_to_remove = 15;
                CountgridDigits = N * N - cells_to_remove;
                levelChanged = true;
                drawButtonEasy(&easyButton);
            }
        }
        else if (mouseX >= normalButton.x && mouseX <= normalButton.x + normalButton.width &&
            mouseY >= normalButton.y - normalButton.height && mouseY <= normalButton.y) {
            if (ButtonNormalState != 1) {
                ButtonNormalState = 1;
                ButtonEasyState = 0;
                ButtonHardState = 0;
                cells_to_remove = 22;
                CountgridDigits = N * N - cells_to_remove;
                levelChanged = true;
                drawButtonNormal(&normalButton);
            }
        }
        else if (mouseX >= hardButton.x && mouseX <= hardButton.x + hardButton.width &&
            mouseY >= hardButton.y - hardButton.height && mouseY <= hardButton.y) {
            if (ButtonHardState != 1) {
                ButtonHardState = 1;
                ButtonEasyState = 0;
                ButtonNormalState = 0;
                cells_to_remove = 30;
                CountgridDigits = N * N - cells_to_remove;
                levelChanged = true;
                drawButtonHard(&hardButton);
            }
        }
        else if (mouseX >= backtrackButton.x && mouseX <= backtrackButton.x + backtrackButton.width &&
            mouseY >= backtrackButton.y - backtrackButton.height && mouseY <= backtrackButton.y) {
            ALG_State = 0;
        }
        else if (mouseX >= logicButton.x && mouseX <= logicButton.x + logicButton.width &&
            mouseY >= logicButton.y - logicButton.height && mouseY <= logicButton.y) {
            ALG_State = 1;
        }
        else if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            gameState = MENU;
        }
        // Если уровень сложности изменился, генерируем новое поле
        if (levelChanged) {
            Del_gridDidits();
            Fill_diagonal();
            Generate_field(0);
            Build_level(cells_to_remove);
        }
    }

    else if (gameState == YOU_WIN) {
        if (mouseX >= homeButton.x && mouseX <= homeButton.x + homeButton.width &&
            mouseY >= homeButton.y - homeButton.height && mouseY <= homeButton.y) {
            gameState = MENU;
        }
        else if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            gameState = GAME;
        }
    }

    else if (gameState == YOU_WIN1) {
        if (mouseX >= homeButton.x && mouseX <= homeButton.x + homeButton.width &&
            mouseY >= homeButton.y - homeButton.height && mouseY <= homeButton.y) {
            gameState = MENU;
        }
        else if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            gameState = GAME1;
        }
    }

    else if (gameState == COMP_MODE) {
        if (mouseX >= createButton.x && mouseX <= createButton.x + createButton.width &&
            mouseY >= createButton.y - createButton.height && mouseY <= createButton.y) {
            Del_gridDidits();
            CountgridDigits = 0;
            gameState = CREATE;
        }
        else if (mouseX >= solveButton.x && mouseX <= solveButton.x + solveButton.width &&
            mouseY >= solveButton.y - solveButton.height && mouseY <= solveButton.y) {
            gameState = SAVED_SUDOKU;
        }
        else if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            gameState = MENU;
        }
    }

    else if (gameState == CREATE) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            gameState = COMP_MODE;
        }
        else if (mouseX >= decideButton.x && mouseX <= decideButton.x + decideButton.width &&
            mouseY >= decideButton.y - decideButton.height && mouseY <= decideButton.y) {

            if (ALG_State == 1) {
                if (!check_wrong_num() && CountgridDigits >= N * N - 30)
                    solve_logical();
            }
            else {
                if (!check_wrong_num()) {
                    initOptions();
                    solveBacktracking();
                }
            }
        }

        SelectCell(mouseX, mouseY);
    }

    else if (gameState == SAVED_SUDOKU) {
        if (mouseX >= backButton.x && mouseX <= backButton.x + backButton.width &&
            mouseY >= backButton.y - backButton.height && mouseY <= backButton.y) {
            gameState = MENU;
        }
        if (mouseX >= save1Button.x && mouseX <= save1Button.x + save1Button.width &&
            mouseY >= save1Button.y - save1Button.height && mouseY <= save1Button.y) {
            loadGame();
            glColor3f(1.0f, 1.0f, 1.0f);
            gameState = GAME1;
        }
    }
}

// Callback для обработки движения мыши
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    float mouseX = (float)((2.0 * xpos / width) - 1.0);
    float mouseY = (float)(1.0 - (2.0 * ypos / height));

    checkButtonHover(mouseX, mouseY);
}

// Callback для обработки кликов мыши
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float mouseX = (float)((2.0 * xpos / width) - 1.0);
        float mouseY = (float)(1.0 - (2.0 * ypos / height));

        handleButtonClick(mouseX, mouseY);
    }
}

// Callback для обработки клавиш
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float cellSize = side / 9;
        float half_side = side / 2;
        float newX = selectedCellX;
        float newY = selectedCellY;

        // Начальное значение, если ещё не выбрана клетка
        if (selectedCellX == -2.0f && selectedCellY == -2.0f) {
            newX = 0.0f;
            newY = 0.0f;
        }

        // Сохранение текущих координат для проверки
        float prevX = newX;
        float prevY = newY;

        // Перемещение по стрелкам
        if (key == GLFW_KEY_UP) newY += cellSize;
        else if (key == GLFW_KEY_DOWN) newY -= cellSize;
        else if (key == GLFW_KEY_LEFT) newX -= cellSize;
        else if (key == GLFW_KEY_RIGHT) newX += cellSize;

        // Проверка выхода за пределы поля
        if (newX < -half_side || newX > half_side) newX = prevX; // Игнор, если за границей по X
        if (newY < -half_side || newY > half_side) newY = prevY; // Игнор, если за границей по Y

        // Обновление координат, если они изменились
        if (newX != selectedCellX || newY != selectedCellY) {
            selectedCellX = newX;
            selectedCellY = newY;
        }


        int j = (selectedCellX + half_side) / cellSize; // Индекс по X (0-8)
        int i = (half_side - selectedCellY) / cellSize; // Индекс по Y (0-8)
        // Установка цифры при нажатии 1-9
        if (gameState == GAME || gameState == GAME1) {
            if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9 && action == GLFW_PRESS) {

                int num;

                if (gridDigits[i][j] == 0 || input_Digits[i][j].color == RED) {

                    num = key - GLFW_KEY_0; // Цифра для записи в массив
                    gridDigits[i][j] = num;

                    if (num != SolutionSudoku[i][j]) {
                        input_Digits[i][j].color = RED; // Цифра неверная!
                    }
                    else {
                        input_Digits[i][j].color = BLUE; // Цифра была введена верная
                        CountgridDigits++;
                    }

                    if (CountgridDigits == N * N && gameState == GAME) {
                        gameState = YOU_WIN;
                        handleGameEnd();
                    }
                    else if (CountgridDigits == N * N && gameState == GAME1) {
                        gameState = YOU_WIN1;
                        handleGameEnd();
                    }
                }
            }
            else if (key == GLFW_KEY_BACKSPACE && input_Digits[i][j].color != BLACK && gridDigits[i][j] != 0) {
                if (input_Digits[i][j].color == BLUE) {
                    gridDigits[i][j] = 0;
                    input_Digits[i][j].color = BLACK;
                    CountgridDigits--;
                }
                else if (input_Digits[i][j].color == RED) {
                    gridDigits[i][j] = 0;
                    input_Digits[i][j].color = BLACK;
                }
            }
        }

        else if (gameState == CREATE) {
            if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9 && action == GLFW_PRESS) {

                if (key - GLFW_KEY_0 == gridDigits[i][j]) return;

                int num;
                num = key - GLFW_KEY_0; // Цифра для записи в массив

                if (Check_cell(num, i, j)) {
                    input_Digits[i][j].color = BLUE;
                }
                else {
                    input_Digits[i][j].color = RED;
                }

                gridDigits[i][j] = num;
            }
            else if (key == GLFW_KEY_BACKSPACE && gridDigits[i][j] != 0) {
                if (input_Digits[i][j].color == BLACK) {
                    gridDigits[i][j] = 0;
                }
                else if (input_Digits[i][j].color == RED) {
                    gridDigits[i][j] = 0;
                    input_Digits[i][j].color = BLACK;
                }
            }

            Countdigits();
        }

    }
}

void Window(GLFWwindow* window) {
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Настройка проекции
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 0.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (gameState == MENU) {
            Menu();
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        else if (gameState == GAME) {
            Game();
            glColor3f(1.0f, 1.0f, 1.0f);
            drawField();
            drawSelectedCell();
            drawDigit();
        }
        else if (gameState == GAME1) {
            Game1();
            glColor3f(1.0f, 1.0f, 1.0f);
            drawField();
            drawSelectedCell();
            drawDigit();
        }
        else if (gameState == SETTINGS) {
            Settings();
            glColor3f(1.0f, 1.0f, 1.0f);
            drawButtonEasy(&easyButton);
            drawButtonNormal(&normalButton);
            drawButtonHard(&hardButton);
            drawFrame();
        }
        else if (gameState == YOU_WIN) {
            Wictory();
        }
        else if (gameState == YOU_WIN1) {
            Wictory();
        }
        else if (gameState == SAVED_SUDOKU) {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            SavedSudoku();
        }
        else if (gameState == COMP_MODE) {
            Comp_mode();
        }
        else if (gameState == EXIT) {
            break;
        }
        else if (gameState == CREATE) {
            Game();
            Create();
            glColor3f(1.0f, 1.0f, 1.0f);
            drawField();
            drawSelectedCell();
            drawDigit();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main() {
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(800, 800, "", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 

    Init_textures();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    srand(time(NULL));

    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    loadLeaderboard();

    Window(window);

    glDeleteTextures(CNT_TEXT, Textures);
    glfwTerminate();

    return 0;
}

