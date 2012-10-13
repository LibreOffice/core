/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>

#include "cairo_canvasbitmap.hxx"

#ifdef CAIRO_HAS_XLIB_SURFACE
# include "cairo_xlib_cairo.hxx"
#elif defined CAIRO_HAS_QUARTZ_SURFACE
# include "cairo_quartz_cairo.hxx"
#elif defined CAIRO_HAS_WIN32_SURFACE
# include "cairo_win32_cairo.hxx"
# include <cairo-win32.h>
#else
# error Native API needed.
#endif

using namespace ::cairo;
using namespace ::com::sun::star;

#ifdef CAIRO_HAS_WIN32_SURFACE
namespace
{
    HBITMAP surface2HBitmap( const SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize )
    {
        // cant seem to retrieve HBITMAP from cairo. copy content then
        HDC hScreenDC=GetDC(NULL);
        HBITMAP hBmpBitmap = CreateCompatibleBitmap( hScreenDC,
                                                     rSize.getX(),
                                                     rSize.getY() );

        HDC     hBmpDC = CreateCompatibleDC( 0 );
        HBITMAP hBmpOld = (HBITMAP) SelectObject( hBmpDC, hBmpBitmap );

        BitBlt( hBmpDC, 0, 0, rSize.getX(), rSize.getX(),
                cairo_win32_surface_get_dc(rSurface->getCairoSurface().get()),
                0, 0, SRCCOPY );

        SelectObject( hBmpDC, hBmpOld );
        DeleteDC( hBmpDC );

        return hBmpBitmap;
    }
}
#endif

namespace cairocanvas
{
    CanvasBitmap::CanvasBitmap( const ::basegfx::B2ISize&  rSize,
                                const SurfaceProviderRef&  rSurfaceProvider,
                                rendering::XGraphicDevice* pDevice,
                                bool                       bHasAlpha ) :
        mpSurfaceProvider( rSurfaceProvider ),
        mpBufferSurface(),
        mpBufferCairo(),
        maSize(rSize),
        mbHasAlpha(bHasAlpha)
    {
        ENSURE_OR_THROW( mpSurfaceProvider.is(),
                          "CanvasBitmap::CanvasBitmap(): Invalid surface or device" );

        OSL_TRACE( "bitmap size: %dx%d", rSize.getX(), rSize.getY() );

        mpBufferSurface = mpSurfaceProvider->createSurface( rSize, bHasAlpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR );
        mpBufferCairo = mpBufferSurface->getCairo();

        maCanvasHelper.init( rSize, *mpSurfaceProvider, pDevice );
        maCanvasHelper.setSurface( mpBufferSurface, bHasAlpha );

        // clear bitmap to 100% transparent
        maCanvasHelper.clear();
    }

    void CanvasBitmap::disposeThis()
    {
        mpSurfaceProvider.clear();

        mpBufferCairo.reset();
        mpBufferSurface.reset();

        // forward to parent
        CanvasBitmap_Base::disposeThis();
    }

    SurfaceSharedPtr CanvasBitmap::getSurface()
    {
        return mpBufferSurface;
    }

    SurfaceSharedPtr CanvasBitmap::createSurface( const ::basegfx::B2ISize& rSize, Content aContent )
    {
        return mpSurfaceProvider->createSurface(rSize,aContent);
    }

    SurfaceSharedPtr CanvasBitmap::createSurface( ::Bitmap& rBitmap )
    {
        return mpSurfaceProvider->createSurface(rBitmap);
    }

    SurfaceSharedPtr CanvasBitmap::changeSurface( bool, bool )
    {
        // non-modifiable surface here
        return SurfaceSharedPtr();
    }

    OutputDevice* CanvasBitmap::getOutputDevice()
    {
        return mpSurfaceProvider->getOutputDevice();
    }

