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

#include <algorithm>
#include <tuple>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/keystoplerp.hxx>
#include <basegfx/tools/lerp.hxx>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/instance.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/virdev.hxx>

#include <canvas/canvastools.hxx>
#include <canvas/parametricpolypolygon.hxx>

#include "cairo_cachedbitmap.hxx"
#include "cairo_canvasbitmap.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_spritecanvas.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CanvasHelper::CanvasHelper() :
        mpSurfaceProvider(nullptr),
        mpDevice(nullptr),
        mpVirtualDevice(),
        mbHaveAlpha(),
        mpCairo(),
        mpSurface(),
        maSize()
    {
    }

    void CanvasHelper::disposing()
    {
        mpSurface.reset();
        mpCairo.reset();
        mpVirtualDevice.disposeAndClear();
        mpDevice = nullptr;
        mpSurfaceProvider = nullptr;
    }

    void CanvasHelper::init( const ::basegfx::B2ISize&  rSizePixel,
                             SurfaceProvider&           rSurfaceProvider,
                             rendering::XGraphicDevice* pDevice )
    {
        maSize = rSizePixel;
        mpSurfaceProvider = &rSurfaceProvider;
        mpDevice = pDevice;
    }

    void CanvasHelper::setSize( const ::basegfx::B2ISize& rSize )
    {
        maSize = rSize;
    }

    void CanvasHelper::setSurface( const SurfaceSharedPtr& pSurface, bool bHasAlpha )
    {
        mbHaveAlpha = bHasAlpha;
        mpVirtualDevice.disposeAndClear();
        mpSurface = pSurface;
        mpCairo = pSurface->getCairo();
    }

    static void setColor( cairo_t* pCairo,
                          const uno::Sequence<double>& rColor )
    {
        if( rColor.getLength() > 3 )
        {
            cairo_set_source_rgba( pCairo,
                                   rColor[0],
                                   rColor[1],
                                   rColor[2],
                                   rColor[3] );
        }
        else if( rColor.getLength() == 3 )
            cairo_set_source_rgb( pCairo,
                                  rColor[0],
                                  rColor[1],
                                  rColor[2] );
    }

    void CanvasHelper::useStates( const rendering::ViewState& viewState,
                                  const rendering::RenderState& renderState,
                                  bool bSetColor )
    {
        cairo_matrix_t aViewMatrix;
        cairo_matrix_t aRenderMatrix;
        cairo_matrix_t aCombinedMatrix;

        cairo_matrix_init( &aViewMatrix,
                           viewState.AffineTransform.m00, viewState.AffineTransform.m10, viewState.AffineTransform.m01,
                           viewState.AffineTransform.m11, viewState.AffineTransform.m02, viewState.AffineTransform.m12);
        cairo_matrix_init( &aRenderMatrix,
                           renderState.AffineTransform.m00, renderState.AffineTransform.m10, renderState.AffineTransform.m01,
                           renderState.AffineTransform.m11, renderState.AffineTransform.m02, renderState.AffineTransform.m12);
        cairo_matrix_multiply( &aCombinedMatrix, &aRenderMatrix, &aViewMatrix);

        if( viewState.Clip.is() )
        {
            SAL_INFO( "canvas.cairo", "view clip");

            aViewMatrix.x0 = basegfx::fround( aViewMatrix.x0 );
            aViewMatrix.y0 = basegfx::fround( aViewMatrix.y0 );
            cairo_set_matrix( mpCairo.get(), &aViewMatrix );
            doPolyPolygonPath( viewState.Clip, Clip );
        }

        aCombinedMatrix.x0 = basegfx::fround( aCombinedMatrix.x0 );
        aCombinedMatrix.y0 = basegfx::fround( aCombinedMatrix.y0 );
        cairo_set_matrix( mpCairo.get(), &aCombinedMatrix );

        if( renderState.Clip.is() )
        {
            SAL_INFO( "canvas.cairo", "render clip BEGIN");

            doPolyPolygonPath( renderState.Clip, Clip );
            SAL_INFO( "canvas.cairo", "render clip END");
        }

        if( bSetColor )
            setColor(mpCairo.get(),renderState.DeviceColor);

        cairo_operator_t compositingMode( CAIRO_OPERATOR_OVER );
        switch( renderState.CompositeOperation )
        {
            case rendering::CompositeOperation::CLEAR:
                compositingMode = CAIRO_OPERATOR_CLEAR;
                break;
            case rendering::CompositeOperation::SOURCE:
                compositingMode = CAIRO_OPERATOR_SOURCE;
                break;
            case rendering::CompositeOperation::DESTINATION:
                compositingMode = CAIRO_OPERATOR_DEST;
                break;
            case rendering::CompositeOperation::OVER:
                compositingMode = CAIRO_OPERATOR_OVER;
                break;
            case rendering::CompositeOperation::UNDER:
                compositingMode = CAIRO_OPERATOR_DEST;
                break;
            case rendering::CompositeOperation::INSIDE:
                compositingMode = CAIRO_OPERATOR_IN;
                break;
            case rendering::CompositeOperation::INSIDE_REVERSE:
                compositingMode = CAIRO_OPERATOR_OUT;
                break;
            case rendering::CompositeOperation::OUTSIDE:
                compositingMode = CAIRO_OPERATOR_DEST_OVER;
                break;
            case rendering::CompositeOperation::OUTSIDE_REVERSE:
                compositingMode = CAIRO_OPERATOR_DEST_OUT;
                break;
            case rendering::CompositeOperation::ATOP:
                compositingMode = CAIRO_OPERATOR_ATOP;
                break;
            case rendering::CompositeOperation::ATOP_REVERSE:
                compositingMode = CAIRO_OPERATOR_DEST_ATOP;
                break;
            case rendering::CompositeOperation::XOR:
                compositingMode = CAIRO_OPERATOR_XOR;
                break;
            case rendering::CompositeOperation::ADD:
                compositingMode = CAIRO_OPERATOR_ADD;
                break;
            case rendering::CompositeOperation::SATURATE:
                compositingMode = CAIRO_OPERATOR_SATURATE;
                break;
        }
        cairo_set_operator( mpCairo.get(), compositingMode );
    }

    void CanvasHelper::clear()
    {
        SAL_INFO( "canvas.cairo", "clear whole area: " << maSize.getX() << " x " << maSize.getY() );

        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            cairo_identity_matrix( mpCairo.get() );
            // this does not really differ from all-zero, as cairo
            // internally converts to premultiplied alpha. but anyway,
            // this keeps it consistent with the other canvas impls
            if( mbHaveAlpha )
                cairo_set_source_rgba( mpCairo.get(), 1.0, 1.0, 1.0, 0.0 );
            else
                cairo_set_source_rgb( mpCairo.get(), 1.0, 1.0, 1.0 );
            cairo_set_operator( mpCairo.get(), CAIRO_OPERATOR_SOURCE );

            cairo_rectangle( mpCairo.get(), 0, 0, maSize.getX(), maSize.getY() );
            cairo_fill( mpCairo.get() );

            cairo_restore( mpCairo.get() );
        }
    }

    void CanvasHelper::drawLine( const rendering::XCanvas*      /*pCanvas*/,
                                 const geometry::RealPoint2D&   aStartPoint,
                                 const geometry::RealPoint2D&   aEndPoint,
                                 const rendering::ViewState&    viewState,
                                 const rendering::RenderState&  renderState )
    {
        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            cairo_set_line_width( mpCairo.get(), 1 );

            useStates( viewState, renderState, true );

            cairo_move_to( mpCairo.get(), aStartPoint.X + 0.5, aStartPoint.Y + 0.5 );
            cairo_line_to( mpCairo.get(), aEndPoint.X + 0.5, aEndPoint.Y + 0.5 );
            cairo_stroke( mpCairo.get() );

            cairo_restore( mpCairo.get() );
        }
    }

    void CanvasHelper::drawBezier( const rendering::XCanvas*            ,
                                   const geometry::RealBezierSegment2D& aBezierSegment,
                                   const geometry::RealPoint2D&         aEndPoint,
                                   const rendering::ViewState&          viewState,
                                   const rendering::RenderState&        renderState )
    {
        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            cairo_set_line_width( mpCairo.get(), 1 );

            useStates( viewState, renderState, true );

            cairo_move_to( mpCairo.get(), aBezierSegment.Px + 0.5, aBezierSegment.Py + 0.5 );
            // tdf#99165 correction of control poinits not needed here, only hairlines drawn
            // (see cairo_set_line_width above)
            cairo_curve_to( mpCairo.get(),
                            aBezierSegment.C1x + 0.5, aBezierSegment.C1y + 0.5,
                            aBezierSegment.C2x + 0.5, aBezierSegment.C2y + 0.5,
                            aEndPoint.X + 0.5, aEndPoint.Y + 0.5 );
            cairo_stroke( mpCairo.get() );

            cairo_restore( mpCairo.get() );
        }
    }

