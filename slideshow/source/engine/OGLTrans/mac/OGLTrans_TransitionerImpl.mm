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
#include <vcl/sysdata.hxx>

#include <boost/noncopyable.hpp>

#include <premac.h>
#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include "aquaOpenGLView.h"
#include <postmac.h>

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

#ifdef DEBUG
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace ::boost::posix_time;

static ptime t1;
static ptime t2;

#define DBG(x) x
#else
#define DBG(x)
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

namespace
{
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
}

/** This is the Transitioner class for OpenGL 3D transitions in
 * slideshow. This class is implicitly
 * constructed from XTransitionFactory.
*/
class OGLTransitionerImpl : private cppu::BaseMutex, private boost::noncopyable, public OGLTransitionerImplBase
{
public:
    explicit OGLTransitionerImpl(OGLTransitionImpl* pOGLTransition);
    bool initWindowFromSlideShowView( const uno::Reference< presentation::XSlideShowView >& xView );
    void setSlides( const Reference< rendering::XBitmap >& xLeavingSlide , const uno::Reference< rendering::XBitmap >& xEnteringSlide );
    static bool initialize( const Reference< presentation::XSlideShowView >& xView );

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

    void createTexture(GLuint* texID,
            bool useMipmap,
            uno::Sequence<sal_Int8>& data,
            const OGLFormat* pFormat );
    void prepareEnvironment ();
    const OGLFormat* chooseFormats();

private:    
    /** After the window has been created, and the slides have been set, we'll initialize the slides with OpenGL.
    */
    void GLInitSlides();


    /// Holds the information of our new child window
    struct GLWindow
    {
        AquaOpenGLView*         pAquaOpenGLView; // Custom Cocoa NSOpenGLView
        NSOpenGLContext*        pOpenGLContext; // our OpenGLContext
        NSRect                  aInitFrame;
        NSView*                 apView;
        NSScreen*               screen;
        unsigned int			bpp;
        unsigned int            Width;
        unsigned int            Height;
        const char*             GLXExtensions;
        const GLubyte*          GLExtensions;

        bool HasGLXExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, (const GLubyte*) GLXExtensions ); }
    bool HasGLExtension( const char* name ) { return gluCheckExtension( (const GLubyte*) name, GLExtensions ); }
    } GLWin;
    
    /** OpenGL handle to the leaving slide's texture
    */
#if defined(MAC_OS_X_VERSION_10_5) || defined(MAC_OS_X_VERSION_10_6)
    GLuint GLleavingSlide;
#else /* build target 10.4 */ 
    unsigned long int GLleavingSlide;
#endif

    /** OpenGL handle to the entering slide's texture
    */
#if defined(MAC_OS_X_VERSION_10_5) || defined(MAC_OS_X_VERSION_10_6)
    GLuint GLenteringSlide;
#else /* build target 10.4 */ 
    unsigned long int GLenteringSlide;
#endif 
    
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
    bool mbRestoreSync;
    bool mbUseLeavingPixmap;
    bool mbUseEnteringPixmap;
//    unx::Pixmap maLeavingPixmap;
//    unx::Pixmap maEnteringPixmap;
    
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
//    bool mbHasTFPVisual;

#ifdef DEBUG
    ptime t3;
    ptime t4;
    ptime t5;
    ptime t6;
    time_duration total_update;
    int frame_count;
#endif
};

// declare the static variables as some gcc versions have problems declaring them automaticaly
/**/
bool OGLTransitionerImpl::cbBrokenTexturesATI;

float OGLTransitionerImpl::cnGLVersion;
/**/
bool OGLTransitionerImpl::cbMesa;

bool OGLTransitionerImpl::cbGLXPresent;


bool OGLTransitionerImpl::initialize( const Reference< presentation::XSlideShowView >& xView )
{
    // not thread safe

    // only once. This part is needed for the hardware identification 
    // [FIXME]: is there a Mac way of doing that ?
    static bool initialized = false;

    if( !initialized ) {
        OGLTransitionerImpl *instance;

        instance = new OGLTransitionerImpl( NULL );
        if( instance->initWindowFromSlideShowView( xView ) ) 
    
        {
            const GLubyte* version = glGetString( GL_VERSION );
            if( version && version[0] ) {
                cnGLVersion = version[0] - '0';
                if( version[1] == '.' && version[2] )
                    cnGLVersion += (version[2] - '0')/10.0;
            } else
                cnGLVersion = 1.0;
            OSL_TRACE("GL version: %s parsed: %f", version, cnGLVersion );

            const GLubyte* vendor = glGetString( GL_VENDOR );
            cbMesa = ( vendor && strstr( (const char *) vendor, "Mesa" ) );
            OSL_TRACE("GL vendor: %s identified as Mesa: %d", vendor, cbMesa );

            /* TODO: check for version once the bug in fglrx driver is fixed */
            cbBrokenTexturesATI = (vendor && strcmp( (const char *) vendor, "ATI Technologies Inc." ) == 0 );

            instance->disposing();
            cbGLXPresent = true;
        } else
            cbGLXPresent = false;

        delete instance;
        initialized = true;
    }

    return cbGLXPresent;
}

