/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impltools.cxx,v $
 * $Revision: 1.13 $
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

#ifndef  _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES  // needed by Visual C++ for math constants
#endif
#include <math.h>           // M_PI definition

#include <rtl/math.hxx>
#include <rtl/logfile.hxx>

#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/metric.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <canvas/base/linepolypolygonbase.hxx>
#include <canvas/canvastools.hxx>

#include "impltools.hxx"
#include "canvasbitmap.hxx"

#include <numeric>


using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace tools
    {
        ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XBitmap >& xBitmap )
        {
            // TODO(F3): CanvasCustomSprite should also be tunnelled
            // through (also implements XIntegerBitmap interface)
            CanvasBitmap* pBitmapImpl = dynamic_cast< CanvasBitmap* >( xBitmap.get() );

            if( pBitmapImpl )
            {
                return pBitmapImpl->getBitmap();
            }
            else
            {
                uno::Reference< lang::XUnoTunnel > xTunnel( xBitmap, uno::UNO_QUERY );
                if( xTunnel.is() )
                {
                    sal_Int64 nPtr = xTunnel->getSomething(
                        vcl::unotools::getTunnelIdentifier(
                            vcl::unotools::Id_BitmapEx ) );

                    if( nPtr )
                        return BitmapEx( *reinterpret_cast<BitmapEx*>(sal::static_int_cast<sal_uIntPtr>(nPtr)) );
                }

                SpriteCanvas* pCanvasImpl = dynamic_cast< SpriteCanvas* >( xBitmap.get() );
                if( pCanvasImpl && pCanvasImpl->getBackBuffer() )
                {
                    const ::VirtualDevice& rVDev( pCanvasImpl->getBackBuffer()->getVirDev() );
                    const ::Point aEmptyPoint;
                    return rVDev.GetBitmapEx( aEmptyPoint,
                                              rVDev.GetOutputSizePixel() );
                }

                // TODO(F1): extract pixel from XBitmap interface
                ENSURE_AND_THROW( false,
                                  "bitmapExFromXBitmap(): could not extract bitmap" );
            }

            return ::BitmapEx();
        }

        bool setupFontTransform( ::Point&                       o_rPoint,
                                 ::Font&                        io_rVCLFont,
                                 const rendering::ViewState&    rViewState,
                                 const rendering::RenderState&  rRenderState,
                                 ::OutputDevice&                rOutDev )
        {
            ::basegfx::B2DHomMatrix aMatrix;

            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         rViewState,
                                                         rRenderState);

            ::basegfx::B2DTuple aScale;
            ::basegfx::B2DTuple aTranslate;
            double nRotate, nShearX;

            aMatrix.decompose( aScale, aTranslate, nRotate, nShearX );

            // #i72417# detecting the 180 degree rotation case manually here.
            if( aScale.getX() < 0.0 &&
                aScale.getY() < 0.0 &&
                basegfx::fTools::equalZero(nRotate) )
            {
                aScale *= -1.0;
                nRotate += M_PI;
            }

            // query font metric _before_ tampering with width and height
            if( !::rtl::math::approxEqual(aScale.getX(), aScale.getY()) )
            {
                // retrieve true font width
                const sal_Int32 nFontWidth( rOutDev.GetFontMetric( io_rVCLFont ).GetWidth() );

                const sal_Int32 nScaledFontWidth( ::basegfx::fround(nFontWidth * aScale.getX()) );

                if( !nScaledFontWidth )
                {
                    // scale is smaller than one pixel - disable text
                    // output altogether
                    return false;
                }

                io_rVCLFont.SetWidth( nScaledFontWidth );
            }

            if( !::rtl::math::approxEqual(aScale.getY(), 1.0) )
            {
                const sal_Int32 nFontHeight( io_rVCLFont.GetHeight() );
                io_rVCLFont.SetHeight( ::basegfx::fround(nFontHeight * aScale.getY()) );
            }

            io_rVCLFont.SetOrientation( static_cast< short >( ::basegfx::fround(-fmod(nRotate, 2*M_PI)*(1800.0/M_PI)) ) );

            // TODO(F2): Missing functionality in VCL: shearing
            o_rPoint.X() = ::basegfx::fround(aTranslate.getX());
            o_rPoint.Y() = ::basegfx::fround(aTranslate.getY());

            return true;
        }

        bool isRectangle( const PolyPolygon& rPolyPoly )
        {
            // exclude some cheap cases first
            if( rPolyPoly.Count() != 1 )
                return false;

            const ::Polygon& rPoly( rPolyPoly[0] );

            USHORT nCount( rPoly.GetSize() );
            if( nCount < 4 )
                return false;

            // delegate to basegfx
            return ::basegfx::tools::isRectangle( rPoly.getB2DPolygon() );
        }


        // VCL-Canvas related
        //---------------------------------------------------------------------

        ::Point mapRealPoint2D( const geometry::RealPoint2D&    rPoint,
                                const rendering::ViewState&     rViewState,
                                const rendering::RenderState&   rRenderState )
        {
            ::basegfx::B2DPoint aPoint( ::basegfx::unotools::b2DPointFromRealPoint2D(rPoint) );

            ::basegfx::B2DHomMatrix aMatrix;
            aPoint *= ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                                   rViewState,
                                                                   rRenderState);

            return ::vcl::unotools::pointFromB2DPoint( aPoint );
        }

        ::PolyPolygon mapPolyPolygon( const ::basegfx::B2DPolyPolygon&  rPoly,
                                      const rendering::ViewState&       rViewState,
                                      const rendering::RenderState&     rRenderState )
        {
            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         rViewState,
                                                         rRenderState);

            ::basegfx::B2DPolyPolygon aTemp( rPoly );

            aTemp.transform( aMatrix );

            return ::PolyPolygon( aTemp );
        }

        ::BitmapEx transformBitmap( const BitmapEx&                 rBitmap,
                                    const ::basegfx::B2DHomMatrix&  rTransform,
                                    const uno::Sequence< double >&  rDeviceColor,
                                    ModulationMode                  eModulationMode )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::tools::transformBitmap()" );
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::vclcanvas::tools::transformBitmap: 0x%X", &rBitmap );

            // calc transformation and size of bitmap to be
            // generated. Note, that the translational components are
            // deleted from the transformation; this can be handled by
            // an offset when painting the bitmap
            const Size                  aBmpSize( rBitmap.GetSizePixel() );
            ::basegfx::B2DRectangle     aDestRect;

            bool bCopyBack( false );

            // calc effective transformation for bitmap
            const ::basegfx::B2DRectangle aSrcRect( 0, 0,
                                                    aBmpSize.Width(),
                                                    aBmpSize.Height() );
            ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                        aSrcRect,
                                                        rTransform );

            // re-center bitmap, such that it's left, top border is
            // aligned with (0,0). The method takes the given
            // rectangle, and calculates a transformation that maps
            // this rectangle unscaled to the origin.
            ::basegfx::B2DHomMatrix aLocalTransform;
            ::canvas::tools::calcRectToOriginTransform( aLocalTransform,
                                                        aSrcRect,
                                                        rTransform );

            const bool bModulateColors( eModulationMode == MODULATE_WITH_DEVICECOLOR &&
                                        rDeviceColor.getLength() > 2 );
            const double nRedModulation( bModulateColors ? rDeviceColor[0] : 1.0 );
            const double nGreenModulation( bModulateColors ? rDeviceColor[1] : 1.0 );
            const double nBlueModulation( bModulateColors ? rDeviceColor[2] : 1.0 );
            const double nAlphaModulation( bModulateColors && rDeviceColor.getLength() > 3 ?
                                           rDeviceColor[3] : 1.0 );

            Bitmap aSrcBitmap( rBitmap.GetBitmap() );
            Bitmap aSrcAlpha;

            // differentiate mask and alpha channel (on-off
            // vs. multi-level transparency)
            if( rBitmap.IsTransparent() )
            {
                if( rBitmap.IsAlpha() )
                    aSrcAlpha = rBitmap.GetAlpha().GetBitmap();
                else
                    aSrcAlpha = rBitmap.GetMask();
            }

            ScopedBitmapReadAccess pReadAccess( aSrcBitmap.AcquireReadAccess(),
                                                aSrcBitmap );
            ScopedBitmapReadAccess pAlphaReadAccess( rBitmap.IsTransparent() ?
                                                     aSrcAlpha.AcquireReadAccess() :
                                                     (BitmapReadAccess*)NULL,
                                                     aSrcAlpha );

            if( pReadAccess.get() == NULL ||
                (pAlphaReadAccess.get() == NULL && rBitmap.IsTransparent()) )
            {
                // TODO(E2): Error handling!
                ENSURE_AND_THROW( false,
                                  "transformBitmap(): could not access source bitmap" );
            }

            // mapping table, to translate pAlphaReadAccess' pixel
            // values into destination alpha values (needed e.g. for
            // paletted 1-bit masks).
            sal_uInt8 aAlphaMap[256];

            if( rBitmap.IsTransparent() )
            {
                if( rBitmap.IsAlpha() )
                {
                    // source already has alpha channel - 1:1 mapping,
                    // i.e. aAlphaMap[0]=0,...,aAlphaMap[255]=255.
                    ::std::iota( aAlphaMap, &aAlphaMap[256], 0 );
                }
                else
                {
                    // mask transparency - determine used palette colors
                    const BitmapColor& rCol0( pAlphaReadAccess->GetPaletteColor( 0 ) );
                    const BitmapColor& rCol1( pAlphaReadAccess->GetPaletteColor( 1 ) );

                    // shortcut for true luminance calculation
                    // (assumes that palette is grey-level)
                    aAlphaMap[0] = rCol0.GetRed();
                    aAlphaMap[1] = rCol1.GetRed();
                }
            }
            // else: mapping table is not used

            const Size aDestBmpSize( ::basegfx::fround( aDestRect.getWidth() ),
                                     ::basegfx::fround( aDestRect.getHeight() ) );

            if( aDestBmpSize.Width() == 0 || aDestBmpSize.Height() == 0 )
                return BitmapEx();

            Bitmap aDstBitmap( aDestBmpSize, aSrcBitmap.GetBitCount(), &pReadAccess->GetPalette() );
            Bitmap aDstAlpha( AlphaMask( aDestBmpSize ).GetBitmap() );

            {
                // just to be on the safe side: let the
                // ScopedAccessors get destructed before
                // copy-constructing the resulting bitmap. This will
                // rule out the possibility that cached accessor data
                // is not yet written back.
                ScopedBitmapWriteAccess pWriteAccess( aDstBitmap.AcquireWriteAccess(),
                                                      aDstBitmap );
                ScopedBitmapWriteAccess pAlphaWriteAccess( aDstAlpha.AcquireWriteAccess(),
                                                           aDstAlpha );


                if( pWriteAccess.get() != NULL &&
                    pAlphaWriteAccess.get() != NULL &&
                    rTransform.isInvertible() )
                {
                    // we're doing inverse mapping here, i.e. mapping
                    // points from the destination bitmap back to the
                    // source
                    ::basegfx::B2DHomMatrix aTransform( aLocalTransform );
                    aTransform.invert();

                    // for the time being, always read as ARGB
                    for( int y=0; y<aDestBmpSize.Height(); ++y )
                    {
                        if( bModulateColors )
                        {
                            // TODO(P2): Have different branches for
                            // alpha-only modulation (color
                            // modulations eq. 1.0)

                            // modulate all color channels with given
                            // values

                            // differentiate mask and alpha channel (on-off
                            // vs. multi-level transparency)
                            if( rBitmap.IsTransparent() )
                            {
                                // Handling alpha and mask just the same...
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        // modulate alpha with
                                        // nAlphaModulation. This is a
                                        // little bit verbose, formula
                                        // is 255 - (255-pixAlpha)*nAlphaModulation
                                        // (invert 'alpha' pixel value,
                                        // to get the standard alpha
                                        // channel behaviour)
                                        pAlphaWriteAccess->SetPixel( y, x,
                                                                     BitmapColor(
                                                                         255U -
                                                                         static_cast<BYTE>(
                                                                             nAlphaModulation*
                                                                             (255U
                                                                              - aAlphaMap[ pAlphaReadAccess->GetPixel(
                                                                                               nSrcY,
                                                                                               nSrcX ).GetIndex() ] ) + .5 ) ) );

                                        BitmapColor aColor( pReadAccess->GetPixel( nSrcY,
                                                                                   nSrcX ) );

                                        aColor.SetRed(
                                            static_cast<BYTE>(
                                                nRedModulation *
                                                aColor.GetRed() + .5 ));
                                        aColor.SetGreen(
                                            static_cast<BYTE>(
                                                nGreenModulation *
                                                aColor.GetGreen() + .5 ));
                                        aColor.SetBlue(
                                            static_cast<BYTE>(
                                                nBlueModulation *
                                                aColor.GetBlue() + .5 ));

                                        pWriteAccess->SetPixel( y, x,
                                                                aColor );
                                    }
                                }
                            }
                            else
                            {
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        // modulate alpha with
                                        // nAlphaModulation. This is a
                                        // little bit verbose, formula
                                        // is 255 - 255*nAlphaModulation
                                        // (invert 'alpha' pixel value,
                                        // to get the standard alpha
                                        // channel behaviour)
                                        pAlphaWriteAccess->SetPixel( y, x,
                                                                     BitmapColor(
                                                                         255U -
                                                                         static_cast<BYTE>(
                                                                             nAlphaModulation*255.0
                                                                             + .5 ) ) );

                                        BitmapColor aColor( pReadAccess->GetPixel( nSrcY,
                                                                                   nSrcX ) );

                                        aColor.SetRed(
                                            static_cast<BYTE>(
                                                nRedModulation *
                                                aColor.GetRed() + .5 ));
                                        aColor.SetGreen(
                                            static_cast<BYTE>(
                                                nGreenModulation *
                                                aColor.GetGreen() + .5 ));
                                        aColor.SetBlue(
                                            static_cast<BYTE>(
                                                nBlueModulation *
                                                aColor.GetBlue() + .5 ));

                                        pWriteAccess->SetPixel( y, x,
                                                                aColor );
                                    }
                                }
                            }
                        }
                        else
                        {
                            // differentiate mask and alpha channel (on-off
                            // vs. multi-level transparency)
                            if( rBitmap.IsTransparent() )
                            {
                                // Handling alpha and mask just the same...
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x,
                                                                     aAlphaMap[
                                                                         pAlphaReadAccess->GetPixel( nSrcY,
                                                                                                     nSrcX ) ] );

                                        pWriteAccess->SetPixel( y, x, pReadAccess->GetPixel( nSrcY,
                                                                                             nSrcX ) );
                                    }
                                }
                            }
                            else
                            {
                                for( int x=0; x<aDestBmpSize.Width(); ++x )
                                {
                                    ::basegfx::B2DPoint aPoint(x,y);
                                    aPoint *= aTransform;

                                    const int nSrcX( ::basegfx::fround( aPoint.getX() ) );
                                    const int nSrcY( ::basegfx::fround( aPoint.getY() ) );
                                    if( nSrcX < 0 || nSrcX >= aBmpSize.Width() ||
                                        nSrcY < 0 || nSrcY >= aBmpSize.Height() )
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(255) );
                                    }
                                    else
                                    {
                                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor(0) );
                                        pWriteAccess->SetPixel( y, x, pReadAccess->GetPixel( nSrcY,
                                                                                             nSrcX ) );
                                    }
                                }
                            }
                        }
                    }

                    bCopyBack = true;
                }
                else
                {
                    // TODO(E2): Error handling!
                    ENSURE_AND_THROW( false,
                                      "transformBitmap(): could not access bitmap" );
                }
            }

            if( bCopyBack )
                return BitmapEx( aDstBitmap, AlphaMask( aDstAlpha ) );
            else
                return BitmapEx();
        }
    }
}