    bool CanvasBitmap::repaint( const SurfaceSharedPtr&       pSurface,
                                const rendering::ViewState&   viewState,
                                const rendering::RenderState& renderState )
    {
        return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle )  throw (uno::RuntimeException)
    {
        uno::Any aRV( sal_Int32(0) );
        // 0 ... get BitmapEx
        // 1 ... get Pixbuf with bitmap RGB content
        // 2 ... get Pixbuf with bitmap alpha mask
        switch( nHandle )
        {
            case 0:
            {
                aRV = uno::Any( reinterpret_cast<sal_Int64>( (BitmapEx*) NULL ) );
                break;
            }
            case 1:
            {
#ifdef CAIRO_HAS_XLIB_SURFACE
                X11Surface* pXlibSurface=dynamic_cast<X11Surface*>(mpBufferSurface.get());
                OSL_ASSERT(pXlibSurface);
                uno::Sequence< uno::Any > args( 3 );
                args[0] = uno::Any( false );  // do not call XFreePixmap on it
                args[1] = uno::Any( pXlibSurface->getPixmap()->mhDrawable );
                args[2] = uno::Any( sal_Int32( pXlibSurface->getDepth() ) );

                aRV = uno::Any( args );
#elif defined CAIRO_HAS_QUARTZ_SURFACE
                QuartzSurface* pQuartzSurface = dynamic_cast<QuartzSurface*>(mpBufferSurface.get());
                OSL_ASSERT(pQuartzSurface);
                uno::Sequence< uno::Any > args( 1 );
                args[0] = uno::Any( sal_IntPtr (pQuartzSurface->getCGContext()) );
                aRV = uno::Any( args );
#elif defined CAIRO_HAS_WIN32_SURFACE
                // TODO(F2): check whether under all circumstances,
                // the alpha channel is ignored here.
                uno::Sequence< uno::Any > args( 1 );
                args[1] = uno::Any( sal_Int64(surface2HBitmap(mpBufferSurface,maSize)) );

                aRV = uno::Any( args );
                // caller frees the bitmap
#else
# error Please define fast prop retrieval for your platform!
#endif
                break;
            }
            case 2:
            {
#ifdef CAIRO_HAS_XLIB_SURFACE
                uno::Sequence< uno::Any > args( 3 );
                SurfaceSharedPtr pAlphaSurface = mpSurfaceProvider->createSurface( maSize, CAIRO_CONTENT_COLOR );
                CairoSharedPtr   pAlphaCairo = pAlphaSurface->getCairo();
                X11Surface* pXlibSurface=dynamic_cast<X11Surface*>(pAlphaSurface.get());
                OSL_ASSERT(pXlibSurface);

                // create RGB image (levels of gray) of alpha channel of original picture
                cairo_set_source_rgba( pAlphaCairo.get(), 1, 1, 1, 1 );
                cairo_set_operator( pAlphaCairo.get(), CAIRO_OPERATOR_SOURCE );
                cairo_paint( pAlphaCairo.get() );
                cairo_set_source_surface( pAlphaCairo.get(), mpBufferSurface->getCairoSurface().get(), 0, 0 );
                cairo_set_operator( pAlphaCairo.get(), CAIRO_OPERATOR_XOR );
                cairo_paint( pAlphaCairo.get() );
                pAlphaCairo.reset();

                X11PixmapSharedPtr pPixmap = pXlibSurface->getPixmap();
                args[0] = uno::Any( true );
                args[1] = ::com::sun::star::uno::Any( pPixmap->mhDrawable );
                args[2] = ::com::sun::star::uno::Any( sal_Int32( pXlibSurface->getDepth () ) );
                pPixmap->clear(); // caller takes ownership of pixmap

                // return pixmap and alphachannel pixmap - it will be used in BitmapEx
                aRV = uno::Any( args );
#elif defined CAIRO_HAS_QUARTZ_SURFACE
                SurfaceSharedPtr pAlphaSurface = mpSurfaceProvider->createSurface( maSize, CAIRO_CONTENT_COLOR );
                CairoSharedPtr   pAlphaCairo = pAlphaSurface->getCairo();
                QuartzSurface* pQuartzSurface=dynamic_cast<QuartzSurface*>(pAlphaSurface.get());
                OSL_ASSERT(pQuartzSurface);

                // create RGB image (levels of gray) of alpha channel of original picture
                cairo_set_source_rgba( pAlphaCairo.get(), 1, 1, 1, 1 );
                cairo_set_operator( pAlphaCairo.get(), CAIRO_OPERATOR_SOURCE );
                cairo_paint( pAlphaCairo.get() );
                cairo_set_source_surface( pAlphaCairo.get(), mpBufferSurface->getCairoSurface().get(), 0, 0 );
                cairo_set_operator( pAlphaCairo.get(), CAIRO_OPERATOR_XOR );
                cairo_paint( pAlphaCairo.get() );
                pAlphaCairo.reset();

                uno::Sequence< uno::Any > args( 1 );
                args[0] = uno::Any( sal_IntPtr (pQuartzSurface->getCGContext()) );
                // return ??? and alphachannel ??? - it will be used in BitmapEx
                aRV = uno::Any( args );
#elif defined CAIRO_HAS_WIN32_SURFACE
                SurfaceSharedPtr pAlphaSurface = mpSurfaceProvider->createSurface( maSize, CAIRO_CONTENT_COLOR );
                CairoSharedPtr   pAlphaCairo = pAlphaSurface->getCairo();

                // create RGB image (levels of gray) of alpha channel of original picture
                cairo_set_source_rgba( pAlphaCairo.get(), 1, 1, 1, 1 );
                cairo_set_operator( pAlphaCairo.get(), CAIRO_OPERATOR_SOURCE );
                cairo_paint( pAlphaCairo.get() );
                cairo_set_source_surface( pAlphaCairo.get(), mpBufferSurface->getCairoSurface().get(), 0, 0 );
                cairo_set_operator( pAlphaCairo.get(), CAIRO_OPERATOR_XOR );
                cairo_paint( pAlphaCairo.get() );
                pAlphaCairo.reset();

                // cant seem to retrieve HBITMAP from cairo. copy content then
                uno::Sequence< uno::Any > args( 1 );
                args[1] = uno::Any( sal_Int64(surface2HBitmap(pAlphaSurface,maSize)) );

                aRV = uno::Any( args );
                // caller frees the bitmap
#else
# error Please define fast prop retrieval for your platform!
#endif
                break;
            }
        }

        return aRV;
    }

#define IMPLEMENTATION_NAME "CairoCanvas.CanvasBitmap"
#define SERVICE_NAME "com.sun.star.rendering.CanvasBitmap"

    ::rtl::OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName == SERVICE_NAME;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
