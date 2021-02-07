/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/types.h>
#include <cassert>
#include <type_traits>

template <typename I> constexpr bool isBetween(I n, sal_Int64 min, sal_Int64 max)
{
    assert(max > 0 && min < 0);
    if constexpr (std::is_signed_v<I>)
        return n >= min && n <= max;
    else
        return n <= sal_uInt64(max);
}

constexpr int gcd(int a, int b) { return b == 0 ? a : gcd(b, a % b); }

// Ensure correct rounding for both positive and negative integers
template <int mul, int div, typename I, std::enable_if_t<std::is_integral_v<I>, int> = 0>
constexpr sal_Int64 MulDiv(I n)
{
    static_assert(mul > 0 && div > 0);
    constexpr int m = mul / gcd(mul, div), d = div / gcd(mul, div);
    assert(isBetween(n, (SAL_MIN_INT64 + d / 2) / m, (SAL_MAX_INT64 - d / 2) / m));
    return (n >= 0 ? (sal_Int64(n) * m + d / 2) : (sal_Int64(n) * m - d / 2)) / d;
}
template <int mul, int div, typename F, std::enable_if_t<std::is_floating_point_v<F>, int> = 0>
constexpr double MulDiv(F f)
{
    static_assert(mul > 0 && div > 0);
    return f * (double(mul) / div);
}

template <int mul, int div, typename I, std::enable_if_t<std::is_integral_v<I>, int> = 0>
constexpr sal_Int64 sanitizeMulDiv(I n)
{
    constexpr int m = mul / gcd(mul, div), d = div / gcd(mul, div);
    if constexpr (m > d)
        if (!isBetween(n, SAL_MIN_INT64 / m * d + d / 2, SAL_MAX_INT64 / m * d - d / 2))
            return n > 0 ? SAL_MAX_INT64 : SAL_MIN_INT64; // saturate
    if (!isBetween(n, (SAL_MIN_INT64 + d / 2) / m, (SAL_MAX_INT64 - d / 2) / m))
        return (n >= 0 ? n + d / 2 : n - d / 2) / d * m; // divide before multiplication
    return MulDiv<mul, div>(n);
}

template <typename N> constexpr auto convertTwipToMm100(N n) { return MulDiv<127, 72>(n); }
template <typename N> constexpr auto convertMm100ToTwip(N n) { return MulDiv<72, 127>(n); }

constexpr sal_Int64 sanitiseMm100ToTwip(sal_Int64 n) { return sanitizeMulDiv<72, 127>(n); }

template <typename N> constexpr auto convertPointToTwip(N n) { return MulDiv<20, 1>(n); }

template <typename N> constexpr auto convertPointToMm100(N n) { return MulDiv<2540, 72>(n); }
template <typename N> constexpr auto convertMm100ToPoint(N n) { return MulDiv<72, 2540>(n); }

// PPT's "master unit" (1/576 inch) <=> mm/100
template <typename N> constexpr auto convertMasterUnitToMm100(N n) { return MulDiv<2540, 576>(n); }
template <typename N> constexpr auto convertMm100ToMasterUnit(N n) { return MulDiv<576, 2540>(n); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
