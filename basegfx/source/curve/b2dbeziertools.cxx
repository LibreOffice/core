/*************************************************************************
 *
 *  $RCSfile: b2dbeziertools.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2003-11-12 12:09:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <limits>
#include <algorithm>

#include <basegfx/curve/b2dbeziertools.hxx>

#ifndef _BGFX_CURVE_B2DCUBICBEZIER_HXX
#include <basegfx/curve/b2dcubicbezier.hxx>
#endif

#ifndef _BGFX_CURVE_B2DQUADRATICBEZIER_HXX
#include <basegfx/curve/b2dquadraticbezier.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif


namespace basegfx
{
    namespace curve
    {
        namespace
        {
            class DistanceErrorFunctor
            {
            public:
                DistanceErrorFunctor( const double& distance ) :
                    mfDistance2( distance*distance ),
                    mfLastDistanceError2( ::std::numeric_limits<double>::max() )
                {
                }

                bool subdivideFurther( const double& P1x, const double& P1y,
                                       const double& P2x, const double& P2y,
                                       const double& P3x, const double& P3y,
                                       const double& P4x, const double& P4y,
                                       const double&, const double& ) // last two values not used here
                {
                    // Perform bezier flatness test (lecture notes from R. Schaback,
                    // Mathematics of Computer-Aided Design, Uni Goettingen, 2000)
                    //
                    // ||P(t) - L(t)|| <= max     ||b_j - b_0 - j/n(b_n - b_0)||
                    //                    0<=j<=n
                    //
                    // What is calculated here is an upper bound to the distance from
                    // a line through b_0 and b_3 (P1 and P4 in our notation) and the
                    // curve. We can drop 0 and n from the running indices, since the
                    // argument of max becomes zero for those cases.
                    const double fJ1x( P2x - P1x - 1.0/3.0*(P4x - P1x) );
                    const double fJ1y( P2y - P1y - 1.0/3.0*(P4y - P1y) );
                    const double fJ2x( P3x - P1x - 2.0/3.0*(P4x - P1x) );
                    const double fJ2y( P3y - P1y - 2.0/3.0*(P4y - P1y) );
                    const double distanceError2( ::std::max( fJ1x*fJ1x + fJ1y*fJ1y,
                                                             fJ2x*fJ2x + fJ2y*fJ2y) );

                    // stop if error measure does not improve anymore. This is a
                    // safety guard against floating point inaccuracies.
                    // stop if distance from line is guaranteed to be bounded by d
                    bool bRet( mfLastDistanceError2 > distanceError2 &&
                               distanceError2 >= mfDistance2 );

                    mfLastDistanceError2 = distanceError2;

                    return bRet;
                }

            private:
                double mfDistance2;
                double mfLastDistanceError2;
            };


            class AngleErrorFunctor
            {
            public:
                AngleErrorFunctor( const double& angleBounds ) :
                    mfTanAngle( tan( angleBounds ) ),
                    mfLastTanAngle( ::std::numeric_limits<double>::max() )
                {
                }

                bool subdivideFurther( const double P1x, const double P1y,
                                       const double P2x, const double P2y,
                                       const double P3x, const double P3y,
                                       const double P4x, const double P4y,
                                       const double Pdx, const double Pdy )
                {
                    // Test angle differences between two lines (ad
                    // and bd), meeting in the t=0.5 division point
                    // (d), and the angle from the other ends of those
                    // lines (b and a, resp.) to the tangents to the
                    // curve at this points:
                    //
                    //        *__________
                    //             ......*b
                    //          ...
                    //        ..
                    //       .
                    // *    *d
                    // |   .
                    // |  .
                    // | .
                    // | .
                    // |.
                    // |.
                    // *
                    // a
                    //
                    // When using half of the angle bound for the
                    // difference to the tangents at a or b, resp.,
                    // this procedure guarantees that no angle in the
                    // resulting line polygon is larger than the
                    // specified angle bound. This is because during
                    // subdivision, adjacent curve segments will have
                    // collinear tangent vectors, thus, when each
                    // side's line segments differs by at most angle/2
                    // from that tangent, the summed difference will
                    // be at most angle (this was modeled after an
                    // idea from Armin Weiss).

                    // To stay within the notation above, a equals P1,
                    // the other end point of the tangent starting at
                    // a is P2, d is Pd, and so forth. The
                    const vector::B2DVector vecAD( Pdx - P1x, Pdy - P1y );
                    const vector::B2DVector vecDB( P4x - Pdx, P4y - Pdy );

                    const double scalarVecADDB( vecAD.scalar( vecDB ) );
                    const double crossVecADDB( vecAD.cross( vecDB ) );

                    const vector::B2DVector vecStartTangent( P2x - P1x, P2y - P1y );
                    const vector::B2DVector vecEndTangent( P4x - P3x, P4y - P3y );

                    const double scalarVecStartTangentAD( vecStartTangent.scalar( vecAD ) );
                    const double crossVecStartTangentAD( vecStartTangent.cross( vecAD ) );

                    const double scalarVecDBEndTangent( vecDB.scalar( vecEndTangent ) );
                    const double crossVecDBEndTangent( vecDB.cross( vecEndTangent ) );


                    double fCurrAngle( ::std::numeric_limits<double>::max() );

                    if( !numeric::fTools::equalZero( scalarVecADDB ) )
                        fCurrAngle = fabs( crossVecADDB / scalarVecADDB );

                    if( !numeric::fTools::equalZero( scalarVecStartTangentAD ) )
                        fCurrAngle = ::std::min( fCurrAngle, fabs( crossVecStartTangentAD / scalarVecStartTangentAD ) );

                    if( !numeric::fTools::equalZero( scalarVecDBEndTangent ) )
                        fCurrAngle = ::std::min( fCurrAngle, fabs( crossVecDBEndTangent / scalarVecDBEndTangent ) );

                    // stop if error measure does not improve anymore. This is a
                    // safety guard against floating point inaccuracies.
                    // stop if angle difference is guaranteed to be bounded by mfTanAngle
                    bool bRet( mfLastTanAngle > fCurrAngle &&
                               fCurrAngle >= mfTanAngle );

                    mfLastTanAngle = fCurrAngle;

                    return bRet;
                }

            private:
                double mfTanAngle;
                double mfLastTanAngle;
            };


            /* Recursively subdivide cubic bezier curve via deCasteljau.

               @param rPoly
               Polygon to append generated points to

               @param d2
               Maximal squared difference of curve to a straight line

               @param P*
               Exactly four points, interpreted as support and control points of
               a cubic bezier curve.

               @param old_distance2
               Last squared distance to line for this recursion
               path. Used as an end condition, if it is no longer
               improving.

               @param recursionDepth
               Depth of recursion. Used as a termination criterion, to
               prevent endless looping.
            */
            template < class ErrorFunctor > int ImplAdaptiveSubdivide( polygon::B2DPolygon&           rPoly,
                                                                       const ErrorFunctor&            rErrorFunctor,
                                                                       const double P1x, const double P1y,
                                                                       const double P2x, const double P2y,
                                                                       const double P3x, const double P3y,
                                                                       const double P4x, const double P4y,
                                                                       int                            recursionDepth )
            {
                // Hard limit on recursion depth, empiric number.
                enum {maxRecursionDepth=128};

                // deCasteljau bezier arc, split at t=0.5
                // Foley/vanDam, p. 508

                // Note that for the pure distance error method, this
                // subdivision could be moved into the if-branch. But
                // since this accounts for saved work only for the
                // very last subdivision step, and we need the
                // subdivided curve for the angle criterium, I think
                // it's justified here.
                const double L1x( P1x ),             L1y( P1y );
                const double L2x( (P1x + P2x)*0.5 ), L2y( (P1y + P2y)*0.5 );
                const double Hx ( (P2x + P3x)*0.5 ), Hy ( (P2y + P3y)*0.5 );
                const double L3x( (L2x + Hx)*0.5 ),  L3y( (L2y + Hy)*0.5 );
                const double R4x( P4x ),             R4y( P4y );
                const double R3x( (P3x + P4x)*0.5 ), R3y( (P3y + P4y)*0.5 );
                const double R2x( (Hx + R3x)*0.5 ),  R2y( (Hy + R3y)*0.5 );
                const double R1x( (L3x + R2x)*0.5 ), R1y( (L3y + R2y)*0.5 );
                const double L4x( R1x ),             L4y( R1y );

                // stop at recursion level 128. This is a safety guard against
                // floating point inaccuracies.
                if( recursionDepth < maxRecursionDepth &&
                    rErrorFunctor.subdivideFurther( P1x, P1y,
                                                    P2x, P2y,
                                                    P3x, P3y,
                                                    P4x, P4y,
                                                    R1x, R1y ) )
                {
                    // subdivide further
                    ++recursionDepth;

                    int nGeneratedPoints(0);

                    nGeneratedPoints += ImplAdaptiveSubdivide(rPoly, rErrorFunctor, L1x, L1y, L2x, L2y, L3x, L3y, L4x, L4y, recursionDepth);
                    nGeneratedPoints += ImplAdaptiveSubdivide(rPoly, rErrorFunctor, R1x, R1y, R2x, R2y, R3x, R3y, R4x, R4y, recursionDepth);

                    // return number of points generated in this
                    // recursion branch
                    return nGeneratedPoints;
                }
                else
                {
                    // requested resolution reached. Add end points to
                    // output iterator.  order is preserved, since
                    // this is so to say depth first traversal.
                    rPoly.append( point::B2DPoint( P1x, P1y ) );

                    // return number of points generated in this
                    // recursion branch
                    return 1;
                }
            }