#define PARAMETRICPOLYPOLYGON_IMPLEMENTATION_NAME "Canvas::ParametricPolyPolygon"

    /** surfaceFromXBitmap Create a surface from XBitmap
     * @param xBitmap bitmap image that will be used for the surface
     * @param bHasAlpha will be set to true if resulting surface has alpha
     *
     * This is a helper function for the other surfaceFromXBitmap().
     * This function tries to create surface from xBitmap by checking if xBitmap is CanvasBitmap or SpriteCanvas.
     *
     * @return created surface or NULL
     **/
    static SurfaceSharedPtr surfaceFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap )
    {
        CanvasBitmap* pBitmapImpl = dynamic_cast< CanvasBitmap* >( xBitmap.get() );
        if( pBitmapImpl )
            return pBitmapImpl->getSurface();

        SurfaceProvider* pSurfaceProvider = dynamic_cast<SurfaceProvider*>( xBitmap.get() );
        if( pSurfaceProvider )
            return pSurfaceProvider->getSurface();

        return SurfaceSharedPtr();
    }

    static ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap )
    {
        // TODO(F1): Add support for floating point bitmap formats
        uno::Reference<rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap,
                                                                  uno::UNO_QUERY_THROW);
        ::BitmapEx aBmpEx = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
        if( !!aBmpEx )
            return aBmpEx;

        // TODO(F1): extract pixel from XBitmap interface
        ENSURE_OR_THROW( false,
                         "bitmapExFromXBitmap(): could not extract BitmapEx" );

        return ::BitmapEx();
    }

    static sal_uInt8 lcl_GetColor(BitmapColor const& rColor)
    {
        sal_uInt8 nTemp(0);
        if (rColor.IsIndex())
        {
            nTemp = rColor.GetIndex();
        }
        else
        {
            nTemp = rColor.GetBlue();
            // greyscale expected here, or what would non-grey colors mean?
            assert(rColor.GetRed() == nTemp && rColor.GetGreen() == nTemp);
        }
        return nTemp;
    }

    static bool readAlpha( BitmapReadAccess* pAlphaReadAcc, long nY, const long nWidth, unsigned char* data, long nOff )
    {
        bool bIsAlpha = false;
        long nX;
        int nAlpha;
        Scanline pReadScan;

        nOff += 3;

        switch( pAlphaReadAcc->GetScanlineFormat() )
        {
            case ScanlineFormat::N8BitTcMask:
                pReadScan = pAlphaReadAcc->GetScanline( nY );
                for( nX = 0; nX < nWidth; nX++ )
                {
                    nAlpha = data[ nOff ] = 255 - ( *pReadScan++ );
                    if( nAlpha != 255 )
                        bIsAlpha = true;
                    nOff += 4;
                }
                break;
            case ScanlineFormat::N8BitPal:
                pReadScan = pAlphaReadAcc->GetScanline( nY );
                for( nX = 0; nX < nWidth; nX++ )
                {
                    BitmapColor const& rColor(
                        pAlphaReadAcc->GetPaletteColor(*pReadScan));
                    pReadScan++;
                    nAlpha = data[ nOff ] = 255 - lcl_GetColor(rColor);
                    if( nAlpha != 255 )
                        bIsAlpha = true;
                    nOff += 4;
                }
                break;
            default:
                SAL_INFO( "canvas.cairo", "fallback to GetColor for alpha - slow, format: " << (int)pAlphaReadAcc->GetScanlineFormat() );
                for( nX = 0; nX < nWidth; nX++ )
                {
                    nAlpha = data[ nOff ] = 255 - pAlphaReadAcc->GetColor( nY, nX ).GetIndex();
                    if( nAlpha != 255 )
                        bIsAlpha = true;
                    nOff += 4;
                }
        }

        return bIsAlpha;
    }


    /** surfaceFromXBitmap Create a surface from XBitmap
     * @param xBitmap bitmap image that will be used for the surface
     * @param rDevice reference to the device into which we want to draw
     * @param data will be filled with alpha data, if xBitmap is alpha/transparent image
     * @param bHasAlpha will be set to true if resulting surface has alpha
     *
     * This function tries various methods for creating a surface from xBitmap. It also uses
     * the helper function surfaceFromXBitmap( xBitmap, bHasAlpha )
     *
     * @return created surface or NULL
     **/
    static SurfaceSharedPtr surfaceFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap, const SurfaceProviderRef& rSurfaceProvider, unsigned char*& data, bool& bHasAlpha )
    {
        bHasAlpha = xBitmap->hasAlpha();
        SurfaceSharedPtr pSurface = surfaceFromXBitmap( xBitmap );
        if( pSurface )
            data = nullptr;
        else
        {
            ::BitmapEx aBmpEx = bitmapExFromXBitmap(xBitmap);
            ::Bitmap aBitmap = aBmpEx.GetBitmap();

            // there's no pixmap for alpha bitmap. we might still
            // use rgb pixmap and only access alpha pixels the
            // slow way. now we just speedup rgb bitmaps
            if( !aBmpEx.IsTransparent() && !aBmpEx.IsAlpha() )
            {
                pSurface = rSurfaceProvider->createSurface( aBitmap );
                data = nullptr;
                bHasAlpha = false;
            }

            if( !pSurface )
            {
                AlphaMask aAlpha = aBmpEx.GetAlpha();

                ::BitmapReadAccess* pBitmapReadAcc = aBitmap.AcquireReadAccess();
                ::BitmapReadAccess* pAlphaReadAcc = nullptr;
                const long      nWidth = pBitmapReadAcc->Width();
                const long      nHeight = pBitmapReadAcc->Height();
                long nX, nY;
                bool bIsAlpha = false;

                if( aBmpEx.IsTransparent() || aBmpEx.IsAlpha() )
                    pAlphaReadAcc = aAlpha.AcquireReadAccess();

                data = static_cast<unsigned char*>(malloc( nWidth*nHeight*4 ));

                long nOff = 0;
                ::Color aColor;
                unsigned int nAlpha = 255;

                for( nY = 0; nY < nHeight; nY++ )
                {
                    ::Scanline pReadScan;

                    switch( pBitmapReadAcc->GetScanlineFormat() )
                    {
                    case ScanlineFormat::N8BitPal:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ )
                        {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff++ ];
                            else
                                nAlpha = data[ nOff++ ] = 255;
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
#endif
                            aColor = pBitmapReadAcc->GetPaletteColor( *pReadScan++ );

#ifdef OSL_BIGENDIAN
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetRed() ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetGreen() ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetBlue() ) )/255 );
#else
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetBlue() ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetGreen() ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetRed() ) )/255 );
                            nOff++;
#endif
                        }
                        break;
                    case ScanlineFormat::N24BitTcBgr:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ )
                        {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff ];
                            else
                                nAlpha = data[ nOff ] = 255;
                            data[ nOff + 3 ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff + 2 ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff + 1 ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            nOff += 4;
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            nOff++;
#endif
                        }
                        break;
                    case ScanlineFormat::N24BitTcRgb:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ )
                        {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff++ ];
                            else
                                nAlpha = data[ nOff++ ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 2 ] ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 1 ] ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 0 ] ) )/255 );
                            pReadScan += 3;
                            nOff++;
#endif
                        }
                        break;
                    case ScanlineFormat::N32BitTcBgra:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ )
                        {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff++ ];
                            else
                                nAlpha = data[ nOff++ ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 2 ] ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 1 ] ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 0 ] ) )/255 );
                            pReadScan += 4;
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            pReadScan++;
                            nOff++;
#endif
                        }
                        break;
                    case ScanlineFormat::N32BitTcRgba:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ )
                        {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff ++ ];
                            else
                                nAlpha = data[ nOff ++ ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255 );
                            pReadScan++;
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 2 ] ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 1 ] ) )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 0 ] ) )/255 );
                            pReadScan += 4;
                            nOff++;
#endif
                        }
                        break;
                    default:
                        SAL_INFO( "canvas.cairo", "fallback to GetColor - slow, format: " << (int)pBitmapReadAcc->GetScanlineFormat() );

                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ )
                        {
                            aColor = pBitmapReadAcc->GetColor( nY, nX );

                            // cairo need premultiplied color values
                            // TODO(rodo) handle endianness
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff++ ];
                            else
                                nAlpha = data[ nOff++ ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetRed() )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetGreen() )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetBlue() )/255 );
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetBlue() )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetGreen() )/255 );
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetRed() )/255 );
                            nOff ++;
