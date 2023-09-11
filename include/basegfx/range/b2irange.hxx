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

#pragma once

#include <ostream>
#include <vector>

#include <basegfx/basegfxdllapi.h>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/basicrange.hxx>
#include <basegfx/range/Range2D.hxx>

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
    class B2IRange : public Range2D<sal_Int32, Int32Traits>
    {
    public:
        B2IRange()
            : Range2D()
        {}

        /// Create degenerate interval consisting of a single point
        explicit B2IRange(const Tuple2D<ValueType>& rTuple)
            : Range2D(rTuple)
        {
        }

        /// Create proper interval between the two given points
        B2IRange(const Tuple2D<ValueType>& rTuple1,
                 const Tuple2D<ValueType>& rTuple2)
            : Range2D(rTuple1, rTuple2)
        {
        }

        B2IRange(ValueType x1, ValueType y1, ValueType x2, ValueType y2)
            : Range2D(x1, y1, x2, y2)
        {}

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
    BASEGFX_DLLPUBLIC std::vector<B2IRange>& computeSetDifference(
                                                std::vector<B2IRange>& o_rResult,
                                                const B2IRange& rFirst,
                                                const B2IRange& rSecond);

    /** Write to char stream */
    template<typename charT, typename traits>
    inline std::basic_ostream<charT, traits>& operator<<(
        std::basic_ostream<charT, traits>& stream, const B2IRange& range)
    {
        if (range.isEmpty())
            return stream << "EMPTY";
        else
            return stream << range.getWidth() << 'x' << range.getHeight() << "@" << range.getMinimum();
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