// LATER
#if 0
            /* Approximate given cubic bezier curve by quadratic bezier segments */
            void ImplQuadBezierApprox( polygon::B2DPolygon&           rPoly,
                                       BitStream& rBits,
                                       Point& rLastPoint,
                                       const double d2,
                                       const double P1x, const double P1y,
                                       const double P2x, const double P2y,
                                       const double P3x, const double P3y,
                                       const double P4x, const double P4y )
            {
                // Check for degenerate case, where the given cubic bezier curve
                // is already quadratic: P4 == 3P3 - 3P2 + P1
                if( P4x == 3.0*P3x - 3.0*P2x + P1x &&
                    P4y == 3.0*P3y - 3.0*P2y + P1y )
                {
                    Impl_addQuadBezier( rBits, rLastPoint,
                                        3.0/2.0*P2x - 1.0/2.0*P1x, 3.0/2.0*P2y - 1.0/2.0*P1y,
                                        P4x, P4y);
                }
                else
                {
                    // Create quadratic segment for given cubic:
                    // Start and end point must coincide, determine quadratic control
                    // point in such a way that it lies on the intersection of the
                    // tangents at start and end point, resp. Thus, both cubic and
                    // quadratic curve segments will match in 0th and 1st derivative
                    // at the start and end points

                    // Intersection of P2P1 and P4P3
                    //           (P2y-P4y)(P3x-P4x)-(P2x-P4x)(P3y-P4y)
                    //  lambda = -------------------------------------
                    //           (P1x-P2x)(P3y-P4y)-(P1y-P2y)(P3x-P4x)
                    //
                    // Intersection point IP is now
                    // IP = P2 + lambda(P1-P2)
                    //
                    const double nominator( (P2y-P4y)*(P3x-P4x) - (P2x-P4x)*(P3y-P4y) );
                    const double denominator( (P1x-P2x)*(P3y-P4y) - (P1y-P2y)*(P3x-P4x) );
                    const double lambda( nominator / denominator );

                    const double IPx( P2x + lambda*( P1x - P2x) );
                    const double IPy( P2y + lambda*( P1y - P2y) );

                    // Introduce some alias names: quadratic start point is P1, end
                    // point is P4, control point is IP
                    const double QP1x( P1x );
                    const double QP1y( P1y );
                    const double QP2x( IPx );
                    const double QP2y( IPy );
                    const double QP3x( P4x );
                    const double QP3y( P4y );

                    // Adapted bezier flatness test (lecture notes from R. Schaback,
                    // Mathematics of Computer-Aided Design, Uni Goettingen, 2000)
                    //
                    // ||C(t) - Q(t)|| <= max     ||c_j - q_j||
                    //                    0<=j<=n
                    //
                    // In this case, we don't need the distance from the cubic bezier
                    // to a straight line, but to a quadratic bezier. The c_j's are
                    // the cubic bezier's bernstein coefficients, the q_j's the
                    // quadratic bezier's. We have the c_j's given, the q_j's can be
                    // calculated from QPi like this (sorry, mixed index notation, we
                    // use [1,n], formulas use [0,n-1]):
                    //
                    // q_0 = QP1 = P1
                    // q_1 = 1/3 QP1 + 2/3 QP2
                    // q_2 = 2/3 QP2 + 1/3 QP3
                    // q_3 = QP3 = P4
                    //
                    // We can drop case 0 and 3, since there the curves coincide
                    // (distance is zero)

                    // calculate argument of max for j=1 and j=2
                    const double fJ1x( P2x - 1.0/3.0*QP1x - 2.0/3.0*QP2x );
                    const double fJ1y( P2y - 1.0/3.0*QP1y - 2.0/3.0*QP2y );
                    const double fJ2x( P3x - 2.0/3.0*QP2x - 1.0/3.0*QP3x );
                    const double fJ2y( P3y - 2.0/3.0*QP2y - 1.0/3.0*QP3y );

                    // stop if distance from cubic curve is guaranteed to be bounded by d
                    // Should denominator be 0: then P1P2 and P3P4 are parallel (P1P2^T R[90,P3P4] = 0.0),
                    // meaning that either we have a straight line or an inflexion point (see else block below)
                    if( 0.0 != denominator &&
                        ::std::max( fJ1x*fJ1x + fJ1y*fJ1y,
                                    fJ2x*fJ2x + fJ2y*fJ2y) < d2 )
                    {
                        // requested resolution reached.
                        // Add end points to output file.
                        // order is preserved, since this is so to say depth first traversal.
                        Impl_addQuadBezier( rBits, rLastPoint,
                                            QP2x, QP2y,
                                            QP3x, QP3y);
                    }
                    else
                    {
                        // Maybe subdivide further

                        // This is for robustness reasons, since the line intersection
                        // method below gets instable if the curve gets closer to a
                        // straight line. If the given cubic bezier does not deviate by
                        // more than d/4 from a straight line, either:
                        //  - take the line (that's what we do here)
                        //  - express the line by a quadratic bezier

                        // Perform bezier flatness test (lecture notes from R. Schaback,
                        // Mathematics of Computer-Aided Design, Uni Goettingen, 2000)
                        //
                        // ||P(t) - L(t)|| <= max     ||b_j - b_0 - j/n(b_n - b_0)||
                        //                    0<=j<=n
                        //
                        // What is calculated here is an upper bound to the distance from
                        // a line through b_0 and b_3 (P1 and P4 in our notation) and the
                        // curve. We can drop 0 and n from the running indices, since the
                        // argument of max becomes zero for those cases.
                        const double fJ1x( P2x - P1x - 1.0/3.0*(P4x - P1x) );
                        const double fJ1y( P2y - P1y - 1.0/3.0*(P4y - P1y) );
                        const double fJ2x( P3x - P1x - 2.0/3.0*(P4x - P1x) );
                        const double fJ2y( P3y - P1y - 2.0/3.0*(P4y - P1y) );

                        // stop if distance from line is guaranteed to be bounded by d/4
                        if( ::std::max( fJ1x*fJ1x + fJ1y*fJ1y,
                                        fJ2x*fJ2x + fJ2y*fJ2y) < d2/16.0 )
                        {
                            // do not subdivide further, add straight line instead
                            Impl_addStraightLine( rBits, rLastPoint, P4x, P4y);
                        }
                        else
                        {
                            // deCasteljau bezier arc, split at t=0.5
                            // Foley/vanDam, p. 508
                            const double L1x( P1x ),             L1y( P1y );
                            const double L2x( (P1x + P2x)*0.5 ), L2y( (P1y + P2y)*0.5 );
                            const double Hx ( (P2x + P3x)*0.5 ), Hy ( (P2y + P3y)*0.5 );
                            const double L3x( (L2x + Hx)*0.5 ),  L3y( (L2y + Hy)*0.5 );
                            const double R4x( P4x ),             R4y( P4y );
                            const double R3x( (P3x + P4x)*0.5 ), R3y( (P3y + P4y)*0.5 );
                            const double R2x( (Hx + R3x)*0.5 ),  R2y( (Hy + R3y)*0.5 );
                            const double R1x( (L3x + R2x)*0.5 ), R1y( (L3y + R2y)*0.5 );
                            const double L4x( R1x ),             L4y( R1y );

                            // subdivide further
                            Impl_quadBezierApprox(rBits, rLastPoint, d2, L1x, L1y, L2x, L2y, L3x, L3y, L4x, L4y);
                            Impl_quadBezierApprox(rBits, rLastPoint, d2, R1x, R1y, R2x, R2y, R3x, R3y, R4x, R4y);
                        }
                    }
                }
            }
