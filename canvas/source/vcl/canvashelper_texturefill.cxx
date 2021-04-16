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

#include <cstdlib>
#include <tuple>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/keystoplerp.hxx>
#include <basegfx/utils/lerp.hxx>
#include <basegfx/utils/tools.hxx>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <tools/poly.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gradient.hxx>

#include <canvas/canvastools.hxx>
#include <parametricpolypolygon.hxx>

#include "canvashelper.hxx"
#include "impltools.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        bool textureFill( OutputDevice&         rOutDev,
                          const GraphicObject&  rGraphic,
                          const ::Point&        rPosPixel,
                          const ::Size&         rNextTileX,
                          const ::Size&         rNextTileY,
                          sal_Int32             nTilesX,
                          sal_Int32             nTilesY,
                          const ::Size&         rTileSize,
                          const GraphicAttr&    rAttr)
        {
            bool bRet( false );
            Point   aCurrPos;
            int     nX, nY;

            for( nY=0; nY < nTilesY; ++nY )
            {
                aCurrPos.setX( rPosPixel.X() + nY*rNextTileY.Width() );
                aCurrPos.setY( rPosPixel.Y() + nY*rNextTileY.Height() );

                for( nX=0; nX < nTilesX; ++nX )
                {
                    // update return value. This method should return true, if
                    // at least one of the looped Draws succeeded.
                    bRet |= rGraphic.Draw(rOutDev,
                                          aCurrPos,
                                          rTileSize,
                                          &rAttr);

                    aCurrPos.AdjustX(rNextTileX.Width() );
                    aCurrPos.AdjustY(rNextTileX.Height() );
                }
            }

            return bRet;
        }


        /** Fill linear or axial gradient

            Since most of the code for linear and axial gradients are
            the same, we've a unified method here
         */
        void fillLinearGradient( OutputDevice&                                  rOutDev,
                                 const ::basegfx::B2DHomMatrix&                 rTextureTransform,
                                 const ::tools::Rectangle&                             rBounds,
                                 unsigned int                                   nStepCount,
                                 const ::canvas::ParametricPolyPolygon::Values& rValues,
                                 const std::vector< ::Color >&                  rColors )
        {
            // determine general position of gradient in relation to
            // the bound rect
            // =====================================================

            ::basegfx::B2DPoint aLeftTop( 0.0, 0.0 );
            ::basegfx::B2DPoint aLeftBottom( 0.0, 1.0 );
            ::basegfx::B2DPoint aRightTop( 1.0, 0.0 );
            ::basegfx::B2DPoint aRightBottom( 1.0, 1.0 );

            aLeftTop    *= rTextureTransform;
            aLeftBottom *= rTextureTransform;
            aRightTop   *= rTextureTransform;
            aRightBottom*= rTextureTransform;

            // calc length of bound rect diagonal
            const ::basegfx::B2DVector aBoundRectDiagonal(
                vcl::unotools::b2DPointFromPoint( rBounds.TopLeft() ) -
                vcl::unotools::b2DPointFromPoint( rBounds.BottomRight() ) );
            const double nDiagonalLength( aBoundRectDiagonal.getLength() );

            // create direction of gradient:
            //     _______
            //     |  |  |
            // ->  |  |  | ...
            //     |  |  |
            //     -------
            ::basegfx::B2DVector aDirection( aRightTop - aLeftTop );
            aDirection.normalize();

            // now, we potentially have to enlarge our gradient area
            // atop and below the transformed [0,1]x[0,1] unit rect,
            // for the gradient to fill the complete bound rect.
            ::basegfx::utils::infiniteLineFromParallelogram( aLeftTop,
                                                             aLeftBottom,
                                                             aRightTop,
                                                             aRightBottom,
                                                             vcl::unotools::b2DRectangleFromRectangle(rBounds) );


            // render gradient
            // ===============

            // First try to use directly VCL's DrawGradient(), as that one is generally
            // a better choice than here decomposing to polygons. The VCL API allows
            // only 2 colors, but that should generally do.
            // Do not use nStepCount, it limits optimized implementations, and it's computed
            // by vclcanvas based on number of colors, so it's practically irrelevant.

            // 2 colors and 2 stops (at 0 and 1) is a linear gradient:
            if( rColors.size() == 2 && rValues.maStops.size() == 2 && rValues.maStops[0] == 0 && rValues.maStops[1] == 1)
            {
                Gradient vclGradient( GradientStyle::Linear, rColors[ 0 ], rColors[ 1 ] );
                ::tools::Polygon aTempPoly( static_cast<sal_uInt16>(5) );
                aTempPoly[0] = ::Point( ::basegfx::fround( aLeftTop.getX() ),
                                        ::basegfx::fround( aLeftTop.getY() ) );
                aTempPoly[1] = ::Point( ::basegfx::fround( aRightTop.getX() ),
                                        ::basegfx::fround( aRightTop.getY() ) );
                aTempPoly[2] = ::Point( ::basegfx::fround( aRightBottom.getX() ),
                                        ::basegfx::fround( aRightBottom.getY() ) );
                aTempPoly[3] = ::Point( ::basegfx::fround( aLeftBottom.getX() ),
                                        ::basegfx::fround( aLeftBottom.getY() ) );
                aTempPoly[4] = aTempPoly[0];
                rOutDev.DrawGradient( aTempPoly, vclGradient );
                return;
            }
            // 3 colors with first and last being equal and 3 stops (at 0, 0.5 and 1) is an axial gradient:
            if( rColors.size() == 3 && rColors[ 0 ] == rColors[ 2 ]
                && rValues.maStops.size() == 3 && rValues.maStops[0] == 0
                && rValues.maStops[1] == 0.5 && rValues.maStops[2] == 1)
            {
                Gradient vclGradient( GradientStyle::Axial, rColors[ 1 ], rColors[ 0 ] );
                ::tools::Polygon aTempPoly( static_cast<sal_uInt16>(5) );
                aTempPoly[0] = ::Point( ::basegfx::fround( aLeftTop.getX() ),
                                        ::basegfx::fround( aLeftTop.getY() ) );
                aTempPoly[1] = ::Point( ::basegfx::fround( aRightTop.getX() ),
                                        ::basegfx::fround( aRightTop.getY() ) );
                aTempPoly[2] = ::Point( ::basegfx::fround( aRightBottom.getX() ),
                                        ::basegfx::fround( aRightBottom.getY() ) );
                aTempPoly[3] = ::Point( ::basegfx::fround( aLeftBottom.getX() ),
                                        ::basegfx::fround( aLeftBottom.getY() ) );
                aTempPoly[4] = aTempPoly[0];
                rOutDev.DrawGradient( aTempPoly, vclGradient );
                return;
            }

            // for linear gradients, it's easy to render
            // non-overlapping polygons: just split the gradient into
            // nStepCount small strips. Prepare the strip now.

            // For performance reasons, we create a temporary VCL
            // polygon here, keep it all the way and only change the
            // vertex values in the loop below (as ::Polygon is a
            // pimpl class, creating one every loop turn would really
            // stress the mem allocator)
            ::tools::Polygon aTempPoly( static_cast<sal_uInt16>(5) );

            OSL_ENSURE( nStepCount >= 3,
                        "fillLinearGradient(): stepcount smaller than 3" );


            // fill initial strip (extending two times the bound rect's
            // diagonal to the 'left'


            // calculate left edge, by moving left edge of the
            // gradient rect two times the bound rect's diagonal to
            // the 'left'. Since we postpone actual rendering into the
            // loop below, we set the _right_ edge here, which will be
            // readily copied into the left edge in the loop below
            const ::basegfx::B2DPoint& rPoint1( aLeftTop - 2.0*nDiagonalLength*aDirection );
            aTempPoly[1] = ::Point( ::basegfx::fround( rPoint1.getX() ),
                                    ::basegfx::fround( rPoint1.getY() ) );

            const ::basegfx::B2DPoint& rPoint2( aLeftBottom - 2.0*nDiagonalLength*aDirection );
            aTempPoly[2] = ::Point( ::basegfx::fround( rPoint2.getX() ),
                                    ::basegfx::fround( rPoint2.getY() ) );


            // iteratively render all other strips


            // ensure that nStepCount matches color stop parity, to
            // have a well-defined middle color e.g. for axial
            // gradients.
            if( (rColors.size() % 2) != (nStepCount % 2) )
                ++nStepCount;

            rOutDev.SetLineColor();

            basegfx::utils::KeyStopLerp aLerper(rValues.maStops);

            // only iterate nStepCount-1 steps, as the last strip is
            // explicitly painted below
            for( unsigned int i=0; i<nStepCount-1; ++i )
            {
                std::ptrdiff_t nIndex;
                double fAlpha;
                std::tie(nIndex,fAlpha)=aLerper.lerp(double(i)/nStepCount);

                rOutDev.SetFillColor(
                    Color( static_cast<sal_uInt8>(basegfx::utils::lerp(rColors[nIndex].GetRed(),rColors[nIndex+1].GetRed(),fAlpha)),
                           static_cast<sal_uInt8>(basegfx::utils::lerp(rColors[nIndex].GetGreen(),rColors[nIndex+1].GetGreen(),fAlpha)),
                           static_cast<sal_uInt8>(basegfx::utils::lerp(rColors[nIndex].GetBlue(),rColors[nIndex+1].GetBlue(),fAlpha)) ));

                // copy right edge of polygon to left edge (and also
                // copy the closing point)
                aTempPoly[0] = aTempPoly[4] = aTempPoly[1];
                aTempPoly[3] = aTempPoly[2];

                // calculate new right edge, from interpolating
                // between start and end line. Note that i is
                // increased by one, to account for the fact that we
                // calculate the right border here (whereas the fill
                // color is governed by the left edge)
                const ::basegfx::B2DPoint& rPoint3(
                    (nStepCount - i-1)/double(nStepCount)*aLeftTop +
                    (i+1)/double(nStepCount)*aRightTop );
                aTempPoly[1] = ::Point( ::basegfx::fround( rPoint3.getX() ),
                                        ::basegfx::fround( rPoint3.getY() ) );

                const ::basegfx::B2DPoint& rPoint4(
                    (nStepCount - i-1)/double(nStepCount)*aLeftBottom +
                    (i+1)/double(nStepCount)*aRightBottom );
                aTempPoly[2] = ::Point( ::basegfx::fround( rPoint4.getX() ),
                                        ::basegfx::fround( rPoint4.getY() ) );

                rOutDev.DrawPolygon( aTempPoly );
            }

            // fill final strip (extending two times the bound rect's
            // diagonal to the 'right'


            // copy right edge of polygon to left edge (and also
            // copy the closing point)
            aTempPoly[0] = aTempPoly[4] = aTempPoly[1];
            aTempPoly[3] = aTempPoly[2];

            // calculate new right edge, by moving right edge of the
            // gradient rect two times the bound rect's diagonal to
            // the 'right'.
            const ::basegfx::B2DPoint& rPoint3( aRightTop + 2.0*nDiagonalLength*aDirection );
            aTempPoly[0] = aTempPoly[4] = ::Point( ::basegfx::fround( rPoint3.getX() ),
                                                   ::basegfx::fround( rPoint3.getY() ) );

            const ::basegfx::B2DPoint& rPoint4( aRightBottom + 2.0*nDiagonalLength*aDirection );
            aTempPoly[3] = ::Point( ::basegfx::fround( rPoint4.getX() ),
                                    ::basegfx::fround( rPoint4.getY() ) );

            rOutDev.SetFillColor( rColors.back() );

            rOutDev.DrawPolygon( aTempPoly );
        }

        void fillPolygonalGradient( OutputDevice&                                  rOutDev,
                                    const ::basegfx::B2DHomMatrix&                 rTextureTransform,
                                    const ::tools::Rectangle&                             rBounds,
                                    unsigned int                                   nStepCount,
                                    const ::canvas::ParametricPolyPolygon::Values& rValues,
                                    const std::vector< ::Color >&                  rColors )
        {
            const ::basegfx::B2DPolygon& rGradientPoly( rValues.maGradientPoly );

            ENSURE_OR_THROW( rGradientPoly.count() > 2,
                              "fillPolygonalGradient(): polygon without area given" );

            // For performance reasons, we create a temporary VCL polygon
            // here, keep it all the way and only change the vertex values
            // in the loop below (as ::Polygon is a pimpl class, creating
            // one every loop turn would really stress the mem allocator)
            ::basegfx::B2DPolygon   aOuterPoly( rGradientPoly );
            ::basegfx::B2DPolygon   aInnerPoly;

            // subdivide polygon _before_ rendering, would otherwise have
            // to be performed on every loop turn.
            if( aOuterPoly.areControlPointsUsed() )
                aOuterPoly = ::basegfx::utils::adaptiveSubdivideByAngle(aOuterPoly);

            aInnerPoly = aOuterPoly;

            // only transform outer polygon _after_ copying it into
            // aInnerPoly, because inner polygon has to be scaled before
            // the actual texture transformation takes place
            aOuterPoly.transform( rTextureTransform );

            // determine overall transformation for inner polygon (might
            // have to be prefixed by anisotrophic scaling)
            ::basegfx::B2DHomMatrix aInnerPolygonTransformMatrix;


            // apply scaling (possibly anisotrophic) to inner polygon


            // scale inner polygon according to aspect ratio: for
            // wider-than-tall bounds (nAspectRatio > 1.0), the inner
            // polygon, representing the gradient focus, must have
            // non-zero width. Specifically, a bound rect twice as wide as
            // tall has a focus polygon of half its width.
            const double nAspectRatio( rValues.mnAspectRatio );
            if( nAspectRatio > 1.0 )
            {
                // width > height case
                aInnerPolygonTransformMatrix.scale( 1.0 - 1.0/nAspectRatio,
                                                    0.0 );
            }
            else if( nAspectRatio < 1.0 )
            {
                // width < height case
                aInnerPolygonTransformMatrix.scale( 0.0,
                                                    1.0 - nAspectRatio );
            }
            else
            {
                // isotrophic case
                aInnerPolygonTransformMatrix.scale( 0.0, 0.0 );
            }

            // and finally, add texture transform to it.
            aInnerPolygonTransformMatrix *= rTextureTransform;

            // apply final matrix to polygon
            aInnerPoly.transform( aInnerPolygonTransformMatrix );


            const sal_uInt32 nNumPoints( aOuterPoly.count() );
            ::tools::Polygon aTempPoly( static_cast<sal_uInt16>(nNumPoints+1) );

            // increase number of steps by one: polygonal gradients have
            // the outermost polygon rendered in rColor2, and the
            // innermost in rColor1. The innermost polygon will never
            // have zero area, thus, we must divide the interval into
            // nStepCount+1 steps. For example, to create 3 steps:

            // |                       |
            // |-------|-------|-------|
            // |                       |
            // 3       2       1       0

            // This yields 4 tick marks, where 0 is never attained (since
            // zero-area polygons typically don't display perceivable
            // color).
            ++nStepCount;

            rOutDev.SetLineColor();

            basegfx::utils::KeyStopLerp aLerper(rValues.maStops);

            // fill background
            rOutDev.SetFillColor( rColors.front() );
            rOutDev.DrawRect( rBounds );

            // render polygon
            // ==============

            for( unsigned int i=1,p; i<nStepCount; ++i )
            {
                const double fT( i/double(nStepCount) );

                std::ptrdiff_t nIndex;
                double fAlpha;
                std::tie(nIndex,fAlpha)=aLerper.lerp(fT);

                // lerp color
                rOutDev.SetFillColor(
                    Color( static_cast<sal_uInt8>(basegfx::utils::lerp(rColors[nIndex].GetRed(),rColors[nIndex+1].GetRed(),fAlpha)),
                           static_cast<sal_uInt8>(basegfx::utils::lerp(rColors[nIndex].GetGreen(),rColors[nIndex+1].GetGreen(),fAlpha)),
                           static_cast<sal_uInt8>(basegfx::utils::lerp(rColors[nIndex].GetBlue(),rColors[nIndex+1].GetBlue(),fAlpha)) ));

                // scale and render polygon, by interpolating between
                // outer and inner polygon.

                for( p=0; p<nNumPoints; ++p )
                {
                    const ::basegfx::B2DPoint& rOuterPoint( aOuterPoly.getB2DPoint(p) );
                    const ::basegfx::B2DPoint& rInnerPoint( aInnerPoly.getB2DPoint(p) );

                    aTempPoly[static_cast<sal_uInt16>(p)] = ::Point(
                        basegfx::fround( fT*rInnerPoint.getX() + (1-fT)*rOuterPoint.getX() ),
                        basegfx::fround( fT*rInnerPoint.getY() + (1-fT)*rOuterPoint.getY() ) );
                }

                // close polygon explicitly
                aTempPoly[static_cast<sal_uInt16>(p)] = aTempPoly[0];

                // TODO(P1): compare with vcl/source/gdi/outdev4.cxx,
                // OutputDevice::ImplDrawComplexGradient(), there's a note
                // that on some VDev's, rendering disjunct poly-polygons
                // is faster!
                rOutDev.DrawPolygon( aTempPoly );
            }
        }

        void doGradientFill( OutputDevice&                                  rOutDev,
                             const ::canvas::ParametricPolyPolygon::Values& rValues,
                             const std::vector< ::Color >&                  rColors,
                             const ::basegfx::B2DHomMatrix&                 rTextureTransform,
                             const ::tools::Rectangle&                      rBounds,
                             unsigned int                                   nStepCount )
        {
            switch( rValues.meType )
            {
                case ::canvas::ParametricPolyPolygon::GradientType::Linear:
                    fillLinearGradient( rOutDev,
                                        rTextureTransform,
                                        rBounds,
                                        nStepCount,
                                        rValues,
                                        rColors );
                    break;

                case ::canvas::ParametricPolyPolygon::GradientType::Elliptical:
                case ::canvas::ParametricPolyPolygon::GradientType::Rectangular:
                    fillPolygonalGradient( rOutDev,
                                           rTextureTransform,
                                           rBounds,
                                           nStepCount,
                                           rValues,
                                           rColors );
                    break;

                default:
                    ENSURE_OR_THROW( false,
                                      "CanvasHelper::doGradientFill(): Unexpected case" );
            }
        }

        int numColorSteps( const ::Color& rColor1, const ::Color& rColor2 )
        {
            return std::max(
                std::abs( rColor1.GetRed() - rColor2.GetRed() ),
                std::max(
                    std::abs( rColor1.GetGreen() - rColor2.GetGreen() ),
                    std::abs( rColor1.GetBlue()  - rColor2.GetBlue() ) ) );
        }

        bool gradientFill( OutputDevice&                                   rOutDev,
                           OutputDevice*                                   p2ndOutDev,
                           const ::canvas::ParametricPolyPolygon::Values&  rValues,
                           const std::vector< ::Color >&                   rColors,
                           const ::tools::PolyPolygon&                     rPoly,
                           const rendering::ViewState&                     viewState,
                           const rendering::RenderState&                   renderState,
                           const rendering::Texture&                       texture,
                           int                                             nTransparency )
        {
            // TODO(T2): It is maybe necessary to lock here, should
            // maGradientPoly someday cease to be const. But then, beware of
            // deadlocks, canvashelper calls this method with locked own
            // mutex.

            // calc step size

            int nColorSteps = 0;
            for( size_t i=0; i<rColors.size()-1; ++i )
                nColorSteps += numColorSteps(rColors[i],rColors[i+1]);

            ::basegfx::B2DHomMatrix aTotalTransform;
            const int nStepCount=
                ::canvas::tools::calcGradientStepCount(aTotalTransform,
                                                       viewState,
                                                       renderState,
                                                       texture,
                                                       nColorSteps);

            rOutDev.SetLineColor();

            // determine maximal bound rect of texture-filled
            // polygon
            const ::tools::Rectangle aPolygonDeviceRectOrig(
                rPoly.GetBoundRect() );

            if( tools::isRectangle( rPoly ) )
            {
                // use optimized output path


                // this distinction really looks like a
                // micro-optimization, but in fact greatly speeds up
                // especially complex gradients. That's because when using
                // clipping, we can output polygons instead of
                // poly-polygons, and don't have to output the gradient
                // twice for XOR

                rOutDev.Push( vcl::PushFlags::CLIPREGION );
                rOutDev.IntersectClipRegion( aPolygonDeviceRectOrig );
                doGradientFill( rOutDev,
                                rValues,
                                rColors,
                                aTotalTransform,
                                aPolygonDeviceRectOrig,
                                nStepCount );
                rOutDev.Pop();

                if( p2ndOutDev && nTransparency < 253 )
                {
                    // HACK. Normally, CanvasHelper does not care about
                    // actually what mp2ndOutDev is...  well, here we do &
                    // assume a 1bpp target - everything beyond 97%
                    // transparency is fully transparent
                    p2ndOutDev->SetFillColor( COL_BLACK );
                    p2ndOutDev->DrawRect( aPolygonDeviceRectOrig );
                }
            }
            else
            {
                const vcl::Region aPolyClipRegion( rPoly );

                rOutDev.Push( vcl::PushFlags::CLIPREGION );
                rOutDev.IntersectClipRegion( aPolyClipRegion );

                doGradientFill( rOutDev,
                                rValues,
                                rColors,
                                aTotalTransform,
                                aPolygonDeviceRectOrig,
                                nStepCount );
                rOutDev.Pop();

                if( p2ndOutDev && nTransparency < 253 )
                {
                    // HACK. Normally, CanvasHelper does not care about
                    // actually what mp2ndOutDev is...  well, here we do &
                    // assume a 1bpp target - everything beyond 97%
                    // transparency is fully transparent
                    p2ndOutDev->SetFillColor( COL_BLACK );
                    p2ndOutDev->DrawPolyPolygon( rPoly );
                }
            }

#ifdef DEBUG_CANVAS_CANVASHELPER_TEXTUREFILL
            // extra-verbosity
            {
                ::basegfx::B2DRectangle aRect(0.0, 0.0, 1.0, 1.0);
                ::basegfx::B2DRectangle aTextureDeviceRect;
                ::basegfx::B2DHomMatrix aTextureTransform;
                ::canvas::tools::calcTransformedRectBounds( aTextureDeviceRect,
                                                            aRect,
                                                            aTextureTransform );
                rOutDev.SetLineColor( COL_RED );
                rOutDev.SetFillColor();
                rOutDev.DrawRect( vcl::unotools::rectangleFromB2DRectangle( aTextureDeviceRect ) );

                rOutDev.SetLineColor( COL_BLUE );
                ::tools::Polygon aPoly1(
                    vcl::unotools::rectangleFromB2DRectangle( aRect ));
                ::basegfx::B2DPolygon aPoly2( aPoly1.getB2DPolygon() );
                aPoly2.transform( aTextureTransform );
                ::tools::Polygon aPoly3( aPoly2 );
                rOutDev.DrawPolygon( aPoly3 );
            }
#endif

            return true;
        }
    }

    uno::Reference< rendering::XCachedPrimitive > CanvasHelper::fillTexturedPolyPolygon( const rendering::XCanvas*                          pCanvas,
                                                                                         const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         const uno::Sequence< rendering::Texture >&         textures )
    {
        ENSURE_ARG_OR_THROW( xPolyPolygon.is(),
                         "CanvasHelper::fillPolyPolygon(): polygon is NULL");
        ENSURE_ARG_OR_THROW( textures.hasElements(),
                         "CanvasHelper::fillTexturedPolyPolygon: empty texture sequence");

        if( mpOutDevProvider )
        {
            tools::OutDevStateKeeper aStateKeeper( mpProtectedOutDevProvider );

            const int nTransparency( setupOutDevState( viewState, renderState, IGNORE_COLOR ) );
            ::tools::PolyPolygon aPolyPoly( tools::mapPolyPolygon(
                                       ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(xPolyPolygon),
                                       viewState, renderState ) );

            // TODO(F1): Multi-texturing
            if( textures[0].Gradient.is() )
            {
                // try to cast XParametricPolyPolygon2D reference to
                // our implementation class.
                ::canvas::ParametricPolyPolygon* pGradient =
                      dynamic_cast< ::canvas::ParametricPolyPolygon* >( textures[0].Gradient.get() );

                if( pGradient && pGradient->getValues().maColors.hasElements() )
                {
                    // copy state from Gradient polypoly locally
                    // (given object might change!)
                    const ::canvas::ParametricPolyPolygon::Values& rValues(
                        pGradient->getValues() );

                    if( rValues.maColors.getLength() < 2 )
                    {
                        rendering::RenderState aTempState=renderState;
                        aTempState.DeviceColor = rValues.maColors[0];
                        fillPolyPolygon(pCanvas, xPolyPolygon, viewState, aTempState);
                    }
                    else
                    {
                        std::vector< ::Color > aColors(rValues.maColors.getLength());
                        std::transform(&rValues.maColors[0],
                                       &rValues.maColors[0]+rValues.maColors.getLength(),
                                       aColors.begin(),
                                       [](const uno::Sequence< double >& aColor) {
                                           return vcl::unotools::stdColorSpaceSequenceToColor( aColor );
                                       } );

                        // TODO(E1): Return value
                        // TODO(F1): FillRule
                        gradientFill( mpOutDevProvider->getOutDev(),
                                      mp2ndOutDevProvider ? &mp2ndOutDevProvider->getOutDev() : nullptr,
                                      rValues,
                                      aColors,
                                      aPolyPoly,
                                      viewState,
                                      renderState,
                                      textures[0],
                                      nTransparency );
                    }
                }
                else
                {
                    // TODO(F1): The generic case is missing here
                    ENSURE_OR_THROW( false,
                                      "CanvasHelper::fillTexturedPolyPolygon(): unknown parametric polygon encountered" );
                }
            }
            else if( textures[0].Bitmap.is() )
            {
                geometry::IntegerSize2D aBmpSize( textures[0].Bitmap->getSize() );

                ENSURE_ARG_OR_THROW( aBmpSize.Width != 0 &&
                                 aBmpSize.Height != 0,
                                 "CanvasHelper::fillTexturedPolyPolygon(): zero-sized texture bitmap" );

                // determine maximal bound rect of texture-filled
                // polygon
                const ::tools::Rectangle aPolygonDeviceRect(
                    aPolyPoly.GetBoundRect() );


                // first of all, determine whether we have a
                // drawBitmap() in disguise
                // =========================================

                const bool bRectangularPolygon( tools::isRectangle( aPolyPoly ) );

                ::basegfx::B2DHomMatrix aTotalTransform;
                ::canvas::tools::mergeViewAndRenderTransform(aTotalTransform,
                                                             viewState,
                                                             renderState);
                ::basegfx::B2DHomMatrix aTextureTransform;
                ::basegfx::unotools::homMatrixFromAffineMatrix( aTextureTransform,
                                                                textures[0].AffineTransform );

                aTotalTransform *= aTextureTransform;

                const ::basegfx::B2DRectangle aRect(0.0, 0.0, 1.0, 1.0);
                ::basegfx::B2DRectangle aTextureDeviceRect;
                ::canvas::tools::calcTransformedRectBounds( aTextureDeviceRect,
                                                            aRect,
                                                            aTotalTransform );

                const ::tools::Rectangle aIntegerTextureDeviceRect(
                    vcl::unotools::rectangleFromB2DRectangle( aTextureDeviceRect ) );

                if( bRectangularPolygon &&
                    aIntegerTextureDeviceRect == aPolygonDeviceRect )
                {
                    rendering::RenderState aLocalState( renderState );
                    ::canvas::tools::appendToRenderState(aLocalState,
                                                         aTextureTransform);
                    ::basegfx::B2DHomMatrix aScaleCorrection;
                    aScaleCorrection.scale( 1.0/aBmpSize.Width,
                                            1.0/aBmpSize.Height );
                    ::canvas::tools::appendToRenderState(aLocalState,
                                                         aScaleCorrection);

                    // need alpha modulation?
                    if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                   1.0 ) )
                    {
                        // setup alpha modulation values
                        aLocalState.DeviceColor.realloc(4);
                        double* pColor = aLocalState.DeviceColor.getArray();
                        pColor[0] =
                        pColor[1] =
                        pColor[2] = 0.0;
                        pColor[3] = textures[0].Alpha;

                        return drawBitmapModulated( pCanvas,
                                                    textures[0].Bitmap,
                                                    viewState,
                                                    aLocalState );
                    }
                    else
                    {
                        return drawBitmap( pCanvas,
                                           textures[0].Bitmap,
                                           viewState,
                                           aLocalState );
                    }
                }
                else
                {
                    // No easy mapping to drawBitmap() - calculate
                    // texturing parameters
                    // ===========================================

                    BitmapEx aBmpEx( tools::bitmapExFromXBitmap( textures[0].Bitmap ) );

                    // scale down bitmap to [0,1]x[0,1] rect, as required
                    // from the XCanvas interface.
                    ::basegfx::B2DHomMatrix aScaling;
                    ::basegfx::B2DHomMatrix aPureTotalTransform; // pure view*render*texture transform
                    aScaling.scale( 1.0/aBmpSize.Width,
                                    1.0/aBmpSize.Height );

                    aTotalTransform = aTextureTransform * aScaling;
                    aPureTotalTransform = aTextureTransform;

                    // combine with view and render transform
                    ::basegfx::B2DHomMatrix aMatrix;
                    ::canvas::tools::mergeViewAndRenderTransform(aMatrix, viewState, renderState);

                    // combine all three transformations into one
                    // global texture-to-device-space transformation
                    aTotalTransform *= aMatrix;
                    aPureTotalTransform *= aMatrix;

                    // analyze transformation, and setup an
                    // appropriate GraphicObject
                    ::basegfx::B2DVector aScale;
                    ::basegfx::B2DPoint  aOutputPos;
                    double               nRotate;
                    double               nShearX;
                    aTotalTransform.decompose( aScale, aOutputPos, nRotate, nShearX );

                    GraphicAttr             aGrfAttr;
                    GraphicObjectSharedPtr  pGrfObj;

                    if( ::basegfx::fTools::equalZero( nShearX ) )
                    {
                        // no shear, GraphicObject is enough (the
                        // GraphicObject only supports scaling, rotation
                        // and translation)

                        // #i75339# don't apply mirror flags, having
                        // negative size values is enough to make
                        // GraphicObject flip the bitmap

                        // The angle has to be mapped from radian to tenths of
                        // degrees with the orientation reversed: [0,2Pi) ->
                        // (3600,0].  Note that the original angle may have
                        // values outside the [0,2Pi) interval.
                        const double nAngleInTenthOfDegrees (3600.0 - basegfx::rad2deg<10>(nRotate));
                        aGrfAttr.SetRotation( Degree10(::basegfx::fround(nAngleInTenthOfDegrees)) );

                        pGrfObj = std::make_shared<GraphicObject>( aBmpEx );
                    }
                    else
                    {
                        // modify output position, to account for the fact
                        // that transformBitmap() always normalizes its output
                        // bitmap into the smallest enclosing box.
                        ::basegfx::B2DRectangle aDestRect;
                        ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                                    ::basegfx::B2DRectangle(0,
                                                                                            0,
                                                                                            aBmpSize.Width,
                                                                                            aBmpSize.Height),
                                                                    aMatrix );

                        aOutputPos.setX( aDestRect.getMinX() );
                        aOutputPos.setY( aDestRect.getMinY() );

                        // complex transformation, use generic affine bitmap
                        // transformation
                        aBmpEx = tools::transformBitmap( aBmpEx,
                                                         aTotalTransform);

                        pGrfObj = std::make_shared<GraphicObject>( aBmpEx );

                        // clear scale values, generated bitmap already
                        // contains scaling
                        aScale.setX( 1.0 ); aScale.setY( 1.0 );

                        // update bitmap size, bitmap has changed above.
                        aBmpSize = vcl::unotools::integerSize2DFromSize(aBmpEx.GetSizePixel());
                    }


                    // render texture tiled into polygon
                    // =================================

                    // calc device space direction vectors. We employ
                    // the following approach for tiled output: the
                    // texture bitmap is output in texture space
                    // x-major order, i.e. tile neighbors in texture
                    // space x direction are rendered back-to-back in
                    // device coordinate space (after the full device
                    // transformation). Thus, the aNextTile* vectors
                    // denote the output position updates in device
                    // space, to get from one tile to the next.
                    ::basegfx::B2DVector aNextTileX( 1.0, 0.0 );
                    ::basegfx::B2DVector aNextTileY( 0.0, 1.0 );
                    aNextTileX *= aPureTotalTransform;
                    aNextTileY *= aPureTotalTransform;

                    ::basegfx::B2DHomMatrix aInverseTextureTransform( aPureTotalTransform );

                    ENSURE_ARG_OR_THROW( aInverseTextureTransform.isInvertible(),
                                     "CanvasHelper::fillTexturedPolyPolygon(): singular texture matrix" );

                    aInverseTextureTransform.invert();

                    // calc bound rect of extended texture area in
                    // device coordinates. Therefore, we first calc
                    // the area of the polygon bound rect in texture
                    // space. To maintain texture phase, this bound
                    // rect is then extended to integer coordinates
                    // (extended, because shrinking might leave some
                    // inner polygon areas unfilled).
                    // Finally, the bound rect is transformed back to
                    // device coordinate space, were we determine the
                    // start point from it.
                    ::basegfx::B2DRectangle aTextureSpacePolygonRect;
                    ::canvas::tools::calcTransformedRectBounds( aTextureSpacePolygonRect,
                                                                vcl::unotools::b2DRectangleFromRectangle(aPolygonDeviceRect),
                                                                aInverseTextureTransform );

                    // calc left, top of extended polygon rect in
                    // texture space, create one-texture instance rect
                    // from it (i.e. rect from start point extending
                    // 1.0 units to the right and 1.0 units to the
                    // bottom). Note that the rounding employed here
                    // is a bit subtle, since we need to round up/down
                    // as _soon_ as any fractional amount is
                    // encountered. This is to ensure that the full
                    // polygon area is filled with texture tiles.
                    const sal_Int32 nX1( ::canvas::tools::roundDown( aTextureSpacePolygonRect.getMinX() ) );
                    const sal_Int32 nY1( ::canvas::tools::roundDown( aTextureSpacePolygonRect.getMinY() ) );
                    const sal_Int32 nX2( ::canvas::tools::roundUp( aTextureSpacePolygonRect.getMaxX() ) );
                    const sal_Int32 nY2( ::canvas::tools::roundUp( aTextureSpacePolygonRect.getMaxY() ) );
                    const ::basegfx::B2DRectangle aSingleTextureRect(
                        nX1, nY1,
                        nX1 + 1.0,
                        nY1 + 1.0 );

                    // and convert back to device space
                    ::basegfx::B2DRectangle aSingleDeviceTextureRect;
                    ::canvas::tools::calcTransformedRectBounds( aSingleDeviceTextureRect,
                                                                aSingleTextureRect,
                                                                aPureTotalTransform );

                    const ::Point aPtRepeat( vcl::unotools::pointFromB2DPoint(
                                                 aSingleDeviceTextureRect.getMinimum() ) );
                    const ::Size  aSz( ::basegfx::fround( aScale.getX() * aBmpSize.Width ),
                                       ::basegfx::fround( aScale.getY() * aBmpSize.Height ) );
                    const ::Size  aIntegerNextTileX( vcl::unotools::sizeFromB2DSize(aNextTileX) );
                    const ::Size  aIntegerNextTileY( vcl::unotools::sizeFromB2DSize(aNextTileY) );

                    const ::Point aPt( textures[0].RepeatModeX == rendering::TexturingMode::NONE ?
                                       ::basegfx::fround( aOutputPos.getX() ) : aPtRepeat.X(),
                                       textures[0].RepeatModeY == rendering::TexturingMode::NONE ?
                                       ::basegfx::fround( aOutputPos.getY() ) : aPtRepeat.Y() );
                    const sal_Int32 nTilesX( textures[0].RepeatModeX == rendering::TexturingMode::NONE ?
                                             1 : nX2 - nX1 );
                    const sal_Int32 nTilesY( textures[0].RepeatModeX == rendering::TexturingMode::NONE ?
                                             1 : nY2 - nY1 );

                    OutputDevice& rOutDev( mpOutDevProvider->getOutDev() );

                    if( bRectangularPolygon )
                    {
                        // use optimized output path


                        // this distinction really looks like a
                        // micro-optimization, but in fact greatly speeds up
                        // especially complex fills. That's because when using
                        // clipping, we can output polygons instead of
                        // poly-polygons, and don't have to output the gradient
                        // twice for XOR

                        // setup alpha modulation
                        if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                       1.0 ) )
                        {
                            // TODO(F1): Note that the GraphicManager has
                            // a subtle difference in how it calculates
                            // the resulting alpha value: it's using the
                            // inverse alpha values (i.e. 'transparency'),
                            // and calculates transOrig + transModulate,
                            // instead of transOrig + transModulate -
                            // transOrig*transModulate (which would be
                            // equivalent to the origAlpha*modulateAlpha
                            // the DX canvas performs)
                            aGrfAttr.SetAlpha(
                                static_cast< sal_uInt8 >(
                                    ::basegfx::fround( 255.0 * textures[0].Alpha ) ) );
                        }

                        rOutDev.IntersectClipRegion( aPolygonDeviceRect );
                        textureFill( rOutDev,
                                     *pGrfObj,
                                     aPt,
                                     aIntegerNextTileX,
                                     aIntegerNextTileY,
                                     nTilesX,
                                     nTilesY,
                                     aSz,
                                     aGrfAttr );

                        if( mp2ndOutDevProvider )
                        {
                            OutputDevice& r2ndOutDev( mp2ndOutDevProvider->getOutDev() );
                            r2ndOutDev.IntersectClipRegion( aPolygonDeviceRect );
                            textureFill( r2ndOutDev,
                                         *pGrfObj,
                                         aPt,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );
                        }
                    }
                    else
                    {
                        // output texture the hard way: XORing out the
                        // polygon
                        // ===========================================

                        if( !::rtl::math::approxEqual( textures[0].Alpha,
                                                       1.0 ) )
                        {
                            // uh-oh. alpha blending is required,
                            // cannot do direct XOR, but have to
                            // prepare the filled polygon within a
                            // VDev
                            ScopedVclPtrInstance< VirtualDevice > pVDev( rOutDev );
                            pVDev->SetOutputSizePixel( aPolygonDeviceRect.GetSize() );

                            // shift output to origin of VDev
                            const ::Point aOutPos( aPt - aPolygonDeviceRect.TopLeft() );
                            aPolyPoly.Translate( ::Point( -aPolygonDeviceRect.Left(),
                                                          -aPolygonDeviceRect.Top() ) );

                            const vcl::Region aPolyClipRegion( aPolyPoly );

                            pVDev->SetClipRegion( aPolyClipRegion );
                            textureFill( *pVDev,
                                         *pGrfObj,
                                         aOutPos,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );

                            // output VDev content alpha-blended to
                            // target position.
                            const ::Point aEmptyPoint;
                            BitmapEx aContentBmp(
                                pVDev->GetBitmapEx( aEmptyPoint,
                                                 pVDev->GetOutputSizePixel() ) );

                            sal_uInt8 nCol( static_cast< sal_uInt8 >(
                                           ::basegfx::fround( 255.0* textures[0].Alpha ) ) );
                            AlphaMask aAlpha( pVDev->GetOutputSizePixel(),
                                              &nCol );

                            BitmapEx aOutputBmpEx( aContentBmp.GetBitmap(), aAlpha );
                            rOutDev.DrawBitmapEx( aPolygonDeviceRect.TopLeft(),
                                                  aOutputBmpEx );

                            if( mp2ndOutDevProvider )
                                mp2ndOutDevProvider->getOutDev().DrawBitmapEx( aPolygonDeviceRect.TopLeft(),
                                                                       aOutputBmpEx );
                        }
                        else
                        {
                            const vcl::Region aPolyClipRegion( aPolyPoly );

                            rOutDev.Push( vcl::PushFlags::CLIPREGION );
                            rOutDev.IntersectClipRegion( aPolyClipRegion );

                            textureFill( rOutDev,
                                         *pGrfObj,
                                         aPt,
                                         aIntegerNextTileX,
                                         aIntegerNextTileY,
                                         nTilesX,
                                         nTilesY,
                                         aSz,
                                         aGrfAttr );
                            rOutDev.Pop();

                            if( mp2ndOutDevProvider )
                            {
                                OutputDevice& r2ndOutDev( mp2ndOutDevProvider->getOutDev() );
                                r2ndOutDev.Push( vcl::PushFlags::CLIPREGION );

                                r2ndOutDev.IntersectClipRegion( aPolyClipRegion );
                                textureFill( r2ndOutDev,
                                             *pGrfObj,
                                             aPt,
                                             aIntegerNextTileX,
                                             aIntegerNextTileY,
                                             nTilesX,
                                             nTilesY,
                                             aSz,
                                             aGrfAttr );
                                r2ndOutDev.Pop();
                            }
                        }
                    }
                }
            }
        }

        // TODO(P1): Provide caching here.
        return uno::Reference< rendering::XCachedPrimitive >(nullptr);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
