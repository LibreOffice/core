/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritecanvas.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:23:21 $
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

#include <canvas/debug.hxx>

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#include <algorithm>

#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include "spritecanvas.hxx"
#include "canvascustomsprite.hxx"
#include "windowgraphicdevice.hxx"

using namespace ::com::sun::star;


#define IMPLEMENTATION_NAME "VCLCanvas::SpriteCanvas"
#define SERVICE_NAME "com.sun.star.rendering.VCLCanvas"

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

namespace vclcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Reference< uno::XComponentContext >& rxContext ) :
        maBounds(),
        mxComponentContext( rxContext ),
        mxDevice(),
        mpBackBuffer(),
        mpRedrawManager(),
    mbIsVisible( false )
    {
    }

    SpriteCanvas::~SpriteCanvas()
    {
    }

    void SAL_CALL SpriteCanvas::disposing()
    {
        tools::LocalGuard aGuard;

    dispose();

        // forward to parent
        SpriteCanvas_Base::disposing();
    }

    uno::Reference< rendering::XAnimatedSprite > SAL_CALL SpriteCanvas::createSpriteFromAnimation( const uno::Reference< rendering::XAnimation >& animation ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return uno::Reference< rendering::XAnimatedSprite >(NULL);
    }

    uno::Reference< rendering::XAnimatedSprite > SAL_CALL SpriteCanvas::createSpriteFromBitmaps( const uno::Sequence< uno::Reference< rendering::XBitmap > >& animationBitmaps,
                                                                                                 sal_Int8                                                     interpolationMode ) throw (lang::IllegalArgumentException, rendering::VolatileContentDestroyedException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return uno::Reference< rendering::XAnimatedSprite >(NULL);
    }

    uno::Reference< rendering::XCustomSprite > SAL_CALL SpriteCanvas::createCustomSprite( const geometry::RealSize2D& spriteSize ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mxDevice.is() )
            return uno::Reference< rendering::XCustomSprite >(); // we're disposed

        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite( spriteSize,
                                    mxDevice,
                                    ImplRef(this)) );
    }

    uno::Reference< rendering::XSprite > SAL_CALL SpriteCanvas::createClonedSprite( const uno::Reference< rendering::XSprite >& original ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return uno::Reference< rendering::XSprite >(NULL);
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen( sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpRedrawManager.get() )
            return sal_False; // disposed

        // hidden windows need not paint anything, thus prevent
        // screen updates, then
        if( !mbIsVisible )
            return sal_False;

        // pass background dirty state to redrawmanager
        if( mbSurfaceDirty )
        {
            mpRedrawManager->backgroundDirty();
            mbSurfaceDirty = false;
        }

        mpRedrawManager->updateScreen( bUpdateAll );

        // commit to screen
        maCanvasHelper.flush();

#if defined(VERBOSE) && defined(DBG_UTIL)
        static ::canvas::tools::ElapsedTime aElapsedTime;

        // log time immediately after surface flip
        OSL_TRACE( "SpriteCanvas::updateScreen(): flip done at %f",
                   aElapsedTime.getElapsedTime() );
#endif

        return sal_True;
    }

    void SAL_CALL SpriteCanvas::initialize( const uno::Sequence< uno::Any >& aArguments ) throw( uno::Exception,
                                                                                                 uno::RuntimeException)
    {
        VERBOSE_TRACE( "SpriteCanvas::initialize called" );

        CHECK_AND_THROW( aArguments.getLength() >= 1,
                         "SpriteCanvas::initialize: wrong number of arguments" );

        // We expect a single Any here, containing a pointer to a valid
        // VCL window, on which to output
        if( aArguments.getLength() >= 1 &&
            aArguments[0].getValueTypeClass() == uno::TypeClass_HYPER )
        {
            // TODO(Q2): This now works for Solaris, but still warns for gcc
            Window* pOutputWindow = (Window*) *reinterpret_cast<const sal_Int64*>(aArguments[0].getValue());

            CHECK_AND_THROW( pOutputWindow != NULL,
                             "SpriteCanvas::initialize: invalid Window pointer" );

            // setup graphic device
            mxDevice = WindowGraphicDevice::ImplRef( new WindowGraphicDevice( *pOutputWindow ) );

            // setup helper
            maCanvasHelper.setGraphicDevice( mxDevice );

            // setup back buffer
            mpBackBuffer.reset( new BackBuffer( *pOutputWindow ) );
            mpBackBuffer->setSize( pOutputWindow->GetOutputSizePixel() );

            // always render into back buffer, don't preserve state
            // (it's our private VDev, after all)
            maCanvasHelper.setOutDev( mpBackBuffer, false );

            // setup RedrawManager
            mpRedrawManager.reset( new RedrawManager( *pOutputWindow,
                                                      mpBackBuffer ) );
        }
    }

    void SAL_CALL SpriteCanvas::dispose(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        maCanvasHelper.disposing();

        mxComponentContext.clear();
        mxDevice.reset();
        mpBackBuffer.reset(),
        mpRedrawManager.reset();
    }

    void SAL_CALL SpriteCanvas::addEventListener( const uno::Reference< lang::XEventListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::removeEventListener( const uno::Reference< lang::XEventListener >& aListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::setPosSize( sal_Int32 nX,
                                            sal_Int32 nY,
                                            sal_Int32 nWidth,
                                            sal_Int32 nHeight,
                                            sal_Int16 nFlags ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

    if( maBounds.X != nX ||
        maBounds.Y != nY ||
        maBounds.Width != nWidth ||
        maBounds.Height != nHeight )
    {
            maBounds.X = nX;
        maBounds.Y = nY;
        maBounds.Width = nWidth;
        maBounds.Height = nHeight;

        if( mpBackBuffer.get() )
            mpBackBuffer->setSize( Size( nWidth,
                         nHeight ) );
    }
    }

    awt::Rectangle SAL_CALL SpriteCanvas::getPosSize(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return maBounds;
    }

    void SAL_CALL SpriteCanvas::setVisible( ::sal_Bool bVisible ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        mbIsVisible = bVisible;
    }

    void SAL_CALL SpriteCanvas::setEnable( ::sal_Bool Enable ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::setFocus(  ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::addWindowListener( const uno::Reference< awt::XWindowListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::removeWindowListener( const uno::Reference< awt::XWindowListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::addKeyListener( const uno::Reference< awt::XKeyListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::removeKeyListener( const uno::Reference< awt::XKeyListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::addMouseListener( const uno::Reference< awt::XMouseListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::removeMouseListener( const uno::Reference< awt::XMouseListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::addPaintListener( const uno::Reference< awt::XPaintListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
    }

    void SAL_CALL SpriteCanvas::removePaintListener( const uno::Reference< awt::XPaintListener >& xListener ) throw (uno::RuntimeException)
    {
        // Ignored
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

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    }

    uno::Reference< uno::XInterface > SAL_CALL SpriteCanvas::createInstance( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::Exception )
    {
        return uno::Reference< uno::XInterface >( static_cast<cppu::OWeakObject*>(new SpriteCanvas( xContext )) );
    }

    // SpriteSurface
    void SpriteCanvas::showSprite( const Sprite::ImplRef& sprite )
    {
        tools::LocalGuard aGuard;

        if( !mpRedrawManager.get() )
            return; // we're disposed

        mpRedrawManager->showSprite( sprite );
    }

    void SpriteCanvas::hideSprite( const Sprite::ImplRef& sprite )
    {
        tools::LocalGuard aGuard;

        if( !mpRedrawManager.get() )
            return; // we're disposed

        mpRedrawManager->hideSprite( sprite );
    }

    void SpriteCanvas::moveSprite( const Sprite::ImplRef&   sprite,
                                   const Point&             rOldPos,
                                   const Point&             rNewPos,
                                   const Size&              rSpriteSize )
    {
        tools::LocalGuard aGuard;

        if( !mpRedrawManager.get() )
            return; // we're disposed

        mpRedrawManager->moveSprite( sprite, rOldPos, rNewPos, rSpriteSize );
    }

    void SpriteCanvas::updateSprite( const Sprite::ImplRef& sprite,
                                     const Point&           rPos,
                                     const Rectangle&       rUpdateArea )
    {
        tools::LocalGuard aGuard;

        if( !mpRedrawManager.get() )
            return; // we're disposed

        mpRedrawManager->updateSprite( sprite, rPos, rUpdateArea );
    }

    bool SpriteCanvas::repaint( const GraphicObjectSharedPtr&   rGrf,
                                const ::Point&                  rPt,
                                const ::Size&                   rSz,
                                const GraphicAttr&              rAttr ) const
    {
        tools::LocalGuard aGuard;

        mbSurfaceDirty = true;

        return maCanvasHelper.repaint( rGrf, rPt, rSz, rAttr );
    }

}

namespace
{
    /* shared lib exports implemented with helpers */
    static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            vclcanvas::SpriteCanvas::createInstance, getImplementationName_SpriteCanvas,
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
