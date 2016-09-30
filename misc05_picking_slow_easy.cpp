// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <iostream>
#include <common/tessshader.hpp>
#include <math.h>

#define POINT_SIZE 5.0f
#define M 5
#define N 8

typedef struct Vertex {
    float XYZW[4];
    float RGBA[4];
    void SetCoords(float *coords) {
        XYZW[0] = coords[0];
        XYZW[1] = coords[1];
        XYZW[2] = coords[2];
        XYZW[3] = coords[3];
    }
    void SetColor(float *color) {
        RGBA[0] = color[0];
        RGBA[1] = color[1];
        RGBA[2] = color[2];
        RGBA[3] = color[3];
    }
};

// ATTN: USE POINT STRUCTS FOR EASIER COMPUTATIONS
typedef struct point {
    float x, y, z;
    point(const float x = 0, const float y = 0, const float z = 0) : x(x), y(y), z(z) {};
    point(float *coords) : x(coords[0]), y(coords[1]), z(coords[2]) {};
    point operator -(const point& a)const {
        return point(x - a.x, y - a.y, z - a.z);
    }
    point operator +(const point& a)const {
        return point(x + a.x, y + a.y, z + a.z);
    }
    point operator *(const float& a)const {
        return point(x*a, y*a, z*a);
    }
    point operator /(const float& a)const {
        return point(x / a, y / a, z / a);
    }
    float* toArray() {
        float array[] = { x, y, z, 1.0f };
        return array;
    }
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void createVAOs(Vertex[], unsigned short[], size_t, size_t, int);
void createObjects(void);
void pickVertex(void);
void moveVertex(void);
void drawScene(void);
void cleanup(void);
static void mouseCallback(GLFWwindow*, int, int, int);
static void keyCallback(GLFWwindow*, int, int, int, int);

// GLOBAL VARIABLES
GLFWwindow* window;
const GLuint window_width = 800, window_height = 600;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = 99999;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;
GLuint bezierCurveShaderID;

// ATTN: INCREASE THIS NUMBER AS YOU CREATE NEW OBJECTS
const GLuint NumObjects = 256;	// number of different "objects" to be drawn
GLuint VertexArrayId[NumObjects] = { 0 };
GLuint VertexBufferId[NumObjects] = { 0 };
GLuint IndexBufferId[NumObjects] = { 0 };
size_t NumVert[NumObjects] = { 0 };

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint pickingColorID;
GLuint LightID;
GLuint BezierMatrixId;
GLuint BezierProjectionId;

// Define objects
Vertex Vertices[] =
{
    { { 0.0f, 1.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } }, // 0
    { { 0.707f, 0.707f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } }, // 1
    { { 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } }, // 2
    { { 0.707f, -0.707f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } }, // 3
    { { 0.0f, -1.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } }, // 4
    { { -0.707f, -0.707f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } }, // 5
    { { -1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } }, // 6
    { { -0.707f, 0.707f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } } // 7
};

Vertex evenMoreVertices[40];

Vertex nVertices[9];

Vertex pVertices[5][257];

const int tessellation_no = 20;

Vertex tempVertices[5];
Vertex bVerticesArray[N * tessellation_no];
Vertex tVerticesArray[N * tessellation_no];

Vertex movingPoint[1];
Vertex tangentPoints[2];
Vertex normalPoints[2];
Vertex binormalPoints[2];

unsigned short Indices[] = {
    0, 1, 2, 3, 4, 5, 6, 7
};

unsigned short nIndices[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8
};

float pickedOriginalColorR;
float pickedOriginalColorG;
float pickedOriginalColorB;
bool isColorChanged = false;
int state;
int rightState;
vec3 mousePos;
int curr_level = 0;
unsigned short pIndices[257];
int prev_max_i = 8;
float pointSize = POINT_SIZE;
float colorBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
float colorYellow[] = {1.0f, 1.0f, 0.0f, 1.0f};
float colorRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
float colorGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};
bool showBezierCurves = false;
bool showBezierUsingTess = false;
bool shouldZTranslate = false;
bool shouldSplitView = false;
bool showAxes = false;
int pointCount = 0;

const size_t IndexCount = sizeof(Indices) / sizeof(unsigned short);
// ATTN: DON'T FORGET TO INCREASE THE ARRAY SIZE IN THE PICKING VERTEX SHADER WHEN YOU ADD MORE PICKING COLORS
float pickingColor[IndexCount] = { 0 / 255.0f, 1 / 255.0f, 2 / 255.0f, 3 / 255.0f,  4 / 255.0f, 5 / 255.0f, 6 / 255.0f, 7 / 255.0f };