bool OGLTransitionerImpl::initWindowFromSlideShowView( const Reference< presentation::XSlideShowView >& xView )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    mxView.set( xView, UNO_QUERY );
    if( !mxView.is() )
        return false;

    /// take the XSlideShowView and extract the parent window from it. see viewmediashape.cxx
    uno::Reference< rendering::XCanvas > xCanvas(mxView->getCanvas(), uno::UNO_QUERY_THROW);
    uno::Sequence< uno::Any > aDeviceParams;
    ::canvas::tools::getDeviceInfo( xCanvas, aDeviceParams );

    ::rtl::OUString aImplName;
    aDeviceParams[ 0 ] >>= aImplName;

    sal_Int64 aVal = 0;
    aDeviceParams[1] >>= aVal;
    Window* pPWindow = reinterpret_cast< Window* >( aVal );
    GLWin.Width = pPWindow->GetSizePixel().Width();
    GLWin.Height = pPWindow->GetSizePixel().Height();
 
    const SystemEnvData* sysData(pPWindow->GetSystemData());

    GLWin.apView = sysData->pView;

    NSOpenGLPixelFormat* fmt = [AquaOpenGLView defaultPixelFormat];
    Window *pFrameWindow = pPWindow->GetWindow(WINDOW_FRAME);
    Size aFrameSize( pFrameWindow->GetSizePixel() );
    Point aScreen( pPWindow->OutputToScreenPixel( Point() ) );
    GLWin.aInitFrame = (NSRect){ { aScreen.X(), aFrameSize.Height() - GLWin.Height - aScreen.Y() }, { GLWin.Width, GLWin.Height } };

    GLWin.pAquaOpenGLView = (AquaOpenGLView *)[[NSOpenGLView alloc]initWithFrame: GLWin.aInitFrame pixelFormat: fmt];
    OSL_ENSURE(GLWin.pAquaOpenGLView, "Could not create NSOPenGLView");
    if( !GLWin.pAquaOpenGLView )
        return false;

    GLWin.pOpenGLContext = [GLWin.pAquaOpenGLView openGLContext];
    [GLWin.pOpenGLContext retain];
    [GLWin.pAquaOpenGLView setOpenGLContext:GLWin.pOpenGLContext];

    if( GLWin.pOpenGLContext == NULL ) {
    OSL_TRACE("unable to create GLX context");
    return false;
    }

    awt::Rectangle aCanvasArea = mxView->getCanvasArea();

    // needed in windowed mode only ?
    if( (aCanvasArea.X != 0) && (aCanvasArea.Y != 0) )
        pWindow->setPosSizePixel(aCanvasArea.X, aCanvasArea.Y, aCanvasArea.Width, aCanvasArea.Height);

    GLWin.Width = aCanvasArea.Width;
    GLWin.Height = aCanvasArea.Height;
    OSL_TRACE("canvas area: %d,%d - %dx%d", aCanvasArea.X, aCanvasArea.Y, aCanvasArea.Width, aCanvasArea.Height);

    mbTextureFromPixmap = GLWin.HasGLXExtension( "GLX_EXT_texture_from_pixmap" );
    mbGenerateMipmap = GLWin.HasGLExtension( "GL_SGIS_generate_mipmap" );

    [GLWin.apView addSubview:GLWin.pAquaOpenGLView];

    if( pWindow )
    {
        pWindow->SetMouseTransparent( sal_True );
        pWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        pWindow->EnableEraseBackground( sal_False );
        pWindow->SetControlForeground();
        pWindow->SetControlBackground();
        pWindow->EnablePaint(sal_False);
        pWindow->SetPosSizePixel(pPWindow->GetPosPixel(),pPWindow->GetSizePixel());

    }


    glShadeModel( GL_SMOOTH );
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor (0.0f, 0.0f, 0.0f, 0.0f);  // R G B A
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    glEnable(GL_TEXTURE_2D);
   
    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    [[GLWin.pAquaOpenGLView openGLContext] flushBuffer];

    if( LeavingBytes.hasElements() && EnteringBytes.hasElements())
       GLInitSlides();//we already have uninitialized slides, let's initialize 

    if( pTransition && pTransition->mnRequiredGLVersion <= cnGLVersion )
        pTransition->prepare( GLleavingSlide, GLenteringSlide );

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

    OSL_TRACE("leaving bitmap area: %dx%d", SlideSize.Width, SlideSize.Height);
    SlideSize = mxEnteringBitmap->getSize();
    OSL_TRACE("entering bitmap area: %dx%d", SlideSize.Width, SlideSize.Height);

