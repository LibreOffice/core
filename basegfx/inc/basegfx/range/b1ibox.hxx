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

#ifndef _BGFX_RANGE_B1IBOX_HXX
#define _BGFX_RANGE_B1IBOX_HXX

#include <basegfx/range/basicbox.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    /** A one-dimensional interval over integers

        This is most easily depicted as a set of integers, bounded by
        a lower and an upper value - but excluding the upper
        value. All inbetween values are included in the set (see also
        http://en.wikipedia.org/wiki/Interval_%28mathematics%29).

        The set is half-open, i.e. the lower bound is included, the
        upper bound not (if you're used to the notation - we're
        talking about [a,b) here, compared to closed [a,b] or fully
        open intervals (a,b)).

        If you don't need a half-open interval, check B1IRange.

        That means, isInside(val) will return true also for values of
        val=a, but not for val=b.

        @see B1IRange
     */
    class B1IBox
    {
        ::basegfx::BasicBox maRange;

    public:
        B1IBox() {}

        /// Create degenerate interval that's still empty
        explicit B1IBox(sal_Int32 nStartValue)
        :   maRange(nStartValue)
        {
        }

        /// Create proper interval between the two given values
        B1IBox(sal_Int32 nStartValue1, sal_Int32 nStartValue2)
        :   maRange(nStartValue1)
        {
            expand(nStartValue2);
        }

        /** Check if the interval set is empty

            @return false, if no value is in this set - having a
            single value included will still return false.
         */
        bool isEmpty() const
        {
            return maRange.isEmpty();
        }

        /// reset the object to empty state again, clearing all values
        void reset()
        {
            maRange.reset();
        }

        bool operator==( const B1IBox& rBox ) const
        {
            return (maRange == rBox.maRange);
        }

        bool operator!=( const B1IBox& rBox ) const
        {
            return (maRange != rBox.maRange);
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMinimum() const
        {
            return maRange.getMinimum();
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMaximum() const
        {
            return maRange.getMaximum();
        }

        /// return difference between upper and lower value. returns 0 for empty sets.
        Int32Traits::DifferenceType getRange() const
        {
            return maRange.getRange();
        }

        /// return middle of upper and lower value. returns 0 for empty sets.
        double getCenter() const
        {
            return maRange.getCenter();
        }

        /// yields true if value is contained in set
        bool isInside(sal_Int32 nValue) const
        {
            return maRange.isInside(nValue);
        }

        /// yields true if rRange is inside, or equal to set
        bool isInside(const B1IBox& rBox) const
        {
            return maRange.isInside(rBox.maRange);
        }

        /// yields true if rRange at least partly inside set
        bool overlaps(const B1IBox& rBox) const
        {
            return maRange.overlaps(rBox.maRange);
        }

        /// add nValue to the set, expanding as necessary
        void expand(sal_Int32 nValue)
        {
            maRange.expand(nValue);
        }

        /// add rBox to the set, expanding as necessary
        void expand(const B1IBox& rBox)
        {
            maRange.expand(rBox.maRange);
        }

        /// calc set intersection
        void intersect(const B1IBox& rBox)
        {
            maRange.intersect(rBox.maRange);
        }

        /// grow set by nValue on both sides
        void grow(sal_Int32 nValue)
        {
            maRange.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_RANGE_B1IBOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
