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

#ifndef INCLUDED_O3TL_TYPED_FLAGS_SET_HXX
#define INCLUDED_O3TL_TYPED_FLAGS_SET_HXX

#include <sal/config.h>

#include <cassert>
#include <type_traits>

namespace o3tl {

namespace detail {

template<typename T> inline
typename std::enable_if<std::is_signed<T>::value, bool>::type isNonNegative(
    T value)
{
    return value >= 0;
}

template<typename T> inline
typename std::enable_if<std::is_unsigned<T>::value, bool>::type isNonNegative(T)
{
    return true;
}

}

template<typename T> struct typed_flags {};

#if defined __GNUC__ && __GNUC__ == 4 && __GNUC_MINOR__ <= 6 && \
    !defined __clang__
// use largest sensible unsigned type as fallback
#define O3TL_STD_UNDERLYING_TYPE_E unsigned long long
#else
#define O3TL_STD_UNDERLYING_TYPE_E typename std::underlying_type<E>::type
#endif

/// Mark a (scoped) enumeration as a set of bit flags, with accompanying
/// operations.
///
///   template<>
///   struct o3tl::typed_flags<TheE>: o3tl::is_typed_flags<TheE, TheM> {};
///
/// All relevant values must be non-negative.  (Typically, the enumeration's
/// underlying type will either be fixed and unsigned, or it will be unfixed---
/// and can thus default to a signed type---and all enumerators will have non-
/// negative values.)
///
/// \param E the enumeration type.
/// \param M the all-bits-set value for the bit flags.
template<typename E, O3TL_STD_UNDERLYING_TYPE_E M>
struct is_typed_flags {
    static_assert(
        M >= 0, "is_typed_flags expects only non-negative bit values");

    typedef E Self;

    class Wrap {
    public:
        explicit Wrap(O3TL_STD_UNDERLYING_TYPE_E value):
            value_(value)
        { assert(detail::isNonNegative(value)); }

        operator E() { return static_cast<E>(value_); }

        explicit operator O3TL_STD_UNDERLYING_TYPE_E() { return value_; }

        explicit operator bool() { return value_ != 0; }

    private:
        O3TL_STD_UNDERLYING_TYPE_E value_;
    };

    static O3TL_STD_UNDERLYING_TYPE_E const mask = M;
};

}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator ~(E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::typed_flags<E>::mask
        & ~static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator ~(
    typename o3tl::typed_flags<E>::Wrap rhs)
{
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::typed_flags<E>::mask
        & ~static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator ^(E lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        ^ static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator ^(
    E lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        ^ static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator ^(
    typename o3tl::typed_flags<E>::Wrap lhs, E rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        ^ static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator ^(
    typename o3tl::typed_flags<E>::Wrap lhs,
    typename o3tl::typed_flags<E>::Wrap rhs)
{
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        ^ static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator &(E lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        & static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator &(
    E lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        & static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator &(
    typename o3tl::typed_flags<E>::Wrap lhs, E rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        & static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator &(
    typename o3tl::typed_flags<E>::Wrap lhs,
    typename o3tl::typed_flags<E>::Wrap rhs)
{
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        & static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator |(E lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        | static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator |(
    E lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        | static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator |(
    typename o3tl::typed_flags<E>::Wrap lhs, E rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        | static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Wrap operator |(
    typename o3tl::typed_flags<E>::Wrap lhs,
    typename o3tl::typed_flags<E>::Wrap rhs)
{
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)
        | static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator &=(E & lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    lhs = lhs & rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator &=(
    E & lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    lhs = lhs & rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator |=(E & lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    lhs = lhs | rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator |=(
    E & lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    lhs = lhs | rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator ^=(E & lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(rhs)));
    lhs = lhs ^ rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator ^=(
    E & lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            static_cast<O3TL_STD_UNDERLYING_TYPE_E>(lhs)));
    lhs = lhs ^ rhs;
    return lhs;
}

#endif /* INCLUDED_O3TL_TYPED_FLAGS_SET_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
