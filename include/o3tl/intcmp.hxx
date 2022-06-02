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

#include <type_traits>
#include <utility>

#include <o3tl/safeint.hxx>

namespace o3tl
{
// An approximation of the C++20 integer comparison functions
// (<https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0586r2.html> "Safe integral
// comparisons"):
#if defined __cpp_lib_integer_comparison_functions

using std::cmp_equal;
using std::cmp_not_equal;
using std::cmp_less;
using std::cmp_greater;
using std::cmp_less_equal;
using std::cmp_greater_equal;

#else

template <typename T1, typename T2> constexpr bool cmp_equal(T1 value1, T2 value2) noexcept
{
    // coverity[same_on_both_sides: FALSE]
    if constexpr (std::is_signed_v<T1> == std::is_signed_v<T2>)
    {
        return value1 == value2;
    }
    else if constexpr (std::is_signed_v<T1>)
    {
        return value1 >= 0 && o3tl::make_unsigned(value1) == value2;
    }
    else
    {
        return value2 >= 0 && value1 == o3tl::make_unsigned(value2);
    }
}

template <typename T1, typename T2> constexpr bool cmp_not_equal(T1 value1, T2 value2) noexcept
{
    return !cmp_equal(value1, value2);
}

template <typename T1, typename T2> constexpr bool cmp_less(T1 value1, T2 value2) noexcept
{
    if constexpr (std::is_signed_v<T1> == std::is_signed_v<T2>)
    {
        return value1 < value2;
    }
    else if constexpr (std::is_signed_v<T1>)
    {
        return value1 < 0 || o3tl::make_unsigned(value1) < value2;
    }
    else
    {
        return value2 >= 0 && value1 < o3tl::make_unsigned(value2);
    }
}

template <typename T1, typename T2> constexpr bool cmp_greater(T1 value1, T2 value2) noexcept
{
    return cmp_less(value2, value1);
}

template <typename T1, typename T2> constexpr bool cmp_less_equal(T1 value1, T2 value2) noexcept
{
    return !cmp_greater(value1, value2);
}

template <typename T1, typename T2> constexpr bool cmp_greater_equal(T1 value1, T2 value2) noexcept
{
    return !cmp_less(value1, value2);
}

#endif

// A convenient operator syntax around the standard integer comparison functions:
template <typename T> struct IntCmp
{
    explicit constexpr IntCmp(T theValue)
        : value(theValue)
    {
    }

    T value;
};

template <typename T1, typename T2> constexpr bool operator==(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return o3tl::cmp_equal(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator!=(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return o3tl::cmp_not_equal(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator<(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return o3tl::cmp_less(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator>(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return o3tl::cmp_greater(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator<=(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return o3tl::cmp_less_equal(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator>=(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return o3tl::cmp_greater_equal(value1.value, value2.value);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
