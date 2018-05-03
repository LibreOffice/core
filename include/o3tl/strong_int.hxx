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

#ifndef INCLUDED_O3TL_STRONG_INT_HXX
#define INCLUDED_O3TL_STRONG_INT_HXX

#include <sal/config.h>
#include <limits>
#include <cassert>
#include <type_traits>

#include <config_global.h>

namespace o3tl
{

#if HAVE_CXX14_CONSTEXPR

namespace detail {

template<typename T1, typename T2> constexpr
typename std::enable_if<
    std::is_signed<T1>::value && std::is_signed<T2>::value, bool>::type
isInRange(T2 value) {
    return value >= std::numeric_limits<T1>::min()
        && value <= std::numeric_limits<T1>::max();
}

template<typename T1, typename T2> constexpr
typename std::enable_if<
    std::is_signed<T1>::value && std::is_unsigned<T2>::value, bool>::type
isInRange(T2 value) {
    return value
        <= static_cast<typename std::make_unsigned<T1>::type>(
            std::numeric_limits<T1>::max());
}

template<typename T1, typename T2> constexpr
typename std::enable_if<
    std::is_unsigned<T1>::value && std::is_signed<T2>::value, bool>::type
isInRange(T2 value) {
    return value >= 0
        && (static_cast<typename std::make_unsigned<T2>::type>(value)
            <= std::numeric_limits<T1>::max());
}

template<typename T1, typename T2> constexpr
typename std::enable_if<
    std::is_unsigned<T1>::value && std::is_unsigned<T2>::value, bool>::type
isInRange(T2 value) {
    return value <= std::numeric_limits<T1>::max();
}

}

#endif

///
/// Wrap up an integer type so that we prevent accidental conversion to other integer types.
///
/// e.g.
///   struct MyIntTag {};
///   typedef o3tl::strong_int<unsigned, MyIntTag> MyInt;
///
/// \param UNDERLYING_TYPE the underlying scalar type
/// \param PHANTOM_TYPE    a type tag, used to distinguish this instantiation of the template
///                        from other instantiations with the same UNDERLYING_TYPE.
///
template <typename UNDERLYING_TYPE, typename PHANTOM_TYPE>
struct strong_int
{
public:
    template<typename T> explicit constexpr strong_int(
        T value,
        typename std::enable_if<std::is_integral<T>::value, int>::type = 0):
        m_value(value)
    {
#if !defined __COVERITY__ && HAVE_CXX14_CONSTEXPR
        // catch attempts to pass in out-of-range values early
        assert(detail::isInRange<UNDERLYING_TYPE>(value)
               && "out of range");
#endif
    }
    strong_int() : m_value(0) {}

    explicit constexpr operator UNDERLYING_TYPE() const { return m_value; }
    explicit operator bool() const { return m_value != 0; }
    UNDERLYING_TYPE get() const { return m_value; }

    bool operator<(strong_int const & other) const { return m_value < other.m_value; }
    bool operator<=(strong_int const & other) const { return m_value <= other.m_value; }
    bool operator>(strong_int const & other) const { return m_value > other.m_value; }
    bool operator>=(strong_int const & other) const { return m_value >= other.m_value; }
    bool operator==(strong_int const & other) const { return m_value == other.m_value; }
    bool operator!=(strong_int const & other) const { return m_value != other.m_value; }
    strong_int& operator++() { ++m_value; return *this; }
    strong_int operator++(int) { UNDERLYING_TYPE nOldValue = m_value; ++m_value; return strong_int(nOldValue); }
    strong_int& operator--() { --m_value; return *this; }
    strong_int operator--(int) { UNDERLYING_TYPE nOldValue = m_value; --m_value; return strong_int(nOldValue); }
    strong_int& operator+=(strong_int const & other) { m_value += other.m_value; return *this; }
    strong_int& operator-=(strong_int const & other) { m_value -= other.m_value; return *this; }

    bool anyOf(strong_int v) const {
      return *this == v;
    }

    template<typename... Args>
    bool anyOf(strong_int first, Args... args) const {
      return *this == first || anyOf(args...);
    }

private:
    UNDERLYING_TYPE m_value;
};

template <typename UT, typename PT>
strong_int<UT,PT> operator+(strong_int<UT,PT> const & lhs, strong_int<UT,PT> const & rhs)
{
    return strong_int<UT,PT>(lhs.get() + rhs.get());
}

template <typename UT, typename PT>
strong_int<UT,PT> operator-(strong_int<UT,PT> const & lhs, strong_int<UT,PT> const & rhs)
{
    return strong_int<UT,PT>(lhs.get() - rhs.get());
}

}; // namespace o3tl

#endif /* INCLUDED_O3TL_STRONG_INT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
