/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_canvashelper.cxx,v $
 * $Revision: 1.15 $
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

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <com/sun/star/rendering/IntegerBitmapFormat.hpp>
#include <com/sun/star/rendering/Endianness.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <comphelper/sequence.hxx>

#include <canvas/canvastools.hxx>
#include <canvas/parametricpolypolygon.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>

#include "cairo_spritecanvas.hxx"
#include "cairo_cachedbitmap.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_canvasbitmap.hxx"

#include <algorithm>

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CanvasHelper::CanvasHelper() :
        mpDevice( NULL ),
        mpVirtualDevice( NULL ),
        mbHaveAlpha(),
        mpCairo( NULL ),
        mpSurface( NULL ),
        mpSurfaceProvider( NULL )
    {
    }

    void CanvasHelper::disposing()
    {
        mpDevice = NULL;

        if( mpVirtualDevice ) {
            delete mpVirtualDevice;
            mpVirtualDevice = NULL;
        }

        if( mpCairo )
        {
            cairo_destroy( mpCairo );
            mpCairo = NULL;
        }

        if( mpSurface ) {
            mpSurface->Unref();
            mpSurface = NULL;
        }
    }

    void CanvasHelper::init( const ::basegfx::B2ISize& rSize,
                             SpriteCanvas&             rDevice )
    {
        maSize = rSize;
        mpDevice = &rDevice;
    }

    void CanvasHelper::setSize( const ::basegfx::B2ISize& rSize )
    {
        maSize = rSize;
    }

    void CanvasHelper::setSurface( Surface* pSurface, bool bHasAlpha, SurfaceProvider* pSurfaceProvider )
    {
        mbHaveAlpha = bHasAlpha;
        mpSurfaceProvider = pSurfaceProvider;

        if( mpSurface ) {
            mpSurface->Unref();
        }

        if( mpVirtualDevice ) {
            delete mpVirtualDevice;
            mpVirtualDevice = NULL;
        }

        mpSurface = pSurface;
        mpSurface->Ref();

        if( mpCairo )
        {
            cairo_destroy( mpCairo );
        }

        mpCairo = pSurface->getCairo();
    }

    void CanvasHelper::useStates( const rendering::ViewState& viewState,
                  const rendering::RenderState& renderState,
                  bool setColor )
    {
    Matrix aViewMatrix;
    Matrix aRenderMatrix;
    Matrix aCombinedMatrix;

    cairo_matrix_init( &aViewMatrix,
                    viewState.AffineTransform.m00, viewState.AffineTransform.m10, viewState.AffineTransform.m01,
                    viewState.AffineTransform.m11, viewState.AffineTransform.m02, viewState.AffineTransform.m12);
    cairo_matrix_init( &aRenderMatrix,
                    renderState.AffineTransform.m00, renderState.AffineTransform.m10, renderState.AffineTransform.m01,
                    renderState.AffineTransform.m11, renderState.AffineTransform.m02, renderState.AffineTransform.m12);
    cairo_matrix_multiply( &aCombinedMatrix, &aRenderMatrix, &aViewMatrix);

        if( viewState.Clip.is() ) {
        OSL_TRACE ("view clip\n");

        aViewMatrix.x0 = basegfx::fround( aViewMatrix.x0 );
        aViewMatrix.y0 = basegfx::fround( aViewMatrix.y0 );
        cairo_set_matrix( mpCairo, &aViewMatrix );
        doPolyPolygonPath( viewState.Clip, Clip );
    }

    aCombinedMatrix.x0 = basegfx::fround( aCombinedMatrix.x0 );
    aCombinedMatrix.y0 = basegfx::fround( aCombinedMatrix.y0 );
    cairo_set_matrix( mpCairo, &aCombinedMatrix );

        if( renderState.Clip.is() ) {
        OSL_TRACE ("render clip BEGIN\n");

        doPolyPolygonPath( renderState.Clip, Clip );
        OSL_TRACE ("render clip END\n");
    }

    if( setColor ) {
        if( renderState.DeviceColor.getLength() > 3 )
        cairo_set_source_rgba( mpCairo,
                       renderState.DeviceColor [0],
                       renderState.DeviceColor [1],
                       renderState.DeviceColor [2],
                       renderState.DeviceColor [3] );
        else if (renderState.DeviceColor.getLength() == 3)
        cairo_set_source_rgb( mpCairo,
                      renderState.DeviceColor [0],
                      renderState.DeviceColor [1],
                      renderState.DeviceColor [2] );
    }

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
    cairo_set_operator( mpCairo, compositingMode );

    }

    void CanvasHelper::clear()
    {
        OSL_TRACE ("clear whole area: %d x %d\n", maSize.getX(), maSize.getY() );

        if( mpCairo )
        {
            cairo_save( mpCairo );

            cairo_identity_matrix( mpCairo );
            cairo_set_operator( mpCairo, CAIRO_OPERATOR_SOURCE );
            cairo_set_source_rgba( mpCairo, 0, 0, 0, 0 );

            cairo_rectangle( mpCairo, 0, 0, maSize.getX(), maSize.getY() );
            cairo_fill( mpCairo );

            cairo_restore( mpCairo );
        }
    }

    void CanvasHelper::drawPoint( const rendering::XCanvas*     /*pCanvas*/,
                                  const geometry::RealPoint2D&  /*aPoint*/,
                                  const rendering::ViewState&   /*viewState*/,
                                  const rendering::RenderState& /*renderState*/ )
    {
    }

    void CanvasHelper::drawLine( const rendering::XCanvas*      /*pCanvas*/,
                                 const geometry::RealPoint2D&   aStartPoint,
                                 const geometry::RealPoint2D&   aEndPoint,
                                 const rendering::ViewState&    viewState,
                                 const rendering::RenderState&  renderState )
    {
    if( mpCairo ) {
        cairo_save( mpCairo );

        cairo_set_line_width( mpCairo, 1 );

        useStates( viewState, renderState, true );

        cairo_move_to( mpCairo, aStartPoint.X + 0.5, aStartPoint.Y + 0.5 );
        cairo_line_to( mpCairo, aEndPoint.X + 0.5, aEndPoint.Y + 0.5 );
        cairo_stroke( mpCairo );

        cairo_restore( mpCairo );
    }
    }

    void CanvasHelper::drawBezier( const rendering::XCanvas*            /*pCanvas*/,
                                   const geometry::RealBezierSegment2D& aBezierSegment,
                                   const geometry::RealPoint2D&         aEndPoint,
                                   const rendering::ViewState&          viewState,
                                   const rendering::RenderState&        renderState )
    {
    if( mpCairo ) {
        cairo_save( mpCairo );

        cairo_set_line_width( mpCairo, 1 );

        useStates( viewState, renderState, true );

        cairo_move_to( mpCairo, aBezierSegment.Px + 0.5, aBezierSegment.Py + 0.5 );
        cairo_curve_to( mpCairo,
                        aBezierSegment.C1x + 0.5, aBezierSegment.C1y + 0.5,
                        aBezierSegment.C2x + 0.5, aBezierSegment.C2y + 0.5,
                        aEndPoint.X + 0.5, aEndPoint.Y + 0.5 );
        cairo_stroke( mpCairo );

        cairo_restore( mpCairo );
    }
    }

