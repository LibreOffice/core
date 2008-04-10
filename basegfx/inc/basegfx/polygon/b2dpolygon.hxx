/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dpolygon.hxx,v $
 * $Revision: 1.13 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#define _BGFX_POLYGON_B2DPOLYGON_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/vector/b2enums.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class ImplB2DPolygon;

namespace basegfx
{
    class B2DPolygon;
    class B2DPoint;
    class B2DVector;
    class B2DHomMatrix;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB2DPolygon > ImplType;

    private:
        // internal data.
        ImplType                                    mpPolygon;

    public:
        B2DPolygon();
        B2DPolygon(const B2DPolygon& rPolygon);
        B2DPolygon(const B2DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount);
        ~B2DPolygon();

        // assignment operator
        B2DPolygon& operator=(const B2DPolygon& rPolygon);

        /// unshare this polygon with all internally shared instances
        void makeUnique();

        // compare operators
        bool operator==(const B2DPolygon& rPolygon) const;
        bool operator!=(const B2DPolygon& rPolygon) const;

        // member count
        sal_uInt32 count() const;

        // Coordinate interface
        basegfx::B2DPoint getB2DPoint(sal_uInt32 nIndex) const;
        void setB2DPoint(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue);

        // Coordinate insert/append
        void insert(sal_uInt32 nIndex, const basegfx::B2DPoint& rPoint, sal_uInt32 nCount = 1);
        void append(const basegfx::B2DPoint& rPoint, sal_uInt32 nCount = 1);

        // ControlPoint interface
        basegfx::B2DPoint getPrevControlPoint(sal_uInt32 nIndex) const;
        basegfx::B2DPoint getNextControlPoint(sal_uInt32 nIndex) const;
        void setPrevControlPoint(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue);
        void setNextControlPoint(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue);
        void setControlPoints(sal_uInt32 nIndex, const basegfx::B2DPoint& rPrev, const basegfx::B2DPoint& rNext);

        // ControlPoint resets
        void resetPrevControlPoint(sal_uInt32 nIndex);
        void resetNextControlPoint(sal_uInt32 nIndex);
        void resetControlPoints(sal_uInt32 nIndex);
        void resetControlPoints();

        // Bezier segment append with control points. The currently last
        // polygon point is implicitly taken as start point.
        void appendBezierSegment(const basegfx::B2DPoint& rNextControlPoint, const basegfx::B2DPoint& rPrevControlPoint, const basegfx::B2DPoint& rPoint);

        // ControlPoint checks
        bool areControlPointsUsed() const;
        bool isPrevControlPointUsed(sal_uInt32 nIndex) const;
        bool isNextControlPointUsed(sal_uInt32 nIndex) const;
        B2VectorContinuity getContinuityInPoint(sal_uInt32 nIndex) const;

        // insert/append other 2D polygons
        void insert(sal_uInt32 nIndex, const B2DPolygon& rPoly, sal_uInt32 nIndex2 = 0, sal_uInt32 nCount = 0);
        void append(const B2DPolygon& rPoly, sal_uInt32 nIndex = 0, sal_uInt32 nCount = 0);

        // remove
        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

        // clear all points
        void clear();

        // closed state
        bool isClosed() const;
        void setClosed(bool bNew);

        // flip polygon direction
        void flip();

        // test if Polygon has double points
        bool hasDoublePoints() const;

        // remove double points, at the begin/end and follow-ups, too
        void removeDoublePoints();

        // apply transformation given in matrix form to the polygon
        void transform(const basegfx::B2DHomMatrix& rMatrix);
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_B2DPOLYGON_HXX */
