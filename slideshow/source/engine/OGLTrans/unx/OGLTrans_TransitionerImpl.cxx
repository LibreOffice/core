/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define GLX_GLXEXT_PROTOTYPES 1
#include "OGLTrans_TransitionImpl.hxx"

#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/presentation/XTransitionFactory.hpp>
#include <com/sun/star/presentation/XTransition.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/factory.hxx>
#include <rtl/ref.hxx>

#include <comphelper/servicedecl.hxx>

#include <canvas/canvastools.hxx>
#include <tools/gen.hxx>
#include <vcl/window.hxx>
#include <vcl/syschild.hxx>

#include <boost/noncopyable.hpp>

#include <GL/gl.h>
#include <GL/glu.h>


#if defined( WNT )
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#elif defined( QUARTZ )
    #include "premac.h"
    #include <Cocoa/Cocoa.h>
    #include "postmac.h"
#elif defined( UNX )
namespace unx
{
#include <X11/keysym.h>
#include <X11/X.h>
#include <GL/glx.h>
#include <GL/glxext.h>
}
#endif
#include <vcl/sysdata.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace ::boost::posix_time;

#endif

using namespace ::com::sun::star;
using ::com::sun::star::beans::XFastPropertySet;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;

namespace
{

typedef cppu::WeakComponentImplHelper1<presentation::XTransition> OGLTransitionerImplBase;

#if OSL_DEBUG_LEVEL > 1
class TimerContext
{
public:
    explicit TimerContext(rtl::OUString const& rWhat)
        : m_aWhat(rWhat)
        , m_aStartTime(microsec_clock::local_time())
    {
    }
    ~TimerContext()
    {
        time_duration const aDuration(microsec_clock::local_time() - m_aStartTime);
        SAL_INFO("slideshow.opengl", m_aWhat << " took: " << aDuration);
    }
private:
    rtl::OUString const m_aWhat;
    ptime const m_aStartTime;
};
#endif

struct OGLFormat
{
    GLint  nInternalFormat;
    GLenum eFormat;
    GLenum eType;
};

/* channel ordering: (0:rgba, 1:bgra, 2:argb, 3:abgr)
 */
int calcComponentOrderIndex(const uno::Sequence<sal_Int8>& rTags)
{
    using namespace rendering::ColorComponentTag;

    static const sal_Int8 aOrderTable[] =
    {
        RGB_RED, RGB_GREEN, RGB_BLUE, ALPHA,
        RGB_BLUE, RGB_GREEN, RGB_RED, ALPHA,
        ALPHA, RGB_RED, RGB_GREEN, RGB_BLUE,
        ALPHA, RGB_BLUE, RGB_GREEN, RGB_RED,
    };

    const sal_Int32 nNumComps(rTags.getLength());
    const sal_Int8* pLine=aOrderTable;
    for(int i=0; i<4; ++i)
    {
        int j=0;
        while( j<4 && j<nNumComps && pLine[j] == rTags[j] )
            ++j;

        // all of the line passed, this is a match!
        if( j==nNumComps )
            return i;

        pLine+=4;
    }

    return -1;
}

// not thread safe
static bool errorTriggered;
int oglErrorHandler( unx::Display* /*dpy*/, unx::XErrorEvent* /*evnt*/ )
{
    errorTriggered = true;

    return 0;
}

/** This is the Transitioner class for OpenGL 3D transitions in
 * slideshow. At the moment, it's Linux only. This class is implicitly
 * constructed from XTransitionFactory.
*/
class OGLTransitionerImpl : private cppu::BaseMutex, private boost::noncopyable, public OGLTransitionerImplBase
{
public:
    OGLTransitionerImpl();
    void setSlides( const Reference< rendering::XBitmap >& xLeavingSlide , const uno::Reference< rendering::XBitmap >& xEnteringSlide );
    void setTransition( OGLTransitionImpl* pOGLTransition );
    bool initialize( const Reference< presentation::XSlideShowView >& xView );

    // XTransition
    virtual void SAL_CALL update( double nTime )
    throw (uno::RuntimeException);
    virtual void SAL_CALL viewChanged( const Reference< presentation::XSlideShowView >& rView,
                       const Reference< rendering::XBitmap >& rLeavingBitmap,
                       const Reference< rendering::XBitmap >& rEnteringBitmap )
    throw (uno::RuntimeException);

protected:
    void disposeContextAndWindow();
    void disposeTextures();

    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing();

    bool isDisposed() const
    {
        return (rBHelper.bDisposed || rBHelper.bInDispose);
    }

    bool createWindow( Window* pPWindow );
    void createTexture( unsigned int* texID,
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
            unx::GLXPixmap pixmap,
            bool usePixmap,
#endif
            bool useMipmap,
            uno::Sequence<sal_Int8>& data,
            const OGLFormat* pFormat );
    void prepareEnvironment ();
    const OGLFormat* chooseFormats();

private:
    static void impl_initializeOnce( bool const bGLXPresent );

    bool initWindowFromSlideShowView( const uno::Reference< presentation::XSlideShowView >& xView );
    /** After the window has been created, and the slides have been set, we'll initialize the slides with OpenGL.
    */
    void GLInitSlides();


    /// Holds the information of our new child window
    struct GLWindow
    {
#if defined( WNT )
    HWND                    hWnd;
    HDC                     hDC;
    HGLRC                   hRC;
#elif defined( QUARTZ )
#elif defined( UNX )
    unx::Display*           dpy;
    int                     screen;
    unx::Window             win;
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::GLXFBConfig        fbc;
#endif
    unx::XVisualInfo*       vi;
    unx::GLXContext         ctx;
#endif
        unsigned int            bpp;
        unsigned int            Width;
        unsigned int            Height;
        const char*             GLXExtensions;
    const GLubyte*          GLExtensions;

        bool HasGLXExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, (const GLubyte*) GLXExtensions ); }
    bool HasGLExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, GLExtensions ); }
    } GLWin;

    /** OpenGL handle to the leaving slide's texture
    */
    unsigned int GLleavingSlide;
    /** OpenGL handle to the entering slide's texture
    */
    unsigned int GLenteringSlide;

    /** pointer to our window which we MIGHT create.
    */
    class SystemChildWindow* pWindow;

    Reference< presentation::XSlideShowView > mxView;
    Reference< rendering::XIntegerBitmap > mxLeavingBitmap;
    Reference< rendering::XIntegerBitmap > mxEnteringBitmap;

    /** raw bytes of the entering bitmap
    */
    uno::Sequence<sal_Int8> EnteringBytes;

    /** raw bytes of the leaving bitmap
    */
    uno::Sequence<sal_Int8> LeavingBytes;