#define CANVASBITMAP_IMPLEMENTATION_NAME "CairoCanvas::CanvasBitmap"
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
    static Surface* surfaceFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap, bool &bHasAlpha )
    {
        CanvasBitmap* pBitmapImpl = dynamic_cast< CanvasBitmap* >( xBitmap.get() );

        if( pBitmapImpl ) {
            // TODO(Q1): Maybe use dynamic_cast here
            bHasAlpha = pBitmapImpl->hasAlpha();

            return pBitmapImpl->getSurface();
        }


                SpriteCanvas* pCanvasImpl = dynamic_cast< SpriteCanvas* >( xBitmap.get() );
                if( pCanvasImpl && pCanvasImpl->getBufferSurface () )
                {
                    bHasAlpha = false;

                    return pCanvasImpl->getBackgroundSurface();
                }

        return NULL;
    }

    static ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap )
    {
    uno::Reference< lang::XUnoTunnel > xTunnel( xBitmap, uno::UNO_QUERY );
    if( xTunnel.is() )
            {
        sal_Int64 nPtr = xTunnel->getSomething( vcl::unotools::getTunnelIdentifier( vcl::unotools::Id_BitmapEx ) );
        if( nPtr )
            return BitmapEx( *reinterpret_cast<BitmapEx*>(reinterpret_cast<void*>(nPtr)) );
        }

    // TODO(F1): extract pixel from XBitmap interface
    ENSURE_AND_THROW( false,
              "bitmapExFromXBitmap(): could not extract BitmapEx" );

    return ::BitmapEx();
    }

    static bool readAlpha( BitmapReadAccess* pAlphaReadAcc, long nY, const long nWidth, unsigned char* data, long nOff )
    {
    bool bIsAlpha = false;
    long nX;
    int nAlpha;
    Scanline pReadScan;

    nOff += 3;

    switch( pAlphaReadAcc->GetScanlineFormat() ) {
    case BMP_FORMAT_8BIT_TC_MASK:
        pReadScan = pAlphaReadAcc->GetScanline( nY );
        for( nX = 0; nX < nWidth; nX++ ) {
        nAlpha = data[ nOff ] = 255 - ( *pReadScan++ );
        if( nAlpha != 255 )
            bIsAlpha = true;
        nOff += 4;
        }
        break;
    case BMP_FORMAT_8BIT_PAL:
        pReadScan = pAlphaReadAcc->GetScanline( nY );
        for( nX = 0; nX < nWidth; nX++ ) {
        nAlpha = data[ nOff ] = 255 - ( pAlphaReadAcc->GetPaletteColor( *pReadScan++ ).GetBlue() );
        if( nAlpha != 255 )
            bIsAlpha = true;
        nOff += 4;
        }
        break;
    default:
        OSL_TRACE( "fallback to GetColor for alpha - slow, format: %d\n", pAlphaReadAcc->GetScanlineFormat() );
        for( nX = 0; nX < nWidth; nX++ ) {
        nAlpha = data[ nOff ] = 255 - pAlphaReadAcc->GetColor( nY, nX ).GetBlue();
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
    static Surface* surfaceFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap, const DeviceRef& rDevice, unsigned char*& data, bool& bHasAlpha )
    {
        Surface* pSurface = surfaceFromXBitmap( xBitmap, bHasAlpha );
        if( pSurface ) {
            pSurface->Ref();
            data = NULL;
        } else {
            BitmapEx aBmpEx = bitmapExFromXBitmap(xBitmap);
            Bitmap aBitmap = aBmpEx.GetBitmap();

            // there's no pixmap for alpha bitmap. we might still
            // use rgb pixmap and only access alpha pixels the
            // slow way. now we just speedup rgb bitmaps
            if( !aBmpEx.IsTransparent() && !aBmpEx.IsAlpha() ) {
                pSurface = rDevice->getSurface( aBitmap );
                data = NULL;
                bHasAlpha = false;
            }

            if( !pSurface ) {
                AlphaMask aAlpha = aBmpEx.GetAlpha();

                BitmapReadAccess*   pBitmapReadAcc = aBitmap.AcquireReadAccess();
                BitmapReadAccess*   pAlphaReadAcc = NULL;
                const long      nWidth = pBitmapReadAcc->Width();
                const long      nHeight = pBitmapReadAcc->Height();
                long nX, nY;
                bool bIsAlpha = false;

                if( aBmpEx.IsTransparent() || aBmpEx.IsAlpha() )
                    pAlphaReadAcc = aAlpha.AcquireReadAccess();

                data = (unsigned char*) malloc( nWidth*nHeight*4 );

                long nOff = 0;
                Color aColor;
                unsigned int nAlpha = 255;

                for( nY = 0; nY < nHeight; nY++ ) {
                    Scanline pReadScan;

                    switch( pBitmapReadAcc->GetScanlineFormat() ) {
                    case BMP_FORMAT_8BIT_PAL:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ ) {
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
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetRed() ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetGreen() ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetBlue() ) )/255);
#else
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetBlue() ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetGreen() ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( aColor.GetRed() ) )/255);
                            nOff++;
#endif
                        }
                        break;
                    case BMP_FORMAT_24BIT_TC_BGR:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ ) {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff ];
                            else
                                nAlpha = data[ nOff ] = 255;
                            data[ nOff + 3 ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff + 2 ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff + 1 ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            nOff += 4;
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            nOff++;
#endif
                        }
                        break;
                    case BMP_FORMAT_24BIT_TC_RGB:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ ) {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff++ ];
                            else
                                nAlpha = data[ nOff++ ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 2 ] ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 1 ] ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 0 ] ) )/255);
                            pReadScan += 3;
                            nOff++;
