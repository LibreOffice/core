/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3ibox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:55:19 $
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

#ifndef _BGFX_RANGE_B3IBOX_HXX
#define _BGFX_RANGE_B3IBOX_HXX

#ifndef _BGFX_POINT_B3IPOINT_HXX
#include <basegfx/point/b3ipoint.hxx>
#endif
#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif
#ifndef _BGFX_TUPLE_B3ITUPLE_HXX
#include <basegfx/tuple/b3ituple.hxx>
#endif
#ifndef _BGFX_TUPLE_B3I64TUPLE_HXX
#include <basegfx/tuple/b3i64tuple.hxx>
#endif

#ifndef _BGFX_RANGE_BASICBOX_HXX
#include <basegfx/range/basicbox.hxx>
#endif

namespace basegfx
{
    class B3IBox
    {
        BasicBox            maRangeX;
        BasicBox            maRangeY;
        BasicBox            maRangeZ;

    public:
        B3IBox()
        {
        }

        explicit B3IBox(const B3ITuple& rTuple) :
            maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY()),
            maRangeZ(rTuple.getZ())
        {
        }

        B3IBox(sal_Int32 x1,
               sal_Int32 y1,
               sal_Int32 z1,
               sal_Int32 x2,
               sal_Int32 y2,
               sal_Int32 z2) :
            maRangeX(x1),
            maRangeY(y1),
            maRangeZ(z1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
            maRangeZ.expand(z2);
        }

        B3IBox(const B3ITuple& rTuple1,
               const B3ITuple& rTuple2) :
            maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY()),
            maRangeZ(rTuple1.getZ())
        {
            expand(rTuple2);
        }

        B3IBox(const B3IBox& rBox) :
            maRangeX(rBox.maRangeX),
            maRangeY(rBox.maRangeY),
            maRangeZ(rBox.maRangeZ)
        {
        }

        bool isEmpty() const
        {
            return maRangeX.isEmpty() || maRangeY.isEmpty() || maRangeZ.isEmpty();
        }

        void reset()
        {
            maRangeX.reset();
            maRangeY.reset();
            maRangeZ.reset();
        }

        bool operator==( const B3IBox& rBox ) const
        {
            return (maRangeX == rBox.maRangeX
                && maRangeY == rBox.maRangeY
                && maRangeZ == rBox.maRangeZ);
        }

        bool operator!=( const B3IBox& rBox ) const
        {
            return (maRangeX != rBox.maRangeX
                || maRangeY != rBox.maRangeY
                || maRangeZ != rBox.maRangeZ);
        }

        void operator=(const B3IBox& rBox)
        {
            maRangeX = rBox.maRangeX;
            maRangeY = rBox.maRangeY;
            maRangeZ = rBox.maRangeZ;
        }

        sal_Int32 getMinX() const
        {
            return maRangeX.getMinimum();
        }

        sal_Int32 getMinY() const
        {
            return maRangeY.getMinimum();
        }

        sal_Int32 getMinZ() const
        {
            return maRangeZ.getMinimum();
        }

        sal_Int32 getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        sal_Int32 getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        sal_Int32 getMaxZ() const
        {
            return maRangeZ.getMaximum();
        }

        sal_Int64 getWidth() const
        {
            return maRangeX.getRange();
        }

        sal_Int64 getHeight() const
        {
            return maRangeY.getRange();
        }

        sal_Int64 getDepth() const
        {
            return maRangeZ.getRange();
        }

        B3IPoint getMinimum() const
        {
            return B3IPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum(),
                maRangeZ.getMinimum()
                );
        }

        B3IPoint getMaximum() const
        {
            return B3IPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum(),
                maRangeZ.getMaximum()
                );
        }

        B3I64Tuple getRange() const
        {
            return B3I64Tuple(
                maRangeX.getRange(),
                maRangeY.getRange(),
                maRangeZ.getRange()
                );
        }

        B3DPoint getCenter() const
        {
            return B3DPoint(
                maRangeX.getCenter(),
                maRangeY.getCenter(),
                maRangeZ.getCenter()
                );
        }

        bool isInside(const B3ITuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                && maRangeZ.isInside(rTuple.getZ())
                );
        }

        bool isInside(const B3IBox& rBox) const
        {
            return (
                maRangeX.isInside(rBox.maRangeX)
                && maRangeY.isInside(rBox.maRangeY)
                && maRangeZ.isInside(rBox.maRangeZ)
                );
        }

        bool overlaps(const B3IBox& rBox) const
        {
            return (
                maRangeX.overlaps(rBox.maRangeX)
                && maRangeY.overlaps(rBox.maRangeY)
                && maRangeZ.overlaps(rBox.maRangeZ)
                );
        }

        void expand(const B3ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
            maRangeZ.expand(rTuple.getZ());
        }

        void expand(const B3IBox& rBox)
        {
            maRangeX.expand(rBox.maRangeX);
            maRangeY.expand(rBox.maRangeY);
            maRangeZ.expand(rBox.maRangeZ);
        }

        void intersect(const B3IBox& rBox)
        {
            maRangeX.intersect(rBox.maRangeX);
            maRangeY.intersect(rBox.maRangeY);
            maRangeZ.intersect(rBox.maRangeZ);
        }

        void grow(sal_Int32 nValue)
        {
            maRangeX.grow(nValue);
            maRangeY.grow(nValue);
            maRangeZ.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RANGE_B3IBOX_HXX */