#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::GLXPixmap LeavingPixmap;
    unx::GLXPixmap EnteringPixmap;
#endif
    bool mbRestoreSync;
    bool mbUseLeavingPixmap;
    bool mbUseEnteringPixmap;
    bool mbFreeLeavingPixmap;
    bool mbFreeEnteringPixmap;
    unx::Pixmap maLeavingPixmap;
    unx::Pixmap maEnteringPixmap;

    /** the form the raw bytes are in for the bitmaps
    */
    rendering::IntegerBitmapLayout SlideBitmapLayout;

    /** the size of the slides
    */
    geometry::IntegerSize2D SlideSize;

    /** Our Transition to be used.
    */
    OGLTransitionImpl* pTransition;

public:
    /** whether we are running on ATI fglrx with bug related to textures
     */
    static bool cbBrokenTexturesATI;

    /** GL version
     */
    static float cnGLVersion;
    float mnGLXVersion;

    /** Whether Mesa is the OpenGL vendor
     */
    static bool cbMesa;

    /**
       whether the display has GLX extension
     */
    static bool cbGLXPresent;

    /**
       whether texture from pixmap extension is available
    */
    bool mbTextureFromPixmap;

    /**
       whether to generate mipmaped textures
    */
    bool mbGenerateMipmap;

    /**
       whether we have visual which can be used for texture_from_pixmap extension
    */
    bool mbHasTFPVisual;

#if OSL_DEBUG_LEVEL > 1
    ptime maUpdateStartTime;
    ptime maUpdateEndTime;
    ptime maStartTime;
    ptime maEndTime;
    time_duration maTotalUpdateDuration;
    int mnFrameCount;
#endif
};

// declare the static variables as some gcc versions have problems declaring them automaticaly
bool OGLTransitionerImpl::cbBrokenTexturesATI;
float OGLTransitionerImpl::cnGLVersion;
bool OGLTransitionerImpl::cbMesa;
bool OGLTransitionerImpl::cbGLXPresent;

bool OGLTransitionerImpl::initialize( const Reference< presentation::XSlideShowView >& xView )
{

    bool const bGLXPresent( initWindowFromSlideShowView( xView ) );
    impl_initializeOnce( bGLXPresent );

    return cbGLXPresent;
}

void OGLTransitionerImpl::impl_initializeOnce( bool const bGLXPresent )
{
    // not thread safe
    static bool initialized = false;

    if( !initialized ) {
        cbGLXPresent = bGLXPresent;
        if ( bGLXPresent ) {
            const GLubyte* version = glGetString( GL_VERSION );
            if( version && version[0] ) {
                cnGLVersion = version[0] - '0';
                if( version[1] == '.' && version[2] )
                    cnGLVersion += (version[2] - '0')/10.0;
            } else
                cnGLVersion = 1.0;
            SAL_INFO("slideshow.opengl", "GL version: " << version << " parsed: " << cnGLVersion << "" );

            const GLubyte* vendor = glGetString( GL_VENDOR );
            cbMesa = ( vendor && strstr( (const char *) vendor, "Mesa" ) );
            SAL_INFO("slideshow.opengl", "GL vendor: " << vendor << " identified as Mesa: " << cbMesa << "" );

            /* TODO: check for version once the bug in fglrx driver is fixed */
            cbBrokenTexturesATI = (vendor && strcmp( (const char *) vendor, "ATI Technologies Inc." ) == 0 );

        }
        initialized = true;
    }
}

#if defined( UNX )
SystemEnvData const*
lcl_createSystemWindow(
        unx::XVisualInfo* const pXVisual,
        Window* const pParentWindow,
        SystemChildWindow** const pChildWindow )
{
    assert(pChildWindow);

    SystemWindowData winData;
    winData.nSize = sizeof(winData);
    SAL_INFO("slideshow.opengl", "using VisualID " << pXVisual->visualid);
    winData.pVisual = (void*)(pXVisual->visual);
    SystemChildWindow* pWindow = new SystemChildWindow(pParentWindow, 0, &winData, sal_False);
    SystemEnvData const* const pChildSysData = pWindow->GetSystemData();
    if( !pChildSysData ) {
        delete pWindow, pWindow=NULL;
    }

    *pChildWindow = pWindow;
    return pChildSysData;
}
#endif

