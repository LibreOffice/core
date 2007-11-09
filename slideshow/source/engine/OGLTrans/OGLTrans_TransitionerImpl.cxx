/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OGLTrans_TransitionerImpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 10:17:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "OGLTrans_TransitionImpl.hxx"

#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/IntegerBitmapFormat.hpp>
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

#include <GL/gl.h>
#include <GL/glu.h>
namespace unx
{
#include <X11/keysym.h>
#include <GL/glx.h>
}

using namespace ::com::sun::star;

namespace
{

typedef cppu::WeakComponentImplHelper1<presentation::XTransition> OGLTransitionerImplBase;

/** This is the Transitioner class for OpenGL 3D transitions in
 * slideshow. At the moment, it's Linux only. This class is implicitly
 * constructed from XTransitionFactory.
*/
class OGLTransitionerImpl : private cppu::BaseMutex, private boost::noncopyable, public OGLTransitionerImplBase
{
public:
    explicit OGLTransitionerImpl(OGLTransitionImpl* pOGLTransition);
    void initWindowFromSlideShowView( const uno::Reference< presentation::XSlideShowView >& xView, double, double);
    void setSlides( const uno::Reference< rendering::XBitmap >& xLeavingSlide , const uno::Reference< rendering::XBitmap >& xEnteringSlide );

    // XTransition
    virtual SAL_CALL void update( double nTime ) throw (uno::RuntimeException);

protected:
    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing();

    bool isDisposed() const
    {
        return (rBHelper.bDisposed || rBHelper.bInDispose);
    }

private:
    /** After the window has been created, and the slides have been set, we'll initialize the slides with OpenGL.
    */
    void GLInitSlides();

    /// Holds the information of our new child window
    struct GLWindow
    {
        unx::Display*           dpy;
        int                     screen;
        unx::Window             win;
        unx::GLXContext         ctx;
        unsigned int            bpp;
        unsigned int            Width;
        unsigned int            Height;
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

    /** raw bytes of the entering bitmap
    */
    uno::Sequence<signed char> EnteringBytes;

    /** raw bytes of the leaving bitmap
    */
    uno::Sequence<signed char> LeavingBytes;

    /** the form the raw bytes are in for the bitmaps
    */
    rendering::IntegerBitmapLayout SlideBitmapLayout;

    /** the size of the slides
    */
    geometry::IntegerSize2D SlideSize;

