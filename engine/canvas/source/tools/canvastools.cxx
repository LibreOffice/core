/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <limits>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <cppuhelper/implbase.hxx>
#include <sal/log.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/window.hxx>

#include <canvas/canvastools.hxx>
#include <unopolypolygon.hxx>


using namespace ::com::sun::star;

namespace canvastools
{
        rendering::RenderState& initRenderState( rendering::RenderState& renderState )
        {
            // setup identity transform
            setIdentityAffineMatrix2D( renderState.AffineTransform );
            renderState.Clip.clear();
            renderState.DeviceColor = cpo::uno::Sequence< double >();

            return renderState;
        }

        rendering::ViewState& initViewState( rendering::ViewState& viewState )
        {
            // setup identity transform
            setIdentityAffineMatrix2D( viewState.AffineTransform );

            return viewState;
        }

        ::basegfx::B2DHomMatrix getViewStateTransform( const rendering::ViewState& viewState )
        {
            ::basegfx::B2DHomMatrix aTransform;
            return ::basegfx::unotools::homMatrixFromAffineMatrix( aTransform, viewState.AffineTransform );
        }

        rendering::ViewState& setViewStateTransform( rendering::ViewState&          viewState,
                                                     const ::basegfx::B2DHomMatrix& transform )
        {
            ::basegfx::unotools::affineMatrixFromHomMatrix( viewState.AffineTransform, transform );

            return viewState;
        }

        ::basegfx::B2DHomMatrix getRenderStateTransform( const rendering::RenderState& renderState )
        {
            ::basegfx::B2DHomMatrix aTransform;
            return ::basegfx::unotools::homMatrixFromAffineMatrix( aTransform, renderState.AffineTransform );
        }

        rendering::RenderState& setRenderStateTransform( rendering::RenderState&        renderState,
                                                         const ::basegfx::B2DHomMatrix& transform )
        {
            ::basegfx::unotools::affineMatrixFromHomMatrix( renderState.AffineTransform, transform );

            return renderState;
        }

        rendering::RenderState& appendToRenderState( rendering::RenderState&        renderState,
                                                   const ::basegfx::B2DHomMatrix&   rTransform )
        {
            ::basegfx::B2DHomMatrix transform = getRenderStateTransform( renderState );
            return setRenderStateTransform( renderState, transform * rTransform );
        }

        rendering::RenderState& prependToRenderState( rendering::RenderState&           renderState,
                                                      const ::basegfx::B2DHomMatrix&    rTransform )
        {
            ::basegfx::B2DHomMatrix transform = getRenderStateTransform( renderState );
            return setRenderStateTransform( renderState, rTransform * transform );
        }

        ::basegfx::B2DHomMatrix& mergeViewAndRenderTransform( ::basegfx::B2DHomMatrix&      combinedTransform,
                                                              const rendering::ViewState&   viewState,
                                                              const rendering::RenderState& renderState )
        {
            ::basegfx::B2DHomMatrix viewTransform;

            ::basegfx::unotools::homMatrixFromAffineMatrix( combinedTransform, renderState.AffineTransform );
            ::basegfx::unotools::homMatrixFromAffineMatrix( viewTransform, viewState.AffineTransform );

            // this statement performs combinedTransform = viewTransform * combinedTransform
            combinedTransform *= viewTransform;

            return combinedTransform;
        }

        geometry::AffineMatrix2D& setIdentityAffineMatrix2D( geometry::AffineMatrix2D& matrix )
        {
            matrix.m00 = 1.0;
            matrix.m01 = 0.0;
            matrix.m02 = 0.0;
            matrix.m10 = 0.0;
            matrix.m11 = 1.0;
            matrix.m12 = 0.0;

            return matrix;
        }

        geometry::Matrix2D& setIdentityMatrix2D( geometry::Matrix2D& matrix )
        {
            matrix.m00 = 1.0;
            matrix.m01 = 0.0;
            matrix.m10 = 0.0;
            matrix.m11 = 1.0;

            return matrix;
        }

        // Create a corrected view transformation out of the give one,
        // which ensures that the rectangle given by (0,0) and
        // rSpriteSize is mapped with its left,top corner to (0,0)
        // again. This is required to properly render sprite
        // animations to buffer bitmaps.
        ::basegfx::B2DHomMatrix calcRectToOriginTransform( const ::basegfx::B2DRange&          i_srcRect,
                                                           const ::basegfx::B2DHomMatrix&      i_transformation )
        {
            ::basegfx::B2DHomMatrix o_transform;
            if( i_srcRect.isEmpty() )
            {
                o_transform = i_transformation;
                return o_transform;
            }

            // transform by given transformation
            ::basegfx::B2DRectangle aTransformedRect = calcTransformedRectBounds(
                                       i_srcRect,
                                       i_transformation );

            // now move resulting left,top point of bounds to (0,0)
            const basegfx::B2DHomMatrix aCorrectedTransform(basegfx::utils::createTranslateB2DHomMatrix(
                -aTransformedRect.getMinX(), -aTransformedRect.getMinY()));

            // prepend to original transformation
            o_transform = aCorrectedTransform * i_transformation;

            return o_transform;
        }

