/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include <sal/types.h>
#include <tools/long.hxx>
#include <o3tl/safeint.hxx>
#include <cassert>
#include <type_traits>

template<typename T>
inline
typename std::enable_if<
             std::is_signed<T>::value || std::is_floating_point<T>::value, long >::type
MinMax(T nVal, tools::Long nMin, tools::Long nMax)
{
    assert(nMin <= nMax);
    if (nVal >= nMin)
    {
        if (nVal <= nMax)
            return static_cast<tools::Long>(nVal);
        else
           return nMax;
    }
    else
    {
        return nMin;
    }
}

template<typename T>
inline
typename std::enable_if<
             std::is_unsigned<T>::value, long >::type
MinMax(T nVal, tools::Long nMin, tools::Long nMax)
{
    assert(nMin <= nMax);
    if (nMax < 0)
    {
        return nMax;
    }
    else
    {
        if (nMin < 0 || nVal >= static_cast<unsigned long>(nMin))
        {
            if (nVal <= static_cast<unsigned long>(nMax))
                return static_cast<tools::Long>(nVal);
            else
                return nMax;
        }
        else
        {
            return nMin;
        }
    }
}

inline sal_uInt32 AlignedWidth4Bytes(sal_uInt32 nWidthBits)
{
    if (nWidthBits > SAL_MAX_UINT32 - 31)
        nWidthBits = SAL_MAX_UINT32;
    else
        nWidthBits += 31;
    return (nWidthBits >> 5) << 2;
}

inline tools::Long FRound( double fVal )
{
    return fVal > 0.0 ? static_cast<tools::Long>( fVal + 0.5 ) : -static_cast<tools::Long>( -fVal + 0.5 );
}

//valid range:  (-180,180]
template <typename T>
[[nodiscard]] inline typename std::enable_if<std::is_signed<T>::value, T>::type
NormAngle180(T angle)
{
    while (angle <= -180)
        angle += 360;
    while (angle > 180)
        angle -= 360;
    return angle;
}

//valid range:  [0,360)
template <typename T> [[nodiscard]] inline T NormAngle360(T angle)
{
    while (angle < 0)
        angle += 360;
    while (angle >= 360)
        angle -= 360;
    return angle;
}

/** Convert 100th-mm to twips

    A twip is 1/20 of a point, one inch is equal to 72 points, and
    one inch is 2,540 100th-mm.

    Thus:
        twips = n * 72 / 2,540 / 20
              = n * 72 / 127

    Adding 63 (half of 127) fixes truncation issues in int arithmetic.

    This formula is (n>=0) ? (n*72+63) / 127 : (n*72-63) / 127
 */
inline sal_Int64 sanitiseMm100ToTwip(sal_Int64 n)
{
    if (n >= 0)
    {
        if (o3tl::checked_multiply<sal_Int64>(n, 72, n) || o3tl::checked_add<sal_Int64>(n, 63, n))
            n = SAL_MAX_INT64;
    }
    else
    {
        if (o3tl::checked_multiply<sal_Int64>(n, 72, n) || o3tl::checked_sub<sal_Int64>(n, 63, n))
            n = SAL_MIN_INT64;
    }
    return n / 127; // 127 is 2,540 100th-mm divided by 20pts
}

/**
* Convert Twips <-> 100th-mm
*/
inline constexpr sal_Int64 TwipsToHMM(sal_Int64 nTwips) { return (nTwips * 127 + 36) / 72; }
inline constexpr sal_Int64 HMMToTwips(sal_Int64 nHMM) { return (nHMM * 72 + 63) / 127; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