#ifdef DEBUG
    t1 = microsec_clock::local_time();
#endif

    mbUseLeavingPixmap = false;
    mbUseEnteringPixmap = false;

    if( !mbUseLeavingPixmap )
    LeavingBytes = mxLeavingBitmap->getData(SlideBitmapLayout,SlideRect);
    if( !mbUseEnteringPixmap )
    EnteringBytes = mxEnteringBitmap->getData(SlideBitmapLayout,SlideRect);

    if(GLWin.pOpenGLContext)//if we have a rendering context, let's init the slides
        GLInitSlides();

    OSL_ENSURE(SlideBitmapLayout.PlaneStride == 0,"only handle no plane stride now");

    /* flush & sync */
    mbRestoreSync = true;
}

void OGLTransitionerImpl::createTexture( GLuint* texID,
//#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
//					 unx::GLXPixmap pixmap,
//					 bool usePixmap,
//#endif
                     bool useMipmap,
                     uno::Sequence<sal_Int8>& data,
                     const OGLFormat* pFormat )
{
    glDeleteTextures( 1, texID );
    glGenTextures( 1, texID );
    glBindTexture( GL_TEXTURE_2D, *texID );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
/*
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::PFNGLXBINDTEXIMAGEEXTPROC myglXBindTexImageEXT = (unx::PFNGLXBINDTEXIMAGEEXTPROC) unx::glXGetProcAddress( (const GLubyte*) "glXBindTexImageEXT" );

    if( usePixmap ) {
      if( mbGenerateMipmap )
          glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, True);
      myglXBindTexImageEXT (GLWin.dpy, pixmap, GLX_FRONT_LEFT_EXT, NULL);
      if( mbGenerateMipmap && useMipmap ) {
          OSL_TRACE("use mipmaps");
          glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); //TRILINEAR FILTERING
      } else {
          glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
      }
    } else {
#endif
*/
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

    prepareEnvironment();

    const OGLFormat* pFormat = NULL;
    if( !mbUseLeavingPixmap || !mbUseEnteringPixmap )
    pFormat = chooseFormats();

    createTexture( &GLleavingSlide,
           pTransition->mbUseMipMapLeaving,
           LeavingBytes,
           pFormat );

    createTexture( &GLenteringSlide,

           pTransition->mbUseMipMapEntering,
           EnteringBytes,
           pFormat );

#ifdef DEBUG
    t2 = microsec_clock::local_time();
    OSL_TRACE("textures created in: %s", to_simple_string( t2 - t1 ).c_str());
#endif
}

void SAL_CALL OGLTransitionerImpl::update( double nTime ) throw (uno::RuntimeException)
{
#ifdef DEBUG
    frame_count ++;
    t3 = microsec_clock::local_time();
    if( frame_count == 1 ) {
    t5 = t3;
    total_update = seconds (0);
    }
#endif
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed() || !cbGLXPresent || pTransition->mnRequiredGLVersion > cnGLVersion)
        return;

    if(GLWin.pOpenGLContext)
        [GLWin.pOpenGLContext makeCurrentContext];

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*
    if(pTransition)
    pTransition->display( nTime, GLleavingSlide, GLenteringSlide, 
                              SlideSize.Width, SlideSize.Height,
                              static_cast<double>(GLWin.Width),
                              static_cast<double>(GLWin.Height) );
*/
    // works but not mandatory
#if defined(MAC_OS_X_VERSION_10_5)
    GLint swapInt = 1;
#else /* build target 10.4 */ 
    long swapInt = 1;