        ::basegfx::B2DRange calcTransformedRectBounds( const ::basegfx::B2DRange&      inRect,
                                                        const ::basegfx::B2DHomMatrix&  transformation )
        {
            ::basegfx::B2DRange outRect;

            if( inRect.isEmpty() )
                return outRect;

            // transform all four extremal points of the rectangle,
            // take bounding rect of those.

            // transform left-top point
            outRect.expand( transformation * inRect.getMinimum() );

            // transform bottom-right point
            outRect.expand( transformation * inRect.getMaximum() );

            ::basegfx::B2DPoint aPoint;

            // transform top-right point
            aPoint.setX( inRect.getMaxX() );
            aPoint.setY( inRect.getMinY() );

            aPoint *= transformation;
            outRect.expand( aPoint );

            // transform bottom-left point
            aPoint.setX( inRect.getMinX() );
            aPoint.setY( inRect.getMaxY() );

            aPoint *= transformation;
            outRect.expand( aPoint );

            // over and out.
            return outRect;
        }

        int calcGradientStepCount( ::basegfx::B2DHomMatrix&      rTotalTransform,
                                   const rendering::ViewState&   viewState,
                                   const rendering::RenderState& renderState,
                                   const rendering::Texture&     texture,
                                   int                           nColorSteps )
        {
            // calculate overall texture transformation (directly from
            // texture to device space).
            ::basegfx::B2DHomMatrix aMatrix;

            rTotalTransform.identity();
            ::basegfx::unotools::homMatrixFromAffineMatrix( rTotalTransform,
                                                            texture.AffineTransform );
            mergeViewAndRenderTransform(aMatrix, viewState, renderState);
            rTotalTransform *= aMatrix; // prepend total view/render transformation

            // determine size of gradient in device coordinate system
            // (to e.g. determine sensible number of gradient steps)
            ::basegfx::B2DPoint aLeftTop( 0.0, 0.0 );
            ::basegfx::B2DPoint aLeftBottom( 0.0, 1.0 );
            ::basegfx::B2DPoint aRightTop( 1.0, 0.0 );
            ::basegfx::B2DPoint aRightBottom( 1.0, 1.0 );

            aLeftTop    *= rTotalTransform;
            aLeftBottom *= rTotalTransform;
            aRightTop   *= rTotalTransform;
            aRightBottom*= rTotalTransform;

            // longest line in gradient bound rect
            const int nGradientSize(
                static_cast<int>(
                    std::max(
                        ::basegfx::B2DVector(aRightBottom-aLeftTop).getLength(),
                        ::basegfx::B2DVector(aRightTop-aLeftBottom).getLength() ) + 1.0 ) );

            // typical number for pixel of the same color (strip size)
            const int nStripSize( nGradientSize < 50 ? 2 : 4 );

            // use at least three steps, and at utmost the number of color
            // steps
            return std::max( 3,
                               std::min(
                                   nGradientSize / nStripSize,
                                   nColorSteps ) );
        }

        void clipOutDev(const rendering::ViewState& viewState,
                        const rendering::RenderState& renderState,
                        OutputDevice& rOutDev)
        {
            // accumulate non-empty clips into one region
            vcl::Region aClipRegion(true);

            if( renderState.Clip.is() )
            {
                ::basegfx::B2DPolyPolygon aClipPoly(
                    b2DPolyPolygonFromXPolyPolygon2D(renderState.Clip) );

                ::basegfx::B2DHomMatrix aMatrix;
                aClipPoly.transform(mergeViewAndRenderTransform(aMatrix, viewState, renderState));

                if( aClipPoly.count() )
                {
                    // setup non-empty clipping
                    vcl::Region aRegion = vcl::Region::GetRegionFromPolyPolygon( ::tools::PolyPolygon( aClipPoly ) );
                    aClipRegion.Intersect( aRegion );
                }
                else
                {
                    // clip polygon is empty
                    aClipRegion.SetEmpty();
                }
            }

            // setup accumulated clip region. Note that setting an
            // empty clip region denotes "clip everything" on the
            // OutputDevice (which is why we translate that into
            // SetClipRegion() here). When both view and render clip
            // are empty, aClipRegion remains default-constructed,
            // i.e. empty, too.
            if( aClipRegion.IsNull() )
                rOutDev.SetClipRegion();
            else
                rOutDev.SetClipRegion( aClipRegion );
        }

        void extractExtraFontProperties(const cpo::uno::Sequence<beans::PropertyValue>& rExtraFontProperties,
                        sal_uInt32 &rEmphasisMark)
        {
            for(const beans::PropertyValue& rPropVal : rExtraFontProperties)
            {
                if (rPropVal.Name == "EmphasisMark")
                    rPropVal.Value >>= rEmphasisMark;
            }
        }