bool OGLTransitionerImpl::createWindow( Window* pPWindow )
{
    const SystemEnvData* sysData(pPWindow->GetSystemData());
#if defined( WNT )
    GLWin.hWnd = sysData->hWnd;
#elif defined( UNX )
    GLWin.dpy = reinterpret_cast<unx::Display*>(sysData->pDisplay);

    if( unx::glXQueryExtension( GLWin.dpy, NULL, NULL ) == false )
        return false;

    GLWin.win = sysData->aWindow;

    SAL_INFO("slideshow.opengl", "parent window: " << GLWin.win);

    unx::XWindowAttributes xattr;
    unx::XGetWindowAttributes( GLWin.dpy, GLWin.win, &xattr );

    GLWin.screen = XScreenNumberOfScreen( xattr.screen );

    unx::XVisualInfo* vi( NULL );
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::XVisualInfo* visinfo;
    unx::XVisualInfo* firstVisual( NULL );
#endif
    static int attrList3[] =
        {
            GLX_RGBA,//only TrueColor or DirectColor
            //single buffered
            GLX_RED_SIZE,4,//use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,//use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,//use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,0,//no depth buffer
            None
        };
    static int attrList2[] =
    {
            GLX_RGBA,//only TrueColor or DirectColor
            /// single buffered
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,1,/// use the maximum depth bits, making sure there is a depth buffer
            None
        };
    static int attrList1[] =
        {
            GLX_RGBA,//only TrueColor or DirectColor
            GLX_DOUBLEBUFFER,/// only double buffer
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,0,/// no depth buffer
            None
        };
    static int attrList0[] =
        {
            GLX_RGBA,//only TrueColor or DirectColor
            GLX_DOUBLEBUFFER,/// only double buffer
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,1,/// use the maximum depth bits, making sure there is a depth buffer
            None
       };
    static int* attrTable[] =
        {
            attrList0,
            attrList1,
            attrList2,
            attrList3,
            NULL
        };
    int** pAttributeTable = attrTable;
    const SystemEnvData* pChildSysData = NULL;
    delete pWindow;
    pWindow=NULL;

#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::GLXFBConfig* fbconfigs = NULL;
    int nfbconfigs, value, i = 0;
#endif

    while( *pAttributeTable )
    {
        // try to find a visual for the current set of attributes
        vi = unx::glXChooseVisual( GLWin.dpy,
                                   GLWin.screen,
                                   *pAttributeTable );

        if( vi ) {
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
            if( !firstVisual )
                firstVisual = vi;
            SAL_INFO("slideshow.opengl", "trying VisualID " << vi->visualid);
            fbconfigs = glXGetFBConfigs (GLWin.dpy, GLWin.screen, &nfbconfigs);
            for ( ; i < nfbconfigs; i++)
            {
                visinfo = glXGetVisualFromFBConfig (GLWin.dpy, fbconfigs[i]);
                if( !visinfo || visinfo->visualid != vi->visualid )
                    continue;

                glXGetFBConfigAttrib (GLWin.dpy, fbconfigs[i], GLX_DRAWABLE_TYPE, &value);
                if (!(value & GLX_PIXMAP_BIT))
                    continue;

                glXGetFBConfigAttrib (GLWin.dpy, fbconfigs[i],
                                        GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                                        &value);
                if (!(value & GLX_TEXTURE_2D_BIT_EXT))
                    continue;

                glXGetFBConfigAttrib (GLWin.dpy, fbconfigs[i],
                                        GLX_BIND_TO_TEXTURE_RGB_EXT,
                                        &value);
                if (value == sal_False)
                    continue;

                glXGetFBConfigAttrib (GLWin.dpy, fbconfigs[i],
                                        GLX_BIND_TO_MIPMAP_TEXTURE_EXT,
                                        &value);
                if (value == sal_False)
                    continue;

                /* TODO: handle non Y inverted cases */
                break;
            }

            if( i != nfbconfigs ) {
                vi = glXGetVisualFromFBConfig( GLWin.dpy, fbconfigs[i] );
                mbHasTFPVisual = true;
                pChildSysData = lcl_createSystemWindow( vi, pPWindow, &pWindow );
                SAL_INFO("slideshow.opengl", "found visual suitable for texture_from_pixmap");
            } else if( firstVisual && pAttributeTable[1] == NULL ) {
                vi = firstVisual;
                mbHasTFPVisual = false;
                pChildSysData = lcl_createSystemWindow( vi, pPWindow, &pWindow );
                SAL_INFO("slideshow.opengl", "did not find visual suitable for texture_from_pixmap, using " << vi->visualid);
            }
#else
            pChildSysData = lcl_createSystemWindow( vi, pPWindow, &pWindow );
#endif
            if ( pChildSysData )
                break;
        }

        ++pAttributeTable;
    }
#endif

#if defined( WNT )
    SystemWindowData winData;
    winData.nSize = sizeof(winData);
    pWindow=new SystemChildWindow(pPWindow, 0, &winData, sal_False);
    pWindow->GetSystemData();
#endif

    if( pWindow )
    {
        pWindow->SetMouseTransparent( sal_True );
        pWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        pWindow->EnableEraseBackground( sal_False );
        pWindow->SetControlForeground();
        pWindow->SetControlBackground();
        pWindow->EnablePaint(sal_False);
#if defined( WNT )
        GLWin.hWnd = sysData->hWnd;
#elif defined( UNX )
        GLWin.dpy = reinterpret_cast<unx::Display*>(pChildSysData->pDisplay);
        GLWin.win = pChildSysData->aWindow;
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    if( mbHasTFPVisual )
        GLWin.fbc = fbconfigs[i];
#endif
        GLWin.vi = vi;
        GLWin.GLXExtensions = unx::glXQueryExtensionsString( GLWin.dpy, GLWin.screen );
        SAL_INFO("slideshow.opengl", "available GLX extensions: " << GLWin.GLXExtensions);
#endif

        return true;
    }

    return false;
}

bool OGLTransitionerImpl::initWindowFromSlideShowView( const Reference< presentation::XSlideShowView >& xView )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    mxView.set( xView, UNO_QUERY );
    if( !mxView.is() )
        return false;

#if OSL_DEBUG_LEVEL > 1
    TimerContext aTimerContext("initWindowFromSlideShowView");
#endif

    /// take the XSlideShowView and extract the parent window from it. see viewmediashape.cxx
    uno::Reference< rendering::XCanvas > xCanvas(mxView->getCanvas(), uno::UNO_QUERY_THROW);
    uno::Sequence< uno::Any > aDeviceParams;
    ::canvas::tools::getDeviceInfo( xCanvas, aDeviceParams );

    ::rtl::OUString aImplName;
    aDeviceParams[ 0 ] >>= aImplName;

    sal_Int64 aVal = 0;
    aDeviceParams[1] >>= aVal;
    if( !createWindow( reinterpret_cast< Window* >( aVal ) ) )
        return false;

    awt::Rectangle aCanvasArea = mxView->getCanvasArea();
    pWindow->SetPosSizePixel(aCanvasArea.X, aCanvasArea.Y, aCanvasArea.Width, aCanvasArea.Height);
    GLWin.Width = aCanvasArea.Width;
    GLWin.Height = aCanvasArea.Height;
    SAL_INFO("slideshow.opengl", "canvas area: " << aCanvasArea.X << "," << aCanvasArea.Y << " - " << aCanvasArea.Width << "x" << aCanvasArea.Height);

#if defined( WNT )
    GLWin.hDC = GetDC(GLWin.hWnd);
#elif defined( UNX )
    GLWin.ctx = glXCreateContext(GLWin.dpy,
                                 GLWin.vi,
                                 0,
                                 GL_TRUE);
    if( GLWin.ctx == NULL ) {
        SAL_INFO("slideshow.opengl", "unable to create GLX context");
        return false;
    }
#endif