#endif
        }

        sal_Int32 adaptiveSubdivideByDistance( polygon::B2DPolygon&     rPoly,
                                               const B2DCubicBezier&    rCurve,
                                               double                   distanceBounds )
        {
            const point::B2DPoint start( rCurve.getStartPoint() );
            const point::B2DPoint control1( rCurve.getControlPointA() );
            const point::B2DPoint control2( rCurve.getControlPointB() );
            const point::B2DPoint end( rCurve.getEndPoint() );

            return ImplAdaptiveSubdivide( rPoly,
                                          DistanceErrorFunctor( distanceBounds ),
                                          start.getX(),     start.getY(),
                                          control1.getX(),  control1.getY(),
                                          control2.getX(),  control2.getY(),
                                          end.getX(),       end.getY(),
                                          0 );
        }

        sal_Int32 adaptiveSubdivideByAngle( polygon::B2DPolygon&    rPoly,
                                            const B2DCubicBezier&   rCurve,
                                            double                  angleBounds )
        {
            const point::B2DPoint start( rCurve.getStartPoint() );
            const point::B2DPoint control1( rCurve.getControlPointA() );
            const point::B2DPoint control2( rCurve.getControlPointB() );
            const point::B2DPoint end( rCurve.getEndPoint() );

            return ImplAdaptiveSubdivide( rPoly,
                                          AngleErrorFunctor( angleBounds ),
                                          start.getX(),     start.getY(),
                                          control1.getX(),  control1.getY(),
                                          control2.getX(),  control2.getY(),
                                          end.getX(),       end.getY(),
                                          0 );
        }

        sal_Int32 adaptiveSubdivideByDistance( polygon::B2DPolygon&     rPoly,
                                               const B2DQuadraticBezier&    rCurve,
                                               double                       distanceBounds )
        {
            // TODO
            return 0;
        }
    }
}

