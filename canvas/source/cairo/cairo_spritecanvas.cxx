/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_spritecanvas.cxx,v $
 * $Revision: 1.10 $
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
#include <comphelper/servicedecl.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include "cairo_spritecanvas.hxx"
#include "cairo_devicehelper.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

#define SERVICE_NAME "com.sun.star.rendering.CairoCanvas"

namespace cairocanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& rxContext ) :
        mxComponentContext( rxContext ),
        mpBackgroundSurface( NULL ),
        mpBackgroundCairo( NULL )
    {
        // #i64742# Only call initialize when not in probe mode
        if( aArguments.getLength() != 0 )
            initialize( aArguments );
    }

    void SpriteCanvas::initialize( const uno::Sequence< uno::Any >& aArguments )
    {
        VERBOSE_TRACE("SpriteCanvas created %p\n", this);

        // At index 1, we expect a system window handle here,
        // containing a pointer to a valid window, on which to output
        // At index 2, we expect the current window bound rect
        // NOTE: type is derived from vcl/source/window/window.cxx (XCanvas setup)
#ifdef QUARTZ
        CHECK_AND_THROW( aArguments.getLength() >= 4 &&
                         aArguments[1].getValueTypeClass() == uno::TypeClass_UNSIGNED_HYPER,
                         "SpriteCanvas::initialize: wrong number of arguments, or wrong types" );
#else
        CHECK_AND_THROW( aArguments.getLength() >= 4 &&
                         aArguments[1].getValueTypeClass() == uno::TypeClass_LONG,
                         "SpriteCanvas::initialize: wrong number of arguments, or wrong types" );
#endif

        awt::Rectangle aRect;
        aArguments[2] >>= aRect;

        sal_Bool bIsFullscreen( sal_False );
        aArguments[3] >>= bIsFullscreen;

        sal_Int64 nWindowPtr = 0;
    aArguments[0] >>= nWindowPtr;
    Window* pOutputWindow = reinterpret_cast<Window*>(nWindowPtr);

        CHECK_AND_THROW( pOutputWindow != NULL,
                         "SpriteCanvas::SpriteCanvas: invalid Window pointer" );

#if defined(CAIRO_HAS_XLIB_SURFACE) && !defined (QUARTZ)
        bool bHasXRender = HasXRender( cairocanvas::GetSysData(pOutputWindow) );
        CHECK_AND_THROW( bHasXRender == true,
                         "SpriteCanvas::SpriteCanvas: No RENDER extension" );
#endif

        Size aPixelSize( pOutputWindow->GetOutputSizePixel() );
        const ::basegfx::B2ISize aSize( aPixelSize.Width(),
                                        aPixelSize.Height() );

        // setup helper
        maDeviceHelper.init( *pOutputWindow,
                             *this,
                             aSize,
                             bIsFullscreen );

        maCanvasHelper.init( maRedrawManager,
                             *this,
                             aSize );
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
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
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

        maCanvasHelper.setSize( rSize );
        maCanvasHelper.setSurface( mpBackgroundSurface, false );
    }

    void SpriteCanvas::flush()
    {
        maDeviceHelper.flush();
    }

    bool SpriteCanvas::repaint( Surface*                        pSurface,
                                const rendering::ViewState&     viewState,
                                const rendering::RenderState&   renderState )
    {
        return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }

    namespace sdecl = comphelper::service_decl;
#if defined (__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ <= 3)
    sdecl::class_<SpriteCanvas, sdecl::with_args<true> > serviceImpl;
    const sdecl::ServiceDecl cairoCanvasDecl(
        serviceImpl,
#else
    const sdecl::ServiceDecl cairoCanvasDecl(
        sdecl::class_<SpriteCanvas, sdecl::with_args<true> >(),
#endif
        "com.sun.star.comp.rendering.CairoCanvas",
        SERVICE_NAME );
}

// The C shared lib entry points
COMPHELPER_SERVICEDECL_EXPORTS1(cairocanvas::cairoCanvasDecl)
