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

#ifndef _BGFX_RANGE_B2IRANGE_HXX
#define _BGFX_RANGE_B2IRANGE_HXX

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/basicrange.hxx>
#include <vector>


namespace basegfx
{
    class B2IRange
    {
    public:
        typedef sal_Int32       ValueType;
        typedef Int32Traits     TraitsType;

        B2IRange()
        {
        }

        explicit B2IRange(const B2ITuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY())
        {
        }

        B2IRange(sal_Int32 x1,
                 sal_Int32 y1,
                 sal_Int32 x2,
                 sal_Int32 y2)
        :   maRangeX(x1),
            maRangeY(y1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
        }

        B2IRange(const B2ITuple& rTuple1,
                 const B2ITuple& rTuple2)
        :   maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY())
        {
            expand( rTuple2 );
        }

        B2IRange(const B2IRange& rRange)
        :   maRangeX(rRange.maRangeX),
            maRangeY(rRange.maRangeY)
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

        bool operator==( const B2IRange& rRange ) const
        {
            return (maRangeX == rRange.maRangeX
                && maRangeY == rRange.maRangeY);
        }

        bool operator!=( const B2IRange& rRange ) const
        {
            return (maRangeX != rRange.maRangeX
                || maRangeY != rRange.maRangeY);
        }

        B2IRange& operator=(const B2IRange& rRange)
        {
            maRangeX = rRange.maRangeX;
            maRangeY = rRange.maRangeY;
            return *this;
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

        bool isInside(const B2IRange& rRange) const
        {
            return (
                maRangeX.isInside(rRange.maRangeX)
                && maRangeY.isInside(rRange.maRangeY)
                );
        }

        bool overlaps(const B2IRange& rRange) const
        {
            return (
                maRangeX.overlaps(rRange.maRangeX)
                && maRangeY.overlaps(rRange.maRangeY)
                );
        }

        void expand(const B2ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
        }

        void expand(const B2IRange& rRange)
        {
            maRangeX.expand(rRange.maRangeX);
            maRangeY.expand(rRange.maRangeY);
        }

        void intersect(const B2IRange& rRange)
        {
            maRangeX.intersect(rRange.maRangeX);
            maRangeY.intersect(rRange.maRangeY);
        }

        void grow(sal_Int32 nValue)
        {
            maRangeX.grow(nValue);
            maRangeY.grow(nValue);
        }

    private:
        typedef ::basegfx::BasicRange< ValueType, TraitsType >  MyBasicRange;

        MyBasicRange        maRangeX;
        MyBasicRange        maRangeY;
    };

    /** Compute the set difference of the two given ranges

        This method calculates the symmetric difference (aka XOR)
        between the two given ranges, and returning the resulting
        ranges. Thus, the result will contain all areas where one, but
        not both ranges lie.

        @param o_rResult
        Result vector. The up to four difference ranges are returned
        within this vector

        @param rFirst
        The first range

        @param rSecond
        The second range

        @return the input vector
     */
    ::std::vector< B2IRange >& computeSetDifference( ::std::vector< B2IRange >& o_rResult,
                                                     const B2IRange&            rFirst,
                                                     const B2IRange&            rSecond );

} // end of namespace basegfx

#endif /* _BGFX_RANGE_B2IRANGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