    /** Our Transition to be used.
    */
    OGLTransitionImpl* pTransition;
};

void OGLTransitionerImpl::initWindowFromSlideShowView( const uno::Reference< presentation::XSlideShowView >& xView, double, double)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    /// take the XSlideShowView and extract the parent window from it. see viewmediashape.cxx
    uno::Reference< rendering::XCanvas > xCanvas(xView->getCanvas(), uno::UNO_QUERY_THROW);
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
    GLWin.dpy = reinterpret_cast<unx::Display*>(sysData->pDisplay);
    GLWin.win = sysData->aWindow;
    GLWin.screen = unx::XDefaultScreen(GLWin.dpy);
    unx::XVisualInfo* vi( NULL );
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
            GLX_RGBA,/// only TrueColor or DirectColor
            /// single buffered
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,1,/// use the maximum depth bits, making sure there is a depth buffer
            None
        };
    static int attrList1[] =
        {
            GLX_RGBA,/// only TrueColor or DirectColor
            GLX_DOUBLEBUFFER,/// only double buffer
            GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
            GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
            GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
            GLX_DEPTH_SIZE,0,/// no depth buffer
            None
        };
    static int attrList0[] =
        {
            GLX_RGBA,/// only TrueColor or DirectColor
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
    while( *pAttributeTable )
    {
        // try to find a visual for the current set of attributes
        vi = unx::glXChooseVisual( GLWin.dpy,
                                   GLWin.screen,
                                   *pAttributeTable );

        if( vi )
        {
            SystemWindowData winData;
            winData.nSize = sizeof(winData);
            winData.pVisual = (void*)(vi->visual);
            pWindow=new SystemChildWindow(pPWindow,
                                          0,
                                          &winData);
            pChildSysData = pWindow->GetSystemData();
            if( pChildSysData )
            {
                break;
            }
            else
            {
                delete pWindow, pWindow=NULL;
            }
        }
        ++pAttributeTable;
    }

    if( pWindow )
    {
        OSL_TRACE("OGLTrans: using VisualID %08X\n",
                  vi->visualid);

        pWindow->SetMouseTransparent( TRUE );
        pWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        pWindow->EnableEraseBackground( FALSE );
        pWindow->SetControlForeground();
        pWindow->SetControlBackground();
        pWindow->EnablePaint(FALSE);
        pWindow->SetPosSizePixel(pPWindow->GetPosPixel(),pPWindow->GetSizePixel());

        GLWin.dpy = reinterpret_cast<unx::Display*>(pChildSysData->pDisplay);
        GLWin.win = pChildSysData->aWindow;
    }

    GLWin.ctx = glXCreateContext(GLWin.dpy,
                                 vi,
                                 0,
                                 GL_TRUE);

    glXMakeCurrent( GLWin.dpy, GLWin.win, GLWin.ctx );

    OSL_ENSURE( glXIsDirect(GLWin.dpy, GLWin.ctx) , "Can't Directly Render to the Screen, perhaps 3D transitions aren't for you." );
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT);
    unx::glXSwapBuffers(GLWin.dpy, GLWin.win);

    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    if( EnteringBytes.hasElements() && EnteringBytes.hasElements())
       GLInitSlides();//we already have uninitialized slides, let's initialize
}

void OGLTransitionerImpl::setSlides( const uno::Reference< rendering::XBitmap >& xLeavingSlide,
                                     const uno::Reference< rendering::XBitmap >& xEnteringSlide )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    uno::Reference< rendering::XIntegerBitmap > LeavingSlideIntBitmap( xLeavingSlide , uno::UNO_QUERY_THROW );
    uno::Reference< rendering::XIntegerBitmap > EnteringSlideIntBitmap( xEnteringSlide , uno::UNO_QUERY_THROW );

    geometry::IntegerRectangle2D SlideRect;
    SlideSize = LeavingSlideIntBitmap->getSize();
    SlideRect.X1 = 0;
    SlideRect.X2 = SlideSize.Width;
    SlideRect.Y1 = 0;
    SlideRect.Y2 = SlideSize.Height;

    LeavingBytes = LeavingSlideIntBitmap->getData(SlideBitmapLayout,SlideRect);
    EnteringBytes = EnteringSlideIntBitmap->getData(SlideBitmapLayout,SlideRect);


    if(GLWin.ctx)//if we have a rendering context, let's init the slides
        GLInitSlides();

    OSL_ENSURE(SlideBitmapLayout.Endianness == 0,"only handle little endian now");
    OSL_ENSURE(SlideBitmapLayout.PlaneStride == 0,"only handle no plane stride now");
}

