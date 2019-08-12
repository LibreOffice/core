/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_FLOAT_INT_CONVERSION_HXX
#define INCLUDED_O3TL_FLOAT_INT_CONVERSION_HXX

#include <sal/config.h>

#include <cmath>
#include <type_traits>

namespace o3tl
{
// Return true iff `value` of floating-point type `F` converts to a value of integral type `I` no
// smaller than `min`:
template <typename F, typename I>
std::enable_if_t<std::is_floating_point_v<F> && std::is_integral_v<I>, bool>
convertsToAtLeast(F value, I min)
{
    // If `F(min)`, `F(min) - F(1)` are too large in magnitude for `F`'s precision, then they either
    // fall into the same bucket, in which case we should return false if `value` represents that
    // bucket, or they are on the boundary of two adjacent buckets, in which case we should return
    // true if `value`represents the higher bucket containing `F(min)`:
    return value > F(min) - F(1);
}

// Return true iff `value` of floating-point type `F` converts to a value of integral type `I` no
// larger than `max`:
template <typename F, typename I>
std::enable_if_t<std::is_floating_point_v<F> && std::is_integral_v<I>, bool>
convertsToAtMost(F value, I max)
{
    // If `F(max)`, `F(max) + F(1)` are too large in magnitude for `F`'s precision, then they either
    // fall into the same bucket, in which case we should return false if `value` represents that
    // bucket, or they are on the boundary of two adjacent buckets, in which case we should return
    // true if `value`represents the lower bucket containing `F(max)`:
    return value < F(max) + F(1);
}

// Return `value` of floating-point type `F` rounded to the nearest integer away from zero (which
// can be useful in calls to convertsToAtLeast/Most(roundAway(x), n), to reject x that are
// smaller/larger than n because they have a fractional part):
template <typename F> std::enable_if_t<std::is_floating_point_v<F>, F> roundAway(F value)
{
    return value >= 0 ? std::ceil(value) : std::floor(value);
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
