/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ogl_spritedevicehelper.hxx"
#include "ogl_spritecanvas.hxx"
#include "ogl_canvasbitmap.hxx"
#include "ogl_canvastools.hxx"
#include "ogl_canvascustomsprite.hxx"
#include "ogl_texturecache.hxx"

#include <canvas/verbosetrace.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/unopolypolygon.hxx>

#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>

#include <vcl/sysdata.hxx>
#include <vcl/syschild.hxx>
#include <vcl/canvastools.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

namespace unx
{
 #include <X11/keysym.h>
 #include <X11/X.h>
 #include <GL/glx.h>
 #include <GL/glxext.h>
}


using namespace ::com::sun::star;

static bool lcl_bErrorTriggered=false;
static int lcl_XErrorHandler( unx::Display*, unx::XErrorEvent* )
{
    lcl_bErrorTriggered = true;
    return 0;
}

/** Dummy vertex processing. Simply uses default pipeline for vertex
   transformation, and forwards texture coodinates to fragment shader
 */
static const char dummyVertexShader[] =
{
    "varying vec2 v_textureCoords2d;                                            "
    "void main(void)                                                            "
    "{                                                                          "
    "    gl_Position = ftransform();                                            "
    "    v_textureCoords2d = gl_MultiTexCoord0.st;                              "
    "}                                                                          "
};

/** Two-color linear gradient
 */
static const char linearTwoColorGradientFragmentShader[] =
{
    "#version 120                                                            \n"
    "uniform vec4   v_startColor4d;                                            "
    "uniform vec4   v_endColor4d;                                              "
    "uniform mat3x2 m_transform;                                               "
    "varying vec2   v_textureCoords2d;                                         "
    "void main(void)                                                           "
    "{                                                                         "
    "    gl_FragColor = mix(v_startColor4d,                                    "
    "                       v_endColor4d,                                      "
    "                       clamp(                                             "
    "                          (m_transform * vec3(v_textureCoords2d,1)).s,    "
    "                          0.0, 1.0));                                     "
    "}                                                                         "
};

/** N-color linear gradient
 */
static const char linearMultiColorGradientFragmentShader[] =
{
    "#version 120                                                            \n"
    "uniform int       i_nColors;                                              "
    "uniform sampler1D t_colorArray4d;                                         "
    "uniform sampler1D t_stopArray1d;                                          "
    "uniform mat3x2    m_transform;                                            "
    "varying vec2      v_textureCoords2d;                                      "
    "                                                                          "
    "int findBucket(float t)                                                   "
    "{                                                                         "
    "    int nMinBucket=0;                                                     "
    "    while( nMinBucket < i_nColors &&                                      "
    "           texture1D(t_stopArray1d, nMinBucket).s < t )                   "
    "        ++nMinBucket;                                                     "
    "    return max(nMinBucket-1,0);                                           "
    "}                                                                         "
    "                                                                          "
    "void main(void)                                                           "
    "{                                                                         "
    "    const float fAlpha =                                                  "
    "        clamp( (m_transform * vec3(v_textureCoords2d,1)).s,               "
    "               0.0, 1.0 );                                                "
    "                                                                          "
    "    const int nMinBucket=findBucket( fAlpha );                            "
    "                                                                          "
    "    const float fLerp =                                                   "
    "        (fAlpha-texture1D(t_stopArray1d, nMinBucket).s) /                 "
    "        (texture1D(t_stopArray1d, nMinBucket+1).s -                       "
    "         texture1D(t_stopArray1d, nMinBucket).s);                         "
    "                                                                          "
    "    gl_FragColor = mix(texture1D(t_colorArray4d, nMinBucket),             "
    "                       texture1D(t_colorArray4d, nMinBucket+1),           "
    "                       fLerp);                                            "
    "}                                                                         "
};

/** Two-color radial gradient
 */
static const char radialTwoColorGradientFragmentShader[] =
{
    "#version 120                                                             \n"
    "uniform vec4   v_startColor4d;                                             "
    "uniform vec4   v_endColor4d;                                               "
    "uniform mat3x2 m_transform;                                                "
    "varying vec2   v_textureCoords2d;                                          "
    "const vec2     v_center2d = vec2(0,0);                                     "
    "void main(void)                                                            "
    "{                                                                          "
    "    gl_FragColor = mix(v_startColor4d,                                     "
    "                       v_endColor4d,                                       "
    "                       1.0 - distance(                                     "
    "                          vec2(                                            "
    "                             m_transform * vec3(v_textureCoords2d,1)),     "
    "                          v_center2d));                                    "
    "}                                                                          "
};

/** Multi-color radial gradient
 */
