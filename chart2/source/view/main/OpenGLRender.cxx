/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GL/glew.h>
#include <vector>
#include <iostream>
#include "OpenGLRender.hxx"
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

using namespace com::sun::star;

#include <boost/scoped_array.hpp>

using namespace std;

#define OPENGL_SHADER( ... )# __VA_ARGS__

const char *ColorFragmemtShader = OPENGL_SHADER (

varying vec3 fragmentColor;

void main()
{
    gl_FragColor = vec4(fragmentColor, 1);
}

);

const char *TransformVertexShader = OPENGL_SHADER (

attribute vec3 vertexPosition_modelspace;
attribute vec3 vertexColor;
varying vec3 fragmentColor;
uniform mat4 MVP;

void main()
{
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
    fragmentColor = vertexColor;
}

);

const char *Line2DFragmemtShader = OPENGL_SHADER (

varying vec4 fragmentColor;

void main()
{
    gl_FragColor = fragmentColor;
}

);

const char *Line2DVertexShader = OPENGL_SHADER (

attribute vec4 vPosition;
uniform vec4 vLineColor;
varying vec4 fragmentColor;

void main()
{
    gl_Position =  vPosition;
    fragmentColor = vLineColor;
}

);

const char *RenderFragmentShader = OPENGL_SHADER (

uniform sampler2D RenderTex;
varying vec2 vTexCoord;

void main()
{
    gl_FragColor = vec4(texture2D(RenderTex, vTexCoord).rgb, 1);
}

);

const char *RenderVertexShader = OPENGL_SHADER (

attribute vec4 vPosition;
attribute vec2 texCoord;
varying vec2 vTexCoord;

void main()
{
    gl_Position =  vPosition;
    vTexCoord = texCoord;
}

);


static GLfloat squareVertices[] = {
    -1.0f, -1.0f, -1.0,
    1.0f, -1.0f, -1.0,
    1.0f,  1.0f, -1.0,
    -1.0f,  1.0f, -1.0
};

static GLfloat coordVertices[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};
#if 0
static const GLfloat g_vertex_buffer_data[] = {
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};

// One color for each vertex. They were generated randomly.
static const GLfloat g_color_buffer_data[] = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f,
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
};
#endif
int static checkGLError(char *file, int line)
{
    GLenum glErr;
    int    retCode = 0;
    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        const GLubyte* sError = gluErrorString(glErr);

        if (sError)
            cout << "GL Error #" << glErr << "(" << gluErrorString(glErr) << ") " << " in File " << file << " at line: " << line << endl;
        else
            cout << "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line << endl;

        retCode = -1;
        return retCode;
    }
    return retCode;
}


#define CHECK_GL_ERROR() checkGLError(__FILE__, __LINE__)

GLint OpenGLRender::LoadShaders(const char *vertexShader,const char *fragmentShader)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;



    // Compile Vertex Shader
    char const * VertexSourcePointer = vertexShader;
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        cout << "vertex shader compile fail : " << VertexShaderErrorMessage[0] << endl;
    }



    // Compile Fragment Shader
    char const * FragmentSourcePointer = fragmentShader;
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        cout << "fragment shader compile fail : " << FragmentShaderErrorMessage[0] << endl;
    }



    // Link the program
    GLint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        cout << "Shader Program fail : " << ProgramErrorMessage[0] << endl;
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

