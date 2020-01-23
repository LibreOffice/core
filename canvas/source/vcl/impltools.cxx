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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/metric.hxx>

#include <canvas/canvastools.hxx>

#include "canvasbitmap.hxx"
#include "impltools.hxx"
#include "spritecanvas.hxx"


using namespace ::com::sun::star;

namespace vclcanvas::tools
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
                SpriteCanvas* pCanvasImpl = dynamic_cast< SpriteCanvas* >( xBitmap.get() );
                if( pCanvasImpl && pCanvasImpl->getBackBuffer() )
                {
                    // TODO(F3): mind the plain Canvas impl. Consolidate with CWS canvas05
                    const ::OutputDevice& rDev( pCanvasImpl->getBackBuffer()->getOutDev() );
                    const ::Point aEmptyPoint;
                    return rDev.GetBitmapEx( aEmptyPoint,
                                             rDev.GetOutputSizePixel() );
                }

                // TODO(F2): add support for floating point bitmap formats
                uno::Reference< rendering::XIntegerReadOnlyBitmap > xIntBmp(
                    xBitmap, uno::UNO_QUERY_THROW );

                ::BitmapEx aBmpEx = vcl::unotools::bitmapExFromXBitmap( xIntBmp );
                if( !!aBmpEx )
                    return aBmpEx;

                // TODO(F1): extract pixel from XBitmap interface
                ENSURE_OR_THROW( false,
                                  "bitmapExFromXBitmap(): could not extract bitmap" );
            }

            return ::BitmapEx();
        }

        bool setupFontTransform( ::Point&                       o_rPoint,
                                 vcl::Font&                    io_rVCLFont,
                                 const rendering::ViewState&    rViewState,
                                 const rendering::RenderState&  rRenderState,
                                 ::OutputDevice const &         rOutDev )
        {
            ::basegfx::B2DHomMatrix aMatrix;

            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         rViewState,
                                                         rRenderState);

            ::basegfx::B2DTuple aScale;
            ::basegfx::B2DTuple aTranslate;
            double nRotate, nShearX;

            aMatrix.decompose( aScale, aTranslate, nRotate, nShearX );

            // query font metric _before_ tampering with width and height
            if( !::rtl::math::approxEqual(aScale.getX(), aScale.getY()) )
            {
                // retrieve true font width
                const sal_Int32 nFontWidth( rOutDev.GetFontMetric( io_rVCLFont ).GetAverageFontWidth() );

                const sal_Int32 nScaledFontWidth( ::basegfx::fround(nFontWidth * aScale.getX()) );

                if( !nScaledFontWidth )
                {
                    // scale is smaller than one pixel - disable text
                    // output altogether
                    return false;
                }

                io_rVCLFont.SetAverageFontWidth( nScaledFontWidth );
            }

            if( !::rtl::math::approxEqual(aScale.getY(), 1.0) )
            {
                const sal_Int32 nFontHeight( io_rVCLFont.GetFontHeight() );
                io_rVCLFont.SetFontHeight( ::basegfx::fround(nFontHeight * aScale.getY()) );
            }

            io_rVCLFont.SetOrientation( static_cast< short >( ::basegfx::fround(-fmod(nRotate, 2*M_PI)*(1800.0/M_PI)) ) );

            // TODO(F2): Missing functionality in VCL: shearing
            o_rPoint.setX( ::basegfx::fround(aTranslate.getX()) );
            o_rPoint.setY( ::basegfx::fround(aTranslate.getY()) );

            return true;
        }

        bool isRectangle( const ::tools::PolyPolygon& rPolyPoly )
        {
            // exclude some cheap cases first
            if( rPolyPoly.Count() != 1 )
                return false;

            const ::tools::Polygon& rPoly( rPolyPoly[0] );

            sal_uInt16 nCount( rPoly.GetSize() );
            if( nCount < 4 )
                return false;

            // delegate to basegfx
            return ::basegfx::utils::isRectangle( rPoly.getB2DPolygon() );
        }


        // VCL-Canvas related


        ::Point mapRealPoint2D( const geometry::RealPoint2D&    rPoint,
                                const rendering::ViewState&     rViewState,
                                const rendering::RenderState&   rRenderState )
        {
            ::basegfx::B2DPoint aPoint( ::basegfx::unotools::b2DPointFromRealPoint2D(rPoint) );

            ::basegfx::B2DHomMatrix aMatrix;
            aPoint *= ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                                   rViewState,
                                                                   rRenderState);

            return vcl::unotools::pointFromB2DPoint( aPoint );
        }

        ::tools::PolyPolygon mapPolyPolygon( const ::basegfx::B2DPolyPolygon&  rPoly,
                                      const rendering::ViewState&       rViewState,
                                      const rendering::RenderState&     rRenderState )
        {
            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         rViewState,
                                                         rRenderState);

            ::basegfx::B2DPolyPolygon aTemp( rPoly );

            aTemp.transform( aMatrix );

            return ::tools::PolyPolygon( aTemp );
        }

        ::BitmapEx transformBitmap( const BitmapEx&                 rBitmap,
                                    const ::basegfx::B2DHomMatrix&  rTransform )
        {
            SAL_INFO( "canvas.vcl", "::vclcanvas::tools::transformBitmap()" );
            SAL_INFO( "canvas.vcl", "::vclcanvas::tools::transformBitmap: 0x" << std::hex << &rBitmap );

            // calc transformation and size of bitmap to be
            // generated. Note, that the translational components are
            // deleted from the transformation; this can be handled by
            // an offset when painting the bitmap
            const Size                  aBmpSize( rBitmap.GetSizePixel() );
            ::basegfx::B2DRectangle     aDestRect;

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

            return vcl::bitmap::CanvasTransformBitmap(rBitmap, rTransform, aDestRect, aLocalTransform);
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
