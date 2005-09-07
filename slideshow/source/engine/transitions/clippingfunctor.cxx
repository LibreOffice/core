/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: clippingfunctor.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:49:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <canvas/debug.hxx>
#include "clippingfunctor.hxx"
#include "transitiontools.hxx"

namespace presentation
{
    namespace internal
    {
        ClippingFunctor::ClippingFunctor(const ParametricPolyPolygonSharedPtr&   rPolygon,
                                         const TransitionInfo&                   rTransitionInfo,
                                         bool                                    bDirectionForward,
                                         bool                                    bModeIn ) :
            mpParametricPoly( rPolygon ),
            maStaticTransformation(),
            maBackgroundRect( createUnitRect() ),
            mbForwardParameterSweep( true ),
            mbSubtractPolygon( false ),
            mbScaleIsotrophically( rTransitionInfo.mbScaleIsotrophically ),
            mbFlip(false)
        {
            ENSURE_AND_THROW( rPolygon.get(),
                              "ClippingFunctor::ClippingFunctor(): Invalid parametric polygon" );

            // maBackgroundRect serves as the minuent when
            // subtracting a given clip polygon from the
            // background. To speed up the clipper algo, avoid
            // actual intersections of the generated
            // poly-polygon with the minuent - i.e. choose the
            // polygon to subtract from sufficiently large.

            // blow up unit rect to (-1,-1),(2,2)
            ::basegfx::B2DHomMatrix aMatrix;
            aMatrix.scale(3.0,3.0);
            aMatrix.translate(-1.0,-1.0);
            maBackgroundRect.transform( aMatrix );

            // extract modification info from maTransitionInfo
            // -----------------------------------------------

            // perform general transformations _before_ the reverse
            // mode changes. This allows the Transition table to be
            // filled more constitently (otherwise, when e.g. rotating
            // a clip 90 degrees, the REVERSEMETHOD_FLIP_X becomes
            // REVERSEMETHOD_FLIP_Y instead)
            if (rTransitionInfo.mnRotationAngle != 0.0 ||
                rTransitionInfo.mnScaleX != 1.0 ||
                rTransitionInfo.mnScaleY != 1.0)
            {
                maStaticTransformation.translate( -0.5, -0.5 );
                // apply further transformations:
                if (rTransitionInfo.mnRotationAngle != 0.0)
                {
                    maStaticTransformation.rotate(
                        rTransitionInfo.mnRotationAngle*F_PI180 );
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
                        ENSURE_AND_THROW(
                            false,
                            "TransitionFactory::TransitionFactory(): Unexpected reverse method" );
                        break;

                    case TransitionInfo::REVERSEMETHOD_IGNORE:
                        break;

                    case TransitionInfo::REVERSEMETHOD_INVERT_SWEEP:
                        mbForwardParameterSweep = !mbForwardParameterSweep;
                        break;

                    case TransitionInfo::REVERSEMETHOD_SUBTRACT_POLYGON:
                        mbSubtractPolygon = !mbSubtractPolygon;
                        break;

                    case TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT:
                        mbForwardParameterSweep = !mbForwardParameterSweep;
                        mbSubtractPolygon = !mbSubtractPolygon;
                        break;

                    case TransitionInfo::REVERSEMETHOD_ROTATE_180:
                        maStaticTransformation.translate( -0.5, -0.5 );
                        maStaticTransformation.rotate( F_PI );
                        maStaticTransformation.translate( 0.5, 0.5 );
                        break;

                    case TransitionInfo::REVERSEMETHOD_FLIP_X:
                        maStaticTransformation.scale( -1.0, 1.0 );
                        maStaticTransformation.translate( 1.0, 0.0 );
                        mbFlip = true;
                        break;

                    case TransitionInfo::REVERSEMETHOD_FLIP_Y:
                        maStaticTransformation.scale( 1.0, -1.0 );
                        maStaticTransformation.translate( 0.0, 1.0 );
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
            if( aClipPoly.areControlPointsUsed() )
                aClipPoly = ::basegfx::tools::adaptiveSubdivideByAngle(aClipPoly);
            if( mbSubtractPolygon )
            {
                // subtract given polygon from background
                // rect. Do that before any transformations.

                // calc maBackgroundRect \ aClipPoly
                // =================================

                aClipPoly = ::basegfx::tools::correctOrientations( aClipPoly );
                aClipPoly = ::basegfx::tools::removeAllIntersections(aClipPoly);
                aClipPoly = ::basegfx::tools::removeNeutralPolygons(aClipPoly, sal_True);
                aClipPoly.flip();
                ::basegfx::B2DPolyPolygon aTmp( maBackgroundRect );
                ::std::swap( aClipPoly, aTmp );
                aClipPoly.append( aTmp );

                // TODO(P1): If former aClipPoly is _strictly_ inside
                // maBackgroundRect, no need to remove intersections
                // (but this optimization strictly speaking belongs
                // into removeIntersections...)
                aClipPoly = ::basegfx::tools::removeAllIntersections(aClipPoly);
                aClipPoly = ::basegfx::tools::removeNeutralPolygons(aClipPoly, sal_True);

                // #72995# one more call to resolve self intersections which
                // may have been built by substracting (see bug)
                //aMergePolyPolygonA.Merge(FALSE);
                aClipPoly = ::basegfx::tools::removeAllIntersections(aClipPoly);
                aClipPoly = ::basegfx::tools::removeNeutralPolygons(aClipPoly, sal_True);
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