// ATTN: ADD YOU PER-OBJECT GLOBAL ARRAY DEFINITIONS HERE

void createObjects(void)
{
    // ATTN: DERIVE YOUR NEW OBJECTS HERE:
    // each has one vertices {pos;color} and one indices array (no picking needed here)
    for(int i = 0; i <= N - 1; i++) {
        pVertices[0][i] = Vertices[i];
        pVertices[0][i].SetColor(colorBlue);
    }
    pVertices[0][N] = pVertices[0][0];
    pVertices[0][N].SetColor(colorBlue);

    int max_i = N * pow(2, curr_level);
    int i;
    if(curr_level != 0) {
        for(int j = 1; j <= curr_level; j++) {
            prev_max_i = N * pow(2, j - 1);
            for(i = 0; i < max_i / 2; i++) {
                point *p_k1_i2;
                if(i == 0) {
                    p_k1_i2 = new point(pVertices[j-1][prev_max_i - 2].XYZW);
                } else if(i == 1) {
                    p_k1_i2 = new point(pVertices[j-1][prev_max_i - 1].XYZW);
                } else {
                    p_k1_i2 = new point(pVertices[j-1][i-2].XYZW);
                }
                point *p_k1_i_minus_1;
                if(i == 0) {
                    p_k1_i_minus_1 = new point(pVertices[j-1][prev_max_i - 1].XYZW);
                } else {
                    p_k1_i_minus_1 = new point(pVertices[j-1][i-1].XYZW);
                }
                point *p_k1_i = new point(pVertices[j-1][i].XYZW);
                point *p_k1_i_plus_1;
                if((i + 1) > prev_max_i) {
                    p_k1_i_plus_1 = new point(pVertices[j-1][1].XYZW);
                } else {
                    p_k1_i_plus_1 = new point(pVertices[j-1][i+1].XYZW);
                }

                point p_k_2i = (*p_k1_i2 + *p_k1_i_minus_1 * 10 + *p_k1_i * 5) / 16;
                point p_k_2i_plus_1 = (*p_k1_i_minus_1 * 5 + *p_k1_i * 10 + *p_k1_i_plus_1) / 16;
                pVertices[j][2*i] = {p_k_2i.x, p_k_2i.y, p_k_2i.z, 1.0f};
                pVertices[j][2*i].SetColor(colorBlue);
                pVertices[j][2*i + 1] = {p_k_2i_plus_1.x, p_k_2i_plus_1.y, p_k_2i_plus_1.z, 1.0f};
                pVertices[j][2*i + 1].SetColor(colorBlue);
            }
            pVertices[j][max_i] = pVertices[j][0];
            pVertices[j][max_i].SetColor(colorBlue);
        }
    }

    if(showBezierCurves || showBezierUsingTess) {
        for(int i = 0; i < N; i++) {
            point *p_i_minus_2, *p_i_minus_1, *p_i, *p_i_plus_1, *p_i_plus_2;
            if(i == 0) {
                p_i_minus_2 = new point(Vertices[N - 2].XYZW);
                p_i_minus_1 = new point(Vertices[N - 1].XYZW);
                p_i = new point(Vertices[i].XYZW);
                p_i_plus_1 = new point(Vertices[i + 1].XYZW);
                p_i_plus_2 = new point(Vertices[i + 2].XYZW);
            } else if(i == 1) {
                p_i_minus_2 = new point(Vertices[N - 1].XYZW);
                p_i_minus_1 = new point(Vertices[i - 1].XYZW);
                p_i = new point(Vertices[i].XYZW);
                p_i_plus_1 = new point(Vertices[i + 1].XYZW);
                p_i_plus_2 = new point(Vertices[i + 2].XYZW);
            } else if(i == (N - 2)) {
                p_i_minus_2 = new point(Vertices[i - 2].XYZW);
                p_i_minus_1 = new point(Vertices[i - 1].XYZW);
                p_i = new point(Vertices[i].XYZW);
                p_i_plus_1 = new point(Vertices[i + 1].XYZW);
                p_i_plus_2 = new point(Vertices[0].XYZW);
            } else if(i == (N - 1)) {
                p_i_minus_2 = new point(Vertices[i - 2].XYZW);
                p_i_minus_1 = new point(Vertices[i - 1].XYZW);
                p_i = new point(Vertices[i].XYZW);
                p_i_plus_1 = new point(Vertices[0].XYZW);
                p_i_plus_2 = new point(Vertices[1].XYZW);
            } else {
                p_i_minus_2 = new point(Vertices[i - 2].XYZW);
                p_i_minus_1 = new point(Vertices[i - 1].XYZW);
                p_i = new point(Vertices[i].XYZW);
                p_i_plus_1 = new point(Vertices[i + 1].XYZW);
                p_i_plus_2 = new point(Vertices[i + 2].XYZW);
            }
            point c_i_0 = (*p_i_minus_2 + *p_i_minus_1 * 11 + *p_i * 11 + *p_i_plus_1) / 24;
            evenMoreVertices[5*i] = {c_i_0.x, c_i_0.y, c_i_0.z, 1.0f};
            evenMoreVertices[5*i].SetColor(colorRed);
            point c_i_1 = (*p_i_minus_1 * 8 + *p_i * 14 + *p_i_plus_1 * 2) / 24;
            evenMoreVertices[5*i + 1] = {c_i_1.x, c_i_1.y, c_i_1.z, 1.0f};
            evenMoreVertices[5*i + 1].SetColor(colorRed);
            point c_i_2 = (*p_i_minus_1 * 4 + *p_i * 16 + *p_i_plus_1 * 4) / 24;
            evenMoreVertices[5*i + 2] = {c_i_2.x, c_i_2.y, c_i_2.z, 1.0f};
            evenMoreVertices[5*i + 2].SetColor(colorRed);
            point c_i_3 = (*p_i_minus_1 * 2 + *p_i * 14 + *p_i_plus_1 * 8) / 24;
            evenMoreVertices[5*i + 3] = {c_i_3.x, c_i_3.y, c_i_3.z, 1.0f};
            evenMoreVertices[5*i + 3].SetColor(colorRed);
            point c_i_4 = (*p_i_minus_1 + *p_i * 11 + *p_i_plus_1 * 11 + *p_i_plus_2) / 24;
            evenMoreVertices[5*i + 4] = {c_i_4.x, c_i_4.y, c_i_4.z, 1.0f};
            evenMoreVertices[5*i + 4].SetColor(colorRed);
        }
    }

    if(showBezierCurves) {
        point *p1, *p2;
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < 5; j++) {
                for(int k = 0; k < tessellation_no; k++) {
                    for(int l = 0; l < 5; l++) {
                        tempVertices[l].SetCoords(evenMoreVertices[5 * i + l].XYZW);
                    }
                    for(int m = 1; m < 5; m++) {
                        for(int n = 0; n < 5 - m; n++) {
                            p1 = new point(tempVertices[n].XYZW);
                            p2 = new point(tempVertices[n + 1].XYZW);
                            point newP = (*p1 * (1 - ((float)k / (float)tessellation_no))) + (*p2 * ((float)k / (float)tessellation_no));
                            if(m == 4) {
                                point tanP = *p2 - *p1;
                                tVerticesArray[i * tessellation_no + k] = {tanP.x, tanP.y, tanP.z, 1.0f};
                            }
                            tempVertices[n] = {newP.x, newP.y, newP.z, 1.0f};
                        }
                    }
                    bVerticesArray[i * tessellation_no + k].SetCoords(tempVertices[0].XYZW);
                    bVerticesArray[i * tessellation_no + k].SetColor(colorYellow);
                }
            }
        }
    }
}