        ::basegfx::B2DPolyPolygon b2DPolyPolygonFromXPolyPolygon2D( const uno::Reference< rendering::XPolyPolygon2D >& xPoly )
        {
            ::canvastools::UnoPolyPolygon* pPolyImpl =
                dynamic_cast< ::canvastools::UnoPolyPolygon* >( xPoly.get() );

            if( pPolyImpl )
            {
                return pPolyImpl->getPolyPolygon();
            }
            else
            {
                // not a known implementation object - try data source
                // interfaces
                const sal_Int32 nPolys( xPoly->getNumberOfPolygons() );

                uno::Reference< rendering::XBezierPolyPolygon2D > xBezierPoly(
                    xPoly,
                    uno::UNO_QUERY );

                if( xBezierPoly.is() )
                {
                    return ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence(
                        xBezierPoly->getBezierSegments( 0,
                                                        nPolys,
                                                        0,
                                                        -1 ) );
                }
                else
                {
                    uno::Reference< rendering::XLinePolyPolygon2D > xLinePoly(
                        xPoly,
                        uno::UNO_QUERY );

                    // no implementation class and no data provider
                    // found - contract violation.
                    if( !xLinePoly.is() )
                    {
                        throw lang::IllegalArgumentException(
                            u"basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(): Invalid input"
                            "poly-polygon, cannot retrieve vertex data"_ustr,
                            uno::Reference< uno::XInterface >(),
                            0 );
                    }

                    return ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence(
                        xLinePoly->getPoints( 0,
                                              nPolys,
                                              0,
                                              -1 ));
                }
            }
        }

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromB2DPolyPolygon( const uno::Reference< rendering::XGraphicDevice >&  xGraphicDevice,
                                                                                    const ::basegfx::B2DPolyPolygon&                    rPolyPoly    )
        {
            uno::Reference< rendering::XPolyPolygon2D > xRes;

            if( !xGraphicDevice.is() )
                return xRes;

            const sal_uInt32 nNumPolies( rPolyPoly.count() );
            sal_uInt32 i;

            if( rPolyPoly.areControlPointsUsed() )
            {
                xRes = xGraphicDevice->createCompatibleBezierPolyPolygon(
                              basegfx::unotools::bezierSequenceSequenceFromB2DPolyPolygon( rPolyPoly ) );
            }
            else
            {
                xRes = xGraphicDevice->createCompatibleLinePolyPolygon(
                              basegfx::unotools::pointSequenceSequenceFromB2DPolyPolygon( rPolyPoly ) );
            }

            for( i=0; i<nNumPolies; ++i )
            {
                xRes->setClosed( i, rPolyPoly.getB2DPolygon(i).isClosed() );
            }

            return xRes;
        }

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromB2DPolygon( const uno::Reference< rendering::XGraphicDevice >&  xGraphicDevice,
                                                                                const ::basegfx::B2DPolygon&                        rPoly    )
        {
            uno::Reference< rendering::XPolyPolygon2D > xRes;

            if( !xGraphicDevice.is() )
                return xRes;

            if( rPoly.areControlPointsUsed() )
            {
                cpo::uno::Sequence< cpo::uno::Sequence< geometry::RealBezierSegment2D > > outputSequence{ basegfx::unotools::bezierSequenceFromB2DPolygon( rPoly )};

                xRes = xGraphicDevice->createCompatibleBezierPolyPolygon( outputSequence );
            }
            else
            {
                cpo::uno::Sequence< cpo::uno::Sequence< geometry::RealPoint2D > > outputSequence{
                 basegfx::unotools::pointSequenceFromB2DPolygon( rPoly )};

                xRes = xGraphicDevice->createCompatibleLinePolyPolygon( outputSequence );
            }

            if( xRes.is() && rPoly.isClosed() )
                xRes->setClosed( 0, true );

            return xRes;
        }

        /// Convert [0,1] double value to [0,255] int
        static sal_Int8 toByteColor( double val )
        {
            return sal::static_int_cast<sal_Int8>(
                basegfx::fround(val*255.0));
        }

        /// Convert [0,255] int value to [0,1] double value
        static double toDoubleColor( sal_uInt8 val )
        {
            return val / 255.0;
        }
  
        cpo::uno::Sequence< double > colorToDoubleSequence( const Color& rColor )
        {
            return colorToStdColorSpaceSequence(rColor);
        }

        Color doubleSequenceToColor( const cpo::uno::Sequence< double >& rColor )
        {
            return stdColorSpaceSequenceToColor(rColor);
        }

        cpo::uno::Sequence< double > colorToStdColorSpaceSequence( const Color& rColor )
        {
            return
            {
                toDoubleColor(rColor.GetRed()),
                toDoubleColor(rColor.GetGreen()),
                toDoubleColor(rColor.GetBlue()),
                toDoubleColor(rColor.GetAlpha())
            };
        }

        Color stdColorSpaceSequenceToColor( const cpo::uno::Sequence< double >& rColor        )
        {
            ENSURE_ARG_OR_THROW( rColor.getLength() == 4,
                                 "color must have 4 channels" );

            Color aColor;

            aColor.SetRed  ( toByteColor(rColor[0]) );
            aColor.SetGreen( toByteColor(rColor[1]) );
            aColor.SetBlue ( toByteColor(rColor[2]) );
            aColor.SetAlpha( toByteColor(rColor[3]) );

            return aColor;
        }

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
