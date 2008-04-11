/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_spritecanvas.cxx,v $
 * $Revision: 1.4 $
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

#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <osl/mutex.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <comphelper/servicedecl.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include "dx_winstuff.hxx"
#include "dx_spritecanvas.hxx"

using namespace ::com::sun::star;

#if DIRECTX_VERSION < 0x0900
# define CANVAS_NAME "DXCanvas"
#else
# define CANVAS_NAME "DX9Canvas"
#endif

#define SERVICE_NAME        "com.sun.star.rendering." CANVAS_NAME
#define IMPLEMENTATION_NAME "com.sun.star.comp.rendering." CANVAS_NAME


namespace dxcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& rxContext ) :
        mxComponentContext( rxContext )
    {
        // #i64742# Only call initialize when not in probe mode
        if( aArguments.getLength() != 0 )
            initialize( aArguments );
    }

    void SpriteCanvas::initialize( const uno::Sequence< uno::Any >& aArguments )
    {
        VERBOSE_TRACE( "SpriteCanvas::initialize called" );

        // At index 1, we expect a HWND handle here, containing a
        // pointer to a valid window, on which to output
        // At index 2, we expect the current window bound rect
        CHECK_AND_THROW( aArguments.getLength() >= 4 &&
                         aArguments[1].getValueTypeClass() == uno::TypeClass_LONG,
                         "SpriteCanvas::initialize: wrong number of arguments, or wrong types" );

        uno::Reference< awt::XWindow > xParentWindow;
        aArguments[4] >>= xParentWindow;
        Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
        if( !pParentWindow )
            throw lang::NoSupportException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                     "Parent window not VCL window, or canvas out-of-process!")),
                NULL);

        awt::Rectangle aRect;
        aArguments[2] >>= aRect;

        sal_Bool bIsFullscreen( sal_False );
        aArguments[3] >>= bIsFullscreen;

        // setup helper
        maDeviceHelper.init( *pParentWindow,
                             *this,
                             aRect,
                             bIsFullscreen );
        maCanvasHelper.setDevice( *this );
        maCanvasHelper.init( maRedrawManager,
                             maDeviceHelper.getRenderModule(),
                             maDeviceHelper.getSurfaceProxy(),
                             maDeviceHelper.getBackBuffer(),
                             ::basegfx::B2ISize() );
    }

    void SAL_CALL SpriteCanvas::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mxComponentContext.clear();

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
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    }

    const IDXRenderModuleSharedPtr& SpriteCanvas::getRenderModule() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maDeviceHelper.getRenderModule();
    }

    const DXBitmapSharedPtr& SpriteCanvas::getBackBuffer() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maDeviceHelper.getBackBuffer();
    }

    namespace sdecl = comphelper::service_decl;
    const sdecl::ServiceDecl dxCanvasDecl(
        sdecl::class_<SpriteCanvas, sdecl::with_args<true> >(),
        IMPLEMENTATION_NAME,
        SERVICE_NAME );
}

// The C shared lib entry points
COMPHELPER_SERVICEDECL_EXPORTS1(dxcanvas::dxCanvasDecl);
