/*************************************************************************
 *
 *  $RCSfile: b2dbeziertools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2003-11-12 12:09:50 $
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

#ifndef _BGFX_CURVE_B2DBEZIERTOOLS_HXX
#define _BGFX_CURVE_B2DBEZIERTOOLS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif


//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        class B2DPolygon;
    }

    namespace curve
    {

        class B2DCubicBezier;
        class B2DQuadraticBezier;

        /** Subdivide given cubic bezier segment.

            This function adaptively subdivides the given bezier
            segment into as much straight line segments as necessary,
            such that the maximal orthogonal distance from any of the
            segments to the true curve is less than the given error
            value.

            @param rPoly
            Output polygon. The subdivided bezier segment is added to
            this polygon via B2DPolygon::append().

            @param rCurve
            The cubic bezier curve to subdivide

            @param distanceBound
            Bound on the maximal distance of the approximation to the
            true curve.

            @return the number of line segments created
         */
        sal_Int32 adaptiveSubdivideByDistance( polygon::B2DPolygon&     rPoly,
                                               const B2DCubicBezier&    rCurve,
                                               double                   distanceBound );

        /** Subdivide given cubic bezier segment.

            This function adaptively subdivides the given bezier
            segment into as much quadratic bezier curve segments as
            necessary, such that the maximal orthogonal distance from
            any of the segments to the true curve is less than the
            given error value.

            @param rPoly
            Output polygon. The subdivided bezier segments are added to
            this polygon via B2DPolygon::append().

            @param rCurve
            The cubic bezier curve to subdivide

            @param distanceBound
            Bound on the maximal distance of the approximation to the
            true curve.

            @return the number of quadratic curve segments created
         */
        sal_Int32 adaptiveDegreeReductionByDistance( polygon::B2DPolygon&       rPoly,
                                                     const B2DCubicBezier&  rCurve,
                                                     double                 distanceBound );


        /** Subdivide given cubic bezier segment.

            This function adaptively subdivides the given bezier
            segment into as much straight line segments as necessary,
            such that the maximal angle change between any adjacent
            lines is less than the given error value.

            @param rPoly
            Output polygon. The subdivided bezier segment is added to
            this polygon via B2DPolygon::append().

            @param rCurve
            The cubic bezier curve to subdivide

            @param angleBound
            Bound on the maximal angle difference between two adjacent
            polygon lines, in degrees.

            @return the number of line segments created
         */
        sal_Int32 adaptiveSubdivideByAngle( polygon::B2DPolygon&    rPoly,
                                            const B2DCubicBezier&   rCurve,
                                            double                  angleBound );

        /** Subdivide given cubic bezier segment.

            This function adaptively subdivides the given bezier
            segment into as much quadratic bezier curve segments as
            necessary, such that the maximal angle difference of the
            control vectors of any generated quadratic bezier segment
            is less than the given error value.

            @param rPoly
            Output polygon. The subdivided bezier segments are added to
            this polygon via B2DPolygon::append().

            @param rCurve
            The cubic bezier curve to subdivide

            @param distanceBound
            Bound on the maximal angle difference between the control
            vectors of any of the generated quadratic bezier
            segments. The angle must be given in degrees.

            @return the number of quadratic curve segments created
         */
        sal_Int32 adaptiveDegreeReductionByAngle( polygon::B2DPolygon&  rPoly,
                                                  const B2DCubicBezier& rCurve,
                                                  double                angleBound );


        /** Subdivide given quadratic bezier segment.

            This function adaptively subdivides the given bezier
            segment into as much straight line segments as necessary,
            such that the maximal orthogonal distance from any of the
            segments to the true curve is less than the given error
            value.

            @param rPoly
            Output polygon. The subdivided bezier segment is added to
            this polygon via B2DPolygon::append().

            @param rCurve
            The cubic bezier curve to subdivide

            @param distanceBound
            Bound on the maximal distance of the approximation to the
            true curve

            @return the number of line segments created
         */
        sal_Int32 adaptiveSubdivideByDistance( polygon::B2DPolygon&         rPoly,
                                               const B2DQuadraticBezier&    rCurve,
                                               double                       distanceBound );

        /** Subdivide given quadratic bezier segment.

            This function adaptively subdivides the given bezier
            segment into as much straight line segments as necessary,
            such that the maximal angle change between any adjacent
            lines is less than the given error value.

            @param rPoly
            Output polygon. The subdivided bezier segment is added to
            this polygon via B2DPolygon::append().

            @param rCurve
            The cubic bezier curve to subdivide

            @param angleBound
            Bound on the maximal angle difference between two adjacent
            polygon lines, in degrees.

            @return the number of line segments created
         */
        sal_Int32 adaptiveSubdivideByAngle( polygon::B2DPolygon&        rPoly,
                                            const B2DQuadraticBezier&   rCurve,
                                            double                      angleBound );

    }
}

#endif // _BGFX_CURVE_B2DBEZIERTOOLS_HXX2
