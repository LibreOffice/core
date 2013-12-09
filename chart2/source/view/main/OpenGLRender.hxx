/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/XDrawPage.hpp>

#if defined( _WIN32 )
#include "prewin.h"
#include "windows.h"
#include "postwin.h"
#endif


#include <GL/gl.h>
#include <GL/glu.h>
#include <vcl/window.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#if defined( _WIN32 )
    #include <GL/glu.h>
    #include <GL/glext.h>
    #include <GL/wglext.h>
#elif defined( MACOSX )
    #include "premac.h"
    #include <Cocoa/Cocoa.h>
    #include "postmac.h"
#elif defined( UNX )
    #include <GL/glu.h>
    #include <GL/glext.h>

namespace unx
{
    #include <X11/keysym.h>
    #include <X11/X.h>
    #define GLX_GLXEXT_PROTOTYPES 1
    #include <GL/glx.h>
    #include <GL/glxext.h>
}
#endif

// Include GLM
#include <list>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"
#define BMP_HEADER_LEN 54

using namespace std;

typedef struct PosVeci3
{
    int x;
    int y;
    int z;
}PosVeci3;
typedef struct PosVecf3
{
    float x;
    float y;
    float z;
}PosVecf3;

typedef struct Line2DPointList
{
    float *pointBuf;;
    int bufLen;
}Line2DPointList;

/// Holds the information of our new child window
struct GLWindow
{
#if defined( _WIN32 )
    HWND                    hWnd;
    HDC                     hDC;
    HGLRC                   hRC;
#elif defined( MACOSX )
#elif defined( UNX )
    unx::Display*           dpy;
    int                     screen;
    unx::Window             win;
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::GLXFBConfig        fbc;
#endif
    unx::XVisualInfo*       vi;
    unx::GLXContext         ctx;

    bool HasGLXExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, (const GLubyte*) GLXExtensions ); }
    const char*             GLXExtensions;
#endif
    unsigned int            bpp;
    unsigned int            Width;
    unsigned int            Height;
    const GLubyte*          GLExtensions;

    bool HasGLExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, GLExtensions ); }
};

class OpenGLRender
{
public:
    OpenGLRender(com::sun::star::uno::Reference<
            com::sun::star::drawing::XShape > xTarget );
    ~OpenGLRender();
    int InitOpenGL(GLWindow);
    int SetViewPoint(PosVeci3 camPos, PosVeci3 orgPos, int headUpFlag);
    int MoveModelf(PosVecf3 trans, PosVecf3 angle, PosVecf3 scale);
    int RenderModelf(float *vertexArray, unsigned int vertexArraySize, float *colorArray, unsigned int colorArraySize);
    int RenderModelf2FBO(float *vertexArray, unsigned int vertexArraySize, float *colorArray, unsigned int colorArraySize);
    void SetWidth(int width);
    void SetHeight(int height);
    int GetWidth();
    int GetHeight();
    void Release();
    int CreateBMPHeader(sal_uInt8 *bmpHeader, int xsize, int ysize);
    int RenderLine2FBO(int wholeFlag);
    int SetLine2DShapePoint(float x, float y, int listLength);
    void SetLine2DColor(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b);
    void SetLine2DWidth(int width);
private:
    GLint LoadShaders(const char *vertexShader,const char *fragmentShader);
    int CreateTextureObj(int width, int height);
    int CreateRenderObj(int width, int height);
    int CreateFrameBufferObj();
    int RenderTexture(GLuint TexID);
    int RenderTexture2FBO(GLuint TexID);
private:
    // Projection matrix : default 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 m_Projection;
    // Camera matrix
    glm::mat4 m_View;
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 m_Model;
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 m_MVP;

    glm::mat4 m_TranslationMatrix;

    glm::mat4 m_RotationMatrix;

    glm::mat4 m_ScaleMatrix;

    GLuint m_ProgramID;

    GLint m_RenderProID;

    GLint m_Line2DProID;

    glm::vec4 m_Line2DColor;

    GLuint m_VertexBuffer;

    GLuint m_ColorBuffer;

    GLint m_MatrixID;

    GLint m_VertexID;

    GLint m_ColorID;

    GLint m_RenderVertexID;

    GLint m_RenderTexCoordID;

    GLint m_Line2DVertexID;

    GLint m_Line2DWholeVertexID;

    GLint m_Line2DColorID;

    GLint m_RenderTexID;

    GLuint m_RenderVertexBuf;

    GLuint m_RenderTexCoordBuf;

    GLuint m_TextureObj[2];

    GLuint m_FboID[2];

    GLuint m_RboID[2];

    int m_iWidth;

    int m_iHeight;

    GLWindow glWin;

    int m_iExternDC;

    int m_iExternRC;

    int m_iPointNum;

    Line2DPointList m_Line2DPoitList;

    int m_iFboIdx;

    float m_fLineWidth;

    float m_fLineAlpha;

    list <Line2DPointList> m_Line2DShapePointList;

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxRenderTarget;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
