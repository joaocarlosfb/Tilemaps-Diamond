#include <GL/freeglut.h>
#include <cstdio>
#include <cmath>
#include <cstring>

//ALUNOS: JOÃO CARLOS FERNANDES BATTASSSINI
//        KAUAN SCHEFFER TEDESCO

static const int  WIN_W   = 800;
static const int  WIN_H   = 600;

static const int  TILE_W  = 128;
static const int  TILE_H  = 64;
static const int  NUM_TILES = 7;

#define MAP_ROWS 6
#define MAP_COLS 6

// MAPA INICIAL - cada numero e o tipo do tile 0 a 6
static int map[MAP_ROWS][MAP_COLS] = {
    { 1, 1, 1, 4, 4, 4 },
    { 1, 0, 1, 4, 3, 4 },
    { 1, 1, 1, 4, 4, 4 },
    { 2, 2, 2, 5, 5, 5 },
    { 2, 6, 2, 5, 0, 5 },
    { 2, 2, 2, 5, 5, 5 },
};

// TILE ONDE O CURSOR ESTA
static int curRow = 0;
static int curCol = 0;

static float originX = WIN_W * 0.5f;
static float originY = TILE_H;

static GLuint tilesetTex = 0;


//CORES RGB
static const unsigned char TILE_RGB[NUM_TILES][3] = {
    { 224, 206, 148 },   // 0 areia
    { 110, 170,  92 },   // 1 grama
    { 232, 140,  56 },   // 2 lava
    {  78, 140, 220 },   // 3 agua
    { 170, 212, 235 },   // 4 gelo
    { 232, 120, 190 },   // 5 rosa
    { 150,  92, 200 },   // 6 roxo
};

// CRIA UMA TEXTURA COM OS 7 TILES LADO A LADO (FORMATO DE LOSANGO)
static void buildTilesetTexture()
{
    const int texW = TILE_W * NUM_TILES;
    const int texH = TILE_H;
    unsigned char *buf = new unsigned char[texW * texH * 4];
    std::memset(buf, 0, texW * texH * 4);

    const float halfW = TILE_W * 0.5f;
    const float halfH = TILE_H * 0.5f;

    for (int t = 0; t < NUM_TILES; ++t) {
        int baseX = t * TILE_W;
        for (int y = 0; y < TILE_H; ++y) {
            for (int x = 0; x < TILE_W; ++x) {
                float dx = (x + 0.5f - halfW) / halfW;
                float dy = (y + 0.5f - halfH) / halfH;
                float d  = std::fabs(dx) + std::fabs(dy);
                if (d > 1.0f) continue;          // fora do losango - transparente

                float shade = 1.08f - 0.30f * (y / (float)TILE_H);

                int r = (int)(TILE_RGB[t][0] * shade);
                int g = (int)(TILE_RGB[t][1] * shade);
                int b = (int)(TILE_RGB[t][2] * shade);
                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;

                int idx = ((y) * texW + (baseX + x)) * 4;
                buf[idx + 0] = (unsigned char)r;
                buf[idx + 1] = (unsigned char)g;
                buf[idx + 2] = (unsigned char)b;
                buf[idx + 3] = 255;
            }
        }
    }

    glGenTextures(1, &tilesetTex);
    glBindTexture(GL_TEXTURE_2D, tilesetTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buf);

    delete[] buf;
}

