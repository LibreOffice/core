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

// Includes <GL/gl.h>
#include "OGLTrans_TransitionImpl.hxx"

#include <sal/types.h>

#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#undef IN
#undef OUT
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

#include <tools/diagnose_ex.h>

#include <vcl/canvastools.hxx>
#include <vcl/window.hxx>
#include <vcl/syschild.hxx>

#include <boost/noncopyable.hpp>

#if defined( _WIN32 )
    // OGLTrans_TransitionImpl.hxx already included <prewin.h> and thus <windows.h>
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
    explicit TimerContext(OUString const& rWhat)
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
    OUString const m_aWhat;
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

#ifdef UNX

// not thread safe
static bool errorTriggered;
int oglErrorHandler( unx::Display* /*dpy*/, unx::XErrorEvent* /*evnt*/ )
{
    errorTriggered = true;

    return 0;
}

#endif

/** This is the Transitioner class for OpenGL 3D transitions in
 * slideshow. At the moment, it's Linux only. This class is implicitly
 * constructed from XTransitionFactory.
*/
class OGLTransitionerImpl : private cppu::BaseMutex, private boost::noncopyable, public OGLTransitionerImplBase
{
public:
    OGLTransitionerImpl();
    void setTransition( boost::shared_ptr<OGLTransitionImpl> pOGLTransition );
    bool initialize( const Reference< presentation::XSlideShowView >& xView,
            const Reference< rendering::XBitmap >& xLeavingSlide,
            const Reference< rendering::XBitmap >& xEnteringSlide );

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

    void setSlides( const Reference< rendering::XBitmap >& xLeavingSlide , const uno::Reference< rendering::XBitmap >& xEnteringSlide );
    void impl_prepareSlides();

    void impl_createTexture( bool useMipmap, uno::Sequence<sal_Int8>& data, const OGLFormat* pFormat );

    bool initWindowFromSlideShowView( const uno::Reference< presentation::XSlideShowView >& xView );
    /** After the window has been created, and the slides have been set, we'll initialize the slides with OpenGL.
    */
    void GLInitSlides();

    void impl_prepareTransition();
    void impl_finishTransition();

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
#ifdef UNX
    unx::Pixmap maLeavingPixmap;
    unx::Pixmap maEnteringPixmap;
#endif

    /** the form the raw bytes are in for the bitmaps
    */
    rendering::IntegerBitmapLayout SlideBitmapLayout;

    /** the size of the slides
    */
    geometry::IntegerSize2D SlideSize;

    /** Our Transition to be used.
    */
    boost::shared_ptr<OGLTransitionImpl> mpTransition;

public:
    /** whether we are running on ATI fglrx with bug related to textures
     */
    static bool cbBrokenTexturesATI;

    /** GL version
     */
    static float cnGLVersion;

#ifdef UNX
    float mnGLXVersion;
#endif

    /**
       Whether the display has GLX extension on X11, always true otherwise (?)
     */
    static bool cbGLXPresent;

    /** Whether Mesa is the OpenGL vendor
     */
    static bool cbMesa;

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

bool OGLTransitionerImpl::initialize( const Reference< presentation::XSlideShowView >& xView,
        const Reference< rendering::XBitmap >& xLeavingSlide,
        const Reference< rendering::XBitmap >& xEnteringSlide )
{
    bool const bGLXPresent( initWindowFromSlideShowView( xView ) );
    impl_initializeOnce( bGLXPresent );

    setSlides( xLeavingSlide, xEnteringSlide );

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
#if defined( _WIN32 )
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
                if( !visinfo )
                    continue;

                unx::VisualID visualid = visinfo->visualid;
                XFree ( visinfo );

                if ( visualid != vi->visualid )
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
            if (vi != firstVisual)
                XFree (vi);

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
            XFree ( fbconfigs );
#else
            pChildSysData = lcl_createSystemWindow( vi, pPWindow, &pWindow );
            XFree ( vi );
#endif
            if ( pChildSysData )
                break;
        }

        ++pAttributeTable;
    }

#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    if ( firstVisual && vi != firstVisual )
        XFree (firstVisual);
#endif

#endif

