/*************************************************************************
 *
 *  $RCSfile: b2dbeziertools.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2003-11-10 13:32:04 $
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

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif


namespace basegfx
{
    namespace curve
    {
        namespace
        {
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
            int ImplAdaptiveSubdivide( polygon::B2DPolygon&           rPoly,
                                       const double                   d2,
                                       const double P1x, const double P1y,
                                       const double P2x, const double P2y,
                                       const double P3x, const double P3y,
                                       const double P4x, const double P4y,
                                       const double                   old_distance2,
                                       int                            recursionDepth )
            {
                // Hard limit on recursion depth, empiric number.
                enum {maxRecursionDepth=128};

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
                const double distance2( ::std::max( fJ1x*fJ1x + fJ1y*fJ1y,
                                                    fJ2x*fJ2x + fJ2y*fJ2y) );

                // stop if error measure does not improve anymore. This is a
                // safety guard against floating point inaccuracies.
                // stop at recursion level 128. This is a safety guard against
                // floating point inaccuracies.
                // stop if distance from line is guaranteed to be bounded by d
                if( old_distance2 > d2 &&
                    recursionDepth < maxRecursionDepth &&
                    distance2 >= d2 )
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
                    ++recursionDepth;

                    int nGeneratedPoints(0);

                    nGeneratedPoints += ImplAdaptiveSubdivide(rPoly, d2, L1x, L1y, L2x, L2y, L3x, L3y, L4x, L4y, distance2, recursionDepth);
                    nGeneratedPoints += ImplAdaptiveSubdivide(rPoly, d2, R1x, R1y, R2x, R2y, R3x, R3y, R4x, R4y, distance2, recursionDepth);

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
        }

        int adaptiveSubdivide( polygon::B2DPolygon&     rPoly,
                               const B2DCubicBezier&    rCurve,
                               double                   distanceBounds )
        {
            const double distance2( distanceBounds*distanceBounds );
            const point::B2DPoint start( rCurve.getStartPoint() );
            const point::B2DPoint control1( rCurve.getControlPointA() );
            const point::B2DPoint control2( rCurve.getControlPointB() );
            const point::B2DPoint end( rCurve.getEndPoint() );

            return ImplAdaptiveSubdivide( rPoly,
                                          distance2,
                                          start.getX(),     start.getY(),
                                          control1.getX(),  control1.getY(),
                                          control2.getX(),  control2.getY(),
                                          end.getX(),       end.getY(),
                                          ::std::numeric_limits<double>::max(),
                                          0 );
        }

        int adaptiveSubdivide( polygon::B2DPolygon&         rPoly,
                               const B2DQuadraticBezier&    rCurve,
                               double                       distanceBounds )
        {
            // TODO
            return 0;
        }
    }
}

