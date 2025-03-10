/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <cmath>
#include <limits>
#include <type_traits>
#include <o3tl/concepts.hxx>

namespace o3tl
{
// Return true iff `value` of floating-point type `F` converts to a value of integral type `I` no
// smaller than `min`:
template <floating_point F, integral I> constexpr bool convertsToAtLeast(F value, I min)
{
    // If `F(min)`, `F(min) - F(1)` are too large in magnitude for `F`'s precision, then they either
    // fall into the same bucket, in which case we should return false if `value` represents that
    // bucket, or they are on the boundary of two adjacent buckets, in which case we should return
    // true if `value`represents the higher bucket containing `F(min)`:
    return value > F(min) - F(1);
}

// Return true iff `value` of floating-point type `F` converts to a value of integral type `I` no
// larger than `max`:
template <floating_point F, integral I> constexpr bool convertsToAtMost(F value, I max)
{
    // If `F(max)`, `F(max) + F(1)` are too large in magnitude for `F`'s precision, then they either
    // fall into the same bucket, in which case we should return false if `value` represents that
    // bucket, or they are on the boundary of two adjacent buckets, in which case we should return
    // true if `value`represents the lower bucket containing `F(max)`:
    return value < F(max) + F(1);
}

// Casts a floating-point to an integer, avoiding overflow. Used like:
//     sal_Int64 n = o3tl::saturating_cast<sal_Int64>(f);
template <integral I, floating_point F> constexpr I saturating_cast(F f)
{
    if constexpr (std::is_signed_v<I>)
        if (!convertsToAtLeast(f, std::numeric_limits<I>::min()))
            return std::numeric_limits<I>::min();
    if (!convertsToAtMost(f, std::numeric_limits<I>::max()))
        return std::numeric_limits<I>::max();
    return f;
}

// Return `value` of floating-point type `F` rounded to the nearest integer away from zero (which
// can be useful in calls to convertsToAtLeast/Most(roundAway(x), n), to reject x that are
// smaller/larger than n because they have a fractional part):
template <floating_point F> F roundAway(F value)
{
    return value >= 0 ? std::ceil(value) : std::floor(value);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