#endif
                        }
                        break;
                    case BMP_FORMAT_32BIT_TC_BGRA:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ ) {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff++ ];
                            else
                                nAlpha = data[ nOff++ ] = pReadScan[ 3 ];
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 2 ] ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 1 ] ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 0 ] ) )/255);
                            pReadScan += 4;
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = pReadScan[ 3 ];
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            pReadScan++;
                            nOff++;
#endif
                        }
                        break;
                    case BMP_FORMAT_32BIT_TC_RGBA:
                        pReadScan = pBitmapReadAcc->GetScanline( nY );
                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ ) {
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff ++ ];
                            else
                                nAlpha = data[ nOff ++ ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( *pReadScan++ ) )/255);
                            pReadScan++;
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 2 ] ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 1 ] ) )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*( pReadScan[ 0 ] ) )/255);
                            pReadScan += 4;
                            nOff++;
#endif
                        }
                        break;
                    default:
                        OSL_TRACE( "fallback to GetColor - slow, format: %d\n", pBitmapReadAcc->GetScanlineFormat() );

                        if( pAlphaReadAcc )
                            if( readAlpha( pAlphaReadAcc, nY, nWidth, data, nOff ) )
                                bIsAlpha = true;

                        for( nX = 0; nX < nWidth; nX++ ) {
                            aColor = pBitmapReadAcc->GetColor( nY, nX );

                            // cairo need premultiplied color values
                            // TODO(rodo) handle endianess
#ifdef OSL_BIGENDIAN
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff++ ];
                            else
                                nAlpha = data[ nOff++ ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetRed() )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetGreen() )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetBlue() )/255);
#else
                            if( pAlphaReadAcc )
                                nAlpha = data[ nOff + 3 ];
                            else
                                nAlpha = data[ nOff + 3 ] = 255;
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetBlue() )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetGreen() )/255);
                            data[ nOff++ ] = sal::static_int_cast<unsigned char>(( nAlpha*aColor.GetRed() )/255);
                            nOff ++;