static const char radialMultiColorGradientFragmentShader[] =
{
    "#version 120                                                             \n"
    "uniform int       i_nColors;                                              "
    "uniform sampler1D t_colorArray4d;                                         "
    "uniform sampler1D t_stopArray1d;                                          "
    "uniform mat3x2    m_transform;                                            "
    "varying vec2      v_textureCoords2d;                                      "
    "const vec2        v_center2d = vec2(0,0);                                 "
    "                                                                          "
    "int findBucket(float t)                                                   "
    "{                                                                         "
    "    int nMinBucket=0;                                                     "
    "    while( nMinBucket < i_nColors &&                                      "
    "           texture1D(t_stopArray1d, nMinBucket).s < t )                   "
    "        ++nMinBucket;                                                     "
    "    return max(nMinBucket-1,0);                                           "
    "}                                                                         "
    "                                                                          "
    "void main(void)                                                           "
    "{                                                                         "
    "    const float fAlpha =                                                  "
    "        clamp( 1.0 - distance(                                            "
    "               vec2( m_transform * vec3(v_textureCoords2d,1)),            "
    "                     v_center2d),                                         "
    "               0.0, 1.0 );                                                "
    "                                                                          "
    "    const int nMinBucket=findBucket( fAlpha );                            "
    "                                                                          "
    "    const float fLerp =                                                   "
    "        (fAlpha-texture1D(t_stopArray1d, nMinBucket).s) /                 "
    "        (texture1D(t_stopArray1d, nMinBucket+1).s -                       "
    "         texture1D(t_stopArray1d, nMinBucket).s);                         "
    "                                                                          "
    "    gl_FragColor = mix(texture1D(t_colorArray4d, nMinBucket),             "
    "                       texture1D(t_colorArray4d, nMinBucket+1),           "
    "                       fLerp);                                            "
    "}                                                                         "
};

/** Two-color rectangular gradient
 */
static const char rectangularTwoColorGradientFragmentShader[] =
{
    "#version 120                                                             \n"
    "uniform vec4   v_startColor4d;                                             "
    "uniform vec4   v_endColor4d;                                               "
    "uniform mat3x2 m_transform;                                                "
    "varying vec2   v_textureCoords2d;                                          "
    "void main(void)                                                            "
    "{                                                                          "
    "    const vec2 v = abs( vec2(m_transform * vec3(v_textureCoords2d,1)) );   "
    "    const float t = max(v.x, v.y);                                         "
    "    gl_FragColor = mix(v_startColor4d,                                     "
    "                       v_endColor4d,                                       "
    "                       1.0-t);                                             "
    "}                                                                          "
};

/** Multi-color rectangular gradient
 */
static const char rectangularMultiColorGradientFragmentShader[] =
{
    "#version 120                                                             \n"
    "uniform int       i_nColors;                                              "
    "uniform sampler1D t_colorArray4d;                                         "
    "uniform sampler1D t_stopArray1d;                                          "
    "uniform mat3x2    m_transform;                                            "
    "varying vec2      v_textureCoords2d;                                      "
    "                                                                          "
    "int findBucket(float t)                                                   "
    "{                                                                         "
    "    int nMinBucket=0;                                                     "
    "    while( nMinBucket < i_nColors &&                                      "
    "           texture1D(t_stopArray1d, nMinBucket).s < t )                   "
    "        ++nMinBucket;                                                     "
    "    return max(nMinBucket-1,0);                                           "
    "}                                                                         "
    "                                                                          "
    "void main(void)                                                           "
    "{                                                                         "
    "    const vec2  v = abs( vec2(m_transform * vec3(v_textureCoords2d,1)) ); "
    "    const float fAlpha = 1 - max(v.x, v.y);                               "
    "                                                                          "
    "    const int nMinBucket=findBucket( fAlpha );                            "
    "                                                                          "
    "    const float fLerp =                                                   "
    "        (fAlpha-texture1D(t_stopArray1d, nMinBucket).s) /                 "
    "        (texture1D(t_stopArray1d, nMinBucket+1).s -                       "
    "         texture1D(t_stopArray1d, nMinBucket).s);                         "
    "                                                                          "
    "    gl_FragColor = mix(texture1D(t_colorArray4d, nMinBucket),             "
    "                       texture1D(t_colorArray4d, nMinBucket+1),           "
    "                       fLerp);                                            "
    "}                                                                         "
};

static void initContext()
{
    // need the backside for mirror effects
    glDisable(GL_CULL_FACE);

    // no perspective, we're 2D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // misc preferences
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
    glShadeModel(GL_FLAT);
}

