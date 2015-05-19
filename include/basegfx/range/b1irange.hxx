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

#ifndef INCLUDED_BASEGFX_RANGE_B1IRANGE_HXX
#define INCLUDED_BASEGFX_RANGE_B1IRANGE_HXX

#include <basegfx/range/basicrange.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    /** A one-dimensional interval over integers

        This is a set of real numbers, bounded by a lower and an upper
        value. All inbetween values are included in the set (see also
        http://en.wikipedia.org/wiki/Interval_%28mathematics%29).

        The set is closed, i.e. the upper and the lower bound are
        included (if you're used to the notation - we're talking about
        [a,b] here, compared to half-open [a,b) or open intervals
        (a,b)).

        That means, isInside(val) will return true also for values of
        val=a or val=b.
     */
    class B1IRange
    {
        ::basegfx::BasicRange< sal_Int32, Int32Traits > maRange;

    public:
        B1IRange() {}

        /// Create degenerate interval consisting of a single double number
        explicit B1IRange(sal_Int32 nStartValue)
        :   maRange(nStartValue)
        {
        }

        /// Create proper interval between the two given values
        B1IRange(sal_Int32 nStartValue1, sal_Int32 nStartValue2)
        :   maRange(nStartValue1)
        {
            expand(nStartValue2);
        }

        /** Check if the interval set is empty

            @return false, if no value is in this set - having a
            single value included will already return true.
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

        bool operator==( const B1IRange& rRange ) const
        {
            return (maRange == rRange.maRange);
        }

        bool operator!=( const B1IRange& rRange ) const
        {
            return (maRange != rRange.maRange);
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
        bool isInside(const B1IRange& rRange) const
        {
            return maRange.isInside(rRange.maRange);
        }

        /// yields true if rRange at least partly inside set
        bool overlaps(const B1IRange& rRange) const
        {
            return maRange.overlaps(rRange.maRange);
        }

        /// yields true if overlaps(rRange) does, and the overlap is larger than infinitesimal
        bool overlapsMore(const B1IRange& rRange) const
        {
            return maRange.overlapsMore(rRange.maRange);
        }

        /// add nValue to the set, expanding as necessary
        void expand(sal_Int32 nValue)
        {
            maRange.expand(nValue);
        }

        /// add rRange to the set, expanding as necessary
        void expand(const B1IRange& rRange)
        {
            maRange.expand(rRange.maRange);
        }

        /// calc set intersection
        void intersect(const B1IRange& rRange)
        {
            maRange.intersect(rRange.maRange);
        }

        /// grow set by nValue on both sides
        void grow(sal_Int32 nValue)
        {
            maRange.grow(nValue);
        }
    };
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_RANGE_B1IRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