#endif
                        }
                    }
                }

                aBitmap.ReleaseAccess( pBitmapReadAcc );
                if( pAlphaReadAcc )
                    aAlpha.ReleaseAccess( pAlphaReadAcc );

                Surface* pImageSurface = new Surface( cairo_image_surface_create_for_data( data,
                                                                                           bIsAlpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24,
                                                                                           nWidth, nHeight, nWidth*4 ) );

                //      pSurface = rDevice->getSurface( ::basegfx::B2ISize( nWidth, nHeight ), bIsAlpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR );
                //      Cairo* pTargetCairo = cairo_create( pSurface );
                //      cairo_set_source_surface( pTargetCairo, pImageSurface, 0, 0 );

                //                  //if( !bIsAlpha )
                //                  //cairo_set_operator( pTargetCairo, CAIRO_OPERATOR_SOURCE );

                //      cairo_paint( pTargetCairo );
                //      cairo_destroy( pTargetCairo );
                //      cairo_surface_destroy( pImageSurface );
                pSurface = pImageSurface;

                bHasAlpha = bIsAlpha;

                OSL_TRACE("image: %d x %d alpha: %d alphaRead %p", nWidth, nHeight, bIsAlpha, pAlphaReadAcc);
            }
        }

        return pSurface;
    }

    static void addColorStop( Pattern* pPattern, double nOffset, const uno::Sequence< double >& rColor )
    {
    if( rColor.getLength() == 3 )
        cairo_pattern_add_color_stop_rgb( pPattern, nOffset, rColor[0], rColor[1], rColor[2] );
    else if( rColor.getLength() == 4 )
        cairo_pattern_add_color_stop_rgba( pPattern, nOffset, rColor[0], rColor[1], rColor[2], rColor[3] );
    }

    static Pattern* patternFromParametricPolyPolygon( ::canvas::ParametricPolyPolygon& rPolygon, Matrix& rMatrix )
    {
    Pattern* pPattern = NULL;
    const ::canvas::ParametricPolyPolygon::Values aValues = rPolygon.getValues();
    double x0, x1, y0, y1, cx, cy, r0, r1;

// undef macros from vclenum.hxx which conflicts with GradientType enum values
#undef GRADIENT_LINEAR
#undef GRADIENT_AXIAL
#undef GRADIENT_ELLIPTICAL

    switch( aValues.meType ) {
    case ::canvas::ParametricPolyPolygon::GRADIENT_LINEAR:
        x0 = 0;
        y0 = 0;
        x1 = 1;
        y1 = 0;
        cairo_matrix_transform_point( &rMatrix, &x0, &y0 );
        cairo_matrix_transform_point( &rMatrix, &x1, &y1 );
        pPattern = cairo_pattern_create_linear( x0, y0, x1, y1 );
        addColorStop( pPattern, 0, aValues.maColor1 );
        addColorStop( pPattern, 1, aValues.maColor2 );
        break;

    // FIXME: NYI
    case ::canvas::ParametricPolyPolygon::GRADIENT_RECTANGULAR:
    case ::canvas::ParametricPolyPolygon::GRADIENT_AXIAL:
        x0 = 0;
        y0 = 0;
        x1 = 1;
        y1 = 0;
        cairo_matrix_transform_point( &rMatrix, &x0, &y0 );
        cairo_matrix_transform_point( &rMatrix, &x1, &y1 );
        pPattern = cairo_pattern_create_linear( x0, y0, x1, y1 );
        addColorStop( pPattern, 0, aValues.maColor1 );
        addColorStop( pPattern, 0.5, aValues.maColor2 );
        addColorStop( pPattern, 1, aValues.maColor1 );
        break;

    case ::canvas::ParametricPolyPolygon::GRADIENT_ELLIPTICAL:
        cx = 0.5;
        cy = 0.5;
        r0 = 0;
        r1 = 0.5;
        Matrix aScaleMatrix;

        cairo_matrix_transform_point( &rMatrix, &cx, &cy );
        cairo_matrix_transform_distance( &rMatrix, &r0, &r1 );
        pPattern = cairo_pattern_create_radial( cx, cy, r0, cx, cy, r1 );
        addColorStop( pPattern, 0, aValues.maColor1 );
        addColorStop( pPattern, 1, aValues.maColor2 );

        if( ! ::rtl::math::approxEqual( aValues.mnAspectRatio, 1 ) ) {
        cairo_matrix_init_scale( &aScaleMatrix, 1, aValues.mnAspectRatio );
        cairo_pattern_set_matrix( pPattern, &aScaleMatrix );
        }
        break;
    }

    return pPattern;
    }

    void doOperation( Operation aOperation,
                      Cairo* pCairo,
                      sal_uInt32 /*nPolygonIndex*/,
                      const uno::Sequence< rendering::Texture >* pTextures,
                      SpriteCanvas* pDevice )
    {
    switch( aOperation ) {
    case Fill:
        /* TODO: multitexturing */
        if( pTextures ) {
        const ::com::sun::star::rendering::Texture& aTexture ( (*pTextures)[0] );
        if( aTexture.Bitmap.is() ) {
            unsigned char* data = NULL;
            bool bHasAlpha = 0;
            Surface* pSurface = surfaceFromXBitmap( (*pTextures)[0].Bitmap, pDevice, data, bHasAlpha );

            if( pSurface ) {
            cairo_pattern_t* pPattern;

            cairo_save( pCairo );

            ::com::sun::star::geometry::AffineMatrix2D aTransform( aTexture.AffineTransform );
            Matrix aScaleMatrix, aTextureMatrix, aScaledTextureMatrix;

            cairo_matrix_init( &aTextureMatrix,
                       aTransform.m00, aTransform.m10, aTransform.m01,
                       aTransform.m11, aTransform.m02, aTransform.m12);

            geometry::IntegerSize2D aSize = aTexture.Bitmap->getSize();

            cairo_matrix_init_scale( &aScaleMatrix, 1.0/aSize.Width, 1.0/aSize.Height );
            cairo_matrix_multiply( &aScaledTextureMatrix, &aTextureMatrix, &aScaleMatrix );
            cairo_matrix_invert( &aScaledTextureMatrix );

#if 0
            OSL_TRACE("slow workaround");
            // workaround for X/glitz and/or cairo bug
            // we create big enough temporary surface, copy texture bitmap there and use it for the pattern
            // it only happens on enlargening matrices with REPEAT mode enabled
            Surface* pTmpSurface = pDevice->getSurface();
            Cairo* pTmpCairo = cairo_create( pTmpSurface );
            cairo_set_source_surface( pTmpCairo, pSurface, 0, 0 );
            cairo_paint( pTmpCairo );
            pPattern = cairo_pattern_create_for_surface( pTmpSurface );
#else

            // we don't care about repeat mode yet, so the workaround is disabled for now
            pPattern = cairo_pattern_create_for_surface( pSurface->mpSurface );
#endif
             if( aTexture.RepeatModeX == rendering::TexturingMode::REPEAT &&
                aTexture.RepeatModeY == rendering::TexturingMode::REPEAT )
                cairo_pattern_set_extend( pPattern, CAIRO_EXTEND_REPEAT );
            aScaledTextureMatrix.x0 = basegfx::fround( aScaledTextureMatrix.x0 );
            aScaledTextureMatrix.y0 = basegfx::fround( aScaledTextureMatrix.y0 );
            cairo_pattern_set_matrix( pPattern, &aScaledTextureMatrix );

            cairo_set_source( pCairo, pPattern );
            if( !bHasAlpha )
                cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
            cairo_fill( pCairo );

            cairo_restore( pCairo );

            cairo_pattern_destroy( pPattern );
            pSurface->Unref();

#if 0
            cairo_destroy( pTmpCairo );
            cairo_surface_destroy( pTmpSurface );
#endif
            }

            if( data )
            free( data );
        } else if( aTexture.Gradient.is() ) {
            uno::Reference< lang::XServiceInfo > xRef( aTexture.Gradient, uno::UNO_QUERY );

            OSL_TRACE( "gradient fill\n" );
            if( xRef.is() &&
            xRef->getImplementationName().equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( PARAMETRICPOLYPOLYGON_IMPLEMENTATION_NAME ) ) ) ) {
                // TODO(Q1): Maybe use dynamic_cast here

                // TODO(E1): Return value
                // TODO(F1): FillRule
            OSL_TRACE( "known implementation\n" );

            ::canvas::ParametricPolyPolygon* pPolyImpl = static_cast< ::canvas::ParametricPolyPolygon* >( aTexture.Gradient.get() );
            ::com::sun::star::geometry::AffineMatrix2D aTransform( aTexture.AffineTransform );
            Matrix aTextureMatrix;

            cairo_matrix_init( &aTextureMatrix,
                       aTransform.m00, aTransform.m10, aTransform.m01,
                       aTransform.m11, aTransform.m02, aTransform.m12);
            Pattern* pPattern = patternFromParametricPolyPolygon( *pPolyImpl, aTextureMatrix );

            if( pPattern ) {
                OSL_TRACE( "filling with pattern\n" );

                cairo_save( pCairo );

                cairo_set_source( pCairo, pPattern );
                cairo_fill( pCairo );
                cairo_restore( pCairo );

                cairo_pattern_destroy( pPattern );
            }
            }
        }
        } else
        cairo_fill( pCairo );
        OSL_TRACE("fill");
    break;
    case Stroke:
        cairo_stroke( pCairo );
        OSL_TRACE("stroke\n");
    break;
    case Clip:
        cairo_clip( pCairo );
        OSL_TRACE("clip\n");
    break;
    }
    }

    static void clipNULL( Cairo *pCairo )
    {
    OSL_TRACE("clipNULL\n");
    Matrix aOrigMatrix, aIdentityMatrix;

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

    void doPolyPolygonImplementation( ::basegfx::B2DPolyPolygon aPolyPolygon,
                    Operation aOperation,
                    Cairo* pCairo,
                    const uno::Sequence< rendering::Texture >* pTextures,
                    SpriteCanvas* pDevice,
                    rendering::FillRule eFillrule )
    {
        if( pTextures )
            CHECK_AND_THROW( pTextures->getLength(),
                             "CanvasHelper::fillTexturedPolyPolygon: empty texture sequence");

    bool bOpToDo = false;
    Matrix aOrigMatrix, aIdentityMatrix;
    double nX, nY, nBX, nBY, nAX, nAY;

    cairo_get_matrix( pCairo, &aOrigMatrix );
    cairo_matrix_init_identity( &aIdentityMatrix );
    cairo_set_matrix( pCairo, &aIdentityMatrix );

    cairo_set_fill_rule( pCairo,
                         eFillrule == rendering::FillRule_EVEN_ODD ?
                         CAIRO_FILL_RULE_EVEN_ODD : CAIRO_FILL_RULE_WINDING );

    for( sal_uInt32 nPolygonIndex = 0; nPolygonIndex < aPolyPolygon.count(); nPolygonIndex++ ) {
        ::basegfx::B2DPolygon aPolygon( aPolyPolygon.getB2DPolygon( nPolygonIndex ) );
        const sal_uInt32 nPointCount( aPolygon.count() );

        if( nPointCount > 1) {
        bool bIsBezier = aPolygon.areControlPointsUsed();
        ::basegfx::B2DPoint aA, aB, aP;

        aP = aPolygon.getB2DPoint( 0 );
        nX = aP.getX();
        nY = aP.getY();

        cairo_matrix_transform_point( &aOrigMatrix, &nX, &nY );

        if( ! bIsBezier ) {
            nX = basegfx::fround( nX );
            nY = basegfx::fround( nY );
        }

        if( aOperation == Stroke ) {
            nX += 0.5;
            nY += 0.5;
        }

        cairo_move_to( pCairo, nX, nY );
        OSL_TRACE( "move to %f,%f", nX, nY );

        if( bIsBezier ) {
            aA = aPolygon.getNextControlPoint( 0 );
            aB = aPolygon.getPrevControlPoint( 1 );
        }

        for( sal_uInt32 j = 1; j < nPointCount; j++ ) {
            aP = aPolygon.getB2DPoint( j );

            nX = aP.getX();
            nY = aP.getY();
            cairo_matrix_transform_point( &aOrigMatrix, &nX, &nY );

            if( ! bIsBezier ) {
            nX = basegfx::fround( nX );
            nY = basegfx::fround( nY );
            }

            if( aOperation == Stroke ) {
            nX += 0.5;
            nY += 0.5;
            }

            if( bIsBezier ) {
            nAX = aA.getX();
            nAY = aA.getY();
            nBX = aB.getX();
            nBY = aB.getY();

            if( aOperation == Stroke ) {
                nAX += 0.5;
                nAY += 0.5;
                nBX += 0.5;
                nBY += 0.5;
            }
            cairo_matrix_transform_point( &aOrigMatrix, &nAX, &nAY );
            cairo_matrix_transform_point( &aOrigMatrix, &nBX, &nBY );
            cairo_curve_to( pCairo, nAX, nAY, nBX, nBY, nX, nY );

            aA = aPolygon.getNextControlPoint( j );
            aB = aPolygon.getPrevControlPoint( ( j + 1 ) % nPointCount );
            } else {
            cairo_line_to( pCairo, nX, nY );
            OSL_TRACE( "line to %f,%f", nX, nY );
            }
            bOpToDo = true;
        }

        if( aPolygon.isClosed() )
            cairo_close_path( pCairo );

        if( aOperation == Fill && pTextures ) {
            cairo_set_matrix( pCairo, &aOrigMatrix );
            doOperation( aOperation, pCairo, nPolygonIndex, pTextures, pDevice );
            cairo_set_matrix( pCairo, &aIdentityMatrix );
        }
        } else {
        OSL_TRACE( "empty polygon for op: %d\n\n", aOperation );
        if( aOperation == Clip ) {
            clipNULL( pCairo );

            return;
        }
        }
    }
    if( bOpToDo && ( aOperation != Fill || !pTextures ) )
        doOperation( aOperation, pCairo );

    cairo_set_matrix( pCairo, &aOrigMatrix );

    if( aPolyPolygon.count() == 0 && aOperation == Clip )
         clipNULL( pCairo );
    }

    void CanvasHelper::doPolyPolygonPath( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                        Operation aOperation,
                        const uno::Sequence< rendering::Texture >* pTextures,
                        Cairo* pCairo ) const
    {
    ::basegfx::B2DPolyPolygon aPoly = ::canvas::tools::polyPolygonFromXPolyPolygon2D( xPolyPolygon );

    if( !pCairo )
        pCairo = mpCairo;

    doPolyPolygonImplementation( aPoly, aOperation, pCairo, pTextures, mpDevice, xPolyPolygon->getFillRule() );
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
    #ifdef CAIRO_CANVAS_PERF_TRACE
    struct timespec aTimer;
    mxDevice->startPerfTrace( &aTimer );
        #endif

    if( mpCairo ) {
        cairo_save( mpCairo );

        cairo_set_line_width( mpCairo, 1 );

        useStates( viewState, renderState, true );
        doPolyPolygonPath( xPolyPolygon, Stroke );

        cairo_restore( mpCairo );
    } else
        OSL_TRACE ("CanvasHelper called after it was disposed");

    #ifdef CAIRO_CANVAS_PERF_TRACE
    mxDevice->stopPerfTrace( &aTimer, "drawPolyPolygon" );
        #endif

    return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokePolyPolygon( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   xPolyPolygon,
                                                                                   const rendering::ViewState&                          viewState,
                                                                                   const rendering::RenderState&                        renderState,
                                                                                   const rendering::StrokeAttributes&                   strokeAttributes )
    {
    #ifdef CAIRO_CANVAS_PERF_TRACE
    struct timespec aTimer;
    mxDevice->startPerfTrace( &aTimer );
        #endif

    if( mpCairo ) {
        cairo_save( mpCairo );

        useStates( viewState, renderState, true );

        cairo_set_line_width( mpCairo, strokeAttributes.StrokeWidth );
        cairo_set_miter_limit( mpCairo, strokeAttributes.MiterLimit );

        // FIXME: cairo doesn't handle end cap so far (rodo)
        switch( strokeAttributes.StartCapType ) {
        case rendering::PathCapType::BUTT:
        cairo_set_line_cap( mpCairo, CAIRO_LINE_CAP_BUTT );
        break;
        case rendering::PathCapType::ROUND:
        cairo_set_line_cap( mpCairo, CAIRO_LINE_CAP_ROUND );
        break;
        case rendering::PathCapType::SQUARE:
        cairo_set_line_cap( mpCairo, CAIRO_LINE_CAP_SQUARE );
        break;
        }

        switch( strokeAttributes.JoinType ) {
        // cairo doesn't have join type NONE so we use MITTER as it's pretty close
        case rendering::PathJoinType::NONE:
        case rendering::PathJoinType::MITER:
        cairo_set_line_join( mpCairo, CAIRO_LINE_JOIN_MITER );
        break;
        case rendering::PathJoinType::ROUND:
        cairo_set_line_join( mpCairo, CAIRO_LINE_JOIN_ROUND );
        break;
        case rendering::PathJoinType::BEVEL:
        cairo_set_line_join( mpCairo, CAIRO_LINE_JOIN_BEVEL );
        break;
        }

        if( strokeAttributes.DashArray.getLength() > 0 ) {
        double* pDashArray = new double[ strokeAttributes.DashArray.getLength() ];
        for( sal_Int32 i=0; i<strokeAttributes.DashArray.getLength(); i++ )
            pDashArray[i]=strokeAttributes.DashArray[i];
        cairo_set_dash( mpCairo, pDashArray, strokeAttributes.DashArray.getLength(), 0 );
        delete[] pDashArray;
        }

        // TODO(rodo) use LineArray of strokeAttributes

        doPolyPolygonPath( xPolyPolygon, Stroke );

        cairo_restore( mpCairo );
    } else
        OSL_TRACE ("CanvasHelper called after it was disposed");

    #ifdef CAIRO_CANVAS_PERF_TRACE
    mxDevice->stopPerfTrace( &aTimer, "strokePolyPolygon" );
        #endif

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTexturedPolyPolygon( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                           const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                           const rendering::ViewState&                          /*viewState*/,
                                                                                           const rendering::RenderState&                        /*renderState*/,
                                                                                           const uno::Sequence< rendering::Texture >&           /*textures*/,
                                                                                           const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::strokeTextureMappedPolyPolygon( const rendering::XCanvas*                           /*pCanvas*/,
                                                                                                const uno::Reference< rendering::XPolyPolygon2D >&  /*xPolyPolygon*/,
                                                                                                const rendering::ViewState&                         /*viewState*/,
                                                                                                const rendering::RenderState&                       /*renderState*/,
                                                                                                const uno::Sequence< rendering::Texture >&          /*textures*/,
                                                                                                const uno::Reference< geometry::XMapping2D >&       /*xMapping*/,
                                                                                                const rendering::StrokeAttributes&                  /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XPolyPolygon2D >   CanvasHelper::queryStrokeShapes( const rendering::XCanvas*                            /*pCanvas*/,
                                                                                   const uno::Reference< rendering::XPolyPolygon2D >&   /*xPolyPolygon*/,
                                                                                   const rendering::ViewState&                          /*viewState*/,
                                                                                   const rendering::RenderState&                        /*renderState*/,
                                                                                   const rendering::StrokeAttributes&                   /*strokeAttributes*/ )
    {
        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                 const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                 const rendering::ViewState&                        viewState,
                                                                                 const rendering::RenderState&                      renderState )
    {
    #ifdef CAIRO_CANVAS_PERF_TRACE
    struct timespec aTimer;
    mxDevice->startPerfTrace( &aTimer );
        #endif

    if( mpCairo ) {
        cairo_save( mpCairo );

        useStates( viewState, renderState, true );
        doPolyPolygonPath( xPolyPolygon, Fill );

        cairo_restore( mpCairo );
    } else
        OSL_TRACE ("CanvasHelper called after it was disposed");

    #ifdef CAIRO_CANVAS_PERF_TRACE
    mxDevice->stopPerfTrace( &aTimer, "fillPolyPolygon" );
        #endif

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas*                          /*pCanvas*/,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         const uno::Sequence< rendering::Texture >&         textures )
    {
    if( mpCairo ) {
        cairo_save( mpCairo );

        useStates( viewState, renderState, true );
        doPolyPolygonPath( xPolyPolygon, Fill, &textures );

        cairo_restore( mpCairo );
    }

        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTextureMappedPolyPolygon( const rendering::XCanvas*                             /*pCanvas*/,
                                                                                              const uno::Reference< rendering::XPolyPolygon2D >&    /*xPolyPolygon*/,
                                                                                              const rendering::ViewState&                           /*viewState*/,
                                                                                              const rendering::RenderState&                         /*renderState*/,
                                                                                              const uno::Sequence< rendering::Texture >&            /*textures*/,
                                                                                              const uno::Reference< geometry::XMapping2D >&         /*xMapping*/ )
    {
        // TODO
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::implDrawBitmapSurface( Surface* pSurface,
                                                                                       const rendering::ViewState& viewState,
                                                                                       const rendering::RenderState& renderState,
                                                                                       const geometry::IntegerSize2D& rSize,
                                                                                       bool /*bModulateColors*/,
                                                                                       bool bHasAlpha )
    {
        uno::Reference< rendering::XCachedPrimitive > rv = uno::Reference< rendering::XCachedPrimitive >(NULL);
                geometry::IntegerSize2D aBitmapSize = rSize;

        if( mpCairo ) {
            cairo_save( mpCairo );

            cairo_rectangle( mpCairo, 0, 0, maSize.getX(), maSize.getY() );
            cairo_clip( mpCairo );

            useStates( viewState, renderState, true );

            //          if( !bHasAlpha )
            //          cairo_set_operator( mpCairo, CAIRO_OPERATOR_SOURCE );

            Matrix aMatrix;

            cairo_get_matrix( mpCairo, &aMatrix );
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

                    Surface* pScaledSurface = mpDevice->getSurface( ::basegfx::B2ISize( aBitmapSize.Width, aBitmapSize.Height ),
                                                                                        bHasAlpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR );
                    Cairo* pCairo = pScaledSurface->getCairo();

                    // cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
                    cairo_scale( pCairo, dWidth/rSize.Width, dHeight/rSize.Height );
                    cairo_set_source_surface( pCairo, pSurface->mpSurface, 0, 0 );
                    cairo_paint( pCairo );

                    cairo_destroy( pCairo );

                    pSurface = pScaledSurface;

                    aMatrix.xx = aMatrix.yy = 1;
                    cairo_set_matrix( mpCairo, &aMatrix );

                    rv = uno::Reference< rendering::XCachedPrimitive >( new CachedBitmap( pSurface, viewState, renderState, mpDevice ) );

                    pSurface->Unref();
                }

            if( !bHasAlpha && mbHaveAlpha /* && mpSurfaceProvider */ )
            {
                double x, y, width, height;

                x = y = 0;
                width = aBitmapSize.Width;
                height = aBitmapSize.Height;
                cairo_matrix_transform_point( &aMatrix, &x, &y );
                cairo_matrix_transform_distance( &aMatrix, &width, &height );

                // in case the bitmap doesn't have alpha and covers whole area
                // we try to change surface to plain rgb
                OSL_TRACE ("chance to change surface to rgb, %f, %f, %f x %f (%d x %d)", x, y, width, height, maSize.getX(), maSize.getY() );
                if( x <= 0 && y <= 0 && x + width >= maSize.getX() && y + height >= maSize.getY() )
                {
                    OSL_TRACE ("trying to change surface to rgb");
                    if( mpSurfaceProvider ) {
                        Surface* pNewSurface = mpSurfaceProvider->changeSurface( false, false );

                        if( pNewSurface )
                            setSurface( pNewSurface, false, mpSurfaceProvider );

                        // set state to new mpCairo
                        useStates( viewState, renderState, true );
                        // use the possibly modified matrix
                        cairo_set_matrix( mpCairo, &aMatrix );
                    }
                }
            }

            cairo_set_source_surface( mpCairo, pSurface->mpSurface, 0, 0 );
             if( !bHasAlpha &&
                ::rtl::math::approxEqual( aMatrix.xx, 1 ) &&
                ::rtl::math::approxEqual( aMatrix.yy, 1 ) &&
                ::rtl::math::approxEqual( aMatrix.x0, 0 ) &&
                ::rtl::math::approxEqual( aMatrix.y0, 0 ) )
                 cairo_set_operator( mpCairo, CAIRO_OPERATOR_SOURCE );
            cairo_rectangle( mpCairo, 0, 0, aBitmapSize.Width, aBitmapSize.Height );
            cairo_clip( mpCairo );
            cairo_paint( mpCairo );
            cairo_restore( mpCairo );
        } else
            OSL_TRACE ("CanvasHelper called after it was disposed");

        return rv; // uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmap( const rendering::XCanvas*                   /*pCanvas*/,
                                                                            const uno::Reference< rendering::XBitmap >& xBitmap,
                                                                            const rendering::ViewState&                 viewState,
                                                                            const rendering::RenderState&               renderState )
    {
    #ifdef CAIRO_CANVAS_PERF_TRACE
    struct timespec aTimer;
    mxDevice->startPerfTrace( &aTimer );
        #endif

    uno::Reference< rendering::XCachedPrimitive > rv;
    unsigned char* data = NULL;
    bool bHasAlpha = 0;
    Surface* pSurface = surfaceFromXBitmap( xBitmap, mpDevice, data, bHasAlpha );
    geometry::IntegerSize2D aSize = xBitmap->getSize();

    if( pSurface ) {
        rv = implDrawBitmapSurface( pSurface, viewState, renderState, aSize, false, bHasAlpha );

        pSurface->Unref();

        if( data )
        free( data );
    } else
        rv = uno::Reference< rendering::XCachedPrimitive >(NULL);

    #ifdef CAIRO_CANVAS_PERF_TRACE
    mxDevice->stopPerfTrace( &aTimer, "drawBitmap" );
        #endif

    return rv;
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::drawBitmapModulated( const rendering::XCanvas*                      /*pCanvas*/,
                                                                                     const uno::Reference< rendering::XBitmap >&    /*xBitmap*/,
                                                                                     const rendering::ViewState&                    /*viewState*/,
                                                                                     const rendering::RenderState&                  /*renderState*/ )
    {
        // TODO(F3): Implement modulated bitmap!

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(NULL);
    }

    uno::Reference< rendering::XGraphicDevice > CanvasHelper::getDevice()
    {
        return uno::Reference< rendering::XGraphicDevice >(mpDevice);
    }

    void CanvasHelper::copyRect( const rendering::XCanvas*                          /*pCanvas*/,
                                 const uno::Reference< rendering::XBitmapCanvas >&  /*sourceCanvas*/,
                                 const geometry::RealRectangle2D&                   /*sourceRect*/,
                                 const rendering::ViewState&                        /*sourceViewState*/,
                                 const rendering::RenderState&                      /*sourceRenderState*/,
                                 const geometry::RealRectangle2D&                   /*destRect*/,
                                 const rendering::ViewState&                        /*destViewState*/,
                                 const rendering::RenderState&                      /*destRenderState*/ )
    {
        // TODO(F2): copyRect NYI
    }

    geometry::IntegerSize2D CanvasHelper::getSize()
    {
        if( !mpDevice )
            geometry::IntegerSize2D(1, 1); // we're disposed

        return ::basegfx::unotools::integerSize2DFromB2ISize( maSize );
    }

    uno::Reference< rendering::XBitmap > CanvasHelper::getScaledBitmap( const geometry::RealSize2D& newSize,
                                                                        sal_Bool                    /*beFast*/ )
    {
#ifdef CAIRO_CANVAS_PERF_TRACE
        struct timespec aTimer;
        mxDevice->startPerfTrace( &aTimer );
#endif

        if( mpCairo ) {
            return uno::Reference< rendering::XBitmap >( new CanvasBitmap( ::basegfx::B2ISize( ::canvas::tools::roundUp( newSize.Width ),
                                                                                               ::canvas::tools::roundUp( newSize.Height ) ),
                                                                           mpDevice, false ) );
        } else
            OSL_TRACE ("CanvasHelper called after it was disposed");

#ifdef CAIRO_CANVAS_PERF_TRACE
        mxDevice->stopPerfTrace( &aTimer, "getScaledBitmap" );
#endif

        return uno::Reference< rendering::XBitmap >();
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getData( rendering::IntegerBitmapLayout&     aLayout,
                                                     const geometry::IntegerRectangle2D& rect )
    {
        if( mpCairo ) {
            const sal_Int32 nWidth( rect.X2 - rect.X1 );
            const sal_Int32 nHeight( rect.Y2 - rect.Y1 );
            uno::Sequence< sal_Int8 > aRes( 4*nWidth*nHeight );
            sal_Int8* pData = aRes.getArray();
            cairo_surface_t* pImageSurface = cairo_image_surface_create_for_data( (unsigned char *) pData,
                                                                                            CAIRO_FORMAT_ARGB32,
                                                                                            nWidth, nHeight, 4*nWidth );
            cairo_t* pCairo = cairo_create( pImageSurface );
            cairo_set_source_surface( pCairo, mpSurface->mpSurface, -rect.X1, -rect.Y1);
            cairo_paint( pCairo );
            cairo_destroy( pCairo );
            cairo_surface_destroy( pImageSurface );

            aLayout.ScanLines = nHeight;
            aLayout.ScanLineBytes = nWidth*4;
            aLayout.ScanLineStride = aLayout.ScanLineBytes;
            aLayout.PlaneStride = 0;
            aLayout.ColorSpace.set( mpDevice );
            aLayout.NumComponents = 4;
            aLayout.ComponentMasks.realloc(4);
            aLayout.ComponentMasks[0] = 0x00FF0000;
            aLayout.ComponentMasks[1] = 0x0000FF00;
            aLayout.ComponentMasks[2] = 0x000000FF;
            aLayout.ComponentMasks[3] = 0xFF000000;
            aLayout.Palette.clear();
            aLayout.Endianness = rendering::Endianness::LITTLE;
            aLayout.Format = rendering::IntegerBitmapFormat::CHUNKY_32BIT;
            aLayout.IsMsbFirst = sal_False;

            return aRes;
        }

        return uno::Sequence< sal_Int8 >();
    }

    void CanvasHelper::setData( const uno::Sequence< sal_Int8 >&        /*data*/,
                                const rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                const geometry::IntegerRectangle2D&     /*rect*/ )
    {
    }

    void CanvasHelper::setPixel( const uno::Sequence< sal_Int8 >&       /*color*/,
                                 const rendering::IntegerBitmapLayout&  /*bitmapLayout*/,
                                 const geometry::IntegerPoint2D&        /*pos*/ )
    {
    }

    uno::Sequence< sal_Int8 > CanvasHelper::getPixel( rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                                      const geometry::IntegerPoint2D&   /*pos*/ )
    {
        return uno::Sequence< sal_Int8 >();
    }

    uno::Reference< rendering::XBitmapPalette > CanvasHelper::getPalette()
    {
        // TODO(F1): Palette bitmaps NYI
        return uno::Reference< rendering::XBitmapPalette >();
    }

    rendering::IntegerBitmapLayout CanvasHelper::getMemoryLayout()
    {
        // TODO(F1): finish memory layout initialization
        rendering::IntegerBitmapLayout aLayout;

        const geometry::IntegerSize2D& rBmpSize( getSize() );

        aLayout.ScanLines = rBmpSize.Height;
        aLayout.ScanLineBytes = rBmpSize.Width * 4;
        aLayout.ScanLineStride = aLayout.ScanLineBytes;
        aLayout.PlaneStride = 0;
        aLayout.ColorSpace.set( mpDevice );
        aLayout.NumComponents = 4;
        aLayout.ComponentMasks.realloc(4);
        aLayout.ComponentMasks[0] = 0x00FF0000;
        aLayout.ComponentMasks[1] = 0x0000FF00;
        aLayout.ComponentMasks[2] = 0x000000FF;
        aLayout.ComponentMasks[3] = 0xFF000000;
        aLayout.Palette.clear();
        aLayout.Endianness = rendering::Endianness::LITTLE;
        aLayout.Format = rendering::IntegerBitmapFormat::CHUNKY_32BIT;
        aLayout.IsMsbFirst = sal_False;

        return aLayout;
    }

    void CanvasHelper::flush() const
    {
    }

    bool CanvasHelper::hasAlpha() const
    {
        return mbHaveAlpha;
    }

    bool CanvasHelper::repaint( Surface* pSurface,
                                const rendering::ViewState& viewState,
                                const rendering::RenderState&   renderState )
    {
        OSL_TRACE("CanvasHelper::repaint");

        if( mpCairo ) {
            cairo_save( mpCairo );

            cairo_rectangle( mpCairo, 0, 0, maSize.getX(), maSize.getY() );
            cairo_clip( mpCairo );

            useStates( viewState, renderState, true );

            Matrix aMatrix;

            cairo_get_matrix( mpCairo, &aMatrix );
            aMatrix.xx = aMatrix.yy = 1;
            cairo_set_matrix( mpCairo, &aMatrix );

            //          if( !bHasAlpha )
            //          cairo_set_operator( mpCairo, CAIRO_OPERATOR_SOURCE );

            cairo_set_source_surface( mpCairo, pSurface->mpSurface, 0, 0 );
            cairo_paint( mpCairo );
            cairo_restore( mpCairo );
        }

        return true;
    }
}