void drawScene(void)
{
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    // Re-clear the screen for real rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!shouldSplitView) {
        glViewport(0.0f, 0.0f, window_width, window_height);
        glUseProgram(programID);
        {
            glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
            glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
            glm::vec3 lightPos = glm::vec3(4, 4, 4);
            glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

            //glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(pointSize);

            glBindVertexArray(VertexArrayId[0]);	// draw Vertices
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);				// update buffer data
                                                                                            //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
            //glDrawElements(GL_LINE_STRIP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            //glBindVertexArray(VertexArrayId[<x>]); etc etc
            glBindVertexArray(VertexArrayId[1]);
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(nVertices), nVertices);
            glDrawArrays(GL_LINE_STRIP, 0, 9);

            createVAOs(pVertices[curr_level], pIndices, sizeof(pVertices[curr_level]), sizeof(pIndices), 2);
            glBindVertexArray(VertexArrayId[2]);
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pVertices[curr_level]), pVertices[curr_level]);
            glDrawArrays(GL_LINE_STRIP, 0, N * pow(2, curr_level) + 1);

            if(showBezierCurves) {
                glBindVertexArray(VertexArrayId[3]);
                glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(evenMoreVertices), evenMoreVertices);
                glDrawElements(GL_POINTS, 40, GL_UNSIGNED_SHORT, (void*)0);

                glBindVertexArray(VertexArrayId[4]);
                glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bVerticesArray), bVerticesArray);
                glDrawArrays(GL_LINE_STRIP, 0, N * tessellation_no);

                if(showAxes) {
                    if(pointCount < N * tessellation_no) {
                        movingPoint[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        movingPoint[0].SetColor(colorYellow);
                        createVAOs(movingPoint, pIndices, sizeof(movingPoint), sizeof(pIndices), 5);
                        glBindVertexArray(VertexArrayId[5]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(movingPoint), movingPoint);
                        glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, (void*)0);

                        tangentPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        tangentPoints[0].SetColor(colorRed);
                        float angle = atan2(tVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[0]);
                        angle += M_PI;
                        Vertex vertex[1];
                        vertex[0] = {{cos(angle) + bVerticesArray[pointCount].XYZW[0], sin(angle) + bVerticesArray[pointCount].XYZW[1], bVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        tangentPoints[1].SetCoords(vertex[0].XYZW);
                        tangentPoints[1].SetColor(colorRed);
                        createVAOs(tangentPoints, pIndices, sizeof(tangentPoints), sizeof(pIndices), 6);
                        glBindVertexArray(VertexArrayId[6]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tangentPoints), tangentPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        normalPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        normalPoints[0].SetColor(colorGreen);
                        vertex[0] = {{-tVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[0], bVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        normalPoints[1].SetCoords(vertex[0].XYZW);
                        normalPoints[1].SetColor(colorGreen);
                        createVAOs(normalPoints, pIndices, sizeof(normalPoints), sizeof(pIndices), 7);
                        glBindVertexArray(VertexArrayId[7]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(normalPoints), normalPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        binormalPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        binormalPoints[0].SetColor(colorBlue);
                        vertex[0] = {{-tVerticesArray[pointCount].XYZW[1], bVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        binormalPoints[1].SetCoords(vertex[0].XYZW);
                        binormalPoints[1].SetColor(colorBlue);
                        createVAOs(binormalPoints, pIndices, sizeof(binormalPoints), sizeof(pIndices), 8);
                        glBindVertexArray(VertexArrayId[8]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(binormalPoints), binormalPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        pointCount++;
                    } else {
                        pointCount = 0;
                    }
                }
            }
            glBindVertexArray(0);
        }

        if(showBezierUsingTess) {
            glUseProgram(bezierCurveShaderID); {
                glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
                glm::mat4 ModelViewMatrix = gViewMatrix * ModelMatrix;

                glUniformMatrix4fv(BezierMatrixId, 1, GL_FALSE, &ModelViewMatrix[0][0]);
                glUniformMatrix4fv(BezierProjectionId, 1, GL_FALSE, &gProjectionMatrix[0][0]);

                //glPointSize(pointSize);

                point *p1, *p2;
                for(int i = 0; i < N; i++) {
                    for(int j = 0; j < 5; j++) {
                        tempVertices[j].SetCoords(evenMoreVertices[5 * i + j].XYZW);
                    }
                    createVAOs(tempVertices, pIndices, sizeof(tempVertices), sizeof(pIndices), i+5);
                    glBindVertexArray(VertexArrayId[i+5]);
                    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[i+5]);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tempVertices), tempVertices);
                    glPatchParameteri(GL_PATCH_VERTICES, 5);
                    glDrawArrays(GL_PATCHES, 0, 5);
                }
            }
        }
    } else {
        glm::mat4 gProjectionMatrix1 = glm::ortho(-4.0f, 4.0f, -1.5f, 1.5f, 0.0f, 100.0f); // In world coordinates
        glViewport(0.0f, window_height / 2.0f, window_width, window_height / 2.0f);
        glUseProgram(programID);
        {
            glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
            glm::mat4 MVP = gProjectionMatrix1 * gViewMatrix * ModelMatrix;

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
            glm::vec3 lightPos = glm::vec3(4, 4, 4);
            glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

            //glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(pointSize);

            glBindVertexArray(VertexArrayId[0]);	// draw Vertices
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);				// update buffer data
                                                                                            //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
            //glDrawElements(GL_LINE_STRIP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            //glBindVertexArray(VertexArrayId[<x>]); etc etc
            glBindVertexArray(VertexArrayId[1]);
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(nVertices), nVertices);
            glDrawArrays(GL_LINE_STRIP, 0, 9);

            createVAOs(pVertices[curr_level], pIndices, sizeof(pVertices[curr_level]), sizeof(pIndices), 2);
            glBindVertexArray(VertexArrayId[2]);
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pVertices[curr_level]), pVertices[curr_level]);
            glDrawArrays(GL_LINE_STRIP, 0, N * pow(2, curr_level) + 1);

            if(showBezierCurves) {
                glBindVertexArray(VertexArrayId[3]);
                glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(evenMoreVertices), evenMoreVertices);
                glDrawElements(GL_POINTS, 40, GL_UNSIGNED_SHORT, (void*)0);

                glBindVertexArray(VertexArrayId[4]);
                glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bVerticesArray), bVerticesArray);
                glDrawArrays(GL_LINE_STRIP, 0, N * tessellation_no);

                if(showAxes) {
                    if(pointCount < N * tessellation_no) {
                        movingPoint[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        movingPoint[0].SetColor(colorYellow);
                        createVAOs(movingPoint, pIndices, sizeof(movingPoint), sizeof(pIndices), 5);
                        glBindVertexArray(VertexArrayId[5]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(movingPoint), movingPoint);
                        glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, (void*)0);

                        tangentPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        tangentPoints[0].SetColor(colorRed);
                        float angle = atan2(tVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[0]);
                        angle += M_PI;
                        Vertex vertex[1];
                        vertex[0] = {{cos(angle) + bVerticesArray[pointCount].XYZW[0], sin(angle) + bVerticesArray[pointCount].XYZW[1], bVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        tangentPoints[1].SetCoords(vertex[0].XYZW);
                        tangentPoints[1].SetColor(colorRed);
                        createVAOs(tangentPoints, pIndices, sizeof(tangentPoints), sizeof(pIndices), 6);
                        glBindVertexArray(VertexArrayId[6]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tangentPoints), tangentPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        normalPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        normalPoints[0].SetColor(colorGreen);
                        vertex[0] = {{-tVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[0], bVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        normalPoints[1].SetCoords(vertex[0].XYZW);
                        normalPoints[1].SetColor(colorGreen);
                        createVAOs(normalPoints, pIndices, sizeof(normalPoints), sizeof(pIndices), 7);
                        glBindVertexArray(VertexArrayId[7]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(normalPoints), normalPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        binormalPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        binormalPoints[0].SetColor(colorBlue);
                        vertex[0] = {{-tVerticesArray[pointCount].XYZW[1], bVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        binormalPoints[1].SetCoords(vertex[0].XYZW);
                        binormalPoints[1].SetColor(colorBlue);
                        createVAOs(binormalPoints, pIndices, sizeof(binormalPoints), sizeof(pIndices), 8);
                        glBindVertexArray(VertexArrayId[8]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(binormalPoints), binormalPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        pointCount++;
                    } else {
                        pointCount = 0;
                    }
                }
            }
            glBindVertexArray(0);
        }

        glUseProgram(programID);
        {
            glViewport(0.0f, 0.0f, window_width, window_height / 2.0f);
            glm::mat4 gProjectionMatrix2 = glm::ortho(-4.0f, 4.0f, -1.5f, 1.5f, 0.0f, 100.0f); // In world coordinates
            glm::mat4 gViewMatrix2 = glm::lookAt(
                glm::vec3(-5.0f, 0.0f, 0.0f), // Camera is at (4,3,3), in World Space
                glm::vec3(0.0f, 0.0f, 0.0f), // and looks at the origin
                glm::vec3(0.0f, 1.0f, 0.0f)  // Head is up (set to 0,-1,0 to look upside-down)
            );
            glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
            glm::mat4 MVP = gProjectionMatrix2 * gViewMatrix2 * ModelMatrix;

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix2[0][0]);
            glm::vec3 lightPos = glm::vec3(4, 4, 4);
            glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

            //glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(pointSize);

            glBindVertexArray(VertexArrayId[0]);	// draw Vertices
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);				// update buffer data
                                                                                            //glDrawElements(GL_LINE_LOOP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            // ATTN: OTHER BINDING AND DRAWING COMMANDS GO HERE, one set per object:
            //glDrawElements(GL_LINE_STRIP, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
            //glBindVertexArray(VertexArrayId[<x>]); etc etc
            glBindVertexArray(VertexArrayId[1]);
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(nVertices), nVertices);
            glDrawArrays(GL_LINE_STRIP, 0, 9);

            createVAOs(pVertices[curr_level], pIndices, sizeof(pVertices[curr_level]), sizeof(pIndices), 2);
            glBindVertexArray(VertexArrayId[2]);
            glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pVertices[curr_level]), pVertices[curr_level]);
            glDrawArrays(GL_LINE_STRIP, 0, N * pow(2, curr_level) + 1);

            if(showBezierCurves) {
                glBindVertexArray(VertexArrayId[3]);
                glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(evenMoreVertices), evenMoreVertices);
                glDrawElements(GL_POINTS, 40, GL_UNSIGNED_SHORT, (void*)0);

                glBindVertexArray(VertexArrayId[4]);
                glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bVerticesArray), bVerticesArray);
                glDrawArrays(GL_LINE_STRIP, 0, N * tessellation_no);

                if(showAxes) {
                    if(pointCount < N * tessellation_no) {
                        movingPoint[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        movingPoint[0].SetColor(colorYellow);
                        createVAOs(movingPoint, pIndices, sizeof(movingPoint), sizeof(pIndices), 5);
                        glBindVertexArray(VertexArrayId[5]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(movingPoint), movingPoint);
                        glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, (void*)0);

                        tangentPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        tangentPoints[0].SetColor(colorRed);
                        float angle = atan2(tVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[0]);
                        angle += M_PI;
                        Vertex vertex[1];
                        vertex[0] = {{cos(angle) + bVerticesArray[pointCount].XYZW[0], sin(angle) + bVerticesArray[pointCount].XYZW[1], bVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        tangentPoints[1].SetCoords(vertex[0].XYZW);
                        tangentPoints[1].SetColor(colorRed);
                        createVAOs(tangentPoints, pIndices, sizeof(tangentPoints), sizeof(pIndices), 6);
                        glBindVertexArray(VertexArrayId[6]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[6]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tangentPoints), tangentPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        normalPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        normalPoints[0].SetColor(colorGreen);
                        vertex[0] = {{-tVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[0], bVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        normalPoints[1].SetCoords(vertex[0].XYZW);
                        normalPoints[1].SetColor(colorGreen);
                        createVAOs(normalPoints, pIndices, sizeof(normalPoints), sizeof(pIndices), 7);
                        glBindVertexArray(VertexArrayId[7]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(normalPoints), normalPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        binormalPoints[0].SetCoords(bVerticesArray[pointCount].XYZW);
                        binormalPoints[0].SetColor(colorBlue);
                        vertex[0] = {{-tVerticesArray[pointCount].XYZW[1], bVerticesArray[pointCount].XYZW[1], tVerticesArray[pointCount].XYZW[2], 1.0f}, {1.0f}};
                        binormalPoints[1].SetCoords(vertex[0].XYZW);
                        binormalPoints[1].SetColor(colorBlue);
                        createVAOs(binormalPoints, pIndices, sizeof(binormalPoints), sizeof(pIndices), 8);
                        glBindVertexArray(VertexArrayId[8]);
                        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[8]);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(binormalPoints), binormalPoints);
                        glDrawArrays(GL_LINE_STRIP, 0, 2);

                        pointCount++;
                    } else {
                        pointCount = 0;
                    }
                }
            }
            glBindVertexArray(0);
        }
    }

    glUseProgram(0);
    // Draw GUI
    TwDraw();

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void pickVertex(void)
{
    // Clear the screen in white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(pickingProgramID);
    {
        glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
        glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniform1fv(pickingColorArrayID, NumVert[0], pickingColor);	// here we pass in the picking marker array

                                                                        // Draw the ponts
                                                                        //glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(pointSize);
        glBindVertexArray(VertexArrayId[0]);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);	// update buffer data
        glDrawElements(GL_POINTS, NumVert[0], GL_UNSIGNED_SHORT, (void*)0);
        glBindVertexArray(0);
    }
    glUseProgram(0);
    // Wait until all the pending drawing commands are really done.
    // Ultra-mega-over slow !
    // There are usually a long time between glDrawElements() and
    // all the fragments completely rasterized.
    glFlush();
    glFinish();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Read the pixel at the center of the screen.
    // You can also use glfwGetMousePos().
    // Ultra-mega-over slow too, even for 1 pixel,
    // because the framebuffer is on the GPU.
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    unsigned char data[4];
    glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

                                                                                     // Convert the color back to an integer ID
    if (!isColorChanged) {
        gPickedIndex = int(data[0]);
    }

    if (gPickedIndex == 255) { // Full white, must be the background !
        gMessage = "background";
    }
    else {
        if (!isColorChanged && gPickedIndex < IndexCount) {
            pickedOriginalColorR = Vertices[gPickedIndex].RGBA[0];
            pickedOriginalColorG = Vertices[gPickedIndex].RGBA[1];
            pickedOriginalColorB = Vertices[gPickedIndex].RGBA[2];
            isColorChanged = true;
        }
        Vertices[gPickedIndex].RGBA[0] = 0.0f;
        Vertices[gPickedIndex].RGBA[1] = 0.0f;
        Vertices[gPickedIndex].RGBA[2] = 0.0f;
    }

    // Uncomment these lines to see the picking shader in effect
    //glfwSwapBuffers(window);
    //continue; // skips the normal rendering
}

