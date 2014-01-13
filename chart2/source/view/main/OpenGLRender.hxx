/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <vcl/font.hxx>

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

#define OPENGL_SCALE_VALUE 20
#define DEBUG_POSITIONING 0

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

typedef std::vector<GLfloat> Line2DPointList;

typedef struct Bubble2DPointList
{
    float x;
    float y;
    float xScale;
    float yScale;
}Bubble2DPointList;

typedef struct Bubble2DCircle
{
    float *pointBuf;
    int bufLen;
}Bubble2DCircle;

typedef struct RectanglePointList
{
    float x;
    float y;
    float z;
    float xScale;
    float yScale;
}RectanglePointList;

typedef struct TextInfo
{
    GLuint texture;
    float x;
    float y;
    float z;
    double rotation;
    float vertex[8];
}TextInfo;

typedef std::vector<GLfloat> Area2DPointList;

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
    int MoveModelf(PosVecf3 trans, PosVecf3 angle, PosVecf3 scale);
    void SetWidth(int width);
    void SetHeight(int height);
    void Release();
    int CreateBMPHeader(sal_uInt8 *bmpHeader, int xsize, int ysize);
    int CreateBMPHeaderRGBA(sal_uInt8 *bmpHeader, int xsize, int ysize);
    int RenderLine2FBO(int wholeFlag);
    int SetLine2DShapePoint(float x, float y, int listLength);
    void SetLine2DColor(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b);
    void SetLine2DWidth(int width);
    BitmapEx GetAsBitmap();
#if defined( _WIN32 )
    bool InitMultisample(PIXELFORMATDESCRIPTOR pfd);
#endif
    bool GetMSAASupport();
    int GetMSAAFormat();
    void SetColor(sal_uInt32 color);
    int Bubble2DShapePoint(float x, float y, float directionX, float directionY);
    int RenderBubble2FBO(int wholeFlag);

    void prepareToRender();
    void renderToBitmap();


    void SetTransparency(sal_uInt32 transparency);
    int RenderRectangleShape();
    int RectangleShapePoint(float x, float y, float directionX, float directionY);

    int CreateTextTexture(::rtl::OUString textValue, sal_uInt32 color, const Font& rFont,
            com::sun::star::awt::Point aPos, com::sun::star::awt::Size aSize, long rotation);
    int RenderTextShape();

    int SetArea2DShapePoint(float x, float y, int listLength);
    int RenderArea2DShape();
    void SetChartTransparencyGradient(long transparencyGradient);

#if DEBUG_POSITIONING
    void renderDebug();
#endif

private:
    GLint LoadShaders(const char *vertexShader,const char *fragmentShader);
    int CreateTextureObj(int width, int height);
    int CreateRenderObj(int width, int height);
    int CreateFrameBufferObj();
    int RenderTexture(GLuint TexID);
    int RenderTexture2FBO(GLuint TexID);
#if defined( _WIN32 )
    int InitTempWindow(HWND *hwnd, int width, int height, PIXELFORMATDESCRIPTOR inPfd);
    bool WGLisExtensionSupported(const char *extension);
#endif
    int CreateMultiSampleFrameBufObj();
    int Create2DCircle(int detail);
    void SetBackGroundColor(sal_uInt32 color1, sal_uInt32 color2);

private:
    // Projection matrix : default 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 m_Projection;
    // Camera matrix
    glm::mat4 m_View;
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 m_Model;
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 m_MVP;

    GLint m_RenderProID;

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

    GLuint m_TextureObj;

    GLuint m_FboID;

    GLuint m_RboID;

    int m_iWidth;

    int m_iHeight;

    GLWindow glWin;

    Line2DPointList m_Line2DPointList;

    float m_fLineWidth;

    std::list <Line2DPointList> m_Line2DShapePointList;

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxRenderTarget;
    bool mbArbMultisampleSupported;
    int m_iArbMultisampleFormat;
    GLint m_iSampleBufs;
    GLint m_iSamples;

    glm::vec4 m_2DColor;
    GLuint m_frameBufferMS;
    GLuint m_renderBufferColorMS;
    GLuint m_renderBufferDepthMS;

    Bubble2DCircle m_Bubble2DCircle;

    Bubble2DPointList m_Bubble2DPointList;

    std::list <Bubble2DPointList> m_Bubble2DShapePointList;
    GLint m_CommonProID;
    GLint m_2DVertexID;
    GLint m_2DColorID;

    float m_fZStep;

    float m_fAlpha;

    RectanglePointList m_RectangleList;

    std::list <RectanglePointList> m_RectangleShapePointList;
    // add for text
    TextInfo m_TextInfo;
    std::list <TextInfo> m_TextInfoList;
    GLint m_TextProID;
    GLint m_TextMatrixID;
    GLint m_TextVertexID;
    GLint m_TextTexCoordID;
    GLuint m_TextTexCoordBuf;
    GLint m_TextTexID;

    Area2DPointList m_Area2DPointList;
    std::list <Area2DPointList> m_Area2DShapePointList;

    GLint m_BackgroundProID;
    GLint m_BackgroundMatrixID;
    GLint m_BackgroundVertexID;
    GLint m_BackgroundColorID;

    float m_BackgroundColor[16];
    glm::vec4 m_ClearColor;

#if DEBUG_POSITIONING
    GLuint m_DebugProID;
    GLuint m_DebugVertexID;
    GLuint m_DebugColorID;
#endif

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
