/*************************************************************************************/

//  Szkielet programu do tworzenia modelu sceny 3-D z wizualizacją osi
//  układu współrzędnych dla rzutowania perspektywicznego

/*************************************************************************************/
#include <GL/glut.h>
#include <iostream>
#include <math.h>

typedef float point3[3];
int step = 0;
static GLfloat viewer[]= {0.0, 0.0, 10.0};
static GLfloat R = 10.0;
static const GLfloat R_min = 2.6;
static const GLfloat R_max = 23.0;

// inicjalizacja położenia obserwatora

static GLfloat theta = 0.0;   // kąt obrotu obiektu
static GLfloat fi = 0.0;   // kąt obrotu obiektu
static GLfloat pix2angle_X;     // przelicznik pikseli na stopnie
static GLfloat pix2angle_Y;

static GLint status = 0;       // stan klawiszy myszy
// 0 - nie naciśnięto żadnego klawisza
// 1 - naciśnięty został lewy klawisz
// 2 - naciśnięty został prawy klawisz

static int x_pos_old = 0;       // poprzednia pozycja kursora myszy

static int delta_x = 0;        // różnica pomiędzy pozycją bieżącą
// i poprzednią kursora myszy

static int y_pos_old = 0;

static int delta_y = 0;

/*************************************************************************************/

typedef float point3[3];
typedef float point2[2];
GLint N = 10;


// Funkcja rysująca osie układu współrzędnych
enum Model{
    POINTS, NET, TRIANGLES
};

Model model = POINTS;
point3** colors;

/*************************************************************************************/
// Funkcja wczytuje dane obrazu zapisanego w formacie TGA w pliku o nazwie
// FileName, alokuje pamięć i zwraca wskaźnik (pBits) do bufora w którym
// umieszczone są dane.
// Ponadto udostępnia szerokość (ImWidth), wysokość (ImHeight) obrazu
// tekstury oraz dane opisujące format obrazu według specyfikacji OpenGL
// (ImComponents) i (ImFormat).
// Jest to bardzo uproszczona wersja funkcji wczytującej dane z pliku TGA.
// Działa tylko dla obrazów wykorzystujących 8, 24, or 32 bitowy kolor.
// Nie obsługuje plików w formacie TGA kodowanych z kompresją RLE.
/*************************************************************************************/


GLbyte *LoadTGAImage(const char *FileName, GLint *ImWidth, GLint *ImHeight, GLint *ImComponents, GLenum *ImFormat)
{

/*************************************************************************************/

// Struktura dla nagłówka pliku  TGA


#pragma pack(1)
    typedef struct
    {
        GLbyte    idlength;
        GLbyte    colormaptype;
        GLbyte    datatypecode;
        unsigned short    colormapstart;
        unsigned short    colormaplength;
        unsigned char     colormapdepth;
        unsigned short    x_orgin;
        unsigned short    y_orgin;
        unsigned short    width;
        unsigned short    height;
        GLbyte    bitsperpixel;
        GLbyte    descriptor;
    }TGAHEADER;
#pragma pack(8)

    FILE *pFile;
    TGAHEADER tgaHeader;
    unsigned long lImageSize;
    short sDepth;
    GLbyte    *pbitsperpixel = NULL;


/*************************************************************************************/

// Wartości domyślne zwracane w przypadku błędu

    *ImWidth = 0;
    *ImHeight = 0;
    *ImFormat = GL_BGR_EXT;
    *ImComponents = GL_RGB8;

    pFile = fopen(FileName, "rb");
    if(pFile == NULL)
        return NULL;

/*************************************************************************************/
// Przeczytanie nagłówka pliku


    fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);


/*************************************************************************************/

// Odczytanie szerokości, wysokości i głębi obrazu

    *ImWidth = tgaHeader.width;
    *ImHeight = tgaHeader.height;
    sDepth = tgaHeader.bitsperpixel / 8;


/*************************************************************************************/
// Sprawdzenie, czy głębia spełnia założone warunki (8, 24, lub 32 bity)

    if(tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 && tgaHeader.bitsperpixel != 32)
        return NULL;

/*************************************************************************************/

// Obliczenie rozmiaru bufora w pamięci


    lImageSize = tgaHeader.width * tgaHeader.height * sDepth;


/*************************************************************************************/

// Alokacja pamięci dla danych obrazu


    pbitsperpixel = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));

    if(pbitsperpixel == NULL)
        return NULL;

    if(fread(pbitsperpixel, lImageSize, 1, pFile) != 1)
    {
        free(pbitsperpixel);
        return NULL;
    }


