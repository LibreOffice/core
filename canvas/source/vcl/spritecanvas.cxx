/*************************************************************************
 *
 *  $RCSfile: spritecanvas.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:15:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
using namespace ::drafts::com::sun::star;


#define IMPLEMENTATION_NAME "VCLCanvas::SpriteCanvas"
#define SERVICE_NAME "drafts.com.sun.star.rendering.VCLCanvas"

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
        mxComponentContext( rxContext ),
        mxDevice(),
        mpBackBuffer(),
        mpRedrawManager()
    {
    }

    SpriteCanvas::~SpriteCanvas()
    {
    }

    void SAL_CALL SpriteCanvas::disposing()
    {
        tools::LocalGuard aGuard;

        mxComponentContext.clear();
        mxDevice.reset();
        mpBackBuffer.reset(),
        mpRedrawManager.reset();

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
            mpBackBuffer->getVirDev().SetOutputSizePixel(
                pOutputWindow->GetOutputSizePixel() );

            // always render into back buffer, don't preserve state
            // (it's our private VDev, after all)
            maCanvasHelper.setOutDev( mpBackBuffer, false );

            // setup RedrawManager
            mpRedrawManager.reset( new RedrawManager( *pOutputWindow,
                                                      mpBackBuffer ) );
        }
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
                                   const Point&             rNewPos )
    {
        tools::LocalGuard aGuard;

        if( !mpRedrawManager.get() )
            return; // we're disposed

        mpRedrawManager->moveSprite( sprite, rOldPos, rNewPos );
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
