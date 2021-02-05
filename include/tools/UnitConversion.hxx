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
#include <limits>
#include <type_traits>

// Ensure correct rounding for both positive and negative integers
template <unsigned mul, unsigned div, typename I, std::enable_if_t<std::is_integral_v<I>, int> = 0>
constexpr sal_Int64 MulDiv(I n)
{
    static_assert(div > 0);
    assert(n < std::numeric_limits<sal_Int64>::max() / mul
           && n > std::numeric_limits<sal_Int64>::min() / mul);
    return (n >= 0) ? (sal_Int64(n) * mul + div / 2) / div : (sal_Int64(n) * mul - div / 2) / div;
}
template <unsigned mul, unsigned div, typename F,
          std::enable_if_t<std::is_floating_point_v<F>, int> = 0>
constexpr double MulDiv(F f)
{
    static_assert(div > 0);
    return f * (double(mul) / div);
}

template <typename N> constexpr auto convertTwipToMm100(N n) { return MulDiv<127, 72>(n); }
template <typename N> constexpr auto convertMm100ToTwip(N n) { return MulDiv<72, 127>(n); }

constexpr sal_Int64 sanitiseMm100ToTwip(sal_Int64 n)
{
    if (n >= std::numeric_limits<sal_Int64>::max() / 72
        || n <= std::numeric_limits<sal_Int64>::min() / 72)
        return n / 127 * 72; // do without correction; can not overflow here
    else
        return convertMm100ToTwip(n);
}

constexpr sal_Int64 convertPointToTwip(sal_Int64 nNumber) { return nNumber * 20; }
constexpr double convertPointToTwip(double fNumber) { return fNumber * 20.0; }

template <typename N> constexpr auto convertPointToMm100(N n) { return MulDiv<2540, 72>(n); }
template <typename N> constexpr auto convertMm100ToPoint(N n) { return MulDiv<72, 2540>(n); }

// PPT's "master unit" (1/576 inch) <=> mm/100
template <typename N> constexpr auto convertMasterUnitToMm100(N n) { return MulDiv<2540, 576>(n); }
template <typename N> constexpr auto convertMm100ToMasterUnit(N n) { return MulDiv<576, 2540>(n); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
