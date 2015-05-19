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

#ifndef INCLUDED_BASEGFX_RANGE_B1DRANGE_HXX
#define INCLUDED_BASEGFX_RANGE_B1DRANGE_HXX

#include <basegfx/range/basicrange.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{

    /** A one-dimensional interval over doubles

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
    class B1DRange
    {
        ::basegfx::BasicRange< double, DoubleTraits >   maRange;

    public:
        B1DRange() {}

        /// Create degenerate interval consisting of a single double number
        explicit B1DRange(double fStartValue)
        :   maRange(fStartValue)
        {
        }

        /// Create proper interval between the two given double values
        B1DRange(double fStartValue1, double fStartValue2)
        :   maRange(fStartValue1)
        {
            expand(fStartValue2);
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

        bool operator==( const B1DRange& rRange ) const
        {
            return (maRange == rRange.maRange);
        }

        bool operator!=( const B1DRange& rRange ) const
        {
            return (maRange != rRange.maRange);
        }

        bool equal(const B1DRange& rRange) const
        {
            return (maRange.equal(rRange.maRange));
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        double getMinimum() const
        {
            return maRange.getMinimum();
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        double getMaximum() const
        {
            return maRange.getMaximum();
        }

        /// return difference between upper and lower value. returns 0 for empty sets.
        double getRange() const
        {
            return maRange.getRange();
        }

        /// return middle of upper and lower value. returns 0 for empty sets.
        double getCenter() const
        {
            return maRange.getCenter();
        }

        /// yields true if value is contained in set
        bool isInside(double fValue) const
        {
            return maRange.isInside(fValue);
        }

        /// yields true if rRange is inside, or equal to set
        bool isInside(const B1DRange& rRange) const
        {
            return maRange.isInside(rRange.maRange);
        }

        /// yields true if rRange at least partly inside set
        bool overlaps(const B1DRange& rRange) const
        {
            return maRange.overlaps(rRange.maRange);
        }

        /// yields true if overlaps(rRange) does, and the overlap is larger than infinitesimal
        bool overlapsMore(const B1DRange& rRange) const
        {
            return maRange.overlapsMore(rRange.maRange);
        }

        /// add fValue to the set, expanding as necessary
        void expand(double fValue)
        {
            maRange.expand(fValue);
        }

        /// add rRange to the set, expanding as necessary
        void expand(const B1DRange& rRange)
        {
            maRange.expand(rRange.maRange);
        }

        /// calc set intersection
        void intersect(const B1DRange& rRange)
        {
            maRange.intersect(rRange.maRange);
        }

        /// grow set by fValue on both sides
        void grow(double fValue)
        {
            maRange.grow(fValue);
        }
    };

} // end of namespace basegfx


#endif // INCLUDED_BASEGFX_RANGE_B1DRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