void OGLTransitionerImpl::GLInitSlides()
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double EyePos(10.0);
    double RealF(-1.0);
    double RealN(1.0);
    double RealL(-1.0);
    double RealR(1.0);
    double RealB(-1.0);
    double RealT(1.0);
    double ClipN(EyePos+2.0*RealN);
    double ClipF(EyePos+2.0*RealF);
    double ClipL(RealL*8.0);
    double ClipR(RealR*8.0);
    double ClipB(RealB*8.0);
    double ClipT(RealT*8.0);
    //This scaling is to take the plane with BottomLeftCorner(-1,-1,0) and TopRightCorner(1,1,0) and map it to the screen after the perspective division.
    glScaled( 1.0 / ( ( ( RealR * 2.0 * ClipN ) / ( EyePos * ( ClipR - ClipL ) ) ) - ( ( ClipR + ClipL ) / ( ClipR - ClipL ) ) ), 1 / ( ( ( RealT * 2.0 * ClipN ) / ( EyePos * ( ClipT - ClipB ) ) ) - ( ( ClipT + ClipB ) / ( ClipT - ClipB ) ) ) , 1.0 );
    glFrustum(ClipL,ClipR,ClipB,ClipT,ClipN,ClipF);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0,0,-EyePos);

    GLint internalFormat;
    GLint Format;
    switch( SlideBitmapLayout.NumComponents )
    {
    case 3:
        Format = GL_RGB;
        break;
    case 4:
        Format = GL_RGBA;
        break;
    default:
        OSL_ENSURE(false, "unsupported number of color components");
        Format = GL_RGB;
    }

    switch( SlideBitmapLayout.Format )
    {
    case rendering::IntegerBitmapFormat::CHUNKY_16BIT:
        switch( Format)
        {
        case GL_RGBA:
            internalFormat = GL_RGBA4;
            break;
        default:
        OSL_ENSURE(false, "unsupported bitmap integer format");
        internalFormat = GL_RGBA;
        }
        break;
    case rendering::IntegerBitmapFormat::CHUNKY_24BIT:
        switch( Format)
        {
        case GL_RGB:
            internalFormat = GL_RGB8;
            Format = GL_BGR;//24 bit bitmaps are BGR, not RGB
            break;
        default:
            OSL_ENSURE(false, "unsupported bitmap integer format");
            internalFormat = GL_RGB;
            Format = GL_BGR;//24 bit bitmaps are BGR, not RGB
        }
        break;
    case rendering::IntegerBitmapFormat::CHUNKY_32BIT:
        switch( Format)
        {
        case GL_RGBA:
            Format = GL_BGRA;//32 bit bitmaps are BGRA, not RGBA
            internalFormat = GL_RGBA8;
            break;
        default:
        OSL_ENSURE(false, "unsupported bitmap integer format");
        internalFormat = GL_RGBA;
        Format = GL_BGRA;//32 bit bitmaps are BGRA, not RGBA
        }
        break;
    default:
        OSL_ENSURE(false, "unsupported bitmap integer format");
        internalFormat = GL_RGB;
    }

    glDeleteTextures(1,&GLleavingSlide);

    glGenTextures(1, &GLleavingSlide);
    glBindTexture(GL_TEXTURE_2D, GLleavingSlide);
    gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat , SlideSize.Width, SlideSize.Height, Format, GL_UNSIGNED_BYTE, &LeavingBytes[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);//TRILINEAR FILTERING
    GLfloat largest_supported_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);//anistropic filtering (to make texturing not suck when looking at polygons from oblique angles)

    OSL_ENSURE(glIsTexture(GLleavingSlide), "Can't generate Leaving slide textures in OpenGL");

    glDeleteTextures(1,&GLenteringSlide);

    glGenTextures(1, &GLenteringSlide);
    glBindTexture(GL_TEXTURE_2D, GLenteringSlide);
    gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat , SlideSize.Width, SlideSize.Height, Format, GL_UNSIGNED_BYTE, &EnteringBytes[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);//TRILINEAR FILTERING
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);//anistropic filtering (to make texturing not suck when looking at polygons from oblique angles)

    OSL_ENSURE( glIsTexture(GLenteringSlide), "Can't generate entering slide textures in OpenGL" );
}

void SAL_CALL OGLTransitionerImpl::update( double nTime ) throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glLoadIdentity();
    if(pTransition)
        pTransition->display( nTime , GLleavingSlide , GLenteringSlide ,
                              SlideSize.Width/static_cast<double>(GLWin.Width),
                              SlideSize.Height/static_cast<double>(GLWin.Height) );
    unx::glXSwapBuffers(GLWin.dpy, GLWin.win);
    if( pWindow )
        pWindow->Show();
}