/*************************************************************************************/

// Ustawienie formatu OpenGL


    switch(sDepth)

    {

        case 3:

            *ImFormat = GL_BGR_EXT;

            *ImComponents = GL_RGB8;

            break;

        case 4:

            *ImFormat = GL_BGRA_EXT;

            *ImComponents = GL_RGBA8;

            break;

        case 1:

            *ImFormat = GL_LUMINANCE;

            *ImComponents = GL_LUMINANCE8;

            break;

    };



    fclose(pFile);



    return pbitsperpixel;

}

/*************************************************************************************/


void calculateNormalVector(int i, point3& normalVector, GLfloat u, GLfloat v){
    GLfloat Xu, Xv, Yu, Yv, Zu, Zv;

    Xu = (GLfloat)(cos(M_PI*v)*((-450) * pow(u, 4) + 900 * pow(u, 3) + (-810) * pow(u, 2) +360 * u - 45));
    Xv = (GLfloat)(M_PI*(90 * pow(u, 5) + (-225) * pow(u, 4) + 270 * pow(u, 3) + (-180) * pow(u, 2) + 45 * u)*sin(M_PI*v));
    Yu = (GLfloat)(640 * pow(u, 3) + (-960) * pow(u, 2) + 320 * u);
    Yv = 0;
    Zu = (GLfloat)(sin(M_PI*v)*((-450) * pow(u, 4) + 900 * pow(u, 3) + (-810) * pow(u, 2) + 360 * u - 45));
    Zv = (GLfloat)((-1)*M_PI*cos(M_PI*v)*(90 * pow(u, 5) + (-225) * pow(u, 4) + 270 * pow(u, 3) + (-180) * pow(u, 2) + 45 * u));

    normalVector[0] = Yu*Zv - Zu*Yv;
    normalVector[1] = Zu*Xv - Xu*Zv;
    normalVector[2] = Xu*Yv - Yu*Xv;

    GLfloat vectorLength = (GLfloat)(sqrt(pow(normalVector[0], 2) + pow(normalVector[1], 2) + pow(normalVector[2], 2)));

    if(i == N-1 ){
        normalVector[0]=0;
        normalVector[1]=1;
        normalVector[2]=0;
        return;
    }
    if(i == 0){
        normalVector[0]=0;
        normalVector[1]=-1;
        normalVector[2]=0;
    }
    if(N%2!=0){
        if(i==N/2){
            normalVector[0] = 0;
            normalVector[1] = 0;
            normalVector[2] = 0;
            return;
        }
        if(i == N/2-1) {
            normalVector[0] = 0;
            normalVector[1] = 1;
            normalVector[2] = 0;
            return;
        }
        if(i==N/2+1){
            normalVector[0] = 0;
            normalVector[1] = -1;
            normalVector[2] = 0;
            return;
        }
    }
    if (vectorLength != 0) {
        normalVector[0] = normalVector[0] / vectorLength;
        normalVector[1] = normalVector[1] / vectorLength;
        normalVector[2] = normalVector[2] / vectorLength;
    }
}