#if defined( _WIN32 )
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
#if defined( _WIN32 )
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

    OUString aImplName;
    aDeviceParams[ 0 ] >>= aImplName;

    sal_Int64 aVal = 0;
    aDeviceParams[1] >>= aVal;
    if( !createWindow( reinterpret_cast< Window* >( aVal ) ) )
        return false;

    awt::Rectangle aCanvasArea = mxView->getCanvasArea();
    pWindow->setPosSizePixel(aCanvasArea.X, aCanvasArea.Y, aCanvasArea.Width, aCanvasArea.Height);
    GLWin.Width = aCanvasArea.Width;
    GLWin.Height = aCanvasArea.Height;
    SAL_INFO("slideshow.opengl", "canvas area: " << aCanvasArea.X << "," << aCanvasArea.Y << " - " << aCanvasArea.Width << "x" << aCanvasArea.Height);

#if defined( _WIN32 )
    GLWin.hDC = GetDC(GLWin.hWnd);
#elif defined( UNX )
    GLWin.ctx = glXCreateContext(GLWin.dpy,
                                 GLWin.vi,
                                 0,
                                 GL_TRUE);
    XFree ( GLWin.vi );
    if( GLWin.ctx == NULL ) {
        SAL_INFO("slideshow.opengl", "unable to create GLX context");
        return false;
    }
#endif

#if defined( _WIN32 )
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
    GLWin.hRC = wglCreateContext(GLWin.hDC);
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
#if defined( _WIN32 )
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

    SlideSize = mxLeavingBitmap->getSize();
    SAL_INFO("slideshow.opengl", "leaving bitmap area: " << SlideSize.Width << "x" << SlideSize.Height);
    SlideSize = mxEnteringBitmap->getSize();
    SAL_INFO("slideshow.opengl", "entering bitmap area: " << SlideSize.Width << "x" << SlideSize.Height);
}


void OGLTransitionerImpl::impl_prepareSlides()
{
    Reference< XFastPropertySet > xLeavingSet( mxLeavingBitmap , UNO_QUERY );
    Reference< XFastPropertySet > xEnteringSet( mxEnteringBitmap , UNO_QUERY );

    geometry::IntegerRectangle2D SlideRect;
    SlideRect.X1 = 0;
    SlideRect.X2 = SlideSize.Width;
    SlideRect.Y1 = 0;
    SlideRect.Y2 = SlideSize.Height;

#ifdef UNX
    unx::glXWaitGL();
    XSync(GLWin.dpy, false);
#endif

    mbUseLeavingPixmap = false;
    mbUseEnteringPixmap = false;

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
    if( !mbUseLeavingPixmap )
        LeavingBytes = mxLeavingBitmap->getData(SlideBitmapLayout,SlideRect);
    if( !mbUseEnteringPixmap )
        EnteringBytes = mxEnteringBitmap->getData(SlideBitmapLayout,SlideRect);

// TODO
#ifdef UNX
    if(GLWin.ctx)//if we have a rendering context, let's init the slides
        GLInitSlides();
#else
    GLInitSlides();
#endif

    SAL_WARN_IF(SlideBitmapLayout.PlaneStride != 0, "slideshow.opengl","only handle no plane stride now");

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

void OGLTransitionerImpl::impl_prepareTransition()
{
    if( mpTransition && mpTransition->getSettings().mnRequiredGLVersion <= cnGLVersion )
        mpTransition->prepare( GLleavingSlide, GLenteringSlide );
}

void OGLTransitionerImpl::impl_finishTransition()
{
    if( mpTransition && mpTransition->getSettings().mnRequiredGLVersion <= cnGLVersion )
        mpTransition->finish();
}

void OGLTransitionerImpl::setTransition( boost::shared_ptr<OGLTransitionImpl> const pTransition )
{
    if ( mpTransition ) // already initialized
        return;

    mpTransition = pTransition;

    impl_prepareSlides();
    impl_prepareTransition();
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
        impl_createTexture( useMipmap, data, pFormat );
    }
#else
    impl_createTexture( useMipmap, data, pFormat );
#endif
    SAL_WARN_IF(!glIsTexture(*texID), "slideshow.opengl", "Can't generate Leaving slide textures in OpenGL");
}

namespace
{
    class OGLColorSpace : public cppu::WeakImplHelper1< com::sun::star::rendering::XIntegerBitmapColorSpace >
    {
    private:
        uno::Sequence< sal_Int8 >  maComponentTags;
        uno::Sequence< sal_Int32 > maBitCounts;