#if defined( WNT )
    PIXELFORMATDESCRIPTOR PixelFormatFront =                    // PixelFormat Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                              // Version Number
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                  // Request An RGBA Format
        (BYTE)32,                       // Select Our Color Depth
        0, 0, 0, 0, 0, 0,               // Color Bits Ignored
        0,                              // No Alpha Buffer
        0,                              // Shift Bit Ignored
        0,                              // No Accumulation Buffer
        0, 0, 0, 0,                     // Accumulation Bits Ignored
        64,                             // 32 bit Z-BUFFER
        0,                              // 0 bit stencil buffer
        0,                              // No Auxiliary Buffer
        0,                              // now ignored
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };
    int WindowPix = ChoosePixelFormat(GLWin.hDC,&PixelFormatFront);
    SetPixelFormat(GLWin.hDC,WindowPix,&PixelFormatFront);
    GLWin.hRC  = wglCreateContext(GLWin.hDC);
    wglMakeCurrent(GLWin.hDC,GLWin.hRC);
#elif defined( UNX )
    if( !glXMakeCurrent( GLWin.dpy, GLWin.win, GLWin.ctx ) ) {
        SAL_INFO("slideshow.opengl", "unable to select current GLX context");
        return false;
    }

    int glxMinor, glxMajor;
    mnGLXVersion = 0;
    if( glXQueryVersion( GLWin.dpy, &glxMajor, &glxMinor ) )
        mnGLXVersion = glxMajor + 0.1*glxMinor;
    SAL_INFO("slideshow.opengl", "available GLX version: " << mnGLXVersion);

    GLWin.GLExtensions = glGetString( GL_EXTENSIONS );
    SAL_INFO("slideshow.opengl", "available GL  extensions: " << GLWin.GLExtensions);

    mbTextureFromPixmap = GLWin.HasGLXExtension( "GLX_EXT_texture_from_pixmap" );
    mbGenerateMipmap = GLWin.HasGLExtension( "GL_SGIS_generate_mipmap" );

    if( GLWin.HasGLXExtension("GLX_SGI_swap_control" ) ) {
        // enable vsync
        typedef GLint (*glXSwapIntervalProc)(GLint);
        glXSwapIntervalProc glXSwapInterval = (glXSwapIntervalProc) unx::glXGetProcAddress( (const GLubyte*) "glXSwapIntervalSGI" );
        if( glXSwapInterval ) {
            int (*oldHandler)(unx::Display* /*dpy*/, unx::XErrorEvent* /*evnt*/);

            // replace error handler temporarily
            oldHandler = unx::XSetErrorHandler( oglErrorHandler );

            errorTriggered = false;

            glXSwapInterval( 1 );

            // sync so that we possibly get an XError
            unx::glXWaitGL();
            XSync(GLWin.dpy, false);

            if( errorTriggered )
                SAL_INFO("slideshow.opengl", "error when trying to set swap interval, NVIDIA or Mesa bug?");
            else
                SAL_INFO("slideshow.opengl", "set swap interval to 1 (enable vsync)");

            // restore the error handler
            unx::XSetErrorHandler( oldHandler );
        }
    }
#endif

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor (0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
#if defined( WNT )
    SwapBuffers(GLWin.hDC);
#elif defined( UNX )
    unx::glXSwapBuffers(GLWin.dpy, GLWin.win);
#endif

    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    return true;
}

void OGLTransitionerImpl::setSlides( const uno::Reference< rendering::XBitmap >& xLeavingSlide,
                                     const uno::Reference< rendering::XBitmap >& xEnteringSlide )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    mxLeavingBitmap.set( xLeavingSlide , UNO_QUERY_THROW );
    mxEnteringBitmap.set( xEnteringSlide , UNO_QUERY_THROW );
    Reference< XFastPropertySet > xLeavingSet( xLeavingSlide , UNO_QUERY );
    Reference< XFastPropertySet > xEnteringSet( xEnteringSlide , UNO_QUERY );

    geometry::IntegerRectangle2D SlideRect;
    SlideSize = mxLeavingBitmap->getSize();
    SlideRect.X1 = 0;
    SlideRect.X2 = SlideSize.Width;
    SlideRect.Y1 = 0;
    SlideRect.Y2 = SlideSize.Height;

    SAL_INFO("slideshow.opengl", "leaving bitmap area: " << SlideSize.Width << "x" << SlideSize.Height);
    SlideSize = mxEnteringBitmap->getSize();
    SAL_INFO("slideshow.opengl", "entering bitmap area: " << SlideSize.Width << "x" << SlideSize.Height);

#ifdef UNX
    unx::glXWaitGL();
    XSync(GLWin.dpy, false);
#endif

    mbUseLeavingPixmap = false;
    mbUseEnteringPixmap = false;

#ifdef UNX
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )

    if( mnGLXVersion >= 1.2999 && mbTextureFromPixmap && xLeavingSet.is() && xEnteringSet.is() && mbHasTFPVisual ) {
        Sequence< Any > leaveArgs;
        Sequence< Any > enterArgs;
        if( (xLeavingSet->getFastPropertyValue( 1 ) >>= leaveArgs) &&
            (xEnteringSet->getFastPropertyValue( 1 ) >>= enterArgs) ) {
            SAL_INFO("slideshow.opengl", "pixmaps available");

            sal_Int32 depth(0);

            leaveArgs[0] >>= mbFreeLeavingPixmap;
            enterArgs[0] >>= mbFreeEnteringPixmap;
            leaveArgs[1] >>= maLeavingPixmap;
            enterArgs[1] >>= maEnteringPixmap;
            leaveArgs[2] >>= depth;

            int pixmapAttribs[] = { GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
                        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGB_EXT,
                        GLX_MIPMAP_TEXTURE_EXT, True,
                        None };


            // sync so that we possibly get an pending XError, before we set our handler.
            // this way we will not miss any error from other code
            unx::glXWaitGL();
            XSync(GLWin.dpy, false);

            int (*oldHandler)(unx::Display* /*dpy*/, unx::XErrorEvent* /*evnt*/);

            // replace error handler temporarily
            oldHandler = unx::XSetErrorHandler( oglErrorHandler );

            errorTriggered = false;
            LeavingPixmap = glXCreatePixmap( GLWin.dpy, GLWin.fbc, maLeavingPixmap, pixmapAttribs );

            // sync so that we possibly get an XError
            unx::glXWaitGL();
            XSync(GLWin.dpy, false);

            if( !errorTriggered )
                mbUseLeavingPixmap = true;
            else {
                SAL_INFO("slideshow.opengl", "XError triggered");
                OSL_TRACE("XError triggered");
                if( mbFreeLeavingPixmap ) {
                    unx::XFreePixmap( GLWin.dpy, maLeavingPixmap );
                    mbFreeLeavingPixmap = false;
                }
                errorTriggered = false;
            }

            EnteringPixmap = glXCreatePixmap( GLWin.dpy, GLWin.fbc, maEnteringPixmap, pixmapAttribs );

            // sync so that we possibly get an XError
            unx::glXWaitGL();
            XSync(GLWin.dpy, false);

            SAL_INFO("slideshow.opengl", "created glx pixmap " << LeavingPixmap << " and " << EnteringPixmap << " depth: " << depth);
            if( !errorTriggered )
                mbUseEnteringPixmap = true;
            else {
                SAL_INFO("slideshow.opengl", "XError triggered");
                if( mbFreeEnteringPixmap ) {
                    unx::XFreePixmap( GLWin.dpy, maEnteringPixmap );
                    mbFreeEnteringPixmap = false;
                }
            }

            // restore the error handler
            unx::XSetErrorHandler( oldHandler );
        }
    }

