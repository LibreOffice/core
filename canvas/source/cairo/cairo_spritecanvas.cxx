/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cairo_spritecanvas.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 03:21:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <osl/mutex.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include "cairo_spritecanvas.hxx"


using namespace ::cairo;
using namespace ::com::sun::star;

#define IMPLEMENTATION_NAME "CairoCanvas::SpriteCanvas"
#define SERVICE_NAME "com.sun.star.rendering.CairoCanvas"

namespace
{
    static ::rtl::OUString SAL_CALL getImplementationName_SpriteCanvas()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    static uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_SpriteCanvas()
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

}

namespace cairocanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Reference< uno::XComponentContext >& rxContext ) :
        mxComponentContext( rxContext ),
        mpBackgroundCairo( NULL ),
        mpBackgroundSurface( NULL )
    {
        OSL_TRACE("SpriteCanvas created %p\n", this);
    }

    void SAL_CALL SpriteCanvas::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mxComponentContext.clear();

        if( mpBackgroundCairo ) {
            cairo_destroy( mpBackgroundCairo );
            mpBackgroundCairo = NULL;
        }

        if( mpBackgroundSurface ) {
            mpBackgroundSurface->Unref();
            mpBackgroundSurface = NULL;
        }

        // forward to parent
        SpriteCanvasBaseT::disposing();
    }

    ::sal_Bool SAL_CALL SpriteCanvas::showBuffer( ::sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : SpriteCanvasBaseT::showBuffer( bUpdateAll );
    }

    ::sal_Bool SAL_CALL SpriteCanvas::switchBuffer( ::sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : SpriteCanvasBaseT::switchBuffer( bUpdateAll );
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen( sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : maCanvasHelper.updateScreen(
            ::basegfx::unotools::b2IRectangleFromAwtRectangle(maBounds),
            bUpdateAll,
            mbSurfaceDirty );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : SpriteCanvasBaseT::updateScreen( bUpdateAll );
    }

    void SAL_CALL SpriteCanvas::initialize( const uno::Sequence< uno::Any >& aArguments ) throw( uno::Exception,
                                                                                                 uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        OSL_TRACE( "SpriteCanvas::initialize called" );
        VERBOSE_TRACE( "SpriteCanvas::initialize called" );

        // At index 1, we expect a system window handle here,
        // containing a pointer to a valid window, on which to output
        // At index 2, we expect the current window bound rect
        CHECK_AND_THROW( aArguments.getLength() >= 4 &&
                         aArguments[1].getValueTypeClass() == uno::TypeClass_LONG,
                         "SpriteCanvas::initialize: wrong number of arguments, or wrong types" );

        awt::Rectangle aRect;
        aArguments[2] >>= aRect;
//         const ::basegfx::B2ISize aSize(aRect.Width,
//                                        aRect.Height);

        sal_Bool bIsFullscreen( sal_False );
        aArguments[3] >>= bIsFullscreen;

    // TODO(Q2): This now works for Solaris, but still warns for gcc
    Window* pOutputWindow = (Window*) *reinterpret_cast<const sal_Int64*>(aArguments[0].getValue());

    Size aPixelSize( pOutputWindow->GetOutputSizePixel() );
        const ::basegfx::B2ISize aSize( aPixelSize.Width(),
                    aPixelSize.Height() );

    CHECK_AND_THROW( pOutputWindow != NULL,
             "SpriteCanvas::initialize: invalid Window pointer" );

    // setup helper
    maDeviceHelper.init( *pOutputWindow,
                 *this,
                 aSize,
                 bIsFullscreen );

        maCanvasHelper.init( maRedrawManager,
                             *this,
                             aSize );
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getImplementationName() throw( uno::RuntimeException )
    {
        return getImplementationName_SpriteCanvas();
    }

    sal_Bool SAL_CALL SpriteCanvas::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL SpriteCanvas::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        return getSupportedServiceNames_SpriteCanvas();
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    }

    uno::Reference< uno::XInterface > SAL_CALL SpriteCanvas::createInstance( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::Exception )
    {
        return uno::Reference< uno::XInterface >( static_cast<cppu::OWeakObject*>( new SpriteCanvas( xContext ) ) );
    }

    Surface* SpriteCanvas::getSurface( const ::basegfx::B2ISize& rSize, Content aContent )
    {
    return maDeviceHelper.getSurface( rSize, aContent );
    }

    Surface* SpriteCanvas::getSurface( Content aContent )
    {
    return maDeviceHelper.getSurface( aContent );
    }

    Surface* SpriteCanvas::getSurface( Bitmap& rBitmap )
    {
    Surface *pSurface = NULL;

    BitmapSystemData aData;
    if( rBitmap.GetSystemData( aData ) ) {
        const Size& rSize = rBitmap.GetSizePixel();

        pSurface = maDeviceHelper.getSurface( aData, rSize );
    }

    return pSurface;
    }

    Surface* SpriteCanvas::getBufferSurface()
    {
    return maDeviceHelper.getBufferSurface();
    }

    Surface* SpriteCanvas::getWindowSurface()
    {
    return maDeviceHelper.getWindowSurface();
    }

    Surface* SpriteCanvas::getBackgroundSurface()
    {
    return mpBackgroundSurface;
    }

    const ::basegfx::B2ISize& SpriteCanvas::getSizePixel()
    {
    return maDeviceHelper.getSizePixel();
    }

    void SpriteCanvas::setSizePixel( const ::basegfx::B2ISize& rSize )
    {
        if( mpBackgroundSurface )
        {
            mpBackgroundSurface->Unref();
        }
        mpBackgroundSurface = maDeviceHelper.getSurface( CAIRO_CONTENT_COLOR );

        if( mpBackgroundCairo )
        {
            cairo_destroy( mpBackgroundCairo );
        }
        mpBackgroundCairo = mpBackgroundSurface->getCairo();

        maCanvasHelper.setSurface( mpBackgroundSurface, false );
    }

    void SpriteCanvas::flush()
    {
    maDeviceHelper.flush();
    }

    bool SpriteCanvas::repaint( Surface* pSurface,
                const rendering::ViewState& viewState,
                const rendering::RenderState&   renderState )
    {
    return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }
}

namespace
{
    /* shared lib exports implemented with helpers */
    static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            cairocanvas::SpriteCanvas::createInstance, getImplementationName_SpriteCanvas,
            getSupportedServiceNames_SpriteCanvas, ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}


/* Exported UNO methods for registration and object creation.
   ==========================================================
 */
extern "C"
{
    void SAL_CALL component_getImplementationEnvironment( const sal_Char**  ppEnvTypeName,
                                                          uno_Environment** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo( lang::XMultiServiceFactory*  xMgr,
                                           registry::XRegistryKey*      xRegistry )
    {
        return ::cppu::component_writeInfoHelper(
            xMgr, xRegistry, s_component_entries );
    }

    void * SAL_CALL component_getFactory( sal_Char const*               implName,
                                          lang::XMultiServiceFactory*   xMgr,
                                          registry::XRegistryKey*       xRegistry )
    {
        return ::cppu::component_getFactoryHelper(
            implName, xMgr, xRegistry, s_component_entries );
    }
}
