/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _BGFX_RANGE_B2IRANGE_HXX
#define _BGFX_RANGE_B2IRANGE_HXX

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/basicrange.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    /** A two-dimensional interval over integers

        This is a set of real numbers, bounded by a lower and an upper
        pair. All inbetween values are included in the set (see also
        http://en.wikipedia.org/wiki/Interval_%28mathematics%29).

        Probably you rather want B2IBox for integers.

        The set is closed, i.e. the upper and the lower bound are
        included (if you're used to the notation - we're talking about
        [a,b] here, compared to half-open [a,b) or open intervals
        (a,b)).

        That means, isInside(val) will return true also for values of
        val=a or val=b.

        @see B2IBox
     */
    class B2IRange
    {
    public:
        typedef sal_Int32       ValueType;
        typedef Int32Traits     TraitsType;

        B2IRange() {}

        /// Create degenerate interval consisting of a single point
        explicit B2IRange(const B2ITuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY())
        {
        }

        /// Create proper interval between the two given integer pairs
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

        /// Create proper interval between the two given points
        B2IRange(const B2ITuple& rTuple1,
                 const B2ITuple& rTuple2)
        :   maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY())
        {
            expand( rTuple2 );
        }

        /** Check if the interval set is empty

            @return false, if no value is in this set - having a
            single point included will already return true.
         */
        bool isEmpty() const
        {
            return maRangeX.isEmpty() || maRangeY.isEmpty();
        }

        /// reset the object to empty state again, clearing all values
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

        /// get lower bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMinX() const
        {
            return maRangeX.getMinimum();
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMinY() const
        {
            return maRangeY.getMinimum();
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        /// return difference between upper and lower X value. returns 0 for empty sets.
        sal_Int64 getWidth() const
        {
            return maRangeX.getRange();
        }

        /// return difference between upper and lower Y value. returns 0 for empty sets.
        sal_Int64 getHeight() const
        {
            return maRangeY.getRange();
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        B2IPoint getMinimum() const
        {
            return B2IPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum()
                );
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        B2IPoint getMaximum() const
        {
            return B2IPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum()
                );
        }

        /// return difference between upper and lower point. returns (0,0) for empty sets.
        B2I64Tuple getRange() const
        {
            return B2I64Tuple(
                maRangeX.getRange(),
                maRangeY.getRange()
                );
        }

        /// return center point of set. returns (0,0) for empty sets.
        B2DPoint getCenter() const
        {
            return B2DPoint(
                maRangeX.getCenter(),
                maRangeY.getCenter()
                );
        }

        /// yields true if given point is contained in set
        bool isInside(const B2ITuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                );
        }

        /// yields true if rRange is inside, or equal to set
        bool isInside(const B2IRange& rRange) const
        {
            return (
                maRangeX.isInside(rRange.maRangeX)
                && maRangeY.isInside(rRange.maRangeY)
                );
        }

        /// yields true if rRange at least partly inside set
        bool overlaps(const B2IRange& rRange) const
        {
            return (
                maRangeX.overlaps(rRange.maRangeX)
                && maRangeY.overlaps(rRange.maRangeY)
                );
        }

        /// yields true if overlaps(rRange) does, and the overlap is larger than infinitesimal
        bool overlapsMore(const B2IRange& rRange) const
        {
            return (
                maRangeX.overlapsMore(rRange.maRangeX)
                && maRangeY.overlapsMore(rRange.maRangeY)
                );
        }

        /// add point to the set, expanding as necessary
        void expand(const B2ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
        }

        /// add rRange to the set, expanding as necessary
        void expand(const B2IRange& rRange)
        {
            maRangeX.expand(rRange.maRangeX);
            maRangeY.expand(rRange.maRangeY);
        }

        /// calc set intersection
        void intersect(const B2IRange& rRange)
        {
            maRangeX.intersect(rRange.maRangeX);
            maRangeY.intersect(rRange.maRangeY);
        }

        /// grow set by nValue on all sides
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
    BASEGFX_DLLPUBLIC ::std::vector< B2IRange >& computeSetDifference( ::std::vector< B2IRange >&   o_rResult,
                                                     const B2IRange&            rFirst,
                                                     const B2IRange&            rSecond );

} // end of namespace basegfx

#endif /* _BGFX_RANGE_B2IRANGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
