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
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/basicrange.hxx>
#include <basegfx/range/Range2D.hxx>

namespace basegfx
{
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
    class SAL_WARN_UNUSED B2DRange : public Range2D<double, DoubleTraits>
    {
    public:
        B2DRange()
            : Range2D()
        {}

        /// Create degenerate interval consisting of a single point
        explicit B2DRange(const Tuple2D<ValueType>& rTuple)
            : Range2D(rTuple)
        {
        }

        /// Create proper interval between the two given points
        B2DRange(const Tuple2D<ValueType>& rTuple1,
                 const Tuple2D<ValueType>& rTuple2)
            : Range2D(rTuple1, rTuple2)
        {
        }

        B2DRange(ValueType x1, ValueType y1, ValueType x2, ValueType y2)
            : Range2D(x1, y1, x2, y2)
        {}

        BASEGFX_DLLPUBLIC explicit B2DRange(const B2IRange& rRange);

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

        /** Transform Range by given transformation matrix. */
        BASEGFX_DLLPUBLIC void transform(const B2DHomMatrix& rMatrix);

        /** Translate Range (ie. move).
            Much faster equivalent of transform(createTranslateB2DHomMatrix(xx)). */
        BASEGFX_DLLPUBLIC void translate(double fTranslateX, double fTranslateY);

        inline void translate(const B2DTuple& rTranslate)
        {
            translate(rTranslate.getX(), rTranslate.getY());
        }

        /** Transform Range by given transformation matrix.

            This operation transforms the Range by transforming all four possible
            extrema points (corners) of the given range and building a new one.
            This means that the range will grow evtl. when a shear and/or rotation
            is part of the transformation.
        */
        BASEGFX_DLLPUBLIC B2DRange& operator*=( const ::basegfx::B2DHomMatrix& rMat );

        /** Get a range filled with (0.0, 0.0, 1.0, 1.0) */
        BASEGFX_DLLPUBLIC static const B2DRange& getUnitB2DRange();
    };

    /** Transform B2DRange by given transformation matrix (see operator*=())
    */
    B2DRange operator*( const B2DHomMatrix& rMat, const B2DRange& rB2DRange );

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
    BASEGFX_DLLPUBLIC std::vector<B2DRange>& computeSetDifference(
                                                std::vector<B2DRange>& o_rResult,
                                                const B2DRange& rFirst,
                                                const B2DRange& rSecond);

    /** Write to char stream */
    template<typename charT, typename traits>
    inline std::basic_ostream<charT, traits>& operator<<(
        std::basic_ostream<charT, traits>& stream, const B2DRange& range)
    {
        return stream << range.getWidth() << "x" << range.getHeight() << "@" << range.getMinimum();
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