int OpenGLRender::InitOpenGL(GLWindow aWindow)
{
    glWin = aWindow;
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    //[mod] by gaowei
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Init the Projection matrix
    m_Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    m_View       = glm::lookAt(glm::vec3(0,0,1), // Camera is at (4,3,-3), in World Space
                               glm::vec3(0,0,0), // and looks at the origin
                               glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                               );
    m_ProgramID = LoadShaders(TransformVertexShader, ColorFragmemtShader);
    glGenBuffers(1, &m_VertexBuffer);
    glGenBuffers(1, &m_ColorBuffer);
    m_MatrixID = glGetUniformLocation(m_ProgramID, "MVP");
    m_VertexID = glGetAttribLocation(m_ProgramID, "vertexPosition_modelspace");
    m_ColorID = glGetAttribLocation(m_ProgramID, "vertexColor");

    m_RenderProID = LoadShaders(RenderVertexShader, RenderFragmentShader);
    m_RenderVertexID = glGetAttribLocation(m_RenderProID, "vPosition");
    m_RenderTexCoordID = glGetAttribLocation(m_RenderProID, "texCoord");
    m_RenderTexID = glGetUniformLocation(m_RenderProID, "RenderTex");

    m_Line2DProID = LoadShaders(Line2DVertexShader, Line2DFragmemtShader);
    m_Line2DVertexID = glGetAttribLocation(m_Line2DProID, "vPosition");
    m_Line2DColorID = glGetUniformLocation(m_Line2DProID, "vLineColor");

    glGenBuffers(1, &m_RenderVertexBuf);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderVertexBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_RenderTexCoordBuf);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderTexCoordBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordVertices), coordVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#if defined( WNT )
    SwapBuffers(glWin.hDC);
    glFlush();
#elif defined( UNX )
    unx::glXSwapBuffers(glWin.dpy, glWin.win);
#endif
    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    return 0;
}


int OpenGLRender::RenderModelf(float *vertexArray, unsigned int vertexArraySize, float *colorArray, unsigned int colorArraySize)
{
    if (vertexArraySize != colorArraySize)
    {
        return -1;
    }
    glViewport(0, 0, m_iWidth, m_iHeight);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexArraySize, vertexArray, GL_STATIC_DRAW);
    //fill color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, colorArraySize, colorArray, GL_STATIC_DRAW);
    m_MVP = m_Projection * m_View * m_Model;
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(m_ProgramID);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(m_VertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glVertexAttribPointer(
        m_VertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );
    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(m_ColorID);
    glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
    glVertexAttribPointer(
        m_ColorID,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
        );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, vertexArraySize / sizeof(float)); // 12*3 indices starting at 0 -> 12 triangles

    glDisableVertexAttribArray(m_VertexID);
    glDisableVertexAttribArray(m_ColorID);
    glUseProgram(0);
#if defined( WNT )
    SwapBuffers(glWin.hDC);
    glFlush();
#elif defined( UNX )
    unx::glXSwapBuffers(glWin.dpy, glWin.win);
#endif
    return 0;
}
int OpenGLRender::RenderModelf2FBO(float *vertexArray, unsigned int vertexArraySize, float *colorArray, unsigned int colorArraySize)
{
    char fileName[256] = {0};
    sprintf(fileName, "D:\\shaderout_%d_%d.bmp", m_iWidth, m_iHeight);
    if (vertexArraySize != colorArraySize)
    {
        return -1;
    }
    glViewport(0, 0, m_iWidth, m_iHeight);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // create a texture object
    CreateTextureObj(m_iWidth, m_iHeight);
    //create render buffer object
    CreateRenderObj(m_iWidth, m_iHeight);
    //create fbo
    CreateFrameBufferObj();
    //fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexArraySize, vertexArray, GL_STATIC_DRAW);
    //fill color buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, colorArraySize, colorArray, GL_STATIC_DRAW);
    m_MVP = m_Projection * m_View * m_Model;
    //bind fbo
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[0]);

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(m_ProgramID);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(m_MatrixID, 1, GL_FALSE, &m_MVP[0][0]);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(m_VertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
    glVertexAttribPointer(
        m_VertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );
    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(m_ColorID);
    glBindBuffer(GL_ARRAY_BUFFER, m_ColorBuffer);
    glVertexAttribPointer(
        m_ColorID,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
        );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, vertexArraySize / sizeof(float)); // 12*3 indices starting at 0 -> 12 triangles
    glDisableVertexAttribArray(m_VertexID);
    glDisableVertexAttribArray(m_ColorID);
    glUseProgram(0);
    int result = 0;
    GLenum fbResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( fbResult != GL_FRAMEBUFFER_COMPLETE )
    {
        result = -1;
    }
