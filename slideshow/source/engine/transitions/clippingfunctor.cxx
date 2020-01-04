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


#include <tools/diagnose_ex.h>
#include "clippingfunctor.hxx"
#include "transitiontools.hxx"

#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

namespace slideshow
{
    namespace internal
    {
        ClippingFunctor::ClippingFunctor(const ParametricPolyPolygonSharedPtr&   rPolygon,
                                         const TransitionInfo&                   rTransitionInfo,
                                         bool                                    bDirectionForward,
                                         bool                                    bModeIn ) :
            mpParametricPoly( rPolygon ),
            maStaticTransformation(),
            mbForwardParameterSweep( true ),
            mbSubtractPolygon( false ),
            mbScaleIsotrophically( rTransitionInfo.mbScaleIsotrophically ),
            mbFlip(false)
        {
            ENSURE_OR_THROW( rPolygon,
                              "ClippingFunctor::ClippingFunctor(): Invalid parametric polygon" );

            // maBackgroundRect serves as the minuent when
            // subtracting a given clip polygon from the
            // background. To speed up the clipper algo, avoid
            // actual intersections of the generated
            // poly-polygon with the minuent - i.e. choose the
            // polygon to subtract from sufficiently large.

            // blow up unit rect to (-1,-1),(2,2)
            // AW: Not needed, just use range
            // ::basegfx::B2DHomMatrix aMatrix;
            // aMatrix.scale(3.0,3.0);
            // aMatrix.translate(-1.0,-1.0);
            // maBackgroundRect.transform( aMatrix );

            // extract modification info from maTransitionInfo


            // perform general transformations _before_ the reverse
            // mode changes. This allows the Transition table to be
            // filled more consistently (otherwise, when e.g. rotating
            // a clip 90 degrees, the ReverseMethod::FlipX becomes
            // ReverseMethod::FlipY instead)
            if (rTransitionInfo.mnRotationAngle != 0.0 ||
                rTransitionInfo.mnScaleX != 1.0 ||
                rTransitionInfo.mnScaleY != 1.0)
            {
                maStaticTransformation.translate( -0.5, -0.5 );
                // apply further transformations:
                if (rTransitionInfo.mnRotationAngle != 0.0)
                {
                    maStaticTransformation.rotate(
                        basegfx::deg2rad(rTransitionInfo.mnRotationAngle) );
                }
                if (rTransitionInfo.mnScaleX != 1.0 ||
                    rTransitionInfo.mnScaleY != 1.0)
                {
                    maStaticTransformation.scale(
                        rTransitionInfo.mnScaleX,
                        rTransitionInfo.mnScaleY );
                }
                maStaticTransformation.translate( 0.5, 0.5 );
            }

            if( !bDirectionForward )
            {
                // Client has requested reversed
                // direction. Apply TransitionInfo's choice
                // for that
                switch( rTransitionInfo.meReverseMethod )
                {
                    default:
                        ENSURE_OR_THROW(
                            false,
                            "TransitionFactory::TransitionFactory(): Unexpected reverse method" );
                        break;

                    case TransitionInfo::ReverseMethod::Ignore:
                        break;

                    case TransitionInfo::ReverseMethod::SubtractAndInvert:
                        mbForwardParameterSweep = !mbForwardParameterSweep;
                        mbSubtractPolygon = !mbSubtractPolygon;
                        break;

                    case TransitionInfo::ReverseMethod::Rotate180:
                        maStaticTransformation = basegfx::utils::createRotateAroundPoint(0.5, 0.5, M_PI)
                            * maStaticTransformation;
                        break;

                    case TransitionInfo::ReverseMethod::FlipX:
                        maStaticTransformation = basegfx::utils::createScaleTranslateB2DHomMatrix(-1.0, 1.0, 1.0, 0.0)
                            * maStaticTransformation;
                        mbFlip = true;
                        break;

                    case TransitionInfo::ReverseMethod::FlipY:
                        maStaticTransformation = basegfx::utils::createScaleTranslateB2DHomMatrix(1.0, -1.0, 0.0, 1.0)
                            * maStaticTransformation;
                        mbFlip = true;
                        break;
                }
            }

            if( !bModeIn )
            {
                // client has requested 'out' mode. Apply
                // TransitionInfo's method of choice
                if( rTransitionInfo.mbOutInvertsSweep )
                    mbForwardParameterSweep = !mbForwardParameterSweep;
                else
                    mbSubtractPolygon = !mbSubtractPolygon;
            }
        }

        ::basegfx::B2DPolyPolygon ClippingFunctor::operator()( double                       nValue,
                                                               const ::basegfx::B2DSize&    rTargetSize )
        {
            // modify clip polygon according to static
            // transformation plus current shape size
            ::basegfx::B2DHomMatrix aMatrix( maStaticTransformation );

            // retrieve current clip polygon
            ::basegfx::B2DPolyPolygon aClipPoly = (*mpParametricPoly)(
                mbForwardParameterSweep ? nValue : 1.0 - nValue );

            // TODO(Q4): workaround here, better be fixed in cppcanvas
            if (aClipPoly.count() == 0)
                aClipPoly.append( basegfx::B2DPolygon() );

            if (mbFlip)
                aClipPoly.flip();

            // currently, clipper cannot cope with curves. Subdivide first
            // AW: Should be no longer necessary; clipping tools are now bezier-safe
            // if( aClipPoly.areControlPointsUsed() )
            //    aClipPoly = ::basegfx::utils::adaptiveSubdivideByAngle(aClipPoly);

            if( mbSubtractPolygon )
            {
                // subtract given polygon from background
                // rect. Do that before any transformations.

                // calc maBackgroundRect \ aClipPoly
                // =================================

                // AW: Simplified
                // use a range with fixed size (-1,-1),(2,2)
                const basegfx::B2DRange aBackgroundRange(-1, -1, 2, 2);
                const basegfx::B2DRange aClipPolyRange(aClipPoly.getB2DRange());

                if(aBackgroundRange.isInside(aClipPolyRange))
                {
                    // combine polygons; make the clip polygon the hole
                    aClipPoly = ::basegfx::utils::correctOrientations(aClipPoly);
                    aClipPoly.flip();
                    aClipPoly.insert(0, basegfx::utils::createPolygonFromRect(aBackgroundRange));
                }
                else
                {
                    // when not completely inside aBackgroundRange clipping is needed
                    // subtract aClipPoly from aBackgroundRange
                    const basegfx::B2DPolyPolygon aBackgroundPolyPoly(basegfx::utils::createPolygonFromRect(aBackgroundRange));
                    aClipPoly = basegfx::utils::solvePolygonOperationDiff(aBackgroundPolyPoly, aClipPoly);
                }
            }

            // scale polygon up to current shape size
            if( mbScaleIsotrophically )
            {
                const double nScale( ::std::max( rTargetSize.getX(),
                                                 rTargetSize.getY() ) );
                aMatrix.scale( nScale, nScale );
                aMatrix.translate( -(nScale-rTargetSize.getX())/2.0,
                                   -(nScale-rTargetSize.getY())/2.0 );
            }
            else
            {
                aMatrix.scale( rTargetSize.getX(),
                               rTargetSize.getY() );
            }

            // apply cumulative transformation to clip polygon
            aClipPoly.transform( aMatrix );

            return aClipPoly;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