#endif
#endif
    if( !mbUseLeavingPixmap )
        LeavingBytes = mxLeavingBitmap->getData(SlideBitmapLayout,SlideRect);
    if( !mbUseEnteringPixmap )
        EnteringBytes = mxEnteringBitmap->getData(SlideBitmapLayout,SlideRect);

// TODO
#ifdef UNX
    if(GLWin.ctx)//if we have a rendering context, let's init the slides
#endif
    GLInitSlides();

    OSL_ENSURE(SlideBitmapLayout.PlaneStride == 0,"only handle no plane stride now");

#ifdef UNX
    /* flush & sync */
    unx::glXWaitGL();
    XSync( GLWin.dpy, false );

    // synchronized X still gives us much smoother play
    // I suspect some issues in above code in slideshow
    // synchronize whole transition for now
    XSynchronize( GLWin.dpy, true );
    mbRestoreSync = true;
#endif
}

void OGLTransitionerImpl::setTransition( OGLTransitionImpl* const pNewTransition )
{
    pTransition = pNewTransition;

    if( pTransition && pTransition->mnRequiredGLVersion <= cnGLVersion )
        pTransition->prepare( GLleavingSlide, GLenteringSlide );
}

void OGLTransitionerImpl::createTexture( unsigned int* texID,
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
                     unx::GLXPixmap pixmap,
                     bool usePixmap,
#endif
                     bool useMipmap,
                     uno::Sequence<sal_Int8>& data,
                     const OGLFormat* pFormat )
{
    glDeleteTextures( 1, texID );
    glGenTextures( 1, texID );
    glBindTexture( GL_TEXTURE_2D, *texID );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::PFNGLXBINDTEXIMAGEEXTPROC myglXBindTexImageEXT = (unx::PFNGLXBINDTEXIMAGEEXTPROC) unx::glXGetProcAddress( (const GLubyte*) "glXBindTexImageEXT" );

    if( usePixmap ) {
        if( mbGenerateMipmap )
            glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, True);
        myglXBindTexImageEXT (GLWin.dpy, pixmap, GLX_FRONT_LEFT_EXT, NULL);
        if( mbGenerateMipmap && useMipmap ) {
            SAL_INFO("slideshow.opengl", "use mipmaps");
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); //TRILINEAR FILTERING
        } else {
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        }
    } else {
#endif
    if( !pFormat )
    {
        // force-convert color to ARGB8888 int color space
        uno::Sequence<sal_Int8> tempBytes(
            SlideBitmapLayout.ColorSpace->convertToIntegerColorSpace(
                data,
                canvas::tools::getStdColorSpace()));
        gluBuild2DMipmaps(GL_TEXTURE_2D,
                          4,
                          SlideSize.Width,
                          SlideSize.Height,
                          GL_RGBA,
                          GL_UNSIGNED_BYTE,
                          &tempBytes[0]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); //TRILINEAR FILTERING

            //anistropic filtering (to make texturing not suck when looking at polygons from oblique angles)
        GLfloat largest_supported_anisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
    } else {
        if( pTransition && !cbBrokenTexturesATI && !useMipmap) {
            glTexImage2D( GL_TEXTURE_2D, 0, pFormat->nInternalFormat, SlideSize.Width, SlideSize.Height, 0, pFormat->eFormat, pFormat->eType, &data[0] );
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        } else {
            gluBuild2DMipmaps( GL_TEXTURE_2D, pFormat->nInternalFormat, SlideSize.Width, SlideSize.Height, pFormat->eFormat, pFormat->eType, &data[0] );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); //TRILINEAR FILTERING

            //anistropic filtering (to make texturing not suck when looking at polygons from oblique angles)
            GLfloat largest_supported_anisotropy;
            glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy );
        }
    }
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    }
#endif
    OSL_ENSURE(glIsTexture(*texID), "Can't generate Leaving slide textures in OpenGL");
}

void OGLTransitionerImpl::prepareEnvironment()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double EyePos(10.0);
    double RealF(1.0);
    double RealN(-1.0);
    double RealL(-1.0);
    double RealR(1.0);
    double RealB(-1.0);
    double RealT(1.0);
    double ClipN(EyePos+5.0*RealN);
    double ClipF(EyePos+15.0*RealF);
    double ClipL(RealL*8.0);
    double ClipR(RealR*8.0);
    double ClipB(RealB*8.0);
    double ClipT(RealT*8.0);
    //This scaling is to take the plane with BottomLeftCorner(-1,-1,0) and TopRightCorner(1,1,0) and map it to the screen after the perspective division.
    glScaled( 1.0 / ( ( ( RealR * 2.0 * ClipN ) / ( EyePos * ( ClipR - ClipL ) ) ) - ( ( ClipR + ClipL ) / ( ClipR - ClipL ) ) ),
              1.0 / ( ( ( RealT * 2.0 * ClipN ) / ( EyePos * ( ClipT - ClipB ) ) ) - ( ( ClipT + ClipB ) / ( ClipT - ClipB ) ) ),
              1.0 );
    glFrustum(ClipL,ClipR,ClipB,ClipT,ClipN,ClipF);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0,0,-EyePos);
}