#if 0
    sal_uInt8 *buf = (sal_uInt8 *)malloc(m_iWidth * m_iHeight * 3 + BMP_HEADER_LEN);
    CreateBMPHeader(buf, m_iWidth, -m_iHeight);
    glReadPixels(0, 0, m_iWidth, m_iHeight, GL_BGR, GL_UNSIGNED_BYTE, buf + BMP_HEADER_LEN);
    FILE *pfile = fopen(fileName,"wb");
    fwrite(buf,m_iWidth * m_iHeight * 3 + BMP_HEADER_LEN, 1, pfile);
    free(buf);
    fclose(pfile);

#else
    boost::scoped_array<sal_uInt8> buf(new sal_uInt8[m_iWidth * m_iHeight * 4]);
    glReadPixels(0, 0, m_iWidth, m_iHeight, GL_RGBA, GL_UNSIGNED_BYTE, buf.get());
    BitmapEx aBmp;
    aBmp.SetSizePixel(Size(m_iWidth, m_iHeight));

    Bitmap aBitmap( aBmp.GetBitmap() );
    Bitmap aAlpha( aBmp.GetAlpha().GetBitmap() );

    Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );
    Bitmap::ScopedWriteAccess pAlphaWriteAccess( aAlpha );

    size_t nCurPos = 0;
    for( size_t y = 0; y < m_iHeight; ++y)
    {
        Scanline pScan = pWriteAccess->GetScanline(y);
        Scanline pAlphaScan = pAlphaWriteAccess->GetScanline(y);

        for( size_t x = 0; x < m_iWidth; ++x )
        {
            *pScan++ = buf[nCurPos];
            *pScan++ = buf[nCurPos+1];
            *pScan++ = buf[nCurPos+2];

            nCurPos += 3;

            *pAlphaScan++ = static_cast<sal_uInt8>( 255 - buf[nCurPos++] );
        }
    }

    aBmp = BitmapEx(aBitmap, aAlpha);
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RenderTexture(m_TextureObj[0]);
#if 0
    sal_uInt8 *buf = (sal_uInt8 *)malloc(m_iWidth * m_iHeight * 3);
    glBindTexture(GL_TEXTURE_2D, m_TextureObj);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, buf);
    FILE *pfile = fopen(fileName,"wb");
    fwrite(buf,m_iWidth * m_iHeight * 3, 1, pfile);
    fclose(pfile);
    free(buf);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
    return 0;
}

int OpenGLRender::SetLine2DShapePoint(float x, float y, int listLength)
{
    if (!m_Line2DPoitList.pointBuf)
    {
        //a new point buffer should be alloc, we should push the old buffer first
        m_Line2DPoitList.bufLen = listLength * sizeof(float) * 2;
        m_Line2DPoitList.pointBuf = (float *)malloc(m_Line2DPoitList.bufLen);
    }

    float zeroX = (float)m_iWidth * 10;
    float zeroY = (float)m_iHeight * 10;
    m_Line2DPoitList.pointBuf[m_iPointNum++] = 3.5 * ((x - zeroX) / zeroX + 0.3);
    m_Line2DPoitList.pointBuf[m_iPointNum++] = 3.5 * ((y - zeroY) / zeroY + 0.25);

    if (m_iPointNum == (listLength << 1))
    {
        m_Line2DShapePointList.push_back(m_Line2DPoitList);
        m_Line2DPoitList.pointBuf = NULL;
        m_iPointNum = 0;
    }
    return 0;
}

