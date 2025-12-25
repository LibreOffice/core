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

#include <algorithm>
#include <cassert>
#include <limits>

namespace o3tl
{
// For use when std::inf is an acceptable result
[[nodiscard]]
#if defined __clang__ || defined __GNUC__
    __attribute__((no_sanitize("float-divide-by-zero")))
#endif
    inline double
    div_allow_zero(double a, double b)
{
#if defined(__COVERITY__) && __COVERITY_MAJOR__ <= 2024
    assert(b != 0 && "suppress floating point divide_by_zero");
#endif
    return a / b;
}

// std::min wrapped to inform coverity that the result is now deemed sanitized
// coverity[ -taint_source ]
template <typename T>[[nodiscard]] inline T sanitizing_min(T a, T b) { return std::min(a, b); }

// To sanitize in/de-crementing value where the result is known by the caller to be guaranteed to fit in
// the source type range without over/under-flow
[[nodiscard]] inline unsigned short sanitizing_inc(unsigned short value)
{
    int res = value + 1;
    assert(res <= std::numeric_limits<unsigned short>::max()
           && "nValue was supposed to be incrementable without overflow");
    return static_cast<unsigned short>(res);
}

[[nodiscard]] inline unsigned short sanitizing_dec(unsigned short value)
{
    int res = value - 1;
    assert(res >= 0 && "nValue was supposed to be decrementable without underflow");
    return static_cast<unsigned short>(res);
}

// coverity[ -taint_source ]
template <typename T1, typename T2> constexpr T1 sanitizing_cast(T2 value)
{
    assert(value >= std::numeric_limits<T1>::min() && value <= std::numeric_limits<T1>::max()
           && "value was supposed to be within bounds of destination type");
    return value;
}

[[nodiscard]] inline short sanitizing_inc(short value)
{
    int res = value + 1;
    return o3tl::sanitizing_cast<short>(res);
}

[[nodiscard]] inline short sanitizing_dec(short value)
{
    int res = value - 1;
    return o3tl::sanitizing_cast<short>(res);
}

// A hammer that can be used when coverity refuses to accept that code is safe
// from overflow.
template <typename T> inline void untaint_for_overrun([[maybe_unused]] T& a)
{
#if defined(__COVERITY__) && __COVERITY_MAJOR__ <= 2024
    __coverity_mark_pointee_as_sanitized__(&a, OVERRUN);
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