#endif
    [[GLWin.pAquaOpenGLView openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval]; // set to vbl sync

    NSOpenGLContext* context = [GLWin.pAquaOpenGLView openGLContext];
    [context makeCurrentContext];

    if(pTransition)
    pTransition->display( nTime, GLleavingSlide, GLenteringSlide, 
                              SlideSize.Width, SlideSize.Height,
                              static_cast<double>(GLWin.Width),
                              static_cast<double>(GLWin.Height) );


    [context flushBuffer];

    if( pWindow )
        pWindow->Show();

#ifdef DEBUG
    t4 = microsec_clock::local_time();

    OSL_TRACE("update time: %f", nTime);
    OSL_TRACE("update took: %s", to_simple_string( t4 - t3 ).c_str());
    total_update += (t4 - t3);
#endif
}

void SAL_CALL OGLTransitionerImpl::viewChanged( const Reference< presentation::XSlideShowView >& rView,
                        const Reference< rendering::XBitmap >& rLeavingBitmap,
                        const Reference< rendering::XBitmap >& rEnteringBitmap )
    throw (uno::RuntimeException)
{
    OSL_TRACE("transitioner: view changed");

    disposeTextures();
    disposeContextAndWindow();

    initWindowFromSlideShowView( rView );

    setSlides( rLeavingBitmap, rEnteringBitmap );
}

void OGLTransitionerImpl::disposeContextAndWindow()
{
 /*
    if(GLWin.pOpenGLContext)
        [GLWin.pOpenGLContext makeCurrentContext];
    if( glGetError() != GL_NO_ERROR )
    {
        OSL_TRACE("glError: %s", (char *)gluErrorString(glGetError()));
        if(GLWin.pOpenGLContext)
            [GLWin.pOpenGLContext clearGLContext];
    }
*/
    if(GLWin.pAquaOpenGLView)
    {
        [GLWin.pAquaOpenGLView clearGLContext];// currentContext]; //clearCurrentContext];
        [GLWin.pAquaOpenGLView willRemoveSubview:GLWin.apView];
        [GLWin.pAquaOpenGLView removeFromSuperview];
        [GLWin.pAquaOpenGLView release];
    }

    if( pWindow )
    {
        delete pWindow;
        pWindow = NULL;
        GLWin.pAquaOpenGLView = nil;
    }
}

void OGLTransitionerImpl::disposeTextures()
{
    if(GLWin.pOpenGLContext)
        [GLWin.pOpenGLContext makeCurrentContext];

    if( !mbUseLeavingPixmap )
    {
        glDeleteTextures(1,&GLleavingSlide);
        GLleavingSlide = 0;
    }

    if( !mbUseEnteringPixmap )
    {
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

#ifdef DEBUG
    OSL_TRACE("dispose %p", this);
    if( frame_count ) {
    t6 = microsec_clock::local_time();
    time_duration duration = t6 - t5;
    OSL_TRACE("whole transition (frames: %d) took: %s fps: %f time spent in updates: %s percentage of transition time: %f%%",
          frame_count, to_simple_string( duration ).c_str(),
          ((double)frame_count*1000000000.0)/duration.total_nanoseconds(),
          to_simple_string( total_update ).c_str(),
          100*(((double)total_update.total_nanoseconds())/((double)duration.total_nanoseconds()))
        );
    }
#endif

    if( pWindow )
    {
        disposeTextures();

        if (pTransition)
            pTransition->finish();

        disposeContextAndWindow();
    }

    if (pTransition)
    delete pTransition;

    mxLeavingBitmap.clear();
    mxEnteringBitmap.clear();
    mxView.clear();
}

OGLTransitionerImpl::OGLTransitionerImpl(OGLTransitionImpl* pOGLTransition) : 
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
    pTransition(pOGLTransition)
{
    GLWin.pAquaOpenGLView = nil;
    DBG(frame_count = 0);
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

        bool bGLXPresent = OGLTransitionerImpl::initialize( view );

        if( 
            ( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::CROSSFADE ) ||
            ( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::FADEOVERCOLOR ) ||
            ( transitionType == animations::TransitionType::IRISWIPE && transitionSubType == animations::TransitionSubType::DIAMOND ) )
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

        rtl::Reference<OGLTransitionerImpl> xRes(
            new OGLTransitionerImpl(pTransition) );
        if( bGLXPresent ) {
            if( !xRes->initWindowFromSlideShowView(view))
                return uno::Reference< presentation::XTransition >();
            xRes->setSlides(leavingBitmap,enteringBitmap);
        }

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