int OpenGLRender::RenderLine2FBO(int wholeFlag)
{
    char fileName[256] = {0};
    sprintf(fileName, "D:\\shaderout_%d_%d_%d.bmp", m_iWidth, m_iHeight, m_iFboIdx);

    glViewport(0, 0, m_iWidth, m_iHeight);
    glClearDepth(1.0f);
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if ((!m_FboID[0]) || (!m_FboID[1]))
    {
        // create a texture object
        CreateTextureObj(m_iWidth, m_iHeight);
        //create render buffer object
        CreateRenderObj(m_iWidth, m_iHeight);
        //create fbo
        CreateFrameBufferObj();
    }
    //bind fbo
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[m_iFboIdx % 2]);

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (wholeFlag)
    {
        if (m_iFboIdx > 0)
        {
           RenderTexture2FBO(m_TextureObj[(m_iFboIdx - 1) % 2]);
        }
    }
    glLineWidth(m_fLineWidth);
    int listNum = m_Line2DShapePointList.size();
    for (int i = 0; i < listNum; i++)
    {
        Line2DPointList &pointList = m_Line2DShapePointList.front();
        //fill vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, pointList.bufLen, pointList.pointBuf, GL_STATIC_DRAW);
        // Use our shader
        glUseProgram(m_Line2DProID);

        glUniform4fv(m_Line2DColorID, 1, &m_Line2DColor[0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(m_Line2DVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
        glVertexAttribPointer(
            m_Line2DVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
        glDrawArrays(GL_LINE_STRIP, 0, pointList.bufLen / sizeof(float) / 2); // 12*3 indices starting at 0 -> 12 triangles
        glDisableVertexAttribArray(m_Line2DWholeVertexID);
        glUseProgram(0);
        m_Line2DShapePointList.pop_front();
        free(pointList.pointBuf);
    }
    m_iPointNum = 0;
    int result = 0;
    GLenum fbResult = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( fbResult != GL_FRAMEBUFFER_COMPLETE )
    {
        result = -1;
    }
#if 0
    sal_uInt8 *buf = (sal_uInt8 *)malloc(m_iWidth * m_iHeight * 3 + BMP_HEADER_LEN);
    CreateBMPHeader(buf, m_iWidth, m_iHeight);
    glReadPixels(0, 0, m_iWidth, m_iHeight, GL_BGR, GL_UNSIGNED_BYTE, buf + BMP_HEADER_LEN);
    FILE *pfile = fopen(fileName,"wb");
    fwrite(buf,m_iWidth * m_iHeight * 3 + BMP_HEADER_LEN, 1, pfile);
    free(buf);
    fclose(pfile);
#else
    boost::scoped_array<sal_uInt8> buf(new sal_uInt8[m_iWidth * m_iHeight * 4]);
    glReadPixels(0, 0, m_iWidth, m_iHeight, GL_BGR, GL_UNSIGNED_BYTE, buf.get());
    BitmapEx aBmp;
    aBmp.SetSizePixel(Size(m_iWidth, m_iHeight));

    Bitmap aBitmap( aBmp.GetBitmap() );
    Bitmap aAlpha( aBmp.GetAlpha().GetBitmap() );

    Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );
    Bitmap::ScopedWriteAccess pAlphaWriteAccess( aAlpha );

    size_t nCurPos = 0;
    for( size_t y = 0; y < m_iHeight; ++y)
    {
        Scanline pScan = pWriteAccess->GetScanline(y);
        Scanline pAlphaScan = pAlphaWriteAccess->GetScanline(y);

        for( size_t x = 0; x < m_iWidth; ++x )
        {
            *pScan++ = buf[nCurPos];
            *pScan++ = buf[nCurPos+1];
            *pScan++ = buf[nCurPos+2];

            nCurPos += 3;

            *pAlphaScan++ = static_cast<sal_uInt8>( 255 - buf[nCurPos++] );
        }
    }

    aBmp = BitmapEx(aBitmap, aAlpha);
    Graphic aGraphic(aBmp);
    uno::Reference< awt::XBitmap> xBmp( aGraphic.GetXGraphic(), uno::UNO_QUERY );
    uno::Reference < beans::XPropertySet > xPropSet ( mxRenderTarget, uno::UNO_QUERY );
    xPropSet->setPropertyValue("Graphic", uno::makeAny(aGraphic.GetXGraphic()));

#endif
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if defined( WNT )
    SwapBuffers(glWin.hDC);
    glFlush();
#elif defined( UNX )
    unx::glXSwapBuffers(glWin.dpy, glWin.win);
#endif
    RenderTexture(m_TextureObj[m_iFboIdx % 2]);
    m_iFboIdx++;
    return 0;
}

int OpenGLRender::RenderTexture2FBO(GLuint TexID)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
    glUseProgram(m_RenderProID);
    glEnableVertexAttribArray(m_RenderVertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderVertexBuf);
    glVertexAttribPointer(
        m_RenderVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );
    glEnableVertexAttribArray(m_RenderTexCoordID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderTexCoordBuf);
    glVertexAttribPointer(
        m_RenderTexCoordID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        2,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );
    glBindTexture(GL_TEXTURE_2D, TexID);
    glUniform1i(m_RenderTexID, 0);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableVertexAttribArray(m_RenderTexCoordID);
    glDisableVertexAttribArray(m_RenderVertexID);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDepthMask(GL_TRUE);
    return 0;
}


int OpenGLRender::RenderTexture(GLuint TexID)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_RenderProID);

    glEnableVertexAttribArray(m_RenderVertexID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderVertexBuf);
    glVertexAttribPointer(
        m_RenderVertexID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );
    glEnableVertexAttribArray(m_RenderTexCoordID);
    glBindBuffer(GL_ARRAY_BUFFER, m_RenderTexCoordBuf);
    glVertexAttribPointer(
        m_RenderTexCoordID,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        2,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );
    glBindTexture(GL_TEXTURE_2D, TexID);
    glUniform1i(m_RenderTexID, 0);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisableVertexAttribArray(m_RenderTexCoordID);
    glDisableVertexAttribArray(m_RenderVertexID);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