const OGLFormat* OGLTransitionerImpl::chooseFormats()
{
    const OGLFormat* pDetectedFormat=NULL;
    uno::Reference<rendering::XIntegerBitmapColorSpace> xIntColorSpace(
        SlideBitmapLayout.ColorSpace);

    if( (xIntColorSpace->getType() == rendering::ColorSpaceType::RGB ||
         xIntColorSpace->getType() == rendering::ColorSpaceType::SRGB) )
    {
        /* table for canvas->OGL format mapping. outer index is number
           of color components (0:3, 1:4), then comes bits per pixel
           (0:16, 1:24, 2:32), then channel ordering: (0:rgba, 1:bgra,
           2:argb, 3:abgr)
         */
        static const OGLFormat lcl_RGB24[] =
        {
            // 24 bit RGB
            {3, GL_BGR, GL_UNSIGNED_BYTE},
            {3, GL_RGB, GL_UNSIGNED_BYTE},
            {3, GL_BGR, GL_UNSIGNED_BYTE},
            {3, GL_RGB, GL_UNSIGNED_BYTE}
        };

#if defined(GL_VERSION_1_2) && defined(GLU_VERSION_1_3)
        // more format constants available
        static const OGLFormat lcl_RGB16[] =
        {
            // 16 bit RGB
            {3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV},
            {3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5},
            {3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV},
            {3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5}
        };

        static const OGLFormat lcl_ARGB16_4[] =
        {
            // 16 bit ARGB
            {4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV},
            {4, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV},
            {4, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4},
            {4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4}
        };

        static const OGLFormat lcl_ARGB16_5[] =
        {
            // 16 bit ARGB
            {4, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV},
            {4, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV},
            {4, GL_BGRA, GL_UNSIGNED_SHORT_5_5_5_1},
            {4, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1}
        };

        static const OGLFormat lcl_ARGB32[] =
        {
            // 32 bit ARGB
            {4, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV},
            {4, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV},
            {4, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8},
            {4, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8}
        };

        const uno::Sequence<sal_Int8> aComponentTags(
            xIntColorSpace->getComponentTags());
        const uno::Sequence<sal_Int32> aComponentBitcounts(
            xIntColorSpace->getComponentBitCounts());
        const sal_Int32 nNumComponents( aComponentBitcounts.getLength() );
        const sal_Int32 nBitsPerPixel( xIntColorSpace->getBitsPerPixel() );

        // supported component ordering?
        const int nComponentOrderIndex(
            calcComponentOrderIndex(aComponentTags));
        if( nComponentOrderIndex != -1 )
        {
            switch( nBitsPerPixel )
            {
                case 16:
                    if( nNumComponents == 3 )
                    {
                        pDetectedFormat = &lcl_RGB16[nComponentOrderIndex];
                    }
                    else if( nNumComponents == 4 )
                    {
                        if( aComponentBitcounts[1] == 4 )
                        {
                            pDetectedFormat = &lcl_ARGB16_4[nComponentOrderIndex];
                        }
                        else if( aComponentBitcounts[1] == 5 )
                        {
                            pDetectedFormat = &lcl_ARGB16_5[nComponentOrderIndex];
                        }
                    }
                    break;
                case 24:
                    if( nNumComponents == 3 )
                    {
                        pDetectedFormat = &lcl_RGB24[nComponentOrderIndex];
                    }
                    break;
                case 32:
                    pDetectedFormat = &lcl_ARGB32[nComponentOrderIndex];
                    break;
            }
        }
#else
        const uno::Sequence<sal_Int8> aComponentTags(
            xIntColorSpace->getComponentTags());
        const int nComponentOrderIndex(calcComponentOrderIndex(aComponentTags));
        if( aComponentTags.getLength() == 3 &&
            nComponentOrderIndex != -1 &&
            xIntColorSpace->getBitsPerPixel() == 24 )
        {
            pDetectedFormat = &lcl_RGB24[nComponentOrderIndex];
        }
#endif
    }

    return pDetectedFormat;
}

void OGLTransitionerImpl::GLInitSlides()
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed() || pTransition->mnRequiredGLVersion > cnGLVersion)
        return;

#if OSL_DEBUG_LEVEL > 1
    TimerContext aTimerContext("texture creation");
#endif

    prepareEnvironment();

    const OGLFormat* pFormat = NULL;
    if( !mbUseLeavingPixmap || !mbUseEnteringPixmap )
        pFormat = chooseFormats();

    createTexture( &GLleavingSlide,
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
           LeavingPixmap,
           mbUseLeavingPixmap,
#endif
           pTransition->mbUseMipMapLeaving,
           LeavingBytes,
           pFormat );

    createTexture( &GLenteringSlide,
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
           EnteringPixmap,
           mbUseEnteringPixmap,
#endif
           pTransition->mbUseMipMapEntering,
           EnteringBytes,
           pFormat );

#ifdef UNX
    unx::glXWaitGL();
    XSync(GLWin.dpy, false);
#endif
}

void SAL_CALL OGLTransitionerImpl::update( double nTime ) throw (uno::RuntimeException)
{
#if OSL_DEBUG_LEVEL > 1
    mnFrameCount ++;
    maUpdateStartTime = microsec_clock::local_time();
    if( mnFrameCount == 1 ) {
        maStartTime = maUpdateStartTime;
        maTotalUpdateDuration = seconds (0);
    }
#endif
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed() || !cbGLXPresent || pTransition->mnRequiredGLVersion > cnGLVersion)
        return;

#ifdef WNT
    wglMakeCurrent(GLWin.hDC,GLWin.hRC);
#endif
#ifdef UNX
    glXMakeCurrent( GLWin.dpy, GLWin.win, GLWin.ctx );
#endif

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(pTransition)
        pTransition->display( nTime, GLleavingSlide, GLenteringSlide,
                              SlideSize.Width, SlideSize.Height,
                              static_cast<double>(GLWin.Width),
                              static_cast<double>(GLWin.Height) );

#if defined( WNT )
    SwapBuffers(GLWin.hDC);
#elif defined( UNX )
    unx::glXSwapBuffers(GLWin.dpy, GLWin.win);
#endif
    if( pWindow )
        pWindow->Show();

#ifdef UNX
    /* flush & sync */
    unx::glXWaitGL();
    XSync( GLWin.dpy, false );
