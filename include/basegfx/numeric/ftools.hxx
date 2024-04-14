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

#include <rtl/math.h>
#include <cmath>
#include <math.h>
#include <basegfx/basegfxdllapi.h>
#include <limits>
#include <algorithm>


// fTools defines

namespace basegfx
{
    /** Round double to nearest integer

        @return the nearest integer
    */
    inline sal_Int32 fround32( double fVal )
    {
        if (fVal >= 0.0)
        {
            if (fVal >= std::numeric_limits<sal_Int32>::max() - .5)
                return std::numeric_limits<sal_Int32>::max();
            return static_cast<sal_Int32>(fVal + .5);
        }
        if (fVal <= std::numeric_limits<sal_Int32>::min() + .5)
            return std::numeric_limits<sal_Int32>::min();
        return static_cast<sal_Int32>(fVal - .5);
    }

    /** Round double to nearest integer

        @return the nearest 64 bit integer
    */
    inline sal_Int64 fround64( double fVal )
    {
        return fVal > 0.0 ? static_cast<sal_Int64>( fVal + .5 ) : -static_cast<sal_Int64>( -fVal + .5 );
    }

    /** Round double to nearest integer

        @return the nearest integer
    */
    template <class Int = sal_Int32>
    inline std::enable_if_t<std::is_integral_v<Int> && std::is_signed_v<Int>, Int>
    fround(double fVal)
    {
        if constexpr (sizeof(Int) == sizeof(sal_Int64))
            return fround64(fVal);
        else
            return fround32(fVal);
    }

    /** Prune a small epsilon range around zero.

        Use this method e.g. for calculating scale values. There, it
        is usually advisable not to set a scaling to 0.0, because that
        yields singular transformation matrices.

        @param fVal
        An arbitrary, but finite and valid number

        @return either fVal, or a small value slightly above (when
        fVal>0) or below (when fVal<0) zero.
     */
    inline double pruneScaleValue( double fVal )
    {
        if(fVal < 0.0)
            return std::min(fVal, -0.00001);
        else
            return std::max(fVal, 0.00001);
    }

    /** Convert value from degrees to radians
     */
    template <int DegMultiple = 1> constexpr double deg2rad( double v )
    {
        // divide first, to get exact values for v being a multiple of
        // 90 degrees
        return v / (90.0 * DegMultiple) * M_PI_2;
    }

    /** Convert value radians to degrees
     */
    template <int DegMultiple = 1> constexpr double rad2deg( double v )
    {
        // divide first, to get exact values for v being a multiple of
        // pi/2
        return v / M_PI_2 * (90.0 * DegMultiple);
    }

    /** Snap v to nearest multiple of fStep, from negative and
        positive side.

        Examples:

        snapToNearestMultiple(-0.1, 0.5) = 0.0
        snapToNearestMultiple(0.1, 0.5) = 0.0
        snapToNearestMultiple(0.25, 0.5) = 0.0
        snapToNearestMultiple(0.26, 0.5) = 0.5
     */
    BASEGFX_DLLPUBLIC double snapToNearestMultiple(double v, const double fStep);

    /** Snap v to the range [0.0 .. fWidth] using modulo
     */
    BASEGFX_DLLPUBLIC double snapToZeroRange(double v, double fWidth);

    /** Snap v to the range [fLow .. fHigh] using modulo
     */
    double snapToRange(double v, double fLow, double fHigh);

    /** return fValue with the sign of fSignCarrier, thus evtl. changed
    */
    inline double copySign(double fValue, double fSignCarrier)
    {
#ifdef _WIN32
        return _copysign(fValue, fSignCarrier);
#else
        return copysign(fValue, fSignCarrier);
#endif
    }

    /** RotateFlyFrame3: Normalize to range defined by [0.0 ... fRange[, independent
        if v is positive or negative.

        Examples:

        normalizeToRange(0.5, -1.0) = 0.0
        normalizeToRange(0.5, 0.0) = 0.0
        normalizeToRange(0.5, 1.0) = 0.5
        normalizeToRange(-0.5, 1.0) = 0.5
        normalizeToRange(-0.3, 1.0) = 0.7
        normalizeToRange(-0.7, 1.0) = 0.3
        normalizeToRange(3.5, 1.0) = 0.5
        normalizeToRange(3.3, 1.0) = 0.3
        normalizeToRange(3.7, 1.0) = 0.7
        normalizeToRange(-3.5, 1.0) = 0.5
        normalizeToRange(-3.3, 1.0) = 0.7
        normalizeToRange(-3.7, 1.0) = 0.3
     */
    BASEGFX_DLLPUBLIC double normalizeToRange(double v, const double fRange);

    namespace fTools
    {
        /// Get threshold value for equalZero and friends
        inline double getSmallValue() { return 0.000000001f; }

        /// Compare against small value
        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool equalZero(const T& rfVal)
        {
            return (fabs(rfVal) <= getSmallValue());
        }

        /// Compare against given small value
        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool equalZero(const T& rfVal, const T& rfSmallValue)
        {
            return (fabs(rfVal) <= rfSmallValue);
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool equal(T const& rfValA, T const& rfValB)
        {
            // changed to approxEqual usage for better numerical correctness
            return rtl_math_approxEqual(rfValA, rfValB);
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool equal(const T& rfValA, const T& rfValB, const T& rfSmallValue)
        {
            return (fabs(rfValA - rfValB) <= rfSmallValue);
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool less(const T& rfValA, const T& rfValB)
        {
            return (rfValA < rfValB && !equal(rfValA, rfValB));
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool lessOrEqual(const T& rfValA, const T& rfValB)
        {
            return (rfValA < rfValB || equal(rfValA, rfValB));
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool more(const T& rfValA, const T& rfValB)
        {
            return (rfValA > rfValB && !equal(rfValA, rfValB));
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool moreOrEqual(const T& rfValA, const T& rfValB)
        {
            return (rfValA > rfValB || equal(rfValA, rfValB));
        }

        template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        inline bool betweenOrEqualEither(const T& rfValA, const T& rfValB, const T& rfValC)
        {
            return (rfValA > rfValB && rfValA < rfValC) || equal(rfValA, rfValB) || equal(rfValA, rfValC);
        }
    };
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
