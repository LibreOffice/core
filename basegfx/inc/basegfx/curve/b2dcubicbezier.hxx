/*************************************************************************
 *
 *  $RCSfile: b2dcubicbezier.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-12-13 08:47:11 $
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

#ifndef _BGFX_CURVE_B2DCUBICBEZIER_HXX
#define _BGFX_CURVE_B2DCUBICBEZIER_HXX

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace basegfx
{
    class B2DPolygon;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DCubicBezier
    {
        ::basegfx::B2DPoint                         maStartPoint;
        ::basegfx::B2DPoint                         maEndPoint;
        ::basegfx::B2DPoint                         maControlPointA;
        ::basegfx::B2DPoint                         maControlPointB;

    public:
        B2DCubicBezier();
        B2DCubicBezier(const B2DCubicBezier& rBezier);
        B2DCubicBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd);
        B2DCubicBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rControlPointA,
            const ::basegfx::B2DPoint& rControlPointB, const ::basegfx::B2DPoint& rEnd);
        ~B2DCubicBezier();

        // assignment operator
        B2DCubicBezier& operator=(const B2DCubicBezier& rBezier);

        // compare operators
        bool operator==(const B2DCubicBezier& rBezier) const;
        bool operator!=(const B2DCubicBezier& rBezier) const;

        // test if vectors are used
        bool isBezier() const;

        // test if contained bezier is trivial and reset vectors accordingly
        void testAndSolveTrivialBezier();

        // get distance between start and end point
        double getEdgeLength() const;

        // get length of control polygon
        double getControlPolygonLength() const;

        // data interface
        ::basegfx::B2DPoint getStartPoint() const { return maStartPoint; }
        void setStartPoint(const ::basegfx::B2DPoint& rValue) { maStartPoint = rValue; }

        ::basegfx::B2DPoint getEndPoint() const { return maEndPoint; }
        void setEndPoint(const ::basegfx::B2DPoint& rValue) { maEndPoint = rValue; }

        ::basegfx::B2DPoint getControlPointA() const { return maControlPointA; }
        void setControlPointA(const ::basegfx::B2DPoint& rValue) { maControlPointA = rValue; }

        ::basegfx::B2DPoint getControlPointB() const { return maControlPointB; }
        void setControlPointB(const ::basegfx::B2DPoint& rValue) { maControlPointB = rValue; }

        // adaptive subdivide by angle criteria
        // #i37443# allow the criteria to get unsharp in recursions
        void adaptiveSubdivideByAngle(B2DPolygon& rTarget, double fAngleBound, bool bAddLastPoint, bool bAllowUnsharpen) const;

        // #i37443# adaptive subdivide by nCount subdivisions
        void adaptiveSubdivideByCount(B2DPolygon& rTarget, sal_uInt32 nCount, bool bAddLastPoint) const;
    };
} // end of namespace basegfx

#endif /* _BGFX_CURVE_B2DCUBICBEZIER_HXX */