#endif
                        }
                    }
                }

                ::Bitmap::ReleaseAccess( pBitmapReadAcc );
                if( pAlphaReadAcc )
                    aAlpha.ReleaseAccess( pAlphaReadAcc );

                SurfaceSharedPtr pImageSurface = rSurfaceProvider->getOutputDevice()->CreateSurface(
                    CairoSurfaceSharedPtr(
                        cairo_image_surface_create_for_data(
                            data,
                            bIsAlpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24,
                            nWidth, nHeight, nWidth*4 ),
                        &cairo_surface_destroy) );
                pSurface = pImageSurface;

                bHasAlpha = bIsAlpha;

                SAL_INFO( "canvas.cairo","image: " << nWidth << " x " << nHeight << " alpha: " << bIsAlpha << " alphaRead " << std::hex << pAlphaReadAcc);
            }
        }

        return pSurface;
    }

    static void addColorStops( cairo_pattern_t* pPattern, const uno::Sequence< uno::Sequence< double > >& rColors, const uno::Sequence< double >& rStops, bool bReverseStops = false )
    {
        int i;

        OSL_ASSERT( rColors.getLength() == rStops.getLength() );

        for( i = 0; i < rColors.getLength(); i++ )
        {
            const uno::Sequence< double >& rColor( rColors[i] );
            float stop = bReverseStops ? 1 - rStops[i] : rStops[i];
            if( rColor.getLength() == 3 )
                cairo_pattern_add_color_stop_rgb( pPattern, stop, rColor[0], rColor[1], rColor[2] );
            else if( rColor.getLength() == 4 )
            {
                double alpha = rColor[3];
                // cairo expects premultiplied alpha
                cairo_pattern_add_color_stop_rgba( pPattern, stop, rColor[0]*alpha, rColor[1]*alpha, rColor[2]*alpha, alpha );
            }
        }
    }

    static uno::Sequence<double> lerp(const uno::Sequence<double>& rLeft, const uno::Sequence<double>& rRight, double fAlpha)
    {
        if( rLeft.getLength() == 3 )
        {
            uno::Sequence<double> aRes(3);
            aRes[0] = basegfx::tools::lerp(rLeft[0],rRight[0],fAlpha);
            aRes[1] = basegfx::tools::lerp(rLeft[1],rRight[1],fAlpha);
            aRes[2] = basegfx::tools::lerp(rLeft[2],rRight[2],fAlpha);
            return aRes;
        }
        else if( rLeft.getLength() == 4 )
        {
            uno::Sequence<double> aRes(4);
            aRes[0] = basegfx::tools::lerp(rLeft[0],rRight[0],fAlpha);
            aRes[1] = basegfx::tools::lerp(rLeft[1],rRight[1],fAlpha);
            aRes[2] = basegfx::tools::lerp(rLeft[2],rRight[2],fAlpha);
            aRes[3] = basegfx::tools::lerp(rLeft[3],rRight[3],fAlpha);
            return aRes;
        }

        return uno::Sequence<double>();
    }

    static cairo_pattern_t* patternFromParametricPolyPolygon( ::canvas::ParametricPolyPolygon& rPolygon )
    {
        cairo_pattern_t* pPattern = nullptr;
        const ::canvas::ParametricPolyPolygon::Values aValues = rPolygon.getValues();
        double x0, x1, y0, y1, cx, cy, r0, r1;

        switch( aValues.meType )
        {
            case ::canvas::ParametricPolyPolygon::GradientType::Linear:
                x0 = 0;
                y0 = 0;
                x1 = 1;
                y1 = 0;
                pPattern = cairo_pattern_create_linear( x0, y0, x1, y1 );
                addColorStops( pPattern, aValues.maColors, aValues.maStops );
                break;

            case ::canvas::ParametricPolyPolygon::GradientType::Elliptical:
                cx = 0;
                cy = 0;
                r0 = 0;
                r1 = 1;

                pPattern = cairo_pattern_create_radial( cx, cy, r0, cy, cy, r1 );
                addColorStops( pPattern, aValues.maColors, aValues.maStops, true );
                break;
            default:
                break;
        }

        return pPattern;
    }

    static void doOperation( Operation aOperation,
                             cairo_t* pCairo,
                             const uno::Sequence< rendering::Texture >* pTextures,
                             const SurfaceProviderRef& pDevice,
                             const basegfx::B2DRange& rBounds )
    {
        switch( aOperation )
        {
            case Fill:
                /* TODO: multitexturing */
                if( pTextures )
                {
                    const css::rendering::Texture& aTexture ( (*pTextures)[0] );
                    if( aTexture.Bitmap.is() )
                    {
                        unsigned char* data = nullptr;
                        bool bHasAlpha = false;
                        SurfaceSharedPtr pSurface = surfaceFromXBitmap( (*pTextures)[0].Bitmap, pDevice, data, bHasAlpha );

                        if( pSurface )
                        {
                            cairo_pattern_t* pPattern;

                            cairo_save( pCairo );

                            css::geometry::AffineMatrix2D aTransform( aTexture.AffineTransform );
                            cairo_matrix_t aScaleMatrix, aTextureMatrix, aScaledTextureMatrix;

                            cairo_matrix_init( &aTextureMatrix,
                                               aTransform.m00, aTransform.m10, aTransform.m01,
                                               aTransform.m11, aTransform.m02, aTransform.m12);

                            geometry::IntegerSize2D aSize = aTexture.Bitmap->getSize();

                            cairo_matrix_init_scale( &aScaleMatrix, 1.0/aSize.Width, 1.0/aSize.Height );
                            cairo_matrix_multiply( &aScaledTextureMatrix, &aTextureMatrix, &aScaleMatrix );
                            cairo_matrix_invert( &aScaledTextureMatrix );

                            // we don't care about repeat mode yet, so the workaround is disabled for now
                            pPattern = cairo_pattern_create_for_surface( pSurface->getCairoSurface().get() );

                            if( aTexture.RepeatModeX == rendering::TexturingMode::REPEAT &&
                                aTexture.RepeatModeY == rendering::TexturingMode::REPEAT )
                            {
                                cairo_pattern_set_extend( pPattern, CAIRO_EXTEND_REPEAT );
                            }
                            else if ( aTexture.RepeatModeX == rendering::TexturingMode::NONE &&
                                      aTexture.RepeatModeY == rendering::TexturingMode::NONE )
                            {
                                cairo_pattern_set_extend( pPattern, CAIRO_EXTEND_NONE );
                            }
                            else if ( aTexture.RepeatModeX == rendering::TexturingMode::CLAMP &&
                                      aTexture.RepeatModeY == rendering::TexturingMode::CLAMP )
                            {
                                cairo_pattern_set_extend( pPattern, CAIRO_EXTEND_PAD );
                            }

                            aScaledTextureMatrix.x0 = basegfx::fround( aScaledTextureMatrix.x0 );
                            aScaledTextureMatrix.y0 = basegfx::fround( aScaledTextureMatrix.y0 );
                            cairo_pattern_set_matrix( pPattern, &aScaledTextureMatrix );

                            cairo_set_source( pCairo, pPattern );
                            if( !bHasAlpha )
                                cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
                            cairo_fill( pCairo );

                            cairo_restore( pCairo );

                            cairo_pattern_destroy( pPattern );
                        }

                        if( data )
                            free( data );
                    }
                    else if( aTexture.Gradient.is() )
                    {
                        uno::Reference< lang::XServiceInfo > xRef( aTexture.Gradient, uno::UNO_QUERY );

                        SAL_INFO( "canvas.cairo", "gradient fill" );
                        if( xRef.is() && xRef->getImplementationName() == PARAMETRICPOLYPOLYGON_IMPLEMENTATION_NAME )
                        {
                            // TODO(Q1): Maybe use dynamic_cast here

                            // TODO(E1): Return value
                            // TODO(F1): FillRule
                            SAL_INFO( "canvas.cairo", "known implementation" );

                            ::canvas::ParametricPolyPolygon* pPolyImpl = static_cast< ::canvas::ParametricPolyPolygon* >( aTexture.Gradient.get() );
                            css::geometry::AffineMatrix2D aTransform( aTexture.AffineTransform );
                            cairo_matrix_t aTextureMatrix;

                            cairo_matrix_init( &aTextureMatrix,
                                               aTransform.m00, aTransform.m10, aTransform.m01,
                                               aTransform.m11, aTransform.m02, aTransform.m12);
                            if( pPolyImpl->getValues().meType == canvas::ParametricPolyPolygon::GradientType::Rectangular )
                            {
                                // no general path gradient yet in cairo; emulate then
                                cairo_save( pCairo );
                                cairo_clip( pCairo );

                                // fill bound rect with start color
                                cairo_rectangle( pCairo, rBounds.getMinX(), rBounds.getMinY(),
                                                 rBounds.getWidth(), rBounds.getHeight() );
                                setColor(pCairo,pPolyImpl->getValues().maColors[0]);
                                cairo_fill(pCairo);

                                cairo_transform( pCairo, &aTextureMatrix );

                                // longest line in gradient bound rect
                                const unsigned int nGradientSize(
                                    static_cast<unsigned int>(
                                        ::basegfx::B2DVector(rBounds.getMinimum() - rBounds.getMaximum()).getLength() + 1.0 ) );

                                // typical number for pixel of the same color (strip size)
                                const unsigned int nStripSize( nGradientSize < 50 ? 2 : 4 );

                                // use at least three steps, and at utmost the number of color
                                // steps
                                const unsigned int nStepCount(
                                    ::std::max(
                                        3U,
                                        ::std::min(
                                            nGradientSize / nStripSize,
                                            128U )) + 1 );

                                const uno::Sequence<double>* pColors=&pPolyImpl->getValues().maColors[0];
                                basegfx::tools::KeyStopLerp aLerper(pPolyImpl->getValues().maStops);
                                for( unsigned int i=1; i<nStepCount; ++i )
                                {
                                    const double fT( i/double(nStepCount) );

                                    std::ptrdiff_t nIndex;
                                    double fAlpha;
                                    std::tie(nIndex,fAlpha)=aLerper.lerp(fT);

                                    setColor(pCairo, lerp(pColors[nIndex], pColors[nIndex+1], fAlpha));
                                    cairo_rectangle( pCairo, -1+fT, -1+fT, 2-2*fT, 2-2*fT );
                                    cairo_fill(pCairo);
                                }

                                cairo_restore( pCairo );
                            }
                            else
                            {
                                cairo_pattern_t* pPattern = patternFromParametricPolyPolygon( *pPolyImpl );

                                if( pPattern )
                                {
                                    SAL_INFO( "canvas.cairo", "filling with pattern" );

                                    cairo_save( pCairo );

                                    cairo_transform( pCairo, &aTextureMatrix );
                                    cairo_set_source( pCairo, pPattern );
                                    cairo_fill( pCairo );
                                    cairo_restore( pCairo );

                                    cairo_pattern_destroy( pPattern );
                                }
                            }
                        }
                    }
                }
                else
                    cairo_fill( pCairo );
                SAL_INFO( "canvas.cairo", "fill");
                break;
            case Stroke:
                cairo_stroke( pCairo );
                SAL_INFO( "canvas.cairo", "stroke");
                break;
            case Clip:
                cairo_clip( pCairo );
                SAL_INFO( "canvas.cairo", "clip");
                break;
        }
    }

    static void clipNULL( cairo_t *pCairo )
    {
        SAL_INFO( "canvas.cairo", "clipNULL");
        cairo_matrix_t aOrigMatrix, aIdentityMatrix;

        /* we set identity matrix here to overcome bug in cairo 0.9.2
           where XCreatePixmap is called with zero width and height.

           it also reaches faster path in cairo clipping code.
        */
        cairo_matrix_init_identity( &aIdentityMatrix );
        cairo_get_matrix( pCairo, &aOrigMatrix );
        cairo_set_matrix( pCairo, &aIdentityMatrix );

        cairo_reset_clip( pCairo );
        cairo_rectangle( pCairo, 0, 0, 1, 1 );
        cairo_clip( pCairo );
        cairo_rectangle( pCairo, 2, 0, 1, 1 );
        cairo_clip( pCairo );

        /* restore the original matrix */
        cairo_set_matrix( pCairo, &aOrigMatrix );
    }

    void doPolyPolygonImplementation( const ::basegfx::B2DPolyPolygon& aPolyPolygon,
                                      Operation aOperation,
                                      cairo_t* pCairo,
                                      const uno::Sequence< rendering::Texture >* pTextures,
                                      const SurfaceProviderRef& pDevice,
                                      rendering::FillRule eFillrule )
    {
        if( pTextures )
            ENSURE_ARG_OR_THROW( pTextures->getLength(),
                                 "CanvasHelper::fillTexturedPolyPolygon: empty texture sequence");

        bool bOpToDo = false;
        cairo_matrix_t aOrigMatrix, aIdentityMatrix;
        double nX, nY, nBX, nBY, nAX, nAY, nLastX(0.0), nLastY(0.0);

        cairo_get_matrix( pCairo, &aOrigMatrix );
        cairo_matrix_init_identity( &aIdentityMatrix );
        cairo_set_matrix( pCairo, &aIdentityMatrix );

        cairo_set_fill_rule( pCairo,
                             eFillrule == rendering::FillRule_EVEN_ODD ?
                             CAIRO_FILL_RULE_EVEN_ODD : CAIRO_FILL_RULE_WINDING );

        for( sal_uInt32 nPolygonIndex = 0; nPolygonIndex < aPolyPolygon.count(); nPolygonIndex++ )
        {
            ::basegfx::B2DPolygon aPolygon( aPolyPolygon.getB2DPolygon( nPolygonIndex ) );
            const sal_uInt32 nPointCount( aPolygon.count() );
            // to correctly render closed curves, need to output first
            // point twice (so output one additional point)
            const sal_uInt32 nExtendedPointCount( nPointCount +
                                                  int(aPolygon.isClosed() && aPolygon.areControlPointsUsed()) );

            if( nPointCount > 1)
            {
                bool bIsBezier = aPolygon.areControlPointsUsed();
                bool bIsRectangle = ::basegfx::tools::isRectangle( aPolygon );
                ::basegfx::B2DPoint aA, aB, aP;

                for( sal_uInt32 j=0; j < nExtendedPointCount; j++ )
                {
                    aP = aPolygon.getB2DPoint( j % nPointCount );

                    nX = aP.getX();
                    nY = aP.getY();
                    cairo_matrix_transform_point( &aOrigMatrix, &nX, &nY );

                    if( ! bIsBezier && (bIsRectangle || aOperation == Clip) )
                    {
                        nX = basegfx::fround( nX );
                        nY = basegfx::fround( nY );
                    }

                    if( aOperation == Stroke )
                    {
                        nX += 0.5;
                        nY += 0.5;
                    }

                    if( j==0 )
                    {
                        cairo_move_to( pCairo, nX, nY );
                        SAL_INFO( "canvas.cairo", "move to " << nX << "," << nY );
                    }
                    else
                    {
                        if( bIsBezier )
                        {
                            aA = aPolygon.getNextControlPoint( (j-1) % nPointCount );
                            aB = aPolygon.getPrevControlPoint( j % nPointCount );

                            nAX = aA.getX();
                            nAY = aA.getY();
                            nBX = aB.getX();
                            nBY = aB.getY();

                            cairo_matrix_transform_point( &aOrigMatrix, &nAX, &nAY );
                            cairo_matrix_transform_point( &aOrigMatrix, &nBX, &nBY );

                            if( aOperation == Stroke )
                            {
                                nAX += 0.5;
                                nAY += 0.5;
                                nBX += 0.5;
                                nBY += 0.5;
                            }

                            // tdf#99165 if the control points are 'empty', create the mathematical
                            // correct replacement ones to avoid problems with the graphical sub-system
                            // tdf#101026 The 1st attempt to create a mathematically correct replacement control
                            // vector was wrong. Best alternative is one as close as possible which means short.
                            if (basegfx::fTools::equal(nAX, nLastX) && basegfx::fTools::equal(nAY, nLastY))
                            {
                                nAX = nLastX + ((nBX - nLastX) * 0.0005);
                                nAY = nLastY + ((nBY - nLastY) * 0.0005);
                            }

                            if(basegfx::fTools::equal(nBX, nX) && basegfx::fTools::equal(nBY, nY))
                            {
                                nBX = nX + ((nAX - nX) * 0.0005);
                                nBY = nY + ((nAY - nY) * 0.0005);
                            }

                            cairo_curve_to( pCairo, nAX, nAY, nBX, nBY, nX, nY );
                        }
                        else
                        {
                            cairo_line_to( pCairo, nX, nY );
                            SAL_INFO( "canvas.cairo", "line to " << nX << "," << nY );
                        }
                        bOpToDo = true;
                    }

                    nLastX = nX;
                    nLastY = nY;
                }

                if( aPolygon.isClosed() )
                    cairo_close_path( pCairo );

            }
            else
            {
                SAL_INFO( "canvas.cairo", "empty polygon for op: " << aOperation );
                if( aOperation == Clip )
                {
                    clipNULL( pCairo );

                    return;
                }
            }
        }

        if( aOperation == Fill && pTextures )
        {
            cairo_set_matrix( pCairo, &aOrigMatrix );
            doOperation( aOperation, pCairo, pTextures, pDevice, aPolyPolygon.getB2DRange() );
            cairo_set_matrix( pCairo, &aIdentityMatrix );
        }

        if( bOpToDo && ( aOperation != Fill || !pTextures ) )
            doOperation( aOperation, pCairo, pTextures, pDevice, aPolyPolygon.getB2DRange() );

        cairo_set_matrix( pCairo, &aOrigMatrix );

        if( aPolyPolygon.count() == 0 && aOperation == Clip )
            clipNULL( pCairo );
    }

    void CanvasHelper::doPolyPolygonPath( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                        Operation aOperation,
                        bool bNoLineJoin,
                        const uno::Sequence< rendering::Texture >* pTextures,
                        cairo_t* pCairo ) const
    {
        const ::basegfx::B2DPolyPolygon& rPolyPoly(
            ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon) );

        if( !pCairo )
            pCairo = mpCairo.get();

        if(bNoLineJoin && Stroke == aOperation)
        {
            // emulate rendering::PathJoinType::NONE by painting single edges
            for(sal_uInt32 a(0); a < rPolyPoly.count(); a++)
            {
                const basegfx::B2DPolygon aCandidate(rPolyPoly.getB2DPolygon(a));
                const sal_uInt32 nPointCount(aCandidate.count());

                if(nPointCount)
                {
                    const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount: nPointCount - 1);
                    basegfx::B2DPolygon aEdge;
                    aEdge.append(aCandidate.getB2DPoint(0));
                    aEdge.append(basegfx::B2DPoint(0.0, 0.0));

                    for(sal_uInt32 b(0); b < nEdgeCount; b++)
                    {
                        const sal_uInt32 nNextIndex((b + 1) % nPointCount);
                        aEdge.setB2DPoint(1, aCandidate.getB2DPoint(nNextIndex));
                        aEdge.setNextControlPoint(0, aCandidate.getNextControlPoint(b % nPointCount));
                        aEdge.setPrevControlPoint(1, aCandidate.getPrevControlPoint(nNextIndex));

                        doPolyPolygonImplementation( basegfx::B2DPolyPolygon(aEdge),
                                                     aOperation,
                                                     pCairo, pTextures,
                                                     mpSurfaceProvider,
                                                     xPolyPolygon->getFillRule() );

                        // prepare next step
                        aEdge.setB2DPoint(0, aEdge.getB2DPoint(1));
                    }
                }
            }
        }
        else
        {
            doPolyPolygonImplementation( rPolyPoly, aOperation,
                                         pCairo, pTextures,
                                         mpSurfaceProvider,
                                         xPolyPolygon->getFillRule() );
        }
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawPolyPolygon( const rendering::XCanvas*                          ,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            cairo_set_line_width( mpCairo.get(), 1 );

            useStates( viewState, renderState, true );
            doPolyPolygonPath( xPolyPolygon, Stroke );

            cairo_restore( mpCairo.get() );
        }
        else
            SAL_INFO( "canvas.cairo", "CanvasHelper called after it was disposed");

