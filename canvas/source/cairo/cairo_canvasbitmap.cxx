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
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

#include "cairo_canvasbitmap.hxx"

#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>

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

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle )  throw (uno::RuntimeException, std::exception)
    {
        uno::Any aRV( sal_Int32(0) );
        // 0 ... get BitmapEx
        // 1 ... get Pixbuf with bitmap RGB content
        // 2 ... get Pixbuf with bitmap alpha mask
        switch( nHandle )
        {
            case 0:
            {
                aRV = uno::Any( reinterpret_cast<sal_Int64>( (BitmapEx *) NULL ) );
                if ( !mbHasAlpha )
                    break;

                ::Size aSize( maSize.getX(), maSize.getY() );
                // FIXME: if we could teach VCL/ about cairo handles, life could
                // be significantly better here perhaps.
                cairo_surface_t *pPixels;
                pPixels = cairo_image_surface_create( CAIRO_FORMAT_ARGB32,
                                                      aSize.Width(), aSize.Height() );
                cairo_t *pCairo = cairo_create( pPixels );
                if( !pPixels || !pCairo )
                    break;

                // suck ourselves from the X server to this buffer so then we can fiddle with
                // Alpha to turn it into the ultra-lame vcl required format and then push it
                // all back again later at vast expense [ urgh ]
                cairo_set_source_surface( pCairo, getSurface()->getCairoSurface().get(), 0, 0 );
                cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
                cairo_paint( pCairo );

                ::Bitmap aRGB( aSize, 24 );
                ::AlphaMask aMask( aSize );

                BitmapWriteAccess *pRGBWrite( aRGB.AcquireWriteAccess() );
                if( pRGBWrite )
                {
                    BitmapWriteAccess *pMaskWrite( aMask.AcquireWriteAccess() );
                    if( pMaskWrite )
                    {
                        cairo_surface_flush(pPixels);
                        unsigned char *pSrc = cairo_image_surface_get_data( pPixels );
                        unsigned int nStride = cairo_image_surface_get_stride( pPixels );
                        for( unsigned long y = 0; y < (unsigned long) aSize.Height(); y++ )
                        {
                            sal_uInt32 *pPix = (sal_uInt32 *)(pSrc + nStride * y);
                            for( unsigned long x = 0; x < (unsigned long) aSize.Width(); x++ )
                            {
                                sal_uInt8 nAlpha = (*pPix >> 24);
                                sal_uInt8 nR = (*pPix >> 16) & 0xff;
                                sal_uInt8 nG = (*pPix >> 8) & 0xff;
                                sal_uInt8 nB = *pPix & 0xff;
                                if( nAlpha != 0 && nAlpha != 255 )
                                {
                                    // Cairo uses pre-multiplied alpha - we do not => re-multiply
                                    nR = (sal_uInt8) MinMax( ((sal_uInt32)nR * 255) / nAlpha, 0, 255 );
                                    nG = (sal_uInt8) MinMax( ((sal_uInt32)nG * 255) / nAlpha, 0, 255 );
                                    nB = (sal_uInt8) MinMax( ((sal_uInt32)nB * 255) / nAlpha, 0, 255 );
                                }
                                pRGBWrite->SetPixel( y, x, BitmapColor( nR, nG, nB ) );
                                pMaskWrite->SetPixelIndex( y, x, 255 - nAlpha );
                                pPix++;
                            }
                        }
                        aMask.ReleaseAccess( pMaskWrite );
                    }
                    aRGB.ReleaseAccess( pRGBWrite );
                }

                // ignore potential errors above. will get caller a
                // uniformely white bitmap, but not that there would
                // be error handling in calling code ...
                ::BitmapEx *pBitmapEx = new ::BitmapEx( aRGB, aMask );

                cairo_destroy( pCairo );
                cairo_surface_destroy( pPixels );

                aRV = uno::Any( reinterpret_cast<sal_Int64>( pBitmapEx ) );
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
                // Always return nothing - for the RGB surface support.
                // Alpha code paths go via the above case 0.
                aRV = uno::Any();
                break;
            }
        }

        return aRV;
    }

    OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException, std::exception)
    {
        return OUString( "CairoCanvas.CanvasBitmap" );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const OUString& ServiceName ) throw (uno::RuntimeException, std::exception)
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException, std::exception)
    {
        uno::Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.rendering.CanvasBitmap";

        return aRet;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
