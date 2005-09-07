/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygon.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:27:45 $
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

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#define _BGFX_POLYGON_B2DPOLYGON_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
    private:
        // internal data.
        ImplB2DPolygon*                             mpPolygon;

        // internal method to force a ref-counted instance to be copied
        // to a modifyable unique copy.
        void implForceUniqueCopy();

    public:
        B2DPolygon();
        B2DPolygon(const B2DPolygon& rPolygon);
        B2DPolygon(const B2DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount);
        ~B2DPolygon();

        // assignment operator
        B2DPolygon& operator=(const B2DPolygon& rPolygon);

        // compare operators
        bool operator==(const B2DPolygon& rPolygon) const;
        bool operator!=(const B2DPolygon& rPolygon) const;

        // member count
        sal_uInt32 count() const;

        // Coordinate interface
        ::basegfx::B2DPoint getB2DPoint(sal_uInt32 nIndex) const;
        void setB2DPoint(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue);

        // Coordinate insert/append
        void insert(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rPoint, sal_uInt32 nCount = 1);
        void append(const ::basegfx::B2DPoint& rPoint, sal_uInt32 nCount = 1);

        // ControlVector interface
        ::basegfx::B2DVector getControlVectorA(sal_uInt32 nIndex) const;
        void setControlVectorA(sal_uInt32 nIndex, const ::basegfx::B2DVector& rValue);
        ::basegfx::B2DVector getControlVectorB(sal_uInt32 nIndex) const;
        void setControlVectorB(sal_uInt32 nIndex, const ::basegfx::B2DVector& rValue);
        bool areControlVectorsUsed() const;

        // ControlPoint interface
        ::basegfx::B2DPoint getControlPointA(sal_uInt32 nIndex) const;
        void setControlPointA(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue);
        ::basegfx::B2DPoint getControlPointB(sal_uInt32 nIndex) const;
        void setControlPointB(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue);
        bool areControlPointsUsed() const { return areControlVectorsUsed(); }

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
        void transform(const ::basegfx::B2DHomMatrix& rMatrix);
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_POLYGON_B2DPOLYGON_HXX */