#endif

#if OSL_DEBUG_LEVEL > 1
    maUpdateEndTime = microsec_clock::local_time();

    SAL_INFO("slideshow.opengl", "update time: " << nTime);
    SAL_INFO("slideshow.opengl", "update took: " << ( maUpdateEndTime - maUpdateStartTime ));
    maTotalUpdateDuration += (maUpdateEndTime - maUpdateStartTime);
#endif
}

void SAL_CALL OGLTransitionerImpl::viewChanged( const Reference< presentation::XSlideShowView >& rView,
                        const Reference< rendering::XBitmap >& rLeavingBitmap,
                        const Reference< rendering::XBitmap >& rEnteringBitmap )
    throw (uno::RuntimeException)
{
    SAL_INFO("slideshow.opengl", "transitioner: view changed");

    disposeTextures();
    disposeContextAndWindow();

    initWindowFromSlideShowView( rView );
    setSlides( rLeavingBitmap, rEnteringBitmap );
}

void OGLTransitionerImpl::disposeContextAndWindow()
{
#if defined( WNT )
    if (GLWin.hRC)
    {
        wglMakeCurrent( GLWin.hDC, 0 );     // kill Device Context
        wglDeleteContext( GLWin.hRC );      // Kill Render Context
        ReleaseDC( GLWin.hWnd, GLWin.hDC );         // Release Window
    }
#elif defined( UNX )
    if(GLWin.ctx)
    {
        glXMakeCurrent(GLWin.dpy, None, NULL);
        if( glGetError() != GL_NO_ERROR ) {
            SAL_INFO("slideshow.opengl", "glError: " << (char *)gluErrorString(glGetError()));
        }
        glXDestroyContext(GLWin.dpy, GLWin.ctx);
        GLWin.ctx = NULL;
    }
#endif
    if( pWindow ) {
        delete pWindow;
        pWindow = NULL;
        GLWin.win = 0;
    }
}

void OGLTransitionerImpl::disposeTextures()
{
#ifdef WNT
    wglMakeCurrent(GLWin.hDC,GLWin.hRC);
#endif
#ifdef UNX
    glXMakeCurrent( GLWin.dpy, GLWin.win, GLWin.ctx );
#endif

#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::PFNGLXRELEASETEXIMAGEEXTPROC myglXReleaseTexImageEXT = (unx::PFNGLXRELEASETEXIMAGEEXTPROC) unx::glXGetProcAddress( (const GLubyte*) "glXReleaseTexImageEXT" );
    if( mbUseLeavingPixmap ) {
        myglXReleaseTexImageEXT( GLWin.dpy, LeavingPixmap, GLX_FRONT_LEFT_EXT );
        glXDestroyGLXPixmap( GLWin.dpy, LeavingPixmap );
        LeavingPixmap = 0;
        if( mbFreeLeavingPixmap ) {
            unx::XFreePixmap( GLWin.dpy, maLeavingPixmap );
            mbFreeLeavingPixmap = false;
            maLeavingPixmap = 0;
        }
    }
    if( mbUseEnteringPixmap ) {
        myglXReleaseTexImageEXT( GLWin.dpy, EnteringPixmap, GLX_FRONT_LEFT_EXT );
        glXDestroyGLXPixmap( GLWin.dpy, EnteringPixmap );
        EnteringPixmap = 0;
        if( mbFreeEnteringPixmap ) {
            unx::XFreePixmap( GLWin.dpy, maEnteringPixmap );
            mbFreeEnteringPixmap = false;
            maEnteringPixmap = 0;
        }
    }
#endif

    if( !mbUseLeavingPixmap ) {
        glDeleteTextures(1,&GLleavingSlide);
        GLleavingSlide = 0;
    }
    if( !mbUseEnteringPixmap ) {
        glDeleteTextures(1,&GLenteringSlide);
        GLleavingSlide = 0;
    }

    mbUseLeavingPixmap = false;
    mbUseEnteringPixmap = false;
}

// we are about to be disposed (someone call dispose() on us)
void OGLTransitionerImpl::disposing()
{
    osl::MutexGuard const guard( m_aMutex );

#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("slideshow.opengl", "dispose " << this);
    if( mnFrameCount ) {
        maEndTime = microsec_clock::local_time();
        time_duration duration = maEndTime - maStartTime;
        SAL_INFO("slideshow.opengl",
                "whole transition (frames: " << mnFrameCount
                << ") took: " << duration
                << " fps: "
                << (((double)mnFrameCount*1000000000.0)/duration.total_nanoseconds())
                << " time spent in updates: " << maTotalUpdateDuration
                << " percentage of transition time: "
                << (100*(((double)maTotalUpdateDuration.total_nanoseconds())/((double)duration.total_nanoseconds())))
                << '%'
            );
    }
#endif

    if( pWindow ) {
        disposeTextures();

        if (pTransition)
            pTransition->finish();

#ifdef UNX
        if( mbRestoreSync ) {
            // try to reestablish synchronize state
            char* sal_synchronize = getenv("SAL_SYNCHRONIZE");
            XSynchronize( GLWin.dpy, sal_synchronize && *sal_synchronize == '1' );
        }
#endif

        disposeContextAndWindow();
    }

    if (pTransition)
        delete pTransition;

    mxLeavingBitmap.clear();
    mxEnteringBitmap.clear();
    mxView.clear();
}

OGLTransitionerImpl::OGLTransitionerImpl() :
    OGLTransitionerImplBase(m_aMutex),
    GLWin(),
    GLleavingSlide( 0 ),
    GLenteringSlide( 0 ),
    pWindow( NULL ),
    mxView(),
    EnteringBytes(),
    LeavingBytes(),
    mbRestoreSync( false ),
    mbUseLeavingPixmap( false ),
    mbUseEnteringPixmap( false ),
    SlideBitmapLayout(),
    SlideSize(),
    pTransition( 0 )
{
#if defined( WNT )
    GLWin.hWnd = 0;
#elif defined( UNX )
    GLWin.ctx = 0;
#endif

#if OSL_DEBUG_LEVEL > 1
    mnFrameCount = 0;
#endif
}

typedef cppu::WeakComponentImplHelper1<presentation::XTransitionFactory> OGLTransitionFactoryImplBase;