#ifdef CAIRO_CANVAS_PERF_TRACE
        mxDevice->stopPerfTrace( &aTimer, "drawPolyPolygon" );
#endif

        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokePolyPolygon( const rendering::XCanvas*                            ,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
                                                                                   const rendering::ViewState&                          viewState,
                                                                                   const rendering::RenderState&                        renderState,
                                                                                   const rendering::StrokeAttributes&                   strokeAttributes )
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            useStates( viewState, renderState, true );

            cairo_matrix_t aMatrix;
            double w = strokeAttributes.StrokeWidth, h = 0;
            cairo_get_matrix( mpCairo.get(), &aMatrix );
            cairo_matrix_transform_distance( &aMatrix, &w, &h );
            cairo_set_line_width( mpCairo.get(), w );

            cairo_set_miter_limit( mpCairo.get(), strokeAttributes.MiterLimit );

            // FIXME: cairo doesn't handle end cap so far (rodo)
            switch( strokeAttributes.StartCapType )
            {
                case rendering::PathCapType::BUTT:
                    cairo_set_line_cap( mpCairo.get(), CAIRO_LINE_CAP_BUTT );
                    break;
                case rendering::PathCapType::ROUND:
                    cairo_set_line_cap( mpCairo.get(), CAIRO_LINE_CAP_ROUND );
                    break;
                case rendering::PathCapType::SQUARE:
                    cairo_set_line_cap( mpCairo.get(), CAIRO_LINE_CAP_SQUARE );
                    break;
            }

            bool bNoLineJoin(false);

            switch( strokeAttributes.JoinType )
            {
                case rendering::PathJoinType::NONE:
                    bNoLineJoin = true;
                    SAL_FALLTHROUGH; // cairo doesn't have join type NONE so we use MITER as it's pretty close
                case rendering::PathJoinType::MITER:
                    cairo_set_line_join( mpCairo.get(), CAIRO_LINE_JOIN_MITER );
                    break;
                case rendering::PathJoinType::ROUND:
                    cairo_set_line_join( mpCairo.get(), CAIRO_LINE_JOIN_ROUND );
                    break;
                case rendering::PathJoinType::BEVEL:
                    cairo_set_line_join( mpCairo.get(), CAIRO_LINE_JOIN_BEVEL );
                    break;
            }

            if( strokeAttributes.DashArray.getLength() > 0 )
            {
                double* pDashArray = new double[ strokeAttributes.DashArray.getLength() ];
                for( sal_Int32 i=0; i<strokeAttributes.DashArray.getLength(); i++ )
                    pDashArray[i] = strokeAttributes.DashArray[i] * w;
                cairo_set_dash( mpCairo.get(), pDashArray, strokeAttributes.DashArray.getLength(), 0 );
                delete[] pDashArray;
            }

            // TODO(rodo) use LineArray of strokeAttributes

            doPolyPolygonPath( xPolyPolygon, Stroke, bNoLineJoin );

            cairo_restore( mpCairo.get() );
        }
        else
            SAL_INFO( "canvas.cairo", "CanvasHelper called after it was disposed");