// fill this function in!
void moveVertex(void)
{
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);

    // retrieve your cursor position
    // get your world coordinates
    // move points

    if (isColorChanged) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        float zpos;
        glReadPixels(xpos, window_height - ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zpos);
        mousePos = glm::unProject(glm::vec3(xpos, ypos, zpos), ModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
        Vertices[gPickedIndex].XYZW[0] = -mousePos.x;
        Vertices[gPickedIndex].XYZW[1] = -mousePos.y;
        if(shouldZTranslate) {
            Vertices[gPickedIndex].XYZW[2] = mousePos.x;
        }
        std::cout << Vertices[gPickedIndex].XYZW[2] << std::endl;
    }

    if (gPickedIndex == 255) { // Full white, must be the background !
        gMessage = "background";
    }
    else {
        std::ostringstream oss;
        oss << "point " << gPickedIndex;
        gMessage = oss.str();
        for (int i = 0; i <= IndexCount; i++) {
            if (i == 8) {
                nVertices[i].SetCoords(Vertices[0].XYZW);
            }
            else {
                nVertices[i].SetCoords(Vertices[i].XYZW);
            }
        }
    }
}

int initWindow(void)
{
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(window_width, window_height, "Sayak Biswas (54584911)", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Initialize the GUI
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(window_width, window_height);
    TwBar * GUI = TwNewBar("Picking");
    TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
    TwDefine(" Picking size='200 100' ");
    TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);


    //TODO: Uncomment and place in a different location
//    TwBar * InfoUI = TwNewBar("Instructions");
//    TwDefine(" Instructions position='200 450' ");
//    TwDefine(" Instructions size='400 120' ");
//    TwSetParam(InfoUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
//    std::string instructions = "";
//    TwAddVarRW(InfoUI, "Left Click to Pick and Hold to Drag", TW_TYPE_STDSTRING, &instructions, NULL);
//    TwAddVarRW(InfoUI, "Hold right mouse button to make points bigger", TW_TYPE_STDSTRING, &instructions, NULL);
//    TwAddVarRW(InfoUI, "Press 1 for subdivision", TW_TYPE_STDSTRING, &instructions, NULL);
//    TwAddVarRW(InfoUI, "Press 2 for De Casteljau's", TW_TYPE_STDSTRING, &instructions, NULL);
//    TwAddVarRW(InfoUI, "Press 3 for Tessellation Engine", TW_TYPE_STDSTRING, &instructions, NULL);

    // Set up inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE); //TODO: Check this later
    glfwSetCursorPos(window, window_width / 2, window_height / 2);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);

    return 0;
}