static void initTransformation(const ::Size& rSize, bool bMirror=false)
{
    // use whole window
    glViewport( 0,0,
                (GLsizei)rSize.Width(),
                (GLsizei)rSize.Height() );

    // model coordinate system is already in device pixel
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(-1.0, (bMirror ? -1.0 : 1.0), 0.0);
    glScaled( 2.0  / rSize.Width(),
              (bMirror ? 2.0 : -2.0) / rSize.Height(),
              1.0 );

    // clear to black
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static boost::shared_ptr<SystemChildWindow> createChildWindow( unx::XVisualInfo*& viWin,
                                                               unx::XVisualInfo*& viPB,
                                                               void*&             fbConfig,
                                                               Window&            rWindow,
                                                               unx::Display*      pDisplay,
                                                               int                nScreen )
{
    // select appropriate visual
    static int winAttrList3[] =
        {
            GLX_RGBA,//only TrueColor or DirectColor
            //single buffered
            GLX_RED_SIZE,4,//use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,//use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,//use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,0,//no depth buffer
            None
        };
    static int pBufAttrList3[] =
        {
            GLX_DOUBLEBUFFER,False,// never doublebuffer pbuffer
            GLX_RED_SIZE,4,//use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,//use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,//use the maximum blue bits, with a minimum of 4 bits
            GLX_ALPHA_SIZE,4,
            GLX_DEPTH_SIZE,0,//no depth buffer
            GLX_RENDER_TYPE,   GLX_RGBA_BIT,
            GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT | GLX_WINDOW_BIT,
            None
        };
    static int winAttrList2[] =
        {
            GLX_RGBA,//only TrueColor or DirectColor
            /// single buffered
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,1,/// use the maximum depth bits, making sure there is a depth buffer
            None
        };
    static int pBufAttrList2[] =
        {
            GLX_DOUBLEBUFFER,False,// never doublebuffer pbuffer
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_ALPHA_SIZE,4,
            GLX_DEPTH_SIZE,1,/// use the maximum depth bits, making sure there is a depth buffer
            GLX_RENDER_TYPE,   GLX_RGBA_BIT,
            GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT | GLX_WINDOW_BIT,
            None
        };
    static int winAttrList1[] =
        {
            GLX_RGBA,//only TrueColor or DirectColor
            GLX_DOUBLEBUFFER,/// only double buffer
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,0,/// no depth buffer
            None
        };
    static int pBufAttrList1[] =
        {
            GLX_DOUBLEBUFFER,False,// never doublebuffer pbuffer
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_ALPHA_SIZE,4,
            GLX_DEPTH_SIZE,0,/// no depth buffer
            GLX_RENDER_TYPE,   GLX_RGBA_BIT,
            GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT | GLX_WINDOW_BIT,
            None
        };
    static int winAttrList0[] =
        {
            GLX_RGBA,//only TrueColor or DirectColor
            GLX_DOUBLEBUFFER,// only double buffer
            GLX_RED_SIZE,4,// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,// use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,1,// use the maximum depth bits, making sure there is a depth buffer
            None
        };
    static int pBufAttrList0[] =
        {
            GLX_DOUBLEBUFFER,False,// never doublebuffer pbuffer
            GLX_RED_SIZE,4,// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,// use the maximum blue bits, with a minimum of 4 bits
            GLX_ALPHA_SIZE,4,
            GLX_DEPTH_SIZE,1,// use the maximum depth bits, making sure there is a depth buffer
            GLX_RENDER_TYPE,   GLX_RGBA_BIT,
            GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT | GLX_WINDOW_BIT,
            None
        };
    static int* winAttrTable[] =
        {
            winAttrList0,
            winAttrList1,
            winAttrList2,
            winAttrList3,
            NULL
        };
    static int* pBufAttrTable[] =
        {
            pBufAttrList0,
            pBufAttrList1,
            pBufAttrList2,
            pBufAttrList3,
            NULL
        };
    int** pWinAttributeTable = winAttrTable;
    int** pBufAttributeTable = pBufAttrTable;

    boost::shared_ptr<SystemChildWindow> pResult;
    unx::GLXFBConfig* fbConfigs=NULL;
    int nConfigs, nVal;
    while( *pWinAttributeTable && *pBufAttributeTable )
    {
        // try to find a window visual for the current set of
        // attributes
        viWin = unx::glXChooseVisual( pDisplay,
                                      nScreen,
                                      *pWinAttributeTable );
        if( viWin )
        {
            // try to find a framebuffer config for the current set of
            // attributes
            fbConfigs = glXChooseFBConfig( pDisplay,
                                           nScreen,
                                           *pBufAttributeTable,
                                           &nConfigs );
            // don't use glXGetFBConfigs, that does not list alpha-configs
            // fbConfigs = unx::glXGetFBConfigs(pDisplay, nScreen, &nConfigs);
            for(int i=0; i<nConfigs; i++)
            {
                viPB = glXGetVisualFromFBConfig(pDisplay, fbConfigs[i]);
                if( viPB && viPB->visualid != viWin->visualid )
                {
                    glXGetFBConfigAttrib(pDisplay,
                                         fbConfigs[i],
                                         GLX_DRAWABLE_TYPE,
                                         &nVal);

                    if( (GLX_PBUFFER_BIT|GLX_WINDOW_BIT|GLX_PIXMAP_BIT)
                        == (nVal & (GLX_PBUFFER_BIT|GLX_WINDOW_BIT|GLX_PIXMAP_BIT)) )
                    {
                        SystemWindowData winData;
                        winData.nSize = sizeof(winData);
                        SAL_INFO("canvas.ogl", "using VisualID " << viWin->visualid << " for OpenGL canvas");
                        winData.pVisual = (void*)(viWin->visual);
                        pResult.reset( new SystemChildWindow(&rWindow, 0, &winData, false) );

                        if( pResult->GetSystemData() )
                        {
                            fbConfig = &fbConfigs[i];
                            return pResult;
                        }

                        pResult.reset();
                    }

                    XFree(viPB);
                }
            }

            XFree(viWin);
        }

        ++pWinAttributeTable;
        ++pBufAttributeTable;
    }

    return pResult;
}


namespace oglcanvas
{
    /** Compile shader program

        Code courtesy rodo
     */
    void SpriteDeviceHelper::compileShader(unsigned int& o_rShaderHandle,
                                           unsigned int  eShaderType,
                                           const char*   pShaderSourceCode)
    {
        GLint nCompileStatus;
        char log[1024];

        o_rShaderHandle = glCreateShader( eShaderType );
        glShaderSource( o_rShaderHandle, 1, &pShaderSourceCode, NULL );
        glCompileShader( o_rShaderHandle );
        glGetShaderInfoLog( o_rShaderHandle, sizeof(log), NULL, log );
        SAL_INFO("canvas.ogl", "shader compile log: " << log);

        glGetShaderiv( o_rShaderHandle, GL_COMPILE_STATUS, &nCompileStatus );
        if( !nCompileStatus )
        {
            glDeleteShader(o_rShaderHandle);
            o_rShaderHandle=0;
        }
    }

    /** Link vertex & fragment shaders

        Code courtesy rodo
     */
    void SpriteDeviceHelper::linkShaders(unsigned int& o_rProgramHandle,
                                         unsigned int  nVertexProgramId,
                                         unsigned int  nFragmentProgramId)
    {
        if( !nVertexProgramId || !nFragmentProgramId )
            return;

        o_rProgramHandle = glCreateProgram();
        glAttachShader( o_rProgramHandle, nVertexProgramId );
        glAttachShader( o_rProgramHandle, nFragmentProgramId );

        char log[1024];
        GLint nProgramLinked;

        glLinkProgram( o_rProgramHandle );
        glGetProgramInfoLog( o_rProgramHandle, sizeof(log), NULL, log );
        SAL_INFO("canvas.ogl", "shader program link log: " << log);
        glGetProgramiv( o_rProgramHandle, GL_LINK_STATUS, &nProgramLinked );

        if( !nProgramLinked )
        {
            glDeleteProgram(o_rProgramHandle);
            o_rProgramHandle=0;
        }
    }

    SpriteDeviceHelper::SpriteDeviceHelper() :
        mpDevice(NULL),
        mpSpriteCanvas(NULL),
        maActiveSprites(),
        maLastUpdate(),
        mpChildWindow(),
        mpDisplay(NULL),
        mpGLContext(NULL),
        mpGLPBufContext(NULL),
        mpFBConfig(NULL),
        mpTextureCache(new TextureCache()),
        mnDummyVertexProgram(0),
        mnLinearTwoColorGradientFragmentProgram(0),
        mnLinearMultiColorGradientFragmentProgram(0),
        mnRadialTwoColorGradientFragmentProgram(0),
        mnRadialMultiColorGradientFragmentProgram(0),
        mnRectangularTwoColorGradientFragmentProgram(0),
        mnRectangularMultiColorGradientFragmentProgram(0),
        mnLinearTwoColorGradientProgram(0),
        mnLinearMultiColorGradientProgram(0),
        mnRadialTwoColorGradientProgram(0),
        mnRadialMultiColorGradientProgram(0),
        mnRectangularTwoColorGradientProgram(0),
        mnRectangularMultiColorGradientProgram(0)
    {}

    void SpriteDeviceHelper::init( Window&               rWindow,
                                   SpriteCanvas&         rSpriteCanvas,
                                   const awt::Rectangle& rViewArea )
    {
        mpSpriteCanvas = &rSpriteCanvas;

        rSpriteCanvas.setWindow(
            uno::Reference<awt::XWindow2>(
                VCLUnoHelper::GetInterface(&rWindow),
                uno::UNO_QUERY_THROW) );

        // init OpenGL
        const SystemEnvData* sysData(rWindow.GetSystemData());
        unx::Display* pDisplay=reinterpret_cast<unx::Display*>(sysData->pDisplay);
        mpDisplay=pDisplay;
        if( !unx::glXQueryExtension(pDisplay, NULL, NULL) )
            return;

        unx::Window xWindow = sysData->aWindow;
        unx::XWindowAttributes xAttr;
        unx::XGetWindowAttributes( pDisplay, xWindow, &xAttr );
        int nScreen = XScreenNumberOfScreen( xAttr.screen );

        unx::Window childXWindow=0;
        unx::XVisualInfo* viWin=NULL;
        unx::XVisualInfo* viPB=NULL;
        mpChildWindow=createChildWindow(viWin,viPB,mpFBConfig,
                                        rWindow,pDisplay,nScreen);

        // tweak SysChild window to act as an input-transparent
        // overlay
        if( mpChildWindow )
        {
            childXWindow=mpChildWindow->GetSystemData()->aWindow;
            mpChildWindow->SetMouseTransparent(true);
            mpChildWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            mpChildWindow->EnableEraseBackground(false);
            mpChildWindow->SetControlForeground();
            mpChildWindow->SetControlBackground();
            mpChildWindow->EnablePaint(false);

            unx::GLXContext pContext1 =
                glXCreateContext(pDisplay,
                                 viWin,
                                 0,
                                 GL_TRUE);
            mpGLContext = pContext1;

            unx::GLXContext pContext2 =
                glXCreateContext( pDisplay,
                                  viPB,
                                  pContext1,
                                  GL_TRUE );
            mpGLPBufContext = pContext2;

            XFree(viWin);
            XFree(viPB);

            if( !glXMakeCurrent( pDisplay,
                                 childXWindow,
                                 pContext1) )
            {
                glXDestroyContext(pDisplay, pContext1);
                glXDestroyContext(pDisplay, pContext2);
                throw lang::NoSupportException("Could not select OpenGL context!", NULL);
            }

            const GLubyte* extensions=glGetString( GL_EXTENSIONS );
            if( gluCheckExtension((const GLubyte*)"GLX_SGI_swap_control", extensions) )
            {
                // try to enable vsync
                typedef GLint (*glXSwapIntervalProc)(GLint);
                glXSwapIntervalProc glXSwapInterval =
                    (glXSwapIntervalProc) unx::glXGetProcAddress((const GLubyte*)"glXSwapIntervalSGI");
                if( glXSwapInterval )
                {
                    int (*oldHandler)(unx::Display*, unx::XErrorEvent*);

                    // synchronize on global mutex - no other ogl
                    // canvas instance permitted to enter here
                    {
                        ::osl::MutexGuard aGuard( *::osl::Mutex::getGlobalMutex() );

                        // replace error handler temporarily
                        oldHandler = unx::XSetErrorHandler( lcl_XErrorHandler );

                        lcl_bErrorTriggered = false;

                        // Note: if this fails, so be it. Buggy
                        // drivers will then not have vsync.
                        glXSwapInterval(1);

                        // sync so that we possibly get an XError
                        unx::glXWaitGL();
                        XSync(pDisplay, false);

                        unx::XSetErrorHandler( oldHandler );
                    }
                }
            }

            // init window context
            initContext();

            // compile & link shaders - code courtesy rodo
            compileShader(mnDummyVertexProgram,
                          GL_VERTEX_SHADER,
                          dummyVertexShader);
            compileShader(mnLinearTwoColorGradientFragmentProgram,
                          GL_FRAGMENT_SHADER,
                          linearTwoColorGradientFragmentShader);
            compileShader(mnLinearMultiColorGradientFragmentProgram,
                          GL_FRAGMENT_SHADER,
                          linearMultiColorGradientFragmentShader);
            compileShader(mnRadialTwoColorGradientFragmentProgram,
                          GL_FRAGMENT_SHADER,
                          radialTwoColorGradientFragmentShader);
            compileShader(mnRadialMultiColorGradientFragmentProgram,
                          GL_FRAGMENT_SHADER,
                          radialMultiColorGradientFragmentShader);
            compileShader(mnRectangularTwoColorGradientFragmentProgram,
                          GL_FRAGMENT_SHADER,
                          rectangularTwoColorGradientFragmentShader);
            compileShader(mnRectangularMultiColorGradientFragmentProgram,
                          GL_FRAGMENT_SHADER,
                          rectangularMultiColorGradientFragmentShader);
            linkShaders(mnLinearTwoColorGradientProgram,
                        mnDummyVertexProgram,
                        mnLinearTwoColorGradientFragmentProgram);
            linkShaders(mnLinearMultiColorGradientProgram,
                        mnDummyVertexProgram,
                        mnLinearMultiColorGradientFragmentProgram);
            linkShaders(mnRadialTwoColorGradientProgram,
                        mnDummyVertexProgram,
                        mnRadialTwoColorGradientFragmentProgram);
            linkShaders(mnRadialMultiColorGradientProgram,
                        mnDummyVertexProgram,
                        mnRadialMultiColorGradientFragmentProgram);
            linkShaders(mnRectangularTwoColorGradientProgram,
                        mnDummyVertexProgram,
                        mnRectangularTwoColorGradientFragmentProgram);
            linkShaders(mnRectangularMultiColorGradientProgram,
                        mnDummyVertexProgram,
                        mnRectangularMultiColorGradientFragmentProgram);

            glXMakeCurrent(pDisplay, None, NULL);
        }

        if( !mpGLContext || glGetError() != GL_NO_ERROR )
            throw lang::NoSupportException(
                "Could not create OpenGL context, or an error occurred doing so!", NULL);

        notifySizeUpdate(rViewArea);
        mpChildWindow->Show();
        // TODO(E3): check for GL_ARB_imaging extension
    }

    void SpriteDeviceHelper::disposing()
    {
        // release all references
        mpSpriteCanvas = NULL;
        mpDevice = NULL;
        mpTextureCache.reset();

        if( mpGLContext )
        {
            glDeleteProgram( mnRectangularTwoColorGradientProgram );
            glDeleteProgram( mnRectangularMultiColorGradientProgram );
            glDeleteProgram( mnRadialTwoColorGradientProgram );
            glDeleteProgram( mnRadialMultiColorGradientProgram );
            glDeleteProgram( mnLinearTwoColorGradientProgram );
            glDeleteProgram( mnLinearMultiColorGradientProgram );
            glDeleteShader( mnRectangularTwoColorGradientFragmentProgram );
            glDeleteShader( mnRectangularMultiColorGradientFragmentProgram );
            glDeleteShader( mnRadialTwoColorGradientFragmentProgram );
            glDeleteShader( mnRadialMultiColorGradientFragmentProgram );
            glDeleteShader( mnLinearTwoColorGradientFragmentProgram );
            glDeleteShader( mnLinearMultiColorGradientFragmentProgram );
            glDeleteShader( mnDummyVertexProgram );

            glXDestroyContext(reinterpret_cast<unx::Display*>(mpDisplay),
                              reinterpret_cast<unx::GLXContext>(mpGLContext));
        }

        mpDisplay = NULL;
        mpGLContext = NULL;
        mpChildWindow.reset();
    }

    geometry::RealSize2D SpriteDeviceHelper::getPhysicalResolution()
    {
        if( !mpChildWindow )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map a one-by-one millimeter box to pixel
        const MapMode aOldMapMode( mpChildWindow->GetMapMode() );
        mpChildWindow->SetMapMode( MapMode(MAP_MM) );
        const Size aPixelSize( mpChildWindow->LogicToPixel(Size(1,1)) );
        mpChildWindow->SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aPixelSize );
    }

    geometry::RealSize2D SpriteDeviceHelper::getPhysicalSize()
    {
        if( !mpChildWindow )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map the pixel dimensions of the output window to millimeter
        const MapMode aOldMapMode( mpChildWindow->GetMapMode() );
        mpChildWindow->SetMapMode( MapMode(MAP_MM) );
        const Size aLogSize( mpChildWindow->PixelToLogic(mpChildWindow->GetOutputSizePixel()) );
        mpChildWindow->SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aLogSize );
    }

    uno::Reference< rendering::XLinePolyPolygon2D > SpriteDeviceHelper::createCompatibleLinePolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&              /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XLinePolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XLinePolyPolygon2D >(
            new ::basegfx::unotools::UnoPolyPolygon(
                ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points )));
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > SpriteDeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBezierPolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XBezierPolyPolygon2D >(
            new ::basegfx::unotools::UnoPolyPolygon(
                ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBitmap > SpriteDeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( size,
                              mpSpriteCanvas,
                              *this,
                              false ) );
    }

    uno::Reference< rendering::XVolatileBitmap > SpriteDeviceHelper::createVolatileBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    uno::Reference< rendering::XBitmap > SpriteDeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( size,
                              mpSpriteCanvas,
                              *this,
                              true ) );
    }

    uno::Reference< rendering::XVolatileBitmap > SpriteDeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    sal_Bool SpriteDeviceHelper::hasFullScreenMode()
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    sal_Bool SpriteDeviceHelper::enterFullScreenMode( sal_Bool /*bEnter*/ )
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    ::sal_Int32 SpriteDeviceHelper::createBuffers( ::sal_Int32 /*nBuffers*/ )
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
        return 1;
    }

    void SpriteDeviceHelper::destroyBuffers()
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
    }

    namespace
    {
        /** Functor providing a StrictWeakOrdering for XSprites (over
            priority)
         */
        struct SpriteComparator
        {
            bool operator()( const ::rtl::Reference<CanvasCustomSprite>& rLHS,
                             const ::rtl::Reference<CanvasCustomSprite>& rRHS ) const
            {
                const double nPrioL( rLHS->getPriority() );
                const double nPrioR( rRHS->getPriority() );

                // if prios are equal, tie-break on ptr value
                return nPrioL == nPrioR ? rLHS.get() < rRHS.get() : nPrioL < nPrioR;
            }
        };
    }

    ::sal_Bool SpriteDeviceHelper::showBuffer( bool bIsVisible, ::sal_Bool /*bUpdateAll*/ )
    {
        // hidden or disposed?
        if( !bIsVisible || !mpChildWindow || !mpSpriteCanvas )
            return false;

        if( !activateWindowContext() )
            return false;

        const ::Size& rOutputSize=mpChildWindow->GetSizePixel();
        initTransformation(rOutputSize);

        // render the actual spritecanvas content
        mpSpriteCanvas->renderRecordedActions();

        // render all sprites (in order of priority) on top of that
        std::vector< ::rtl::Reference<CanvasCustomSprite> > aSprites;
        std::copy(maActiveSprites.begin(),
                  maActiveSprites.end(),
                  std::back_insert_iterator<
                       std::vector< ::rtl::Reference< CanvasCustomSprite > > >(aSprites));
        std::sort(aSprites.begin(),
                  aSprites.end(),
                  SpriteComparator());
        std::for_each(aSprites.begin(),
                      aSprites.end(),
                      boost::mem_fn(&CanvasCustomSprite::renderSprite));


        // frame counter, other info
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslated(-1.0, 1.0, 0.0);
        glScaled( 2.0  / rOutputSize.Width(),
                  -2.0 / rOutputSize.Height(),
                  1.0 );

        const double denominator( maLastUpdate.getElapsedTime() );
        maLastUpdate.reset();

        const double fps(denominator == 0.0 ? 100.0 : 1.0/denominator);
        std::vector<double> aVec; aVec.push_back(fps);
        aVec.push_back(maActiveSprites.size());
        aVec.push_back(mpTextureCache->getCacheSize());
        aVec.push_back(mpTextureCache->getCacheMissCount());
        aVec.push_back(mpTextureCache->getCacheHitCount());
        renderOSD( aVec, 20 );

        // switch buffer, sync etc.
        const unx::Window aXWindow=mpChildWindow->GetSystemData()->aWindow;
        unx::glXSwapBuffers(reinterpret_cast<unx::Display*>(mpDisplay),
                            aXWindow);
        mpChildWindow->Show();
        unx::glXWaitGL();
        XSync( reinterpret_cast<unx::Display*>(mpDisplay), false );

        // flush texture cache, such that it does not build up
        // indefinitely.
        // TODO: have max cache size/LRU time in config, prune only on
        // demand
        mpTextureCache->prune();

        return true;
    }

    ::sal_Bool SpriteDeviceHelper::switchBuffer( bool bIsVisible, ::sal_Bool bUpdateAll )
    {
        // no difference for VCL canvas
        return showBuffer( bIsVisible, bUpdateAll );
    }

    uno::Any SpriteDeviceHelper::isAccelerated() const
    {
        return ::com::sun::star::uno::makeAny(false);
    }

    uno::Any SpriteDeviceHelper::getDeviceHandle() const
    {
        return uno::makeAny( reinterpret_cast< sal_Int64 >(mpChildWindow.get()) );
    }

    uno::Any SpriteDeviceHelper::getSurfaceHandle() const
    {
        return uno::Any();
    }

    uno::Reference<rendering::XColorSpace> SpriteDeviceHelper::getColorSpace() const
    {
        // always the same
        return uno::Reference<rendering::XColorSpace>(
            ::canvas::tools::getStdColorSpace(),
            uno::UNO_QUERY);
    }

    void SpriteDeviceHelper::notifySizeUpdate( const awt::Rectangle& rBounds )
    {
        if( mpChildWindow )
            mpChildWindow->setPosSizePixel(
                0,0,rBounds.Width,rBounds.Height);
    }

    void SpriteDeviceHelper::dumpScreenContent() const
    {
        SAL_INFO("canvas.ogl", BOOST_CURRENT_FUNCTION );
    }

    void SpriteDeviceHelper::show( const ::rtl::Reference< CanvasCustomSprite >& xSprite )
    {
        maActiveSprites.insert(xSprite);
    }

    void SpriteDeviceHelper::hide( const ::rtl::Reference< CanvasCustomSprite >& xSprite )
    {
        maActiveSprites.erase(xSprite);
    }

    static void setupUniforms( unsigned int                                 nProgramId,
                               const ::basegfx::B2DHomMatrix&               rTexTransform )
    {
        const GLint nTransformLocation = glGetUniformLocation(nProgramId,
                                                             "m_transform" );
        // OGL is column-major
        float aTexTransform[] =
            {
                float(rTexTransform.get(0,0)), float(rTexTransform.get(1,0)),
                float(rTexTransform.get(0,1)), float(rTexTransform.get(1,1)),
                float(rTexTransform.get(0,2)), float(rTexTransform.get(1,2))
            };
        glUniformMatrix3x2fv(nTransformLocation,1,false,aTexTransform);
    }

    static void setupUniforms( unsigned int                   nProgramId,
                               const rendering::ARGBColor*    pColors,
                               const uno::Sequence< double >& rStops,
                               const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        glUseProgram(nProgramId);

        GLuint nColorsTexture;
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &nColorsTexture);
        glBindTexture(GL_TEXTURE_1D, nColorsTexture);

        const sal_Int32 nColors=rStops.getLength();
        glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, nColors, 0, GL_RGBA, GL_DOUBLE, pColors );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        GLuint nStopsTexture;
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &nStopsTexture);
        glBindTexture(GL_TEXTURE_1D, nStopsTexture);

        glTexImage1D( GL_TEXTURE_1D, 0, GL_ALPHA, nColors, 0, GL_ALPHA, GL_DOUBLE, rStops.getConstArray() );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        const GLint nColorArrayLocation = glGetUniformLocation(nProgramId,
                                                               "t_colorArray4d" );
        glUniform1i( nColorArrayLocation, 0 ); // unit 0

        const GLint nStopArrayLocation = glGetUniformLocation(nProgramId,
                                                              "t_stopArray1d" );
        glUniform1i( nStopArrayLocation, 1 ); // unit 1

        const GLint nNumColorLocation = glGetUniformLocation(nProgramId,
                                                             "i_nColors" );
        glUniform1i( nNumColorLocation, nColors-1 );

        setupUniforms(nProgramId,rTexTransform);
    }

    static void setupUniforms( unsigned int                   nProgramId,
                               const rendering::ARGBColor&    rStartColor,
                               const rendering::ARGBColor&    rEndColor,
                               const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        glUseProgram(nProgramId);

        const GLint nStartColorLocation = glGetUniformLocation(nProgramId,
                                                               "v_startColor4d" );
        glUniform4f(nStartColorLocation,
                    rStartColor.Red,
                    rStartColor.Green,
                    rStartColor.Blue,
                    rStartColor.Alpha);

        const GLint nEndColorLocation = glGetUniformLocation(nProgramId,
                                                             "v_endColor4d" );
        glUniform4f(nEndColorLocation,
                    rEndColor.Red,
                    rEndColor.Green,
                    rEndColor.Blue,
                    rEndColor.Alpha);

        setupUniforms(nProgramId,rTexTransform);
    }

    void SpriteDeviceHelper::useLinearGradientShader( const rendering::ARGBColor*    pColors,
                                                      const uno::Sequence< double >& rStops,
                                                      const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        if( rStops.getLength() > 2 )
            setupUniforms(mnLinearMultiColorGradientProgram, pColors, rStops, rTexTransform);
        else
            setupUniforms(mnLinearTwoColorGradientProgram, pColors[0], pColors[1], rTexTransform);
    }

    void SpriteDeviceHelper::useRadialGradientShader( const rendering::ARGBColor*    pColors,
                                                      const uno::Sequence< double >& rStops,
                                                      const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        if( rStops.getLength() > 2 )
            setupUniforms(mnRadialMultiColorGradientProgram, pColors, rStops, rTexTransform);
        else
            setupUniforms(mnRadialTwoColorGradientProgram, pColors[0], pColors[1], rTexTransform);
    }

    void SpriteDeviceHelper::useRectangularGradientShader( const rendering::ARGBColor*    pColors,
                                                           const uno::Sequence< double >& rStops,
                                                           const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        if( rStops.getLength() > 2 )
            setupUniforms(mnRectangularMultiColorGradientProgram, pColors, rStops, rTexTransform);
        else
            setupUniforms(mnRectangularTwoColorGradientProgram, pColors[0], pColors[1], rTexTransform);
    }

    bool SpriteDeviceHelper::activatePBufferContext(const ::basegfx::B2IVector& rSize,
                                                    unsigned int                PBuffer) const
    {
        if( !glXMakeCurrent( reinterpret_cast<unx::Display*>(mpDisplay),
                             PBuffer,
                             reinterpret_cast<unx::GLXContext>(mpGLPBufContext)) )
        {
            SAL_INFO("canvas.ogl", "SpriteDeviceHelper::activatePBufferContext(): cannot activate GL context");
            return false;
        }

        initContext();
        initTransformation(
            ::Size(
                rSize.getX(),
                rSize.getY()),
            true);

        return true;
    }

    bool SpriteDeviceHelper::activateWindowContext() const
    {
        const unx::Window aXWindow=mpChildWindow->GetSystemData()->aWindow;
        if( !glXMakeCurrent( reinterpret_cast<unx::Display*>(mpDisplay),
                             aXWindow,
                             reinterpret_cast<unx::GLXContext>(mpGLContext)) )
        {
            SAL_INFO("canvas.ogl", "SpriteDeviceHelper::activateWindowContext(): cannot activate GL context");
            return false;
        }

        return true;
    }

    bool SpriteDeviceHelper::updatePBufferTexture( const ::basegfx::B2IVector& rSize,
                                                   unsigned int                nTextId ) const
    {
        glBindTexture( GL_TEXTURE_2D, nTextId );
        glEnable(GL_TEXTURE_2D);
        glCopyTexSubImage2D( GL_TEXTURE_2D,
                             0, 0, 0, 0, 0,
                             rSize.getX(),
                             rSize.getY() );
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }

    namespace
    {
        class BufferContextImpl : public IBufferContext
        {
            ::basegfx::B2IVector       maSize;
            const SpriteDeviceHelper&  mrDeviceHelper;
            unx::GLXPbuffer            mpPBuffer;
#if 0
            unx::Display*              mpDisplay;
#endif
            unsigned int               mnTexture;

            virtual bool startBufferRendering() SAL_OVERRIDE
            {
                return mrDeviceHelper.activatePBufferContext(maSize,mpPBuffer);
            }

            virtual bool endBufferRendering() SAL_OVERRIDE
            {
                mrDeviceHelper.updatePBufferTexture(maSize,mnTexture);
                if( !mrDeviceHelper.activateWindowContext() )
                    return false;

                glBindTexture( GL_TEXTURE_2D, mnTexture );

                return true;
            }

        public:
            BufferContextImpl(const SpriteDeviceHelper&   rDeviceHelper,
                              unx::GLXPbuffer             pBuffer,
                              unx::Display*
#if 0
                                                          pDisplay
#endif
                              ,
                              const ::basegfx::B2IVector& rSize) :
                maSize(rSize),
                mrDeviceHelper(rDeviceHelper),
                mpPBuffer(pBuffer),
#if 0
                mpDisplay(pDisplay),
#endif
                mnTexture(0)
            {
                glGenTextures( 1, &mnTexture );
#if 1
                glBindTexture( GL_TEXTURE_2D, mnTexture );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
                              maSize.getX(), maSize.getY(),
                              0, GL_RGBA, GL_UNSIGNED_BYTE, new int[maSize.getX()*maSize.getY()] );
#endif
            }

            virtual ~BufferContextImpl()
            {
#if 0
                glBindTexture(GL_TEXTURE_2D, 0);
                glDeleteTextures( 1, &mnTexture );
                glXDestroyPbuffer( mpDisplay, mpPBuffer );
#endif
            }
        };
    }

    IBufferContextSharedPtr SpriteDeviceHelper::createBufferContext(const ::basegfx::B2IVector& rSize) const
    {
        int pBufAttribs[] =
            {
                GLX_PBUFFER_WIDTH,   rSize.getX(),
                GLX_PBUFFER_HEIGHT,  rSize.getY(),
                GLX_LARGEST_PBUFFER, False,
                None
            };

        unx::GLXPbuffer pBuffer;
        pBuffer = unx::glXCreatePbuffer( reinterpret_cast<unx::Display*>(mpDisplay),
                                         *reinterpret_cast<unx::GLXFBConfig*>(mpFBConfig),
                                         pBufAttribs );

        IBufferContextSharedPtr pRet;
        if( pBuffer )
            pRet.reset(new BufferContextImpl(
                           *this,
                           pBuffer,
                           reinterpret_cast<unx::Display*>(mpDisplay),
                           rSize));

        return pRet;
    }

    TextureCache& SpriteDeviceHelper::getTextureCache() const
    {
        return *mpTextureCache;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
