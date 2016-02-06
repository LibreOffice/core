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

#include <sal/config.h>

#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>

#include <canvas/canvastools.hxx>

#include "cairo_canvasbitmap.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

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

        SAL_INFO(
            "canvas.cairo",
            "bitmap size: " << rSize.getX() << "x" << rSize.getY());

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

    SurfaceSharedPtr CanvasBitmap::createSurface( const ::basegfx::B2ISize& rSize, int aContent )
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
                aRV = uno::Any( reinterpret_cast<sal_Int64>( nullptr ) );
                if ( !mbHasAlpha )
                    break;

                ::Size aSize( maSize.getX(), maSize.getY() );
                // FIXME: if we could teach VCL/ about cairo handles, life could
                // be significantly better here perhaps.
                cairo_surface_t *pPixels;
                pPixels = cairo_image_surface_create( CAIRO_FORMAT_ARGB32,
                                                      aSize.Width(), aSize.Height() );
                cairo_t *pCairo = cairo_create( pPixels );
                if( !pPixels || !pCairo || cairo_status(pCairo) != CAIRO_STATUS_SUCCESS )
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
                            sal_uInt32 *pPix = reinterpret_cast<sal_uInt32 *>(pSrc + nStride * y);
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
                    ::Bitmap::ReleaseAccess( pRGBWrite );
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
                aRV = getOutputDevice()->GetNativeSurfaceHandle(mpBufferSurface, maSize);
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
        uno::Sequence< OUString > aRet { "com.sun.star.rendering.CanvasBitmap" };

        return aRet;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
