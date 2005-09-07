/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dquadraticbezier.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:24:47 $
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

#ifndef _BGFX_CURVE_B2DQUADRATICBEZIER_HXX
#define _BGFX_CURVE_B2DQUADRATICBEZIER_HXX

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DQuadraticBezier
    {
        ::basegfx::B2DPoint                         maStartPoint;
        ::basegfx::B2DPoint                         maEndPoint;
        ::basegfx::B2DPoint                         maControlPoint;

    public:
        B2DQuadraticBezier();
        B2DQuadraticBezier(const B2DQuadraticBezier& rBezier);
        B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart, const ::basegfx::B2DPoint& rEnd);
        B2DQuadraticBezier(const ::basegfx::B2DPoint& rStart,
            const ::basegfx::B2DPoint& rControlPoint, const ::basegfx::B2DPoint& rEnd);
        ~B2DQuadraticBezier();

        // assignment operator
        B2DQuadraticBezier& operator=(const B2DQuadraticBezier& rBezier);

        // compare operators
        bool operator==(const B2DQuadraticBezier& rBezier) const;
        bool operator!=(const B2DQuadraticBezier& rBezier) const;

        // test if control point is placed on the edge
        bool isBezier() const;

        // data interface
        ::basegfx::B2DPoint getStartPoint() const { return maStartPoint; }
        void setStartPoint(const ::basegfx::B2DPoint& rValue) { maStartPoint = rValue; }

        ::basegfx::B2DPoint getEndPoint() const { return maEndPoint; }
        void setEndPoint(const ::basegfx::B2DPoint& rValue) { maEndPoint = rValue; }

        ::basegfx::B2DPoint getControlPoint() const { return maControlPoint; }
        void setControlPoint(const ::basegfx::B2DPoint& rValue) { maControlPoint = rValue; }
    };
} // end of namespace basegfx

#endif /* _BGFX_CURVE_B2DQUADRATICBEZIER_HXX */