// we are about to be disposed (someone call dispose() on us)
void OGLTransitionerImpl::disposing()
{
    osl::MutexGuard const guard( m_aMutex );
    glDeleteTextures(1,&GLleavingSlide);
    glDeleteTextures(1,&GLenteringSlide);
    if(GLWin.ctx)
    {
        OSL_ENSURE( glXMakeCurrent(GLWin.dpy, None, NULL) , "Error releasing glX context" );
        glXDestroyContext(GLWin.dpy, GLWin.ctx);
        GLWin.ctx = NULL;
    }
    delete pWindow;
    delete pTransition;
}

OGLTransitionerImpl::OGLTransitionerImpl(OGLTransitionImpl* pOGLTransition) :
    OGLTransitionerImplBase(m_aMutex),
    GLWin(),
    GLleavingSlide( 0 ),
    GLenteringSlide( 0 ),
    pWindow( NULL ),
    EnteringBytes(),
    LeavingBytes(),
    SlideBitmapLayout(),
    SlideSize(),
    pTransition(pOGLTransition)
{
    GLWin.ctx = 0;
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
        return createTransition(transitionType,
                                transitionSubType,
                                uno::Reference< presentation::XSlideShowView >(),
                                uno::Reference< rendering::XBitmap >(),
                                uno::Reference< rendering::XBitmap >(),
                                geometry::RealPoint2D()).is();
    }

    virtual uno::Reference< presentation::XTransition > SAL_CALL createTransition(
        ::sal_Int16                                           transitionType,
        ::sal_Int16                                           transitionSubType,
        const uno::Reference< presentation::XSlideShowView >& view,
        const uno::Reference< rendering::XBitmap >&           leavingBitmap,
        const uno::Reference< rendering::XBitmap >&           enteringBitmap,
        const geometry::RealPoint2D&                          slideOffset ) throw (uno::RuntimeException)
    {
        if( transitionType != animations::TransitionType::MISCSHAPEWIPE )
            return uno::Reference< presentation::XTransition >();

        OGLTransitionImpl* pTransition = new OGLTransitionImpl();
        switch( transitionSubType )
        {
            case animations::TransitionSubType::ACROSS:
                pTransition->makeNByMTileFlip(5,5);
                break;
            case animations::TransitionSubType::CORNERSOUT:
                pTransition->makeOutsideCubeFaceToLeft();
                break;
            case animations::TransitionSubType::CIRCLE:
                pTransition->makeRevolvingCircles(5,20);
                break;
            case animations::TransitionSubType::FANOUTHORIZONTAL:
                pTransition->makeHelix(20);
                break;
            case animations::TransitionSubType::CORNERSIN :
                pTransition->makeInsideCubeFaceToLeft();
                break;
            default:
                return uno::Reference< presentation::XTransition >();
        }

        rtl::Reference<OGLTransitionerImpl> xRes(
            new OGLTransitionerImpl(pTransition) );
        xRes->initWindowFromSlideShowView(view,slideOffset.X,slideOffset.Y);
        xRes->setSlides(leavingBitmap,enteringBitmap);

        return uno::Reference<presentation::XTransition>(xRes.get());
    }
};

}

namespace sdecl = comphelper::service_decl;
#if defined (__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ <= 3)
 sdecl::class_<OGLTransitionFactoryImpl> serviceImpl;
 const sdecl::ServiceDecl OGLTransitionFactoryDecl(
     serviceImpl,
#else
 const sdecl::ServiceDecl OGLTransitionFactoryDecl(
     sdecl::class_<OGLTransitionFactoryImpl>(),
#endif
    "com.sun.star.comp.presentation.OGLTransitionFactory",
    "com.sun.star.presentation.TransitionFactory" );

// The C shared lib entry points
COMPHELPER_SERVICEDECL_EXPORTS1(OGLTransitionFactoryDecl)
