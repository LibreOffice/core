/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _BGFX_RANGE_B2IBOX_HXX
#define _BGFX_RANGE_B2IBOX_HXX

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/basicbox.hxx>
#include <vector>


namespace basegfx
{
    class B2IBox
    {
    public:
        typedef sal_Int32       ValueType;
        typedef Int32Traits     TraitsType;

        B2IBox()
        {
        }

        explicit B2IBox(const B2ITuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY())
        {
        }

        B2IBox(sal_Int32 x1,
               sal_Int32 y1,
               sal_Int32 x2,
               sal_Int32 y2) :
            maRangeX(x1),
            maRangeY(y1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
        }

        B2IBox(const B2ITuple& rTuple1,
               const B2ITuple& rTuple2) :
            maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY())
        {
            expand( rTuple2 );
        }

        B2IBox(const B2IBox& rBox) :
            maRangeX(rBox.maRangeX),
            maRangeY(rBox.maRangeY)
        {
        }

        bool isEmpty() const
        {
            return maRangeX.isEmpty() || maRangeY.isEmpty();
        }

        void reset()
        {
            maRangeX.reset();
            maRangeY.reset();
        }

        bool operator==( const B2IBox& rBox ) const
        {
            return (maRangeX == rBox.maRangeX
                && maRangeY == rBox.maRangeY);
        }

        bool operator!=( const B2IBox& rBox ) const
        {
            return (maRangeX != rBox.maRangeX
                || maRangeY != rBox.maRangeY);
        }

        void operator=(const B2IBox& rBox)
        {
            maRangeX = rBox.maRangeX;
            maRangeY = rBox.maRangeY;
        }

        sal_Int32 getMinX() const
        {
            return maRangeX.getMinimum();
        }

        sal_Int32 getMinY() const
        {
            return maRangeY.getMinimum();
        }

        sal_Int32 getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        sal_Int32 getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        sal_Int64 getWidth() const
        {
            return maRangeX.getRange();
        }

        sal_Int64 getHeight() const
        {
            return maRangeY.getRange();
        }

        B2IPoint getMinimum() const
        {
            return B2IPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum()
                );
        }

        B2IPoint getMaximum() const
        {
            return B2IPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum()
                );
        }

        B2I64Tuple getRange() const
        {
            return B2I64Tuple(
                maRangeX.getRange(),
                maRangeY.getRange()
                );
        }

        B2DPoint getCenter() const
        {
            return B2DPoint(
                maRangeX.getCenter(),
                maRangeY.getCenter()
                );
        }

        bool isInside(const B2ITuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                );
        }

        bool isInside(const B2IBox& rBox) const
        {
            return (
                maRangeX.isInside(rBox.maRangeX)
                && maRangeY.isInside(rBox.maRangeY)
                );
        }

        bool overlaps(const B2IBox& rBox) const
        {
            return (
                maRangeX.overlaps(rBox.maRangeX)
                && maRangeY.overlaps(rBox.maRangeY)
                );
        }

        void expand(const B2ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
        }

        void expand(const B2IBox& rBox)
        {
            maRangeX.expand(rBox.maRangeX);
            maRangeY.expand(rBox.maRangeY);
        }

        void intersect(const B2IBox& rBox)
        {
            maRangeX.intersect(rBox.maRangeX);
            maRangeY.intersect(rBox.maRangeY);
        }

        void grow(sal_Int32 nValue)
        {
            maRangeX.grow(nValue);
            maRangeY.grow(nValue);
        }

    private:
        BasicBox        maRangeX;
        BasicBox        maRangeY;
    };

    /** Compute the set difference of the two given boxes

        This method calculates the symmetric difference (aka XOR)
        between the two given boxes, and returning the resulting
        boxes. Thus, the result will contain all areas where one, but
        not both boxes lie.

        @param o_rResult
        Result vector. The up to four difference boxes are returned
        within this vector

        @param rFirst
        The first box

        @param rSecond
        The second box

        @return the input vector
     */
    ::std::vector< B2IBox >& computeSetDifference( ::std::vector< B2IBox >& o_rResult,
                                                   const B2IBox&            rFirst,
                                                   const B2IBox&            rSecond );

} // end of namespace basegfx

#endif /* _BGFX_RANGE_B2IBOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
