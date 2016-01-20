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

#ifndef INCLUDED_BASEGFX_RANGE_B2IBOX_HXX
#define INCLUDED_BASEGFX_RANGE_B2IBOX_HXX

#include <ostream>
#include <vector>

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/basicbox.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    /** A two-dimensional interval over integers

        This is most easily depicted as a set of integers, bounded by
        a lower and an upper value - but excluding the upper
        value. All inbetween values are included in the set (see also
        http://en.wikipedia.org/wiki/Interval_%28mathematics%29).

        The set is half-open, i.e. the lower bound is included, the
        upper bound not (if you're used to the notation - we're
        talking about [a,b) here, compared to closed [a,b] or fully
        open intervals (a,b)).

        If you don't need a half-open interval, check B2IRange.

        That means, isInside(val) will return true also for values of
        val=a, but not for val=b.

        Alternatively, consider this a rectangle, where the rightmost
        pixel column and the bottommost pixel row are excluded - this
        is much like polygon filling. As a result, filling a given
        rectangle with basebmp::BitmapDevice::fillPolyPolygon(), will
        affect exactly the same set of pixel as isInside() would
        return true for.

        @see B2IRange
     */
    class B2IBox
    {
    public:
        typedef sal_Int32       ValueType;
        typedef Int32Traits     TraitsType;

        B2IBox() {}

        /// Create degenerate interval that's still empty
        explicit B2IBox(const B2ITuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY())
        {
        }

        /// Create proper interval between the two given points
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

        /// Create proper interval between the two given points
        B2IBox(const B2ITuple& rTuple1,
               const B2ITuple& rTuple2) :
            maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY())
        {
            expand( rTuple2 );
        }

        /** Check if the interval set is empty

            @return false, if no value is in this set - having a
            single value included will still return false.
         */
        bool isEmpty() const
        {
            return maRangeX.isEmpty() || maRangeY.isEmpty();
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

        /// yields true if point is contained in set
        bool isInside(const B2ITuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                );
        }

        /// add point to the set, expanding as necessary
        void expand(const B2ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
        }

        /// calc set intersection
        void intersect(const B2IBox& rBox)
        {
            maRangeX.intersect(rBox.maRangeX);
            maRangeY.intersect(rBox.maRangeY);
        }

    private:
        BasicBox        maRangeX;
        BasicBox        maRangeY;
    };

} // end of namespace basegfx

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const basegfx::B2IBox& box )
{
    if (box.isEmpty())
        return stream << "EMPTY";
    else
        return stream << box.getWidth() << 'x' << box.getHeight()
                      << "@(" << box.getMinX() << "," << box.getMinY() << ")";
}

#endif // INCLUDED_BASEGFX_RANGE_B2IBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