class OGLTransitionFactoryImpl : private cppu::BaseMutex, public OGLTransitionFactoryImplBase
{
public:
    explicit OGLTransitionFactoryImpl( const uno::Reference< uno::XComponentContext >& ) :
        OGLTransitionFactoryImplBase(m_aMutex)
    {}

    // XTransitionFactory
    virtual ::sal_Bool SAL_CALL hasTransition( ::sal_Int16 transitionType, ::sal_Int16 transitionSubType ) throw (uno::RuntimeException)
    {
        if( transitionType == animations::TransitionType::MISCSHAPEWIPE ) {
            switch( transitionSubType )
            {
                case animations::TransitionSubType::ACROSS:
                case animations::TransitionSubType::CORNERSOUT:
                case animations::TransitionSubType::CIRCLE:
                case animations::TransitionSubType::FANOUTHORIZONTAL:
                case animations::TransitionSubType::CORNERSIN:
                case animations::TransitionSubType::LEFTTORIGHT:
                case animations::TransitionSubType::TOPTOBOTTOM:
                case animations::TransitionSubType::TOPRIGHT:
                case animations::TransitionSubType::TOPLEFT:
                case animations::TransitionSubType::BOTTOMRIGHT:
                case animations::TransitionSubType::BOTTOMLEFT:
                case animations::TransitionSubType::TOPCENTER:
                case animations::TransitionSubType::RIGHTCENTER:
                case animations::TransitionSubType::BOTTOMCENTER:
                    return sal_True;

                default:
                    return sal_False;
            }
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::CROSSFADE ) {
            return sal_True;
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::FADEOVERCOLOR ) {
            return sal_True;
        } else if( transitionType == animations::TransitionType::IRISWIPE && transitionSubType == animations::TransitionSubType::DIAMOND ) {
            return sal_True;
        } else if( transitionType == animations::TransitionType::ZOOM && transitionSubType == animations::TransitionSubType::ROTATEIN ) {
            return sal_True;
        } else
            return sal_False;
    }

    virtual uno::Reference< presentation::XTransition > SAL_CALL createTransition(
        ::sal_Int16                                           transitionType,
        ::sal_Int16                                           transitionSubType,
        const uno::Reference< presentation::XSlideShowView >& view,
        const uno::Reference< rendering::XBitmap >&           leavingBitmap,
        const uno::Reference< rendering::XBitmap >&           enteringBitmap )
    throw (uno::RuntimeException)
    {
        if( !hasTransition( transitionType, transitionSubType ) )
            return uno::Reference< presentation::XTransition >();

        rtl::Reference< OGLTransitionerImpl > xRes( new OGLTransitionerImpl() );
        if ( !xRes->initialize( view ) )
            return uno::Reference< presentation::XTransition >();

        if( OGLTransitionerImpl::cbMesa && (
            ( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::CROSSFADE ) ||
            ( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::FADEOVERCOLOR ) ||
            ( transitionType == animations::TransitionType::IRISWIPE && transitionSubType == animations::TransitionSubType::DIAMOND ) ) )
            return uno::Reference< presentation::XTransition >();

        OGLTransitionImpl* pTransition = NULL;

        if( transitionType == animations::TransitionType::MISCSHAPEWIPE ) {
            pTransition = new OGLTransitionImpl();
            switch( transitionSubType )
                {
                case animations::TransitionSubType::ACROSS:
                    pTransition->makeNByMTileFlip(8,6);
                    break;
                case animations::TransitionSubType::CORNERSOUT:
                    pTransition->makeOutsideCubeFaceToLeft();
                    break;
                case animations::TransitionSubType::CIRCLE:
                    pTransition->makeRevolvingCircles(8,128);
                    break;
                case animations::TransitionSubType::FANOUTHORIZONTAL:
                    pTransition->makeHelix(20);
                    break;
                case animations::TransitionSubType::CORNERSIN:
                    pTransition->makeInsideCubeFaceToLeft();
                    break;
                case animations::TransitionSubType::LEFTTORIGHT:
                    pTransition->makeFallLeaving();
                    break;
                case animations::TransitionSubType::TOPTOBOTTOM:
                    pTransition->makeTurnAround();
                    break;
                case animations::TransitionSubType::TOPRIGHT:
                    pTransition->makeTurnDown();
                    break;
                case animations::TransitionSubType::TOPLEFT:
                    pTransition->makeIris();
                    break;
                case animations::TransitionSubType::BOTTOMRIGHT:
                    pTransition->makeRochade();
                    break;
                case animations::TransitionSubType::BOTTOMLEFT:
                    pTransition->makeVenetianBlinds( true, 8 );
                    break;
                case animations::TransitionSubType::TOPCENTER:
                    pTransition->makeVenetianBlinds( false, 6 );
                    break;
                case animations::TransitionSubType::RIGHTCENTER:
                    pTransition->makeStatic();
                    break;
                case animations::TransitionSubType::BOTTOMCENTER:
                    pTransition->makeDissolve();
                    break;
                }
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::CROSSFADE ) {
            pTransition = new OGLTransitionImpl();
            pTransition->makeFadeSmoothly();
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::FADEOVERCOLOR ) {
            pTransition = new OGLTransitionImpl();
            pTransition->makeFadeThroughBlack();
        } else if( transitionType == animations::TransitionType::IRISWIPE && transitionSubType == animations::TransitionSubType::DIAMOND ) {
            pTransition = new OGLTransitionImpl();
            pTransition->makeDiamond();
        } else if( transitionType == animations::TransitionType::ZOOM && transitionSubType == animations::TransitionSubType::ROTATEIN ) {
            pTransition = new OGLTransitionImpl();
            pTransition->makeNewsflash();
        }

        if ( !pTransition )
            return uno::Reference< presentation::XTransition >();

        xRes->setTransition( pTransition );
        xRes->setSlides(leavingBitmap,enteringBitmap);

        return uno::Reference<presentation::XTransition>(xRes.get());
    }
};

}

namespace sdecl = comphelper::service_decl;
 const sdecl::ServiceDecl OGLTransitionFactoryDecl(
     sdecl::class_<OGLTransitionFactoryImpl>(),
    "com.sun.star.comp.presentation.OGLTransitionFactory",
    "com.sun.star.presentation.TransitionFactory" );

// The C shared lib entry points
COMPHELPER_SERVICEDECL_EXPORTS1(ogltrans, OGLTransitionFactoryDecl)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