void initOpenGL(void)
{
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    for(int i = 0; i < 256; i++) {
        pIndices[i] = i;
    }

    // Projection matrix : 45\B0 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    //glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

                                                                            // Camera matrix
    gViewMatrix = glm::lookAt(
        glm::vec3(0, 0, -5), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
    pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");
    bezierCurveShaderID = LoadTessShaders("BezierCurve.tessshader", "BezierCurve.evalshader", "BezierCurve.vertexshader", "BezierCurve.fragmentshader");


    // Get a handle for our "MVP" uniform
    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");
    PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
    // Get a handle for our "pickingColorID" uniform
    pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
    pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
    // Get a handle for our "LightPosition" uniform
    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    BezierMatrixId = glGetUniformLocation(bezierCurveShaderID, "matModelView");
    BezierProjectionId = glGetUniformLocation(bezierCurveShaderID, "matProjection");

    createVAOs(Vertices, Indices, sizeof(Vertices), sizeof(Indices), 0);
    createObjects();

    float lineColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    for (int i = 0; i <= IndexCount; i++) {
        if (i == 8) {
            nVertices[i].SetCoords(Vertices[0].XYZW);
            nVertices[i].SetColor(lineColor);
        } else {
            nVertices[i].SetCoords(Vertices[i].XYZW);
            nVertices[i].SetColor(lineColor);
        }
    }

    // ATTN: create VAOs for each of the newly created objects here:
    // createVAOs(<fill this appropriately>);
    createVAOs(nVertices, nIndices, sizeof(nVertices), sizeof(nIndices), 1);
    //createVAOs(pVertices[curr_level], pIndices, sizeof(pVertices[curr_level]), sizeof(pIndices), 2);
    createVAOs(evenMoreVertices, pIndices, sizeof(evenMoreVertices), sizeof(pIndices), 3);
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], size_t BufferSize, size_t IdxBufferSize, int ObjectId) {

    NumVert[ObjectId] = IdxBufferSize / (sizeof(GLubyte));

    GLenum ErrorCheckValue = glGetError();
    size_t VertexSize = sizeof(Vertices[0]);
    size_t RgbOffset = sizeof(Vertices[0].XYZW);

    // Create Vertex Array Object
    glGenVertexArrays(1, &VertexArrayId[ObjectId]);
    glBindVertexArray(VertexArrayId[ObjectId]);

    // Create Buffer for vertex data
    glGenBuffers(1, &VertexBufferId[ObjectId]);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
    glBufferData(GL_ARRAY_BUFFER, BufferSize, Vertices, GL_STATIC_DRAW);

    // Create Buffer for indices
    glGenBuffers(1, &IndexBufferId[ObjectId]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IdxBufferSize, Indices, GL_STATIC_DRAW);

    // Assign vertex attributes
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);

    glEnableVertexAttribArray(0);	// position
    glEnableVertexAttribArray(1);	// color

                                    // Disable our Vertex Buffer Object
    glBindVertexArray(0);

    ErrorCheckValue = glGetError();
    if (ErrorCheckValue != GL_NO_ERROR)
    {
        fprintf(
            stderr,
            "ERROR: Could not create a VBO: %s \n",
            gluErrorString(ErrorCheckValue)
        );
    }
}

