/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dpolygon.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 09:55:05 $
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

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#define _BGFX_POLYGON_B3DPOLYGON_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef INCLUDED_O3TL_COW_WRAPPER_HXX
#include <o3tl/cow_wrapper.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class ImplB3DPolygon;

namespace basegfx
{
    class B3DPolygon;
    class B3DPoint;
    class B3DHomMatrix;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B3DPolygon
    {
    public:
        typedef o3tl::cow_wrapper< ImplB3DPolygon > ImplType;

    private:
        // internal data.
        ImplType                                    mpPolygon;

    public:
        B3DPolygon();
        B3DPolygon(const B3DPolygon& rPolygon);
        B3DPolygon(const B3DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount);
        ~B3DPolygon();

        // assignment operator
        B3DPolygon& operator=(const B3DPolygon& rPolygon);

        /// unshare this polygon with all internally shared instances
        void makeUnique();

        // compare operators
        bool operator==(const B3DPolygon& rPolygon) const;
        bool operator!=(const B3DPolygon& rPolygon) const;

        // member count
        sal_uInt32 count() const;

        // Coordinate interface
        ::basegfx::B3DPoint getB3DPoint(sal_uInt32 nIndex) const;
        void setB3DPoint(sal_uInt32 nIndex, const ::basegfx::B3DPoint& rValue);

        // Coordinate insert/append
        void insert(sal_uInt32 nIndex, const ::basegfx::B3DPoint& rPoint, sal_uInt32 nCount = 1);
        void append(const ::basegfx::B3DPoint& rPoint, sal_uInt32 nCount = 1);

        // insert/append other 2D polygons
        void insert(sal_uInt32 nIndex, const B3DPolygon& rPoly, sal_uInt32 nIndex2 = 0, sal_uInt32 nCount = 0);
        void append(const B3DPolygon& rPoly, sal_uInt32 nIndex = 0, sal_uInt32 nCount = 0);

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
        void transform(const ::basegfx::B3DHomMatrix& rMatrix);
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////


#endif /* _BGFX_POLYGON_B3DPOLYGON_HXX */