// DESENHA UM TILE NA POSICAO (ROW, COL) USANDO O TIPO TYPE
static void drawTile(int row, int col, int type, float alpha)
{
    const float hw = TILE_W * 0.5f;
    const float hh = TILE_H * 0.5f;
    // posicao na tela do mapa diamond
    float cx = originX + (col - row) * hw;
    float cy = originY + (col + row) * hh + hh;

    const float texW = (float)(TILE_W * NUM_TILES);
    const float texH = (float)TILE_H;
    // recorta o tile certo pra dentro da textura
    float u0 = (type * TILE_W + 0.5f) / texW;
    float u1 = ((type + 1) * TILE_W - 0.5f) / texW;
    float v0 = 0.5f / texH;
    float v1 = (TILE_H - 0.5f) / texH;

    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
        glTexCoord2f(u0, v0); glVertex2f(cx - hw, cy - hh);
        glTexCoord2f(u1, v0); glVertex2f(cx + hw, cy - hh);
        glTexCoord2f(u1, v1); glVertex2f(cx + hw, cy + hh);
        glTexCoord2f(u0, v1); glVertex2f(cx - hw, cy + hh);
    glEnd();
}

// CONTORNO DO TILE DO CURSOR
static void drawTileOutline(int row, int col)
{
    const float hw = TILE_W * 0.5f;
    const float hh = TILE_H * 0.5f;
    float cx = originX + (col - row) * hw;
    float cy = originY + (col + row) * hh + hh;

    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 0.2f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cx,      cy - hh);
        glVertex2f(cx + hw, cy);
        glVertex2f(cx,      cy + hh);
        glVertex2f(cx - hw, cy);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

static void drawText(float x, float y, const char *s)
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    for (const char *p = s; *p; ++p)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
    glEnable(GL_TEXTURE_2D);
}

static void display()
{
    glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, tilesetTex);

    for (int row = 0; row < MAP_ROWS; ++row)
        for (int col = 0; col < MAP_COLS; ++col)
            drawTile(row, col, map[row][col], 1.0f);

    drawTileOutline(curRow, curCol);

    char line[128];
    std::snprintf(line, sizeof(line),
                  "Tile atual [i=%d, j=%d]  tipo=%d", curRow, curCol, map[curRow][curCol]);
    drawText(10, 18, line);
    drawText(10, WIN_H - 28, "Mover: W/A/S/D + Q/E/Z/C (8 dir)  |  Trocar tile: 1-7 ou ESPACO  |  ESC: sair");

    glutSwapBuffers();
}

// MOVE O CURSOR
static void move(int drow, int dcol)
{
    int r = curRow + drow;
    int c = curCol + dcol;
    if (r >= 0 && r < MAP_ROWS && c >= 0 && c < MAP_COLS) {
        curRow = r;
        curCol = c;
        glutPostRedisplay();
    }
}

// TROCA O TIPO DE TILE
static void setTile(int type)
{
    if (type >= 0 && type < NUM_TILES) {
        map[curRow][curCol] = type;
        glutPostRedisplay();
    }
}

static void onKey(unsigned char key, int, int)
{
    switch (key) {
        case 'w': case 'W': move(-1, -1); break; // N
        case 's': case 'S': move(+1, +1); break; // S
        case 'a': case 'A': move(+1, -1); break; // Oeste
        case 'd': case 'D': move(-1, +1); break; // Leste
        case 'q': case 'Q': move( 0, -1); break; // NO
        case 'e': case 'E': move(-1,  0); break; // NE
        case 'z': case 'Z': move(+1,  0); break; // SO
        case 'c': case 'C': move( 0, +1); break; // SE

        case ' ': setTile((map[curRow][curCol] + 1) % NUM_TILES); break;
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7':
            setTile(key - '1');
            break;

        case 27: glutLeaveMainLoop(); break;     // ESC
    }
}

static void onSpecial(int key, int, int)
{
    switch (key) {
        case GLUT_KEY_UP:    move(-1, -1); break; // N
        case GLUT_KEY_DOWN:  move(+1, +1); break; // S
        case GLUT_KEY_LEFT:  move(+1, -1); break; // Oeste
        case GLUT_KEY_RIGHT: move(-1, +1); break; // Leste
    }
}

// PROJECAO 2D
static void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WIN_W, WIN_H, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Tilemap Isometrico - Diamond (OpenGL)");

    buildTilesetTexture();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(onKey);
    glutSpecialFunc(onSpecial);

    glutMainLoop();
    return 0;
}
