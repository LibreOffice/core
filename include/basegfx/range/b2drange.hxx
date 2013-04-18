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

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#define _BGFX_RANGE_B2DRANGE_HXX

#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/range/basicrange.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    // predeclarations
    class B2IRange;
    class B2DHomMatrix;

    /** A two-dimensional interval over doubles

        This is a set of real numbers, bounded by a lower and an upper
        pair. All inbetween values are included in the set (see also
        http://en.wikipedia.org/wiki/Interval_%28mathematics%29).

        The set is closed, i.e. the upper and the lower bound are
        included (if you're used to the notation - we're talking about
        [a,b] here, compared to half-open [a,b) or open intervals
        (a,b)).

        That means, isInside(val) will return true also for values of
        val=a or val=b.

        @see B1DRange
     */
    class B2DRange
    {
    public:
        typedef double          ValueType;
        typedef DoubleTraits    TraitsType;

        B2DRange() {}

        /// Create degenerate interval consisting of a single point
        explicit B2DRange(const B2DTuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY())
        {
        }

        /// Create proper interval between the two given double pairs
        B2DRange(double x1,
                 double y1,
                 double x2,
                 double y2)
        :   maRangeX(x1),
            maRangeY(y1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
        }

        /// Create proper interval between the two given points
        B2DRange(const B2DTuple& rTuple1,
                 const B2DTuple& rTuple2)
        :   maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY())
        {
            expand( rTuple2 );
        }

        BASEGFX_DLLPUBLIC explicit B2DRange(const B2IRange& rRange);

        /** Check if the interval set is empty

            @return false, if no value is in this set - having a
            single point included will already return true.
         */
        bool isEmpty() const
        {
            return (
                maRangeX.isEmpty()
                || maRangeY.isEmpty()
                );
        }

        /// reset the object to empty state again, clearing all values
        void reset()
        {
            maRangeX.reset();
            maRangeY.reset();
        }

        bool operator==( const B2DRange& rRange ) const
        {
            return (maRangeX == rRange.maRangeX
                && maRangeY == rRange.maRangeY);
        }

        bool operator!=( const B2DRange& rRange ) const
        {
            return (maRangeX != rRange.maRangeX
                || maRangeY != rRange.maRangeY);
        }

        bool equal(const B2DRange& rRange) const
        {
            return (maRangeX.equal(rRange.maRangeX)
                    && maRangeY.equal(rRange.maRangeY));
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        double getMinX() const
        {
            return maRangeX.getMinimum();
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        double getMinY() const
        {
            return maRangeY.getMinimum();
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        double getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        double getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        /// return difference between upper and lower X value. returns 0 for empty sets.
        double getWidth() const
        {
            return maRangeX.getRange();
        }

        /// return difference between upper and lower Y value. returns 0 for empty sets.
        double getHeight() const
        {
            return maRangeY.getRange();
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        B2DPoint getMinimum() const
        {
            return B2DPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum()
                );
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        B2DPoint getMaximum() const
        {
            return B2DPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum()
                );
        }

        /// return difference between upper and lower point. returns (0,0) for empty sets.
        B2DVector getRange() const
        {
            return B2DVector(
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

        /// return center X value of set. returns 0 for empty sets.
        double getCenterX() const
        {
            return maRangeX.getCenter();
        }

        /// return center Y value of set. returns 0 for empty sets.
        double getCenterY() const
        {
            return maRangeY.getCenter();
        }

        /// yields true if given point is contained in set
        bool isInside(const B2DTuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                );
        }

        /// yields true if rRange is inside, or equal to set
        bool isInside(const B2DRange& rRange) const
        {
            return (
                maRangeX.isInside(rRange.maRangeX)
                && maRangeY.isInside(rRange.maRangeY)
                );
        }

        /// yields true if rRange at least partly inside set
        bool overlaps(const B2DRange& rRange) const
        {
            return (
                maRangeX.overlaps(rRange.maRangeX)
                && maRangeY.overlaps(rRange.maRangeY)
                );
        }

        /// yields true if overlaps(rRange) does, and the overlap is larger than infinitesimal
        bool overlapsMore(const B2DRange& rRange) const
        {
            return (
                maRangeX.overlapsMore(rRange.maRangeX)
                && maRangeY.overlapsMore(rRange.maRangeY)
                );
        }

        /// add point to the set, expanding as necessary
        void expand(const B2DTuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
        }

        /// add rRange to the set, expanding as necessary
        void expand(const B2DRange& rRange)
        {
            maRangeX.expand(rRange.maRangeX);
            maRangeY.expand(rRange.maRangeY);
        }

        /// calc set intersection
        void intersect(const B2DRange& rRange)
        {
            maRangeX.intersect(rRange.maRangeX);
            maRangeY.intersect(rRange.maRangeY);
        }

        /// grow set by fValue on all sides
        void grow(double fValue)
        {
            maRangeX.grow(fValue);
            maRangeY.grow(fValue);
        }

        BASEGFX_DLLPUBLIC void transform(const B2DHomMatrix& rMatrix);

    private:
        typedef ::basegfx::BasicRange< ValueType, TraitsType >  MyBasicRange;

        MyBasicRange        maRangeX;
        MyBasicRange        maRangeY;
    };

    /** Round double to nearest integer for 2D range

        @return the nearest integer for this range
    */
    BASEGFX_DLLPUBLIC B2IRange fround(const B2DRange& rRange);

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
    BASEGFX_DLLPUBLIC ::std::vector< B2DRange >& computeSetDifference( ::std::vector< B2DRange >&   o_rResult,
                                                     const B2DRange&            rFirst,
                                                     const B2DRange&            rSecond );

} // end of namespace basegfx


#endif /* _BGFX_RANGE_B2DRANGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