#if defined( WNT )
    SwapBuffers(glWin.hDC);
    glFlush();
#elif defined( UNX )
    unx::glXSwapBuffers(glWin.dpy, glWin.win);
#endif
    return 0;
}


int OpenGLRender::CreateTextureObj(int width, int height)
{
    glGenTextures(1, &m_TextureObj[0]);
    glBindTexture(GL_TEXTURE_2D, m_TextureObj[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_TextureObj[1]);
    glBindTexture(GL_TEXTURE_2D, m_TextureObj[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);
    return 0;
}

int OpenGLRender::CreateRenderObj(int width, int height)
{
    glGenRenderbuffers(1, &m_RboID[0]);
    CHECK_GL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, m_RboID[0]);
    CHECK_GL_ERROR();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    CHECK_GL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERROR();

    glGenRenderbuffers(1, &m_RboID[1]);
    CHECK_GL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, m_RboID[1]);
    CHECK_GL_ERROR();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    CHECK_GL_ERROR();
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERROR();
    return 0;
}

int OpenGLRender::SetViewPoint(PosVeci3 camPos, PosVeci3 orgPos, int headUpFlag)
{
    m_View = glm::lookAt(glm::vec3(camPos.x, camPos.y, camPos.z), // Camera is at (4,3,-3), in World Space
                         glm::vec3(orgPos.x, orgPos.y ,orgPos.z), // and looks at the origin
                         glm::vec3(0, (headUpFlag >= 0 ? 1 : -1) , 0)  // Head is up (set to 0,-1,0 to look upside-down)
                         );
    m_Projection = glm::perspective(45.0f, (float)m_iWidth / (float)m_iHeight, 0.1f, 100.0f);
    return 0;
}

int OpenGLRender::MoveModelf(PosVecf3 trans, PosVecf3 angle, PosVecf3 scale)
{
    m_TranslationMatrix = glm::translate(glm::vec3(trans.x, trans.y, trans.z));
    m_ScaleMatrix = glm::scale(glm::vec3(scale.x, scale.y, scale.z));
    m_RotationMatrix = glm::eulerAngleYXZ(angle.y, angle.x, angle.z);
    m_Model = m_TranslationMatrix * m_RotationMatrix * m_ScaleMatrix;
    return 0;
}


int OpenGLRender::CreateFrameBufferObj()
{
    GLenum status;
    // create a framebuffer object, you need to delete them when program exits.
    glGenFramebuffers(1, &m_FboID[0]);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[0]);
    glBindTexture(GL_TEXTURE_2D, m_TextureObj[0]);
    // attach a texture to FBO color attachement point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureObj[0], 0);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindTexture(GL_TEXTURE_2D, 0);
    // attach a renderbuffer to depth attachment point
    glBindRenderbuffer(GL_RENDERBUFFER, m_RboID[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RboID[0]);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &m_FboID[1]);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboID[1]);
    glBindTexture(GL_TEXTURE_2D, m_TextureObj[1]);
    // attach a texture to FBO color attachement point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureObj[1], 0);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindTexture(GL_TEXTURE_2D, 0);
    // attach a renderbuffer to depth attachment point
    glBindRenderbuffer(GL_RENDERBUFFER, m_RboID[1]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RboID[1]);
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

