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

#include <cstdint>
#include <type_traits>
#include <utility>

namespace o3tl
{
// A convenient operator syntax around the standard integer comparison functions:
template <typename T> struct IntCmp
{
    explicit constexpr IntCmp(T theValue)
        : value(theValue)
    {
    }

    T value;
};

// Specializations for character types that can't be used with cmp_* directly, which is OK when
// using the explicit IntCmp:

template <>
struct IntCmp<char> : IntCmp<std::conditional_t<std::is_signed_v<char>, signed char, unsigned char>>
{
    using IntCmp<std::conditional_t<std::is_signed_v<char>, signed char, unsigned char>>::IntCmp;
};

template <> struct IntCmp<char16_t> : IntCmp<std::uint_least16_t>
{
    using IntCmp<std::uint_least16_t>::IntCmp;
};

template <typename T1, typename T2> constexpr bool operator==(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return std::cmp_equal(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator!=(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return std::cmp_not_equal(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator<(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return std::cmp_less(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator>(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return std::cmp_greater(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator<=(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return std::cmp_less_equal(value1.value, value2.value);
}

template <typename T1, typename T2> constexpr bool operator>=(IntCmp<T1> value1, IntCmp<T2> value2)
{
    return std::cmp_greater_equal(value1.value, value2.value);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
