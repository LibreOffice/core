/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_OPENGL_RENDER_HXX
#define VCL_OPENGL_RENDER_HXX

#if defined( MACOSX )
#elif defined( UNX )
#  include <prex.h>
#  include "GL/glxew.h"
#  include <postx.h>
#elif defined( _WIN32 )
#  include "prewin.h"
#  include "windows.h"
#  include "postwin.h"
#endif

#include <GL/glew.h>

#include <vcl/window.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/bitmapex.hxx>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <vcl/font.hxx>


#if defined( _WIN32 )
#include <GL/glext.h>
#include <GL/wglext.h>
#elif defined( MACOSX )
#elif defined( UNX )
#include <GL/glext.h>
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#include <GL/glxext.h>

#endif

// Include GLM
#include <list>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"

#define OPENGL_SCALE_VALUE 20
#define DEBUG_POSITIONING 0
#define RENDER_TO_FILE 0

struct PosVecf3
{
    float x;
    float y;
    float z;
};

typedef std::vector<GLfloat> Line2DPointList;

struct Bubble2DPointList
{
    float x;
    float y;
    float xScale;
    float yScale;
};

typedef std::vector<GLfloat> Bubble2DCircle;

struct RectanglePointList
{
    float points[12];
};

struct TextInfo
{
    GLuint texture;
    double rotation;
    float vertex[12];
    float nDx;
    float nDy;
};

typedef std::vector<GLfloat> Area2DPointList;
typedef std::vector<GLfloat> PieSegment2DPointList;
typedef std::vector<GLfloat> PointList;

/// Holds the information of our new child window
struct VCL_DLLPUBLIC GLWindow
{
#if defined( _WIN32 )
    HWND                    hWnd;
    HDC                     hDC;
    HGLRC                   hRC;
#elif defined( MACOSX )
#elif defined( UNX )
    Display*           dpy;
    int                     screen;
    XLIB_Window             win;
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    GLXFBConfig        fbc;
#endif
    XVisualInfo*       vi;
    GLXContext         ctx;

    bool HasGLXExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, (const GLubyte*) GLXExtensions ); }
    const char*             GLXExtensions;
#endif
    unsigned int            bpp;
    unsigned int            Width;
    unsigned int            Height;
    const GLubyte*          GLExtensions;

    bool HasGLExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, GLExtensions ); }

    GLWindow()
        :
#if defined( _WIN32 )
#elif defined( MACOSX )
#elif defined( UNX )
        dpy(NULL),
        screen(0),
        win(0),
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
        fbc(0),
#endif
        vi(NULL),
        ctx(0),
        GLXExtensions(NULL),
#endif
        bpp(0),
        Width(0),
        Height(0),
        GLExtensions(NULL)
    {
    }
};

class VCL_DLLPUBLIC OpenGLRender
{
public:
    OpenGLRender(com::sun::star::uno::Reference<
            com::sun::star::drawing::XShape > xTarget );
    ~OpenGLRender();
    int InitOpenGL(GLWindow);
    int MoveModelf(PosVecf3 trans, PosVecf3 angle, PosVecf3 scale);
    void SetSize(int width, int height);
    void Release();
#if RENDER_TO_FILE
    int CreateBMPHeader(sal_uInt8 *bmpHeader, int xsize, int ysize);
#endif
    int RenderLine2FBO(int wholeFlag);
    int SetLine2DShapePoint(float x, float y, int listLength);
    void SetLine2DColor(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b, sal_uInt8 nAlpha);
    void SetLine2DWidth(int width);
    BitmapEx GetAsBitmap();
#if defined( _WIN32 )
    bool InitMultisample(PIXELFORMATDESCRIPTOR pfd);
    bool GetMSAASupport();
    int GetMSAAFormat();
#endif
    void SetColor(sal_uInt32 color, sal_uInt8 nAlpha);
    int Bubble2DShapePoint(float x, float y, float directionX, float directionY);
    int RenderBubble2FBO(int wholeFlag);

    void prepareToRender();
    void renderToBitmap();

    int RenderRectangleShape(bool bBorder, bool bFill);
    int RectangleShapePoint(float x, float y, float directionX, float directionY);

    int CreateTextTexture(const BitmapEx& rBitmapEx,
            const com::sun::star::awt::Point& aPos, const com::sun::star::awt::Size& aSize,
            long rotation, const com::sun::star::drawing::HomogenMatrix3& rTrans);
    int RenderTextShape();

    int SetArea2DShapePoint(float x, float y, int listLength);
    int RenderArea2DShape();
    void SetChartTransparencyGradient(long transparencyGradient);

    void GeneratePieSegment2D(double, double, double, double);
    int RenderPieSegment2DShape(float, float, float);

    int RenderSymbol2DShape(float, float, float, float, sal_Int32);
#if DEBUG_POSITIONING
    void renderDebug();
#endif

    void SetBackGroundColor(sal_uInt32 color1, sal_uInt32 color2, sal_uInt8 nAlpha);
private:
    SAL_DLLPRIVATE GLint LoadShaders(const OUString& rVertexShaderName, const OUString& rFragmentShaderName);
    SAL_DLLPRIVATE int CreateTextureObj(int width, int height);
    SAL_DLLPRIVATE int CreateRenderObj(int width, int height);
    SAL_DLLPRIVATE int CreateFrameBufferObj();
#if defined( _WIN32 )
    SAL_DLLPRIVATE int InitTempWindow(HWND *hwnd, int width, int height, PIXELFORMATDESCRIPTOR inPfd);
    SAL_DLLPRIVATE bool WGLisExtensionSupported(const char *extension);
#endif
    SAL_DLLPRIVATE int CreateMultiSampleFrameBufObj();
    SAL_DLLPRIVATE int Create2DCircle(int detail);

private:
    // Projection matrix : default 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 m_Projection;
    // Camera matrix
    glm::mat4 m_View;
    // Model matrix : an identity matrix (model will be at the origin
    glm::mat4 m_Model;
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 m_MVP;

    GLuint m_VertexBuffer;

    GLuint m_ColorBuffer;

    GLint m_MatrixID;

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
#if defined( _WIN32 )
    int m_iArbMultisampleFormat;
#endif

    glm::vec4 m_2DColor;
    GLuint m_frameBufferMS;
    GLuint m_renderBufferColorMS;
    GLuint m_renderBufferDepthMS;

    Bubble2DCircle m_Bubble2DCircle;

    std::list <Bubble2DPointList> m_Bubble2DShapePointList;
    GLint m_CommonProID;
    GLint m_2DVertexID;
    GLint m_2DColorID;

    float m_fZStep;

    std::list <RectanglePointList> m_RectangleShapePointList;
    // add for text
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

    std::list <PieSegment2DPointList> m_PieSegment2DShapePointList;
    PointList m_Symbol2DPointList;
    std::list<PointList> m_Symbol2DShapePointList;

    GLuint m_SymbolProID;
    GLuint m_SymbolVertexID;
    GLuint m_SymbolMatrixID;
    GLuint m_SymbolColorID;
    GLuint m_SymbolShapeID;

#if DEBUG_POSITIONING
    GLuint m_DebugProID;
    GLuint m_DebugVertexID;
    GLuint m_DebugColorID;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