void OpenGLRender::Release()
{
    glDeleteBuffers(1, &m_VertexBuffer);
    glDeleteBuffers(1, &m_ColorBuffer);
    glDeleteProgram(m_ProgramID);
    glDeleteBuffers(1, &m_RenderVertexBuf);
    glDeleteBuffers(1, &m_RenderTexCoordBuf);
    glDeleteProgram(m_RenderProID);
    glDeleteFramebuffers(1, &m_FboID[0]);
    glDeleteFramebuffers(1, &m_FboID[1]);
    glDeleteTextures(1, &m_TextureObj[0]);
    glDeleteTextures(1, &m_TextureObj[1]);
    glDeleteRenderbuffers(1, &m_RboID[0]);
    glDeleteRenderbuffers(1, &m_RboID[1]);
#if defined( WNT )
    wglMakeCurrent(NULL, NULL);
    if (!m_iExternRC)
        wglDeleteContext(glWin.hRC);
    if (!m_iExternDC)
        ReleaseDC(glWin.hWnd, glWin.hDC);
#elif defined( UNX )

    glXMakeCurrent(glWin.dpy, None, NULL);
    if( glGetError() != GL_NO_ERROR ) {
        SAL_INFO("slideshow.opengl", "glError: " << (char *)gluErrorString(glGetError()));
    }
    glXDestroyContext(glWin.dpy, glWin.ctx);
    glWin.ctx = NULL;
    glWin.win = 0;

#endif
}


OpenGLRender::OpenGLRender(uno::Reference< drawing::XShape > xTarget)
{
    //[mod] by gaowei
    m_Model = glm::mat4(1.0f);
    m_TranslationMatrix = glm::translate(m_Model, glm::vec3(0.0f, 0.0f, 0.0f));
    m_ScaleMatrix = glm::scale(m_Model, glm::vec3(1.0f, 1.0f, 1.0f));
    m_RotationMatrix = glm::eulerAngleYXZ(0.0f, 0.0f, 0.0f);
    m_iWidth = 0;
    m_iHeight = 0;
    m_Line2DColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
    m_iPointNum = 0;
    memset(&m_Line2DPoitList, 0, sizeof(Line2DPointList));
    m_iFboIdx = 0;
    m_FboID[0] = 0;
    m_FboID[1] = 0;
    m_TextureObj[0] = 0;
    m_TextureObj[1] = 0;
    m_RboID[0] = 0;
    m_RboID[1] = 0;
    m_fLineAlpha = 1.0;

    mxRenderTarget = xTarget;
    mxRenderTarget->setPosition(awt::Point(0,0));
}
OpenGLRender::~OpenGLRender()
{
}

void OpenGLRender::SetWidth(int width)
{
    m_iWidth = width;
}
void OpenGLRender::SetHeight(int height)
{
    m_iHeight = height;
}

int OpenGLRender::GetWidth()
{
    return m_iWidth;
}
int OpenGLRender::GetHeight()
{
    return m_iHeight;
}
int OpenGLRender::CreateBMPHeader(sal_uInt8 *bmpHeader, int xsize, int ysize)
{
    unsigned char header[BMP_HEADER_LEN] = {
        0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
        54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0
    };

    long file_size = (long)xsize * (long)ysize * 3 + 54;
    header[2] = (unsigned char)(file_size &0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    long width = xsize;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) &0x000000ff;
    header[20] = (width >> 16) &0x000000ff;
    header[21] = (width >> 24) &0x000000ff;

    long height = -ysize;
    header[22] = height &0x000000ff;
    header[23] = (height >> 8) &0x000000ff;
    header[24] = (height >> 16) &0x000000ff;
    header[25] = (height >> 24) &0x000000ff;
    memcpy(bmpHeader, header, BMP_HEADER_LEN);
    return 0;

}

void OpenGLRender::SetLine2DColor(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b)
{
    m_Line2DColor = glm::vec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, m_fLineAlpha);
}

void OpenGLRender::SetLine2DWidth(int width)
{
    m_fLineWidth = (float)width / 10.0f;
    m_fLineWidth = (m_fLineWidth < 0.001) ? 0.001 : m_fLineWidth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