void cleanup(void)
{
    // Cleanup VBO and shader
    for (int i = 0; i < NumObjects; i++) {
        glDeleteBuffers(1, &VertexBufferId[i]);
        glDeleteBuffers(1, &IndexBufferId[i]);
        glDeleteVertexArrays(1, &VertexArrayId[i]);
    }
    glDeleteProgram(programID);
    glDeleteProgram(pickingProgramID);
    glDeleteProgram(bezierCurveShaderID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        pickVertex();
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        if (isColorChanged && gPickedIndex < IndexCount) {
            Vertices[gPickedIndex].RGBA[0] = pickedOriginalColorR;
            Vertices[gPickedIndex].RGBA[1] = pickedOriginalColorG;
            Vertices[gPickedIndex].RGBA[2] = pickedOriginalColorB;
            Vertices[gPickedIndex].RGBA[3] = 1.0f;
            isColorChanged = false;
        }
    }
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_1 && action == GLFW_PRESS) {
        if(curr_level == M - 1) {
            curr_level = 0;
        } else {
            curr_level += 1;
        }
    }

    if(key == GLFW_KEY_2 && action == GLFW_PRESS) {
        if(!showBezierCurves) {
            showBezierCurves = true;
            createObjects();
            createVAOs(bVerticesArray, pIndices, sizeof(bVerticesArray), sizeof(pIndices), 4);
        } else {
            showBezierCurves = false;
        }
    }

    if(key == GLFW_KEY_3 && action == GLFW_PRESS) {
        if(!showBezierUsingTess) {
            showBezierUsingTess = true;
        } else {
            showBezierUsingTess = false;
        }
    }

    if((key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) && action == GLFW_PRESS) {
        shouldZTranslate = true;
    } else {
        shouldZTranslate = false;
    }

    if(key == GLFW_KEY_4 && action == GLFW_PRESS) {
        if(shouldSplitView) {
            shouldSplitView = false;
        } else {
            shouldSplitView = true;
        }
    }

    if(key == GLFW_KEY_5 && action == GLFW_PRESS) {
        if(showAxes) {
            showAxes = false;
        } else {
            showAxes = true;
        }
    }
}

int main(void)
{
    // initialize window
    int errorCode = initWindow();
    if (errorCode != 0)
        return errorCode;

    // initialize OpenGL pipeline
    initOpenGL();

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    do {
        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
                                             // printf and reset
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // DRAGGING: move current (picked) vertex with cursor
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
            moveVertex();
        glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
        rightState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        if (rightState == GLFW_PRESS) {
            pointSize += 1.0f;
        }

        if (rightState == GLFW_RELEASE) {
            if (pointSize > POINT_SIZE) {
                pointSize -= 1.0f;
            }
        }

        // DRAWING SCENE
        createObjects();	// re-evaluate curves in case vertices have been moved
        drawScene();


    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    cleanup();

    return 0;
}