        virtual ::sal_Int8 SAL_CALL getType(  ) throw (uno::RuntimeException)
        {
            return rendering::ColorSpaceType::RGB;
        }
        virtual uno::Sequence< ::sal_Int8 > SAL_CALL getComponentTags(  ) throw (uno::RuntimeException)
        {
            return maComponentTags;
        }
        virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (uno::RuntimeException)
        {
            return rendering::RenderingIntent::PERCEPTUAL;
        }
        virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) throw (uno::RuntimeException)
        {
            return uno::Sequence< beans::PropertyValue >();
        }
        virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >& deviceColor,
                                                                    const uno::Reference< rendering::XColorSpace >& targetColorSpace ) throw (lang::IllegalArgumentException,
                                                                                                                                                uno::RuntimeException)
        {
            // TODO(P3): if we know anything about target
            // colorspace, this can be greatly sped up
            uno::Sequence<rendering::ARGBColor> aIntermediate(
                convertToARGB(deviceColor));
            return targetColorSpace->convertFromARGB(aIntermediate);
        }
        virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const double*  pIn( deviceColor.getConstArray() );
            const sal_Size nLen( deviceColor.getLength() );
            ENSURE_ARG_OR_THROW2(nLen%4==0,
                                    "number of channels no multiple of 4",
                                    static_cast<rendering::XColorSpace*>(this), 0);

            uno::Sequence< rendering::RGBColor > aRes(nLen/4);
            rendering::RGBColor* pOut( aRes.getArray() );
            for( sal_Size i=0; i<nLen; i+=4 )
            {
                *pOut++ = rendering::RGBColor(pIn[0],pIn[1],pIn[2]);
                pIn += 4;
            }
            return aRes;
        }
        virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const double*  pIn( deviceColor.getConstArray() );
            const sal_Size nLen( deviceColor.getLength() );
            ENSURE_ARG_OR_THROW2(nLen%4==0,
                                    "number of channels no multiple of 4",
                                    static_cast<rendering::XColorSpace*>(this), 0);

            uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
            rendering::ARGBColor* pOut( aRes.getArray() );
            for( sal_Size i=0; i<nLen; i+=4 )
            {
                *pOut++ = rendering::ARGBColor(pIn[3],pIn[0],pIn[1],pIn[2]);
                pIn += 4;
            }
            return aRes;
        }
        virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const double*  pIn( deviceColor.getConstArray() );
            const sal_Size nLen( deviceColor.getLength() );
            ENSURE_ARG_OR_THROW2(nLen%4==0,
                                    "number of channels no multiple of 4",
                                    static_cast<rendering::XColorSpace*>(this), 0);

            uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
            rendering::ARGBColor* pOut( aRes.getArray() );
            for( sal_Size i=0; i<nLen; i+=4 )
            {
                *pOut++ = rendering::ARGBColor(pIn[3],pIn[3]*pIn[0],pIn[3]*pIn[1],pIn[3]*pIn[2]);
                pIn += 4;
            }
            return aRes;
        }
        virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const rendering::RGBColor* pIn( rgbColor.getConstArray() );
            const sal_Size             nLen( rgbColor.getLength() );

            uno::Sequence< double > aRes(nLen*4);
            double* pColors=aRes.getArray();
            for( sal_Size i=0; i<nLen; ++i )
            {
                *pColors++ = pIn->Red;
                *pColors++ = pIn->Green;
                *pColors++ = pIn->Blue;
                *pColors++ = 1.0;
                ++pIn;
            }
            return aRes;
        }
        virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
            const sal_Size              nLen( rgbColor.getLength() );

            uno::Sequence< double > aRes(nLen*4);
            double* pColors=aRes.getArray();
            for( sal_Size i=0; i<nLen; ++i )
            {
                *pColors++ = pIn->Red;
                *pColors++ = pIn->Green;
                *pColors++ = pIn->Blue;
                *pColors++ = pIn->Alpha;
                ++pIn;
            }
            return aRes;
        }
        virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
            const sal_Size              nLen( rgbColor.getLength() );

            uno::Sequence< double > aRes(nLen*4);
            double* pColors=aRes.getArray();
            for( sal_Size i=0; i<nLen; ++i )
            {
                *pColors++ = pIn->Red/pIn->Alpha;
                *pColors++ = pIn->Green/pIn->Alpha;
                *pColors++ = pIn->Blue/pIn->Alpha;
                *pColors++ = pIn->Alpha;
                ++pIn;
            }
            return aRes;
        }

        // XIntegerBitmapColorSpace
        virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) throw (uno::RuntimeException)
        {
            return 32;
        }
        virtual uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) throw (uno::RuntimeException)
        {
            return maBitCounts;
        }
        virtual ::sal_Int8 SAL_CALL getEndianness(  ) throw (uno::RuntimeException)
        {
            return util::Endianness::LITTLE;
        }
        virtual uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                const uno::Reference< rendering::XColorSpace >& targetColorSpace ) throw (lang::IllegalArgumentException,
                                                                                                                                                        uno::RuntimeException)
        {
            if( dynamic_cast<OGLColorSpace*>(targetColorSpace.get()) )
            {
                const sal_Int8* pIn( deviceColor.getConstArray() );
                const sal_Size  nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                        "number of channels no multiple of 4",
                                        static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence<double> aRes(nLen);
                double* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                    *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                    *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                    *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                }
                return aRes;
            }
            else
            {
                // TODO(P3): if we know anything about target
                // colorspace, this can be greatly sped up
                uno::Sequence<rendering::ARGBColor> aIntermediate(
                    convertIntegerToARGB(deviceColor));
                return targetColorSpace->convertFromARGB(aIntermediate);
            }
        }
        virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                    const uno::Reference< rendering::XIntegerBitmapColorSpace >& targetColorSpace ) throw (lang::IllegalArgumentException,
                                                                                                                                                                        uno::RuntimeException)
        {
            if( dynamic_cast<OGLColorSpace*>(targetColorSpace.get()) )
            {
                // it's us, so simply pass-through the data
                return deviceColor;
            }
            else
            {
                // TODO(P3): if we know anything about target
                // colorspace, this can be greatly sped up
                uno::Sequence<rendering::ARGBColor> aIntermediate(
                    convertIntegerToARGB(deviceColor));
                return targetColorSpace->convertIntegerFromARGB(aIntermediate);
            }
        }
        virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertIntegerToRGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const sal_Int8* pIn( deviceColor.getConstArray() );
            const sal_Size  nLen( deviceColor.getLength() );
            ENSURE_ARG_OR_THROW2(nLen%4==0,
                                    "number of channels no multiple of 4",
                                    static_cast<rendering::XColorSpace*>(this), 0);

            uno::Sequence< rendering::RGBColor > aRes(nLen/4);
            rendering::RGBColor* pOut( aRes.getArray() );
            for( sal_Size i=0; i<nLen; i+=4 )
            {
                *pOut++ = rendering::RGBColor(
                    vcl::unotools::toDoubleColor(pIn[0]),
                    vcl::unotools::toDoubleColor(pIn[1]),
                    vcl::unotools::toDoubleColor(pIn[2]));
                pIn += 4;
            }
            return aRes;
        }

        virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const sal_Int8* pIn( deviceColor.getConstArray() );
            const sal_Size  nLen( deviceColor.getLength() );
            ENSURE_ARG_OR_THROW2(nLen%4==0,
                                    "number of channels no multiple of 4",
                                    static_cast<rendering::XColorSpace*>(this), 0);

            uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
            rendering::ARGBColor* pOut( aRes.getArray() );
            for( sal_Size i=0; i<nLen; i+=4 )
            {
                *pOut++ = rendering::ARGBColor(
                    vcl::unotools::toDoubleColor(pIn[3]),
                    vcl::unotools::toDoubleColor(pIn[0]),
                    vcl::unotools::toDoubleColor(pIn[1]),
                    vcl::unotools::toDoubleColor(pIn[2]));
                pIn += 4;
            }
            return aRes;
        }

        virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const sal_Int8* pIn( deviceColor.getConstArray() );
            const sal_Size  nLen( deviceColor.getLength() );
            ENSURE_ARG_OR_THROW2(nLen%4==0,
                                    "number of channels no multiple of 4",
                                    static_cast<rendering::XColorSpace*>(this), 0);

            uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
            rendering::ARGBColor* pOut( aRes.getArray() );
            for( sal_Size i=0; i<nLen; i+=4 )
            {
                const sal_Int8 nAlpha( pIn[3] );
                *pOut++ = rendering::ARGBColor(
                    vcl::unotools::toDoubleColor(nAlpha),
                    vcl::unotools::toDoubleColor(nAlpha*pIn[0]),
                    vcl::unotools::toDoubleColor(nAlpha*pIn[1]),
                    vcl::unotools::toDoubleColor(nAlpha*pIn[2]));
                pIn += 4;
            }
            return aRes;
        }

        virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const rendering::RGBColor* pIn( rgbColor.getConstArray() );
            const sal_Size             nLen( rgbColor.getLength() );

            uno::Sequence< sal_Int8 > aRes(nLen*4);
            sal_Int8* pColors=aRes.getArray();
            for( sal_Size i=0; i<nLen; ++i )
            {
                *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                *pColors++ = -1;
                ++pIn;
            }
            return aRes;
        }

        virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
            const sal_Size              nLen( rgbColor.getLength() );

            uno::Sequence< sal_Int8 > aRes(nLen*4);
            sal_Int8* pColors=aRes.getArray();
            for( sal_Size i=0; i<nLen; ++i )
            {
                *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                *pColors++ = vcl::unotools::toByteColor(pIn->Alpha);
                ++pIn;
            }
            return aRes;
        }

        virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
        {
            const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
            const sal_Size              nLen( rgbColor.getLength() );

            uno::Sequence< sal_Int8 > aRes(nLen*4);
            sal_Int8* pColors=aRes.getArray();
            for( sal_Size i=0; i<nLen; ++i )
            {
                *pColors++ = vcl::unotools::toByteColor(pIn->Red/pIn->Alpha);
                *pColors++ = vcl::unotools::toByteColor(pIn->Green/pIn->Alpha);
                *pColors++ = vcl::unotools::toByteColor(pIn->Blue/pIn->Alpha);
                *pColors++ = vcl::unotools::toByteColor(pIn->Alpha);
                ++pIn;
            }
            return aRes;
        }

    public:
        OGLColorSpace() :
            maComponentTags(4),
            maBitCounts(4)
        {
            sal_Int8*  pTags = maComponentTags.getArray();
            sal_Int32* pBitCounts = maBitCounts.getArray();
            pTags[0] = rendering::ColorComponentTag::RGB_RED;
            pTags[1] = rendering::ColorComponentTag::RGB_GREEN;
            pTags[2] = rendering::ColorComponentTag::RGB_BLUE;
            pTags[3] = rendering::ColorComponentTag::ALPHA;

            pBitCounts[0] =
            pBitCounts[1] =
            pBitCounts[2] =
            pBitCounts[3] = 8;
        }
    };

    struct OGLColorSpaceHolder : public rtl::StaticWithInit<uno::Reference<rendering::XIntegerBitmapColorSpace>, OGLColorSpaceHolder>
    {
        uno::Reference<rendering::XIntegerBitmapColorSpace> operator()()
        {
            return new OGLColorSpace();
        }
    };

    uno::Reference<rendering::XIntegerBitmapColorSpace>
    getOGLColorSpace()
    {
        return OGLColorSpaceHolder::get();
    }
}

