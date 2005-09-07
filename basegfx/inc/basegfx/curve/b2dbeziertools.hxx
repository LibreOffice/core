/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dbeziertools.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:24:14 $
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

#ifndef _BGFX_CURVE_B2DBEZIERTOOLS_HXX
#define _BGFX_CURVE_B2DBEZIERTOOLS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif


//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DPolygon;
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

        @param bAddEndPoint
        Defines if the end point of the segment shall be added. This
        is handy for subdividing a number of segments int one destination
        polygon without the need to remove double points afterwards.

        @return the number of line segments created
    */
    sal_Int32 adaptiveSubdivideByDistance( B2DPolygon&              rPoly,
                                           const B2DCubicBezier&    rCurve,
                                           double                   distanceBound,
                                           bool                     bAddEndPoint );

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
    sal_Int32 adaptiveDegreeReductionByDistance( B2DPolygon&            rPoly,
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
        polygon lines, in degrees. Values greater than |90| are
        truncated to 90 degrees. You won't use them, anyway.

        @param bAddEndPoint
        Defines if the end point of the segment shall be added. This
        is handy for subdividing a number of segments int one destination
        polygon without the need to remove double points afterwards.

        @return the number of line segments created
    */
    sal_Int32 adaptiveSubdivideByAngle( B2DPolygon&             rPoly,
                                        const B2DCubicBezier&   rCurve,
                                        double                  angleBound,
                                        bool                    bAddEndPoint);

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
        segments. The angle must be given in degrees. Values
        greater than |90| are truncated to 90 degrees. You won't
        use them, anyway.

        @return the number of quadratic curve segments created
    */
    sal_Int32 adaptiveDegreeReductionByAngle( B2DPolygon&           rPoly,
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

        @param bAddEndPoint
        Defines if the end point of the segment shall be added. This
        is handy for subdividing a number of segments int one destination
        polygon without the need to remove double points afterwards.

        @return the number of line segments created
    */
    sal_Int32 adaptiveSubdivideByDistance( B2DPolygon&                  rPoly,
                                           const B2DQuadraticBezier&    rCurve,
                                           double                       distanceBound,
                                           bool                         bAddEndPoint );

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
        polygon lines, in degrees. Values greater than |90| are
        truncated to 90 degrees. You won't use them, anyway.

        @param bAddEndPoint
        Defines if the end point of the segment shall be added. This
        is handy for subdividing a number of segments int one destination
        polygon without the need to remove double points afterwards.

        @return the number of line segments created
    */
    sal_Int32 adaptiveSubdivideByAngle( B2DPolygon&                 rPoly,
                                        const B2DQuadraticBezier&   rCurve,
                                        double                      angleBound,
                                        bool                        bAddEndPoint );

}
#endif /* _BGFX_CURVE_B2DBEZIERTOOLS_HXX */