#ifdef CAIRO_CANVAS_PERF_TRACE
        mxDevice->stopPerfTrace( &aTimer, "strokePolyPolygon" );
#endif

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTexturedPolyPolygon( const rendering::XCanvas*                            ,
                                                                                           const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                           const rendering::ViewState&                          /*viewState*/,
                                                                                           const rendering::RenderState&                        /*renderState*/,
                                                                                           const uno::Sequence< rendering::Texture >&           /*textures*/,
                                                                                           const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTextureMappedPolyPolygon( const rendering::XCanvas*                           ,
                                                                                                const uno::Reference< rendering::XPolyPolygon2D >&  /*xPolyPolygon*/,
                                                                                                const rendering::ViewState&                         /*viewState*/,
                                                                                                const rendering::RenderState&                       /*renderState*/,
                                                                                                const uno::Sequence< rendering::Texture >&          /*textures*/,
                                                                                                const uno::Reference< geometry::XMapping2D >&       /*xMapping*/,
                                                                                                const rendering::StrokeAttributes&                  /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XPolyPolygon2D >   CanvasHelper::queryStrokeShapes( const rendering::XCanvas*                            ,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                   const rendering::ViewState&                          /*viewState*/,
                                                                                   const rendering::RenderState&                        /*renderState*/,
                                                                                   const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillPolyPolygon( const rendering::XCanvas*                          ,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            useStates( viewState, renderState, true );
            doPolyPolygonPath( xPolyPolygon, Fill );

            cairo_restore( mpCairo.get() );
        }
        else
            SAL_INFO( "canvas.cairo", "CanvasHelper called after it was disposed");

#ifdef CAIRO_CANVAS_PERF_TRACE
        mxDevice->stopPerfTrace( &aTimer, "fillPolyPolygon" );
#endif

        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas*                          ,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         const uno::Sequence< rendering::Texture >&         textures )
    {
        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            useStates( viewState, renderState, true );
            doPolyPolygonPath( xPolyPolygon, Fill, false, &textures );

            cairo_restore( mpCairo.get() );
        }

        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTextureMappedPolyPolygon( const rendering::XCanvas*                             ,
                                                                                              const uno::Reference< rendering::XPolyPolygon2D >&    /*xPolyPolygon*/,
                                                                                              const rendering::ViewState&                           /*viewState*/,
                                                                                              const rendering::RenderState&                         /*renderState*/,
                                                                                              const uno::Sequence< rendering::Texture >&            /*textures*/,
                                                                                              const uno::Reference< geometry::XMapping2D >&         /*xMapping*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::implDrawBitmapSurface( const rendering::XCanvas*        pCanvas,
                                                                                       const SurfaceSharedPtr&          pInputSurface,
                                                                                       const rendering::ViewState&      viewState,
                                                                                       const rendering::RenderState&    renderState,
                                                                                       const geometry::IntegerSize2D&   rSize,
                                                                                       bool                             bModulateColors,
                                                                                       bool                             bHasAlpha )
    {
        SurfaceSharedPtr pSurface=pInputSurface;
        uno::Reference< rendering::XCachedPrimitive > rv(nullptr);
        geometry::IntegerSize2D aBitmapSize = rSize;

        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            cairo_rectangle( mpCairo.get(), 0, 0, maSize.getX(), maSize.getY() );
            cairo_clip( mpCairo.get() );

            useStates( viewState, renderState, true );

            cairo_matrix_t aMatrix;

            cairo_get_matrix( mpCairo.get(), &aMatrix );
            if( ! ::rtl::math::approxEqual( aMatrix.xx, 1 ) &&
                ! ::rtl::math::approxEqual( aMatrix.yy, 1 ) &&
                ::rtl::math::approxEqual( aMatrix.x0, 0 ) &&
                ::rtl::math::approxEqual( aMatrix.y0, 0 ) &&
                basegfx::fround( rSize.Width * aMatrix.xx ) > 8 &&
                basegfx::fround( rSize.Height* aMatrix.yy ) > 8 )
            {
                double dWidth, dHeight;

                dWidth = basegfx::fround( rSize.Width * aMatrix.xx );
                dHeight = basegfx::fround( rSize.Height* aMatrix.yy );
                aBitmapSize.Width = static_cast<sal_Int32>( dWidth );
                aBitmapSize.Height = static_cast<sal_Int32>( dHeight );

                SurfaceSharedPtr pScaledSurface = mpSurfaceProvider->createSurface(
                    ::basegfx::B2ISize( aBitmapSize.Width, aBitmapSize.Height ),
                    bHasAlpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR );
                CairoSharedPtr pCairo = pScaledSurface->getCairo();

                cairo_set_operator( pCairo.get(), CAIRO_OPERATOR_SOURCE );
                // add 0.5px to size to avoid rounding errors in cairo, leading sometimes to random data on the image right/bottom borders
                cairo_scale( pCairo.get(), (dWidth+0.5)/rSize.Width, (dHeight+0.5)/rSize.Height );
                cairo_set_source_surface( pCairo.get(), pSurface->getCairoSurface().get(), 0, 0 );
                cairo_paint( pCairo.get() );

                pSurface = pScaledSurface;

                aMatrix.xx = aMatrix.yy = 1;
                cairo_set_matrix( mpCairo.get(), &aMatrix );

                rv.set(
                    new CachedBitmap( pSurface, viewState, renderState,
                                      // cast away const, need to
                                      // change refcount (as this is
                                      // ~invisible to client code,
                                      // still logically const)
                                      const_cast< rendering::XCanvas* >(pCanvas)) );
            }

            if( !bHasAlpha && mbHaveAlpha )
            {
                double x, y, width, height;

                x = y = 0;
                width = aBitmapSize.Width;
                height = aBitmapSize.Height;
                cairo_matrix_transform_point( &aMatrix, &x, &y );
                cairo_matrix_transform_distance( &aMatrix, &width, &height );

                // in case the bitmap doesn't have alpha and covers whole area
                // we try to change surface to plain rgb
                SAL_INFO( "canvas.cairo","chance to change surface to rgb, " << x << ", " << y << ", " << width << " x " << height << " (" << maSize.getX() << " x " << maSize.getY() << ")" );
                if( x <= 0 && y <= 0 && x + width >= maSize.getX() && y + height >= maSize.getY() )
                {
                    SAL_INFO( "canvas.cairo","trying to change surface to rgb");
                    if( mpSurfaceProvider ) {
                        SurfaceSharedPtr pNewSurface = mpSurfaceProvider->changeSurface();

                        if( pNewSurface )
                            setSurface( pNewSurface, false );

                        // set state to new mpCairo.get()
                        useStates( viewState, renderState, true );
                        // use the possibly modified matrix
                        cairo_set_matrix( mpCairo.get(), &aMatrix );
                    }
                }
            }

            cairo_set_source_surface( mpCairo.get(), pSurface->getCairoSurface().get(), 0, 0 );
            if( !bHasAlpha &&
                ::rtl::math::approxEqual( aMatrix.xx, 1 ) &&
                ::rtl::math::approxEqual( aMatrix.yy, 1 ) &&
                ::rtl::math::approxEqual( aMatrix.x0, 0 ) &&
                ::rtl::math::approxEqual( aMatrix.y0, 0 ) )
                cairo_set_operator( mpCairo.get(), CAIRO_OPERATOR_SOURCE );
            cairo_pattern_set_extend( cairo_get_source(mpCairo.get()), CAIRO_EXTEND_PAD );
            cairo_rectangle( mpCairo.get(), 0, 0, aBitmapSize.Width, aBitmapSize.Height );
            cairo_clip( mpCairo.get() );

            if( bModulateColors )
                cairo_paint_with_alpha( mpCairo.get(), renderState.DeviceColor[3] );
            else
                cairo_paint( mpCairo.get() );
            cairo_restore( mpCairo.get() );
        }
        else
            SAL_INFO( "canvas.cairo", "CanvasHelper called after it was disposed");

        return rv; // uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmap( const rendering::XCanvas*                   pCanvas,
                                                                            const uno::Reference< rendering::XBitmap >& xBitmap,
                                                                            const rendering::ViewState&                 viewState,
                                                                            const rendering::RenderState&               renderState )
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        uno::Reference< rendering::XCachedPrimitive > rv;
        unsigned char* data = nullptr;
        bool bHasAlpha = false;
        SurfaceSharedPtr pSurface = surfaceFromXBitmap( xBitmap, mpSurfaceProvider, data, bHasAlpha );
        geometry::IntegerSize2D aSize = xBitmap->getSize();

        if( pSurface )
        {
            rv = implDrawBitmapSurface( pCanvas, pSurface, viewState, renderState, aSize, false, bHasAlpha );

            if( data )
                free( data );
        }
        else
            rv.set(nullptr);

#ifdef CAIRO_CANVAS_PERF_TRACE
        mxDevice->stopPerfTrace( &aTimer, "drawBitmap" );
#endif

        return rv;
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmapModulated( const rendering::XCanvas*                      pCanvas,
                                                                                     const uno::Reference< rendering::XBitmap >&    xBitmap,
                                                                                     const rendering::ViewState&                    viewState,
                                                                                     const rendering::RenderState&                  renderState )
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        uno::Reference< rendering::XCachedPrimitive > rv;
        unsigned char* data = nullptr;
        bool bHasAlpha = false;
        SurfaceSharedPtr pSurface = surfaceFromXBitmap( xBitmap, mpSurfaceProvider, data, bHasAlpha );
        geometry::IntegerSize2D aSize = xBitmap->getSize();

        if( pSurface )
        {
            rv = implDrawBitmapSurface( pCanvas, pSurface, viewState, renderState, aSize, true, bHasAlpha );

            if( data )
                free( data );
        }
        else
            rv.set(nullptr);

#ifdef CAIRO_CANVAS_PERF_TRACE
        mxDevice->stopPerfTrace( &aTimer, "drawBitmap" );
#endif

        return rv;
    }


    geometry::IntegerSize2D CanvasHelper::getSize()
    {
        if( !mpSurfaceProvider )
            return geometry::IntegerSize2D(1, 1); // we're disposed

        return ::basegfx::unotools::integerSize2DFromB2ISize( maSize );
    }

    uno::Reference< rendering::XBitmap > CanvasHelper::getScaledBitmap( const geometry::RealSize2D& newSize,
                                                                        bool                       /*beFast*/ )
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        if( mpCairo )
        {
            return uno::Reference< rendering::XBitmap >( new CanvasBitmap( ::basegfx::B2ISize( ::canvas::tools::roundUp( newSize.Width ),
                                                                                               ::canvas::tools::roundUp( newSize.Height ) ),
                                                                           mpSurfaceProvider, mpDevice, false ) );
        }
        else
            SAL_INFO( "canvas.cairo", "CanvasHelper called after it was disposed");

#ifdef CAIRO_CANVAS_PERF_TRACE
        mxDevice->stopPerfTrace( &aTimer, "getScaledBitmap" );
#endif

        return uno::Reference< rendering::XBitmap >();
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getData( rendering::IntegerBitmapLayout&     aLayout,
                                                     const geometry::IntegerRectangle2D& rect )
    {
        if( mpCairo )
        {
            const sal_Int32 nWidth( rect.X2 - rect.X1 );
            const sal_Int32 nHeight( rect.Y2 - rect.Y1 );
            const cairo_format_t eFormat( mbHaveAlpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24 );
            uno::Sequence< sal_Int8 > aRes( 4*nWidth*nHeight );
            sal_Int8* pData = aRes.getArray();
            cairo_surface_t* pImageSurface = cairo_image_surface_create_for_data( reinterpret_cast<unsigned char *>(pData),
                                                                                  eFormat,
                                                                                  nWidth, nHeight, 4*nWidth );
            cairo_t* pCairo = cairo_create( pImageSurface );
            cairo_set_source_surface( pCairo, mpSurface->getCairoSurface().get(), -rect.X1, -rect.Y1);
            cairo_paint( pCairo );
            cairo_destroy( pCairo );
            cairo_surface_destroy( pImageSurface );

            aLayout = impl_getMemoryLayout( nWidth, nHeight );

            return aRes;
        }

        return uno::Sequence< sal_Int8 >();
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getPixel( rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                                      const geometry::IntegerPoint2D&   /*pos*/ )
    {
        return uno::Sequence< sal_Int8 >();
    }

    namespace
    {
        class CairoColorSpace : public cppu::WeakImplHelper< css::rendering::XIntegerBitmapColorSpace >
        {
        private:
            uno::Sequence< sal_Int8 >  maComponentTags;
            uno::Sequence< sal_Int32 > maBitCounts;

            virtual ::sal_Int8 SAL_CALL getType(  ) throw (uno::RuntimeException, std::exception) override
            {
                return rendering::ColorSpaceType::RGB;
            }
            virtual uno::Sequence< ::sal_Int8 > SAL_CALL getComponentTags(  ) throw (uno::RuntimeException, std::exception) override
            {
                return maComponentTags;
            }
            virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (uno::RuntimeException, std::exception) override
            {
                return rendering::RenderingIntent::PERCEPTUAL;
            }
            virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) throw (uno::RuntimeException, std::exception) override
            {
                return uno::Sequence< beans::PropertyValue >();
            }
            virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >& deviceColor,
                                                                        const uno::Reference< rendering::XColorSpace >& targetColorSpace ) throw (lang::IllegalArgumentException,
                                                                                                                                                  uno::RuntimeException, std::exception) override
            {
                // TODO(P3): if we know anything about target
                // colorspace, this can be greatly sped up
                uno::Sequence<rendering::ARGBColor> aIntermediate(
                    convertToARGB(deviceColor));
                return targetColorSpace->convertFromARGB(aIntermediate);
            }
            virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const double*  pIn( deviceColor.getConstArray() );
                const sal_Size nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                rendering::RGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    const double fAlpha(pIn[3]);
                    if( fAlpha == 0.0 )
                        *pOut++ = rendering::RGBColor(0.0, 0.0, 0.0);
                    else
                        *pOut++ = rendering::RGBColor(pIn[2]/fAlpha,pIn[1]/fAlpha,pIn[0]/fAlpha);
                    pIn += 4;
                }
                return aRes;
            }
            virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const double*  pIn( deviceColor.getConstArray() );
                const sal_Size nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                rendering::ARGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    const double fAlpha(pIn[3]);
                    if( fAlpha == 0.0 )
                        *pOut++ = rendering::ARGBColor(0.0, 0.0, 0.0, 0.0);
                    else
                        *pOut++ = rendering::ARGBColor(fAlpha,pIn[2]/fAlpha,pIn[1]/fAlpha,pIn[0]/fAlpha);
                    pIn += 4;
                }
                return aRes;
            }
            virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const double*  pIn( deviceColor.getConstArray() );
                const sal_Size nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                rendering::ARGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    *pOut++ = rendering::ARGBColor(pIn[3],pIn[2],pIn[1],pIn[1]);
                    pIn += 4;
                }
                return aRes;
            }
            virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size             nLen( rgbColor.getLength() );

                uno::Sequence< double > aRes(nLen*4);
                double* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = pIn->Blue;
                    *pColors++ = pIn->Green;
                    *pColors++ = pIn->Red;
                    *pColors++ = 1.0;
                    ++pIn;
                }
                return aRes;
            }
            virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size              nLen( rgbColor.getLength() );

                uno::Sequence< double > aRes(nLen*4);
                double* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = pIn->Alpha*pIn->Blue;
                    *pColors++ = pIn->Alpha*pIn->Green;
                    *pColors++ = pIn->Alpha*pIn->Red;
                    *pColors++ = pIn->Alpha;
                    ++pIn;
                }
                return aRes;
            }
            virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size              nLen( rgbColor.getLength() );

                uno::Sequence< double > aRes(nLen*4);
                double* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = pIn->Blue;
                    *pColors++ = pIn->Green;
                    *pColors++ = pIn->Red;
                    *pColors++ = pIn->Alpha;
                    ++pIn;
                }
                return aRes;
            }

            // XIntegerBitmapColorSpace
            virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) throw (uno::RuntimeException, std::exception) override
            {
                return 32;
            }
            virtual uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) throw (uno::RuntimeException, std::exception) override
            {
                return maBitCounts;
            }
            virtual ::sal_Int8 SAL_CALL getEndianness(  ) throw (uno::RuntimeException, std::exception) override
            {
                return util::Endianness::LITTLE;
            }
            virtual uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                 const uno::Reference< rendering::XColorSpace >& targetColorSpace )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                if( dynamic_cast<CairoColorSpace*>(targetColorSpace.get()) )
                {
                    const sal_Int8* pIn( deviceColor.getConstArray() );
                    const sal_Size  nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence<double> aRes(nLen);
                    double* pOut( aRes.getArray() );
                    for( sal_Size i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                        *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                        *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                        *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                    }
                    return aRes;
                }
                else
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertIntegerToARGB(deviceColor));
                    return targetColorSpace->convertFromARGB(aIntermediate);
                }
            }
            virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                     const uno::Reference< rendering::XIntegerBitmapColorSpace >& targetColorSpace )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                if( dynamic_cast<CairoColorSpace*>(targetColorSpace.get()) )
                {
                    // it's us, so simply pass-through the data
                    return deviceColor;
                }
                else
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertIntegerToARGB(deviceColor));
                    return targetColorSpace->convertIntegerFromARGB(aIntermediate);
                }
            }
            virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertIntegerToRGB( const uno::Sequence< ::sal_Int8 >& deviceColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const sal_Int8* pIn( deviceColor.getConstArray() );
                const sal_Size  nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                rendering::RGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    const double fAlpha((sal_uInt8)pIn[3]);
                    if( fAlpha )
                        *pOut++ = rendering::RGBColor(
                            pIn[2]/fAlpha,
                            pIn[1]/fAlpha,
                            pIn[0]/fAlpha);
                    else
                        *pOut++ = rendering::RGBColor(0,0,0);
                    pIn += 4;
                }
                return aRes;
            }

            virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const uno::Sequence< ::sal_Int8 >& deviceColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const sal_Int8* pIn( deviceColor.getConstArray() );
                const sal_Size  nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                rendering::ARGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    const double fAlpha((sal_uInt8)pIn[3]);
                    if( fAlpha )
                        *pOut++ = rendering::ARGBColor(
                            fAlpha/255.0,
                            pIn[2]/fAlpha,
                            pIn[1]/fAlpha,
                            pIn[0]/fAlpha);
                    else
                        *pOut++ = rendering::ARGBColor(0,0,0,0);
                    pIn += 4;
                }
                return aRes;
            }
            virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const uno::Sequence< ::sal_Int8 >& deviceColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const sal_Int8* pIn( deviceColor.getConstArray() );
                const sal_Size  nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                rendering::ARGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    *pOut++ = rendering::ARGBColor(
                        vcl::unotools::toDoubleColor(pIn[3]),
                        vcl::unotools::toDoubleColor(pIn[2]),
                        vcl::unotools::toDoubleColor(pIn[1]),
                        vcl::unotools::toDoubleColor(pIn[0]));
                    pIn += 4;
                }
                return aRes;
            }

            virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size             nLen( rgbColor.getLength() );

                uno::Sequence< sal_Int8 > aRes(nLen*4);
                sal_Int8* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                    *pColors++ = -1;
                    ++pIn;
                }
                return aRes;
            }

            virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size              nLen( rgbColor.getLength() );

                uno::Sequence< sal_Int8 > aRes(nLen*4);
                sal_Int8* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    const double fAlpha(pIn->Alpha);
                    *pColors++ = vcl::unotools::toByteColor(fAlpha*pIn->Blue);
                    *pColors++ = vcl::unotools::toByteColor(fAlpha*pIn->Green);
                    *pColors++ = vcl::unotools::toByteColor(fAlpha*pIn->Red);
                    *pColors++ = vcl::unotools::toByteColor(fAlpha);
                    ++pIn;
                }
                return aRes;
            }
            virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size              nLen( rgbColor.getLength() );

                uno::Sequence< sal_Int8 > aRes(nLen*4);
                sal_Int8* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Alpha);
                    ++pIn;
                }
                return aRes;
            }

        public:
            CairoColorSpace() :
                maComponentTags(4),
                maBitCounts(4)
            {
                sal_Int8*  pTags = maComponentTags.getArray();
                sal_Int32* pBitCounts = maBitCounts.getArray();
                pTags[0] = rendering::ColorComponentTag::RGB_BLUE;
                pTags[1] = rendering::ColorComponentTag::RGB_GREEN;
                pTags[2] = rendering::ColorComponentTag::RGB_RED;
                pTags[3] = rendering::ColorComponentTag::PREMULTIPLIED_ALPHA;

                pBitCounts[0] =
                    pBitCounts[1] =
                    pBitCounts[2] =
                    pBitCounts[3] = 8;
            }
        };

        class CairoNoAlphaColorSpace : public cppu::WeakImplHelper< css::rendering::XIntegerBitmapColorSpace >
        {
        private:
            uno::Sequence< sal_Int8 >  maComponentTags;
            uno::Sequence< sal_Int32 > maBitCounts;

            virtual ::sal_Int8 SAL_CALL getType(  ) throw (uno::RuntimeException, std::exception) override
            {
                return rendering::ColorSpaceType::RGB;
            }
            virtual uno::Sequence< ::sal_Int8 > SAL_CALL getComponentTags(  ) throw (uno::RuntimeException, std::exception) override
            {
                return maComponentTags;
            }
            virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (uno::RuntimeException, std::exception) override
            {
                return rendering::RenderingIntent::PERCEPTUAL;
            }
            virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) throw (uno::RuntimeException, std::exception) override
            {
                return uno::Sequence< beans::PropertyValue >();
            }
            virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >& deviceColor,
                                                                        const uno::Reference< rendering::XColorSpace >& targetColorSpace ) throw (lang::IllegalArgumentException,
                                                                                                                                                  uno::RuntimeException, std::exception) override
            {
                // TODO(P3): if we know anything about target
                // colorspace, this can be greatly sped up
                uno::Sequence<rendering::ARGBColor> aIntermediate(
                    convertToARGB(deviceColor));
                return targetColorSpace->convertFromARGB(aIntermediate);
            }
            virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const double*  pIn( deviceColor.getConstArray() );
                const sal_Size nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                rendering::RGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    *pOut++ = rendering::RGBColor(pIn[2], pIn[1], pIn[0]);
                    pIn += 4;
                }
                return aRes;
            }
            uno::Sequence< rendering::ARGBColor > impl_convertToARGB( const uno::Sequence< double >& deviceColor )
            {
                const double*  pIn( deviceColor.getConstArray() );
                const sal_Size nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                rendering::ARGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    *pOut++ = rendering::ARGBColor(1.0, pIn[2], pIn[1], pIn[0]);
                    pIn += 4;
                }
                return aRes;
            }
            virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                return impl_convertToARGB( deviceColor );
            }
            virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                return impl_convertToARGB( deviceColor );
            }
            virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size             nLen( rgbColor.getLength() );

                uno::Sequence< double > aRes(nLen*4);
                double* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = pIn->Blue;
                    *pColors++ = pIn->Green;
                    *pColors++ = pIn->Red;
                    *pColors++ = 1.0; // the value does not matter
                    ++pIn;
                }
                return aRes;
            }
            uno::Sequence< double > impl_convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor )
            {
                const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size              nLen( rgbColor.getLength() );

                uno::Sequence< double > aRes(nLen*4);
                double* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = pIn->Blue;
                    *pColors++ = pIn->Green;
                    *pColors++ = pIn->Red;
                    *pColors++ = 1.0; // the value does not matter
                    ++pIn;
                }
                return aRes;
            }
            virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                return impl_convertFromARGB( rgbColor );
            }
            virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                return impl_convertFromARGB( rgbColor );
            }

            // XIntegerBitmapColorSpace
            virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) throw (uno::RuntimeException, std::exception) override
            {
                return 32;
            }
            virtual uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) throw (uno::RuntimeException, std::exception) override
            {
                return maBitCounts;
            }
            virtual ::sal_Int8 SAL_CALL getEndianness(  ) throw (uno::RuntimeException, std::exception) override
            {
                return util::Endianness::LITTLE;
            }
            virtual uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                 const uno::Reference< rendering::XColorSpace >& targetColorSpace )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                if( dynamic_cast<CairoColorSpace*>(targetColorSpace.get()) )
                {
                    const sal_Int8* pIn( deviceColor.getConstArray() );
                    const sal_Size  nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence<double> aRes(nLen);
                    double* pOut( aRes.getArray() );
                    for( sal_Size i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                        *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                        *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                        *pOut++ = 1.0; // the value does not matter
                    }
                    return aRes;
                }
                else
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertIntegerToARGB(deviceColor));
                    return targetColorSpace->convertFromARGB(aIntermediate);
                }
            }
            virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                     const uno::Reference< rendering::XIntegerBitmapColorSpace >& targetColorSpace )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                if( dynamic_cast<CairoNoAlphaColorSpace*>(targetColorSpace.get()) )
                {
                    // it's us, so simply pass-through the data
                    return deviceColor;
                }
                else
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertIntegerToARGB(deviceColor));
                    return targetColorSpace->convertIntegerFromARGB(aIntermediate);
                }
            }
            virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertIntegerToRGB( const uno::Sequence< ::sal_Int8 >& deviceColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const sal_Int8* pIn( deviceColor.getConstArray() );
                const sal_Size  nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                rendering::RGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    *pOut++ = rendering::RGBColor( pIn[2], pIn[1], pIn[0] );
                    pIn += 4;
                }
                return aRes;
            }

            virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const uno::Sequence< ::sal_Int8 >& deviceColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                return impl_convertIntegerToARGB( deviceColor );
            }
            virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const uno::Sequence< ::sal_Int8 >& deviceColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                return impl_convertIntegerToARGB( deviceColor );
            }
            uno::Sequence< rendering::ARGBColor > impl_convertIntegerToARGB( const uno::Sequence< ::sal_Int8 >& deviceColor )
            {
                const sal_Int8* pIn( deviceColor.getConstArray() );
                const sal_Size  nLen( deviceColor.getLength() );
                ENSURE_ARG_OR_THROW2(nLen%4==0,
                                     "number of channels no multiple of 4",
                                     static_cast<rendering::XColorSpace*>(this), 0);

                uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                rendering::ARGBColor* pOut( aRes.getArray() );
                for( sal_Size i=0; i<nLen; i+=4 )
                {
                    *pOut++ = rendering::ARGBColor(
                        1.0,
                        vcl::unotools::toDoubleColor(pIn[2]),
                        vcl::unotools::toDoubleColor(pIn[1]),
                        vcl::unotools::toDoubleColor(pIn[0]));
                    pIn += 4;
                }
                return aRes;
            }

            virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size             nLen( rgbColor.getLength() );

                uno::Sequence< sal_Int8 > aRes(nLen*4);
                sal_Int8* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                    *pColors++ = -1; // the value does not matter
                    ++pIn;
                }
                return aRes;
            }

            virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                return impl_convertIntegerFromARGB( rgbColor );
            }
            virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor )
                throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception) override
            {
                return impl_convertIntegerFromARGB( rgbColor );
            }
            uno::Sequence< ::sal_Int8 > impl_convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor )
            {
                const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                const sal_Size              nLen( rgbColor.getLength() );

                uno::Sequence< sal_Int8 > aRes(nLen*4);
                sal_Int8* pColors=aRes.getArray();
                for( sal_Size i=0; i<nLen; ++i )
                {
                    *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                    *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                    *pColors++ = -1; // the value does not matter
                    ++pIn;
                }
                return aRes;
            }

        public:
            CairoNoAlphaColorSpace() :
                maComponentTags(3),
                maBitCounts(3)
            {
                sal_Int8*  pTags = maComponentTags.getArray();
                sal_Int32* pBitCounts = maBitCounts.getArray();
                pTags[0] = rendering::ColorComponentTag::RGB_BLUE;
                pTags[1] = rendering::ColorComponentTag::RGB_GREEN;
                pTags[2] = rendering::ColorComponentTag::RGB_RED;

                pBitCounts[0] =
                    pBitCounts[1] =
                    pBitCounts[2] = 8;
            }
        };

        struct CairoNoAlphaColorSpaceHolder : public rtl::StaticWithInit<uno::Reference<rendering::XIntegerBitmapColorSpace>,
                                                                     CairoNoAlphaColorSpaceHolder>
        {
            uno::Reference<rendering::XIntegerBitmapColorSpace> operator()()
            {
                return new CairoNoAlphaColorSpace();
            }
        };

        struct CairoColorSpaceHolder : public rtl::StaticWithInit<uno::Reference<rendering::XIntegerBitmapColorSpace>,
                                                                     CairoColorSpaceHolder>
        {
            uno::Reference<rendering::XIntegerBitmapColorSpace> operator()()
            {
                return new CairoColorSpace();
            }
        };

    }

    rendering::IntegerBitmapLayout CanvasHelper::getMemoryLayout()
    {
        if( !mpCairo )
            return rendering::IntegerBitmapLayout(); // we're disposed

        const geometry::IntegerSize2D aSize(getSize());

        return impl_getMemoryLayout( aSize.Width, aSize.Height );
    }

    rendering::IntegerBitmapLayout
    CanvasHelper::impl_getMemoryLayout( const sal_Int32 nWidth, const sal_Int32 nHeight )
    {
        rendering::IntegerBitmapLayout aLayout;

        aLayout.ScanLines = nHeight;
        aLayout.ScanLineBytes = nWidth*4;
        aLayout.ScanLineStride = aLayout.ScanLineBytes;
        aLayout.PlaneStride = 0;
        aLayout.ColorSpace = mbHaveAlpha ? CairoColorSpaceHolder::get() : CairoNoAlphaColorSpaceHolder::get();
        aLayout.Palette.clear();
        aLayout.IsMsbFirst = false;

        return aLayout;
    }


    bool CanvasHelper::repaint( const SurfaceSharedPtr&          pSurface,
                                const rendering::ViewState&      viewState,
                                const rendering::RenderState&    renderState )
    {
        SAL_INFO( "canvas.cairo", "CanvasHelper::repaint");

        if( mpCairo )
        {
            cairo_save( mpCairo.get() );

            cairo_rectangle( mpCairo.get(), 0, 0, maSize.getX(), maSize.getY() );
            cairo_clip( mpCairo.get() );

            useStates( viewState, renderState, true );

            cairo_matrix_t aMatrix;

            cairo_get_matrix( mpCairo.get(), &aMatrix );
            aMatrix.xx = aMatrix.yy = 1;
            cairo_set_matrix( mpCairo.get(), &aMatrix );

            cairo_set_source_surface( mpCairo.get(), pSurface->getCairoSurface().get(), 0, 0 );
            cairo_paint( mpCairo.get() );
            cairo_restore( mpCairo.get() );
        }

        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