void OGLTransitionerImpl::impl_createTexture(
                     bool useMipmap,
                     uno::Sequence<sal_Int8>& data,
                     const OGLFormat* pFormat )
{
    if( !pFormat )
    {
        // force-convert color to ARGB8888 int color space
        uno::Sequence<sal_Int8> tempBytes(
            SlideBitmapLayout.ColorSpace->convertToIntegerColorSpace(
                data,
                getOGLColorSpace()));
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
        if( mpTransition && !cbBrokenTexturesATI && !useMipmap) {
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
                    if ( nNumComponents == 4 )
                    {
                        pDetectedFormat = &lcl_ARGB32[nComponentOrderIndex];
                    }
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

    if (isDisposed() || mpTransition->getSettings().mnRequiredGLVersion > cnGLVersion)
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
           mpTransition->getSettings().mbUseMipMapLeaving,
           LeavingBytes,
           pFormat );

    createTexture( &GLenteringSlide,
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
           EnteringPixmap,
           mbUseEnteringPixmap,
#endif
           mpTransition->getSettings().mbUseMipMapEntering,
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

    if (isDisposed() || !cbGLXPresent || mpTransition->getSettings().mnRequiredGLVersion > cnGLVersion)
        return;

#ifdef _WIN32
    wglMakeCurrent(GLWin.hDC,GLWin.hRC);
#endif
#ifdef UNX
    glXMakeCurrent( GLWin.dpy, GLWin.win, GLWin.ctx );
#endif

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(mpTransition)
        mpTransition->display( nTime, GLleavingSlide, GLenteringSlide,
                              SlideSize.Width, SlideSize.Height,
                              static_cast<double>(GLWin.Width),
                              static_cast<double>(GLWin.Height) );

#if defined( _WIN32 )
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

    impl_finishTransition();
    disposeTextures();
    disposeContextAndWindow();

    initWindowFromSlideShowView( rView );
    setSlides( rLeavingBitmap, rEnteringBitmap );
    impl_prepareSlides();
    impl_prepareTransition();
}

void OGLTransitionerImpl::disposeContextAndWindow()
{
#if defined( _WIN32 )
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
        GLWin.win = 0;
    }
#endif
    if( pWindow ) {
        delete pWindow;
        pWindow = NULL;
    }
}

void OGLTransitionerImpl::disposeTextures()
{
#ifdef _WIN32
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
        GLenteringSlide = 0;
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
        impl_finishTransition();
        disposeTextures();

#ifdef UNX
        if( mbRestoreSync ) {
            // try to reestablish synchronize state
            char* sal_synchronize = getenv("SAL_SYNCHRONIZE");
            XSynchronize( GLWin.dpy, sal_synchronize && *sal_synchronize == '1' );
        }
#endif

        disposeContextAndWindow();
    }

    mpTransition.reset();

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
    SlideSize()
{
#if defined( _WIN32 )
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
        if ( !xRes->initialize( view, leavingBitmap, enteringBitmap ) )
            return uno::Reference< presentation::XTransition >();

        if( OGLTransitionerImpl::cbMesa && (
            ( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::CROSSFADE ) ||
            ( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::FADEOVERCOLOR ) ||
            ( transitionType == animations::TransitionType::IRISWIPE && transitionSubType == animations::TransitionSubType::DIAMOND ) ) )
            return uno::Reference< presentation::XTransition >();

        boost::shared_ptr<OGLTransitionImpl> pTransition;

        if( transitionType == animations::TransitionType::MISCSHAPEWIPE ) {
            switch( transitionSubType )
                {
                case animations::TransitionSubType::ACROSS:
                    pTransition = makeNByMTileFlip(8,6);
                    break;
                case animations::TransitionSubType::CORNERSOUT:
                    pTransition = makeOutsideCubeFaceToLeft();
                    break;
                case animations::TransitionSubType::CIRCLE:
                    pTransition = makeRevolvingCircles(8,128);
                    break;
                case animations::TransitionSubType::FANOUTHORIZONTAL:
                    pTransition = makeHelix(20);
                    break;
                case animations::TransitionSubType::CORNERSIN:
                    pTransition = makeInsideCubeFaceToLeft();
                    break;
                case animations::TransitionSubType::LEFTTORIGHT:
                    pTransition = makeFallLeaving();
                    break;
                case animations::TransitionSubType::TOPTOBOTTOM:
                    pTransition = makeTurnAround();
                    break;
                case animations::TransitionSubType::TOPRIGHT:
                    pTransition = makeTurnDown();
                    break;
                case animations::TransitionSubType::TOPLEFT:
                    pTransition = makeIris();
                    break;
                case animations::TransitionSubType::BOTTOMRIGHT:
                    pTransition = makeRochade();
                    break;
                case animations::TransitionSubType::BOTTOMLEFT:
                    pTransition = makeVenetianBlinds( true, 8 );
                    break;
                case animations::TransitionSubType::TOPCENTER:
                    pTransition = makeVenetianBlinds( false, 6 );
                    break;
                case animations::TransitionSubType::RIGHTCENTER:
                    pTransition = makeStatic();
                    break;
                case animations::TransitionSubType::BOTTOMCENTER:
                    pTransition = makeDissolve();
                    break;
                }
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::CROSSFADE ) {
            pTransition = makeFadeSmoothly();
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::FADEOVERCOLOR ) {
            pTransition = makeFadeThroughBlack();
        } else if( transitionType == animations::TransitionType::IRISWIPE && transitionSubType == animations::TransitionSubType::DIAMOND ) {
            pTransition = makeDiamond();
        } else if( transitionType == animations::TransitionType::ZOOM && transitionSubType == animations::TransitionSubType::ROTATEIN ) {
            pTransition = makeNewsflash();
        }

        if ( !pTransition )
            return uno::Reference< presentation::XTransition >();

        xRes->setTransition( pTransition );

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