void Ostroslup(GLfloat a){

    point3 tr1[3];
    point3 tr2[3];
    point3 tr3[3];
    point3 tr4[3];
    point3 tr5[3];
    point3 tr6[3];

    //podstawa
    //piewszy tojkat
    tr1[0][0] = -a/2;
    tr1[0][1] = 0;
    tr1[0][2] = 0;

    tr1[1][0] = a/2;
    tr1[1][1] = 0;
    tr1[1][2] = 0;

    tr1[2][0] = -a/2;
    tr1[2][1] = 0;
    tr1[2][2] = a;

    //drugi trojkat
    tr2[0][0] = a/2;
    tr2[0][1] = 0;
    tr2[0][2] = 0;

    tr2[1][0] = a/2;
    tr2[1][1] = 0;
    tr2[1][2] = a;

    tr2[2][0] = -a/2;
    tr2[2][1] = 0;
    tr2[2][2] = a;

    //ściana tylna
    tr3[0][0] = -a/2;
    tr3[0][1] = 0;
    tr3[0][2] = 0;

    tr3[1][0] = a/2;
    tr3[1][1] = 0;
    tr3[1][2] = 0;

    tr3[2][0] = 0;
    tr3[2][1] = a/2;
    tr3[2][2] = a/2;

    //ściana przednia
    tr4[0][0] = -a/2;
    tr4[0][1] = 0;
    tr4[0][2] = a;

    tr4[1][0] = a/2;
    tr4[1][1] = 0;
    tr4[1][2] = a;

    tr4[2][0] = 0;
    tr4[2][1] = a/2;
    tr4[2][2] = a/2;

    //ściana prawa
    tr5[0][0] = a/2;
    tr5[0][1] = 0;
    tr5[0][2] = 0;

    tr5[1][0] = a/2;
    tr5[1][1] = 0;
    tr5[1][2] = a;

    tr5[2][0] = 0;
    tr5[2][1] = a/2;
    tr5[2][2] = a/2;

    //ściana lewa
    tr6[0][0] = -a/2;
    tr6[0][1] = 0;
    tr6[0][2] = 0;

    tr6[1][0] = -a/2;
    tr6[1][1] = 0;
    tr6[1][2] = a;

    tr6[2][0] = 0;
    tr6[2][1] = a/2;
    tr6[2][2] = a/2;



    glBegin(GL_TRIANGLES);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(tr1[0]);

        glNormal3f(0.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(tr1[1]);

        glNormal3f(0.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3fv(tr1[2]);
    glEnd();
    glBegin(GL_TRIANGLES);
        glNormal3f(0.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(tr2[0]);

        glNormal3f(0.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3fv(tr2[1]);

        glNormal3f(0.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3fv(tr2[2]);

    glEnd();
    glBegin(GL_TRIANGLES);
        glNormal3f(0.0f, (float)sqrt(2), -(float)sqrt(2));
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(tr3[1]);

        glNormal3f(0.0f, (float)sqrt(2), -(float)sqrt(2));
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(tr3[0]);

        glNormal3f(0.0f, (float)sqrt(2), -(float)sqrt(2));
        glTexCoord2f(0.5f, 1.0f);
        glVertex3fv(tr3[2]);
    glEnd();
    glBegin(GL_TRIANGLES);
        glNormal3f(0.0f, (float)sqrt(2), (float)sqrt(2));
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(tr4[0]);

        glNormal3f(0.0f, (float)sqrt(2), (float)sqrt(2));
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(tr4[1]);

        glNormal3f(0.0f, (float)sqrt(2), (float)sqrt(2));
        glTexCoord2f(0.5f, 1.0f);
        glVertex3fv(tr4[2]);
    glEnd();
    glBegin(GL_TRIANGLES);
        glNormal3f((float)sqrt(2), (float)sqrt(2), 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(tr5[1]);

        glNormal3f((float)sqrt(2), (float)sqrt(2), 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(tr5[0]);

        glNormal3f((float)sqrt(2), (float)sqrt(2), 0.0f);
        glTexCoord2f(0.5f, 1.0f);
        glVertex3fv(tr5[2]);
    glEnd();
    glBegin(GL_TRIANGLES);
        glNormal3f(-(float)sqrt(2), (float)sqrt(2), 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(tr6[0]);

        glNormal3f(-(float)sqrt(2), (float)sqrt(2), 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(tr6[1]);

        glNormal3f(-(float)sqrt(2), (float)sqrt(2), 0.0f);
        glTexCoord2f(0.5f, 1.0f);
        glVertex3fv(tr6[2]);
    glEnd();

}

void Egg(GLint n)
{
    /*Tworzenie siatki dziedziny parametrycznej*/
    point2** param = new point2*[n];

    for(int i = 0; i < n; i++)
    {
        param[i] = new point2[n];

        for(int k = 0; k < n; k++)
        {
            GLfloat u = (float)i/(n-1);
            GLfloat v = (float)k/(n-1);

            param[i][k][0] = u;
            param[i][k][1] = v;
        }
    }

    point3** egg_p = new point3*[n];
    point3** normVec = new point3*[n];
    for(int i = 0; i < n; i++)
    {
        egg_p[i] = new point3[n];
        normVec[i] = new point3[n];
        for(int k = 0; k < n; k++)
        {
            calculateNormalVector(i, normVec[i][k], param[i][k][0], param[i][k][1]);
            egg_p[i][k][0] = (float)((-90*pow(param[i][k][0], 5) + 225*pow(param[i][k][0], 4) - 270*pow(param[i][k][0], 3) + 180*pow(param[i][k][0],2) - 45*param[i][k][0])*cos(M_PI*param[i][k][1]));
            egg_p[i][k][1] = (float)(160*pow(param[i][k][0], 4) - 320*pow(param[i][k][0], 3) + 160*pow(param[i][k][0], 2));
            egg_p[i][k][2] = (float)((-90*pow(param[i][k][0], 5) + 225*pow(param[i][k][0], 4) - 270*pow(param[i][k][0], 3) + 180*pow(param[i][k][0],2) - 45*param[i][k][0])*sin(M_PI*param[i][k][1]));
        }
    }

    float STEP = 1/n-1;

    //Przechodzimy przez wszystkie poziomy jajka od dołu do góry
    for (int i = 0; i < n/2; i++) {
        //Przechodzimy przez wszystkie punkty na jednym poziomie (piersćieniu)
        for (int k = 0; k < n-1; k++) {
            glBegin(GL_TRIANGLES);

            glNormal3f(normVec[i][k][0],normVec[i][k][1],normVec[i][k][2]);
            //glColor3f(colors[i][k][0], colors[i][k][1], colors[i][k][2]);
            glTexCoord2f(i*STEP, k*STEP);
            glVertex3f(egg_p[i][k][0], egg_p[i][k][1] - 5.0f, egg_p[i][k][2]);

            glNormal3f(normVec[i][k+1][0],normVec[i][k+1][1],normVec[i][k+1][2]);
            //glColor3f(colors[i][k + 1][0], colors[i][k + 1][1], colors[i][k + 1][2]);
            glTexCoord2f(i*STEP, (k+1)*STEP);
            glVertex3f(egg_p[i][k + 1][0], egg_p[i][k + 1][1] - 5.0f, egg_p[i][k + 1][2]);

            if(i==n/2-1)
                glNormal3f(0.0f, 1.0f, 0.0f);
            else
                glNormal3f(normVec[i+1][k][0],normVec[i+1][k][1],normVec[i+1][k][2]);
            //glColor3f(colors[i + 1][k][0], colors[i + 1][k][1], colors[i + 1][k][2]);
            glTexCoord2f((i+1)*STEP, k*STEP);
            glVertex3f(egg_p[i + 1][k][0], egg_p[i + 1][k][1] - 5.0f, egg_p[i + 1][k][2]);

            glEnd();
            glBegin(GL_TRIANGLES);


            glNormal3f(normVec[i][k+1][0],normVec[i][k+1][1],normVec[i][k+1][2]);
            //glColor3f(colors[i][k + 1][0], colors[i][k + 1][1], colors[i][k + 1][2]);
            glTexCoord2f(i*STEP, (k+1)*STEP);
            glVertex3f(egg_p[i][k + 1][0], egg_p[i][k + 1][1] - 5.0f, egg_p[i][k + 1][2]);


            //glColor3f(1.0f, 1.0f, 1.0f);
            if(i==n/2-1)
                glNormal3f(0.0f, 1.0f, 0.0f);
            else
                glNormal3f(normVec[i+1][k+1][0],normVec[i+1][k+1][1],normVec[i+1][k+1][2]);
            //glColor3f(colors[i + 1][k + 1][0], colors[i + 1][k + 1][1], colors[i + 1][k + 1][2]);
            glTexCoord2f((i+1)*STEP, (k+1)*STEP);
            glVertex3f(egg_p[i + 1][k + 1][0], egg_p[i + 1][k + 1][1] - 5.0f, egg_p[i + 1][k + 1][2]);


            if(i==n/2-1)
                glNormal3f(0.0f, 1.0f, 0.0f);
            else
                glNormal3f(normVec[i+1][k][0],normVec[i+1][k][1],normVec[i+1][k][2]);
            //glColor3f(colors[i + 1][k][0], colors[i + 1][k][1], colors[i + 1][k][2]);
            glTexCoord2f((i+1)*STEP, k*STEP);
            glVertex3f(egg_p[i + 1][k][0], egg_p[i + 1][k][1] - 5.0f, egg_p[i + 1][k][2]);

            glEnd();

        }
    }

    //Przechodzimy przez wszystkie poziomy jajka od dołu do góry
    for (int i = n/2; i < n-1; i++) {
        //Przechodzimy przez wszystkie punkty na jednym poziomie (piersćieniu)
        for (int k = 0; k < n - 1; k++) {
            if(i==n/2){
                glBegin(GL_TRIANGLES);
                if(N%2==0)
                    glNormal3f(0.0f, 1.0f, 0.0f);
                //glColor3f(1.0f, 1.0f, 1.0f);
                //glColor3f(colors[i][k][0], colors[i][k][1], colors[i][k][2]);
                glTexCoord2f(1.0f-i*STEP, k*STEP);
                glVertex3f(egg_p[i][k][0], egg_p[i][k][1] - 5.0f, egg_p[i][k][2]);

                if(N%2==0)
                    glNormal3f(0.0f, 1.0f, 0.0f);
                //glColor3f(1.0f, 1.0f, 1.0f);
                //glColor3f(colors[i][k + 1][0], colors[i][k + 1][1], colors[i][k + 1][2]);
                glTexCoord2f(1.0f-i*STEP, (k+1)*STEP);
                glVertex3f(egg_p[i][k + 1][0], egg_p[i][k + 1][1] - 5.0f, egg_p[i][k + 1][2]);

                //glColor3f(1.0f, 1.0f, 1.0f);
                glNormal3f(-normVec[i+1][k][0],-normVec[i+1][k][1],-normVec[i+1][k][2]);
                //glColor3f(colors[i + 1][k][0], colors[i + 1][k][1], colors[i + 1][k][2]);
                glTexCoord2f(1.0f-(i+1)*STEP, k*STEP);
                glVertex3f(egg_p[i + 1][k][0], egg_p[i + 1][k][1] - 5.0f, egg_p[i + 1][k][2]);
                glEnd();

                glBegin(GL_TRIANGLES);
                 if(N%2==0)
                     glNormal3f(0.0f, 1.0f, 0.0f);
                //glColor3f(colors[i][k + 1][0], colors[i][k + 1][1], colors[i][k + 1][2]);
                glTexCoord2f(1.0f-i*STEP, (k+1)*STEP);
                glVertex3f(egg_p[i][k + 1][0], egg_p[i][k + 1][1] - 5.0f, egg_p[i][k + 1][2]);

                 //glColor3f(1.0f, 1.0f, 1.0f);
                glNormal3f(-normVec[i+1][k+1][0],-normVec[i+1][k+1][1],-normVec[i+1][k+1][2]);
                //glColor3f(colors[i + 1][k + 1][0], colors[i + 1][k + 1][1], colors[i + 1][k + 1][2]);
                glTexCoord2f(1.0f-(i+1)*STEP, (k+1)*STEP);
                glVertex3f(egg_p[i + 1][k + 1][0], egg_p[i + 1][k + 1][1] - 5.0f, egg_p[i + 1][k + 1][2]);

                 //glColor3f(1.0f, 1.0f, 1.0f);
                glNormal3f(-normVec[i+1][k][0],-normVec[i+1][k][1],-normVec[i+1][k][2]);
                //glColor3f(colors[i + 1][k][0], colors[i + 1][k][1], colors[i + 1][k][2]);
                glTexCoord2f(1.0f-(i+1)*STEP, k*STEP);
                glVertex3f(egg_p[i + 1][k][0], egg_p[i + 1][k][1] - 5.0f, egg_p[i + 1][k][2]);

                glEnd();

            }
            else{
                glBegin(GL_TRIANGLES);
                //glColor3f(1.0f, 1.0f, 1.0f);
                 glNormal3f(-normVec[i][k][0],-normVec[i][k][1],-normVec[i][k][2]);
                //glColor3f(colors[i][k][0], colors[i][k][1], colors[i][k][2]);
                glTexCoord2f(1.0f-i*STEP, k*STEP);
                glVertex3f(egg_p[i][k][0], egg_p[i][k][1] - 5.0f, egg_p[i][k][2]);

                 //glColor3f(1.0f, 1.0f, 1.0f);

                glNormal3f(-normVec[i][k+1][0],-normVec[i][k+1][1],-normVec[i][k+1][2]);
                //glColor3f(colors[i][k + 1][0], colors[i][k + 1][1], colors[i][k + 1][2]);
                glTexCoord2f(1.0f-i*STEP, (k+1)*STEP);
                glVertex3f(egg_p[i][k + 1][0], egg_p[i][k + 1][1] - 5.0f, egg_p[i][k + 1][2]);

                //glColor3f(1.0f, 1.0f, 1.0f);
                glNormal3f(-normVec[i+1][k][0],-normVec[i+1][k][1],-normVec[i+1][k][2]);
                //glColor3f(colors[i + 1][k][0], colors[i + 1][k][1], colors[i + 1][k][2]);
                glTexCoord2f(1.0f-(i+1)*STEP, k*STEP);
                glVertex3f(egg_p[i + 1][k][0], egg_p[i + 1][k][1] - 5.0f, egg_p[i + 1][k][2]);
                 glEnd();

                glBegin(GL_TRIANGLES);


                glNormal3f(-normVec[i][k+1][0],-normVec[i][k+1][1],-normVec[i][k+1][2]);
                //glColor3f(colors[i][k + 1][0], colors[i][k + 1][1], colors[i][k + 1][2]);
                glTexCoord2f(1.0f-i*STEP, (k+1)*STEP);
                glVertex3f(egg_p[i][k + 1][0], egg_p[i][k + 1][1] - 5.0f, egg_p[i][k + 1][2]);


                //glColor3f(1.0f, 1.0f, 1.0f);
                glNormal3f(-normVec[i+1][k+1][0],-normVec[i+1][k+1][1],-normVec[i+1][k+1][2]);
                //glColor3f(colors[i + 1][k + 1][0], colors[i + 1][k + 1][1], colors[i + 1][k + 1][2]);
                glTexCoord2f(1.0f-(i+1)*STEP, (k+1)*STEP);
                glVertex3f(egg_p[i + 1][k + 1][0], egg_p[i + 1][k + 1][1] - 5.0f, egg_p[i + 1][k + 1][2]);


                //glColor3f(1.0f, 1.0f, 1.0f);
                glNormal3f(-normVec[i+1][k][0],-normVec[i+1][k][1],-normVec[i+1][k][2]);
                //glColor3f(colors[i + 1][k][0], colors[i + 1][k][1], colors[i + 1][k][2]);
                glTexCoord2f(1.0f-(i+1)*STEP, k*STEP);
                glVertex3f(egg_p[i + 1][k][0], egg_p[i + 1][k][1] - 5.0f, egg_p[i + 1][k][2]);

                glEnd();
            }
        }
    }


}
/*************************************************************************************/
// Funkcja "bada" stan myszy i ustawia wartości odpowiednich zmiennych globalnych

void Mouse(int btn, int state, int x, int y)
{


    if(btn==GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        x_pos_old=x;        // przypisanie aktualnie odczytanej pozycji kursora
        y_pos_old=y;                     // jako pozycji poprzedniej
        status = 1;          // wcięnięty został lewy klawisz myszy
    }
    else
    if(btn==GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        y_pos_old = y;
        status = 2;
    }
    else
        status = 0;          // nie został wcięnięty żaden klawisz

}



/*************************************************************************************/
// Funkcja "monitoruje" położenie kursora myszy i ustawia wartości odpowiednich
// zmiennych globalnych

void Motion( GLsizei x, GLsizei y )
{


    delta_x = x - x_pos_old;     // obliczenie różnicy położenia kursora myszy

    x_pos_old = x;            // podstawienie bieżącego położenia jako poprzednie

    delta_y = y - y_pos_old;

    y_pos_old = y;

    glutPostRedisplay();     // przerysowanie obrazu sceny
}

/*************************************************************************************/

// Funkcja rysująca osie układu wspó?rz?dnych
void Axes(void)
{

    point3  x_min = {-5.0, 0.0, 0.0};
    point3  x_max = { 5.0, 0.0, 0.0};
    // pocz?tek i koniec obrazu osi x

    point3  y_min = {0.0, -5.0, 0.0};
    point3  y_max = {0.0,  5.0, 0.0};
    // pocz?tek i koniec obrazu osi y

    point3  z_min = {0.0, 0.0, -5.0};
    point3  z_max = {0.0, 0.0,  5.0};
    //  pocz?tek i koniec obrazu osi y

    glColor3f(1.0f, 0.0f, 0.0f);  // kolor rysowania osi - czerwony
    glBegin(GL_LINES); // rysowanie osi x

    glVertex3fv(x_min);
    glVertex3fv(x_max);

    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);  // kolor rysowania - zielony
    glBegin(GL_LINES);  // rysowanie osi y

    glVertex3fv(y_min);
    glVertex3fv(y_max);

    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);  // kolor rysowania - niebieski
    glBegin(GL_LINES); // rysowanie osi z

    glVertex3fv(z_min);
    glVertex3fv(z_max);

    glEnd();

}

/*************************************************************************************/

// Funkcja określająca co ma być rysowane (zawsze wywoływana, gdy trzeba
// przerysować scenę)



void RenderScene(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Czyszczenie okna aktualnym kolorem czyszczącym

    glLoadIdentity();
    // Czyszczenie macierzy bie??cej

    gluLookAt(viewer[0],viewer[1],viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // Zdefiniowanie położenia obserwatora



    glColor3f(1.0f, 1.0f, 1.0f);
    // Ustawienie koloru rysowania na biały

    if(status == 1)                     // jeśli lewy klawisz myszy wcięnięty
    {


        theta += delta_x*pix2angle_X;
        fi += delta_y*pix2angle_Y;   // modyfikacja kąta obrotu o kat proporcjonalny
        // do różnicy położeń kursora myszy

    }
    if(status == 2)
    {
        viewer[2] += delta_y*0.05;
        if (viewer[2] > R_max)
            viewer[2] = R_max;

        if(viewer[2] < R_min)
            viewer[2] = R_min;


    }

    gluLookAt(viewer[0],viewer[1],viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(theta, 0.0, 1.0, 0.0);  //obrót obiektu o nowy kąt
    glRotatef(fi, 1.0, 0.0, 0.0);

    Axes();
    // Narysowanie osi przy pomocy funkcji zdefiniowanej powyżej
    std::cout<<std::endl;
    std::cout<<N<<std::endl<<step<<std::endl;
    Egg(N);
    //Ostroslup(5.0f);

    glFlush();
    // Przekazanie poleceń rysujących do wykonania

    glutSwapBuffers();



}
/*************************************************************************************/

// Funkcja ustalająca stan renderowania

void keys(unsigned char key, int x, int y)
{
    if(key == 'p') model = POINTS;
    if(key == 'w') model = NET;
    if(key == 's') model = TRIANGLES;
    if(key == '.')step++;
    if(key == ',')step--;
    if(key == '+')
    {
        N += 5;
        delete[] colors;
        colors = new point3*[N];

        for(int i = 0; i < N; i++)
        {
            colors[i] = new point3[N];

            for(int k = 0; k < N; k++)
            {
                colors[i][k][0] = (float)(rand()%1001)/1000;
                colors[i][k][1] = (float)(rand()%1001)/1000;
                colors[i][k][2] = (float)(rand()%1001)/1000;
            }
        }
    }

    if(key == '-' && N >= 10)
    {
        N = N - 5;
        delete[] colors;
        colors = new point3*[N];

        for(int i = 0; i < N; i++)
        {
            colors[i] = new point3[N];

            for(int k = 0; k < N; k++)
            {
                colors[i][k][0] = (float)(rand()%1001)/1000;
                colors[i][k][1] = (float)(rand()%1001)/1000;
                colors[i][k][2] = (float)(rand()%1001)/1000;
            }
        }
    }
    RenderScene(); // przerysowanie obrazu sceny
}


void MyInit(void)
{

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /*************************************************************************************/

// Zmienne dla obrazu tekstury



    GLbyte *pBytes;
    GLint ImWidth, ImHeight, ImComponents;
    GLenum ImFormat;
// Kolor czyszczący (wypełnienia okna) ustawiono na czarny
    /*************************************************************************************/

/*************************************************************************************/

// Teksturowanie będzie prowadzone tyko po jednej stronie ściany

    //glEnable(GL_CULL_FACE);

/*************************************************************************************/

//  Przeczytanie obrazu tekstury z pliku o nazwie tekstura.tga

    pBytes = LoadTGAImage("t_512.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);

    /*************************************************************************************/

// Zdefiniowanie tekstury 2-D

    glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);

/*************************************************************************************/

// Zwolnienie pamięci

    free(pBytes);

/*************************************************************************************/

// Włączenie mechanizmu teksturowania

    glEnable(GL_TEXTURE_2D);

/*************************************************************************************/

// Ustalenie trybu teksturowania

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

/*************************************************************************************/

// Określenie sposobu nakładania tekstur

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//  Definicja materiału z jakiego zrobiony jest czajnik
//  i definicja źródła światła

/*************************************************************************************/


/*************************************************************************************/
// Definicja materiału z jakiego zrobiony jest czajnik

    GLfloat mat_ambient[]  = {1.0, 1.0, 1.0, 1.0};
    // współczynniki ka =[kar,kag,kab] dla światła otoczenia

    GLfloat mat_diffuse[]  = {1.0, 1.0, 1.0, 1.0};
    // współczynniki kd =[kdr,kdg,kdb] światła rozproszonego

    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    // współczynniki ks =[ksr,ksg,ksb] dla światła odbitego

    GLfloat mat_shininess  = {20.0};
    // współczynnik n opisujący połysk powierzchni

/*************************************************************************************/
// Definicja źródła światła

    GLfloat light_position[] = {0.0, 0.0, 10.0, 1.0};
    // położenie źródła


    GLfloat light_ambient[] = {0.1, 0.1, 0.1, 1.0};
    // składowe intensywności świecenia źródła światła otoczenia
    // Ia = [Iar,Iag,Iab]

    GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    // składowe intensywności świecenia źródła światła powodującego
    // odbicie dyfuzyjne Id = [Idr,Idg,Idb]

    GLfloat light_specular[]= {1.0, 1.0, 1.0, 1.0};
    // składowe intensywności świecenia źródła światła powodującego
    // odbicie kierunkowe Is = [Isr,Isg,Isb]

    GLfloat att_constant  = {1.0};
    // składowa stała ds dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

    GLfloat att_linear    = {0.05};
    // składowa liniowa dl dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

    GLfloat att_quadratic  = {0.001};
    // składowa kwadratowa dq dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

/*************************************************************************************/
// Ustawienie parametrów materiału i źródła światła

/*************************************************************************************/
// Ustawienie patrametrów materiału


    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

/*************************************************************************************/
// Ustawienie parametrów źródła

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);


/*************************************************************************************/
// Ustawienie opcji systemu oświetlania sceny

    glShadeModel(GL_SMOOTH); // właczenie łagodnego cieniowania
    glEnable(GL_LIGHTING);   // właczenie systemu oświetlenia sceny
    glEnable(GL_LIGHT0);     // włączenie źródła o numerze 0
    glEnable(GL_DEPTH_TEST); // włączenie mechanizmu z-bufora

/*************************************************************************************/

}

/*************************************************************************************/

// Funkcja ma za zadanie utrzymanie stałych proporcji rysowanych
// w przypadku zmiany rozmiarów okna.
// Parametry vertical i horizontal (wysokość i szerokość okna) są
// przekazywane do funkcji za każdym razem gdy zmieni się rozmiar okna.



void ChangeSize(GLsizei horizontal, GLsizei vertical)
{

    pix2angle_X = 360/(float)horizontal;  // przeliczenie pikseli na stopnie
    pix2angle_Y = 360/(float)vertical;  // przeliczenie pikseli na stopnie

    glMatrixMode(GL_PROJECTION);
    // Przełączenie macierzy bieżącej na macierz projekcji

    glLoadIdentity();
    // Czyszcznie macierzy bieżącej

    gluPerspective(70, 1.0, 1.0, 50.0);
    // Ustawienie parametrów dla rzutu perspektywicznego


    if(horizontal <= vertical)
        glViewport(0, (vertical-horizontal)/2, horizontal, horizontal);

    else
        glViewport((horizontal-vertical)/2, 0, vertical, vertical);
    // Ustawienie wielkości okna okna widoku (viewport) w zależności
    // relacji pomiędzy wysokością i szerokością okna

    glMatrixMode(GL_MODELVIEW);
    // Przełączenie macierzy bieżącej na macierz widoku modelu

    glLoadIdentity();
    // Czyszczenie macierzy bieżącej

}

/*************************************************************************************/

// Główny punkt wejścia programu. Program działa w trybie konsoli



int main(int argc, char** argv)
{
    srand(time(NULL));

    colors = new point3*[N];

    for(int i = 0; i < N; i++)
    {
        colors[i] = new point3[N];

        for(int k = 0; k < N; k++)
        {
            colors[i][k][0] = (float)(rand()%1001)/1000;
            colors[i][k][1] = (float)(rand()%1001)/1000;
            colors[i][k][2] = (float)(rand()%1001)/1000;
        }
    }

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB |GLUT_DEPTH);

    glutInitWindowSize(300, 300);

    glutCreateWindow("Obserwator jajka");

    glutDisplayFunc(RenderScene);
    // Określenie, że funkcja RenderScene będzie funkcją zwrotną
    // (callback function).  Będzie ona wywoływana za każdym razem
    // gdy zajdzie potrzeba przerysowania okna

    glutKeyboardFunc(keys);

    glutReshapeFunc(ChangeSize);
    // Dla aktualnego okna ustala funkcję zwrotną odpowiedzialną
    // za zmiany rozmiaru okna

    glutMouseFunc(Mouse);
    // Ustala funkcję zwrotną odpowiedzialną za badanie stanu myszy

    glutMotionFunc(Motion);
    // Ustala funkcję zwrotną odpowiedzialną za badanie ruchu myszy

    MyInit();
    // Funkcja MyInit() (zdefiniowana powyżej) wykonuje wszelkie
    // inicjalizacje konieczne  przed przystąpieniem do renderowania

    glEnable(GL_DEPTH_TEST);
    // Włączenie mechanizmu usuwania niewidocznych elementów sceny

    glutMainLoop();
    // Funkcja uruchamia szkielet biblioteki GLUT
    return 0;
}

/*************************************************************************************/