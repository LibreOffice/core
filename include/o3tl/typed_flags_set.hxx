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

#include <o3tl/underlyingenumvalue.hxx>
#include <sal/types.h>

namespace o3tl {

namespace detail {

template<typename T> constexpr
typename std::enable_if<std::is_signed<T>::value, bool>::type isNonNegative(
    T value)
{
    return value >= 0;
}

template<typename T> constexpr
typename std::enable_if<std::is_unsigned<T>::value, bool>::type isNonNegative(T)
{
    return true;
}

}

template<typename T> struct typed_flags {};

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
template<typename E, typename std::underlying_type<E>::type M>
struct is_typed_flags {
    static_assert(
        M >= 0, "is_typed_flags expects only non-negative bit values");

    typedef E Self;

    class Wrap {
    public:
        typedef is_typed_flags Unwrapped;

        explicit constexpr Wrap(typename std::underlying_type<E>::type value):
            value_(value)
        {
            assert(detail::isNonNegative(value));
            assert(
                static_cast<typename std::underlying_type<E>::type>(~0) == M
                    // avoid "operands don't affect result" warnings when M
                    // covers all bits of the underlying type
                || (value & ~M) == 0);
        }

        constexpr operator E() const { return static_cast<E>(value_); }

        explicit constexpr operator typename std::underlying_type<E>::type()
            const
        { return value_; }

        explicit constexpr operator bool() const { return value_ != 0; }

    private:
        typename std::underlying_type<E>::type value_;
    };

    static typename std::underlying_type<E>::type const mask = M;
};

}

template<typename E>
constexpr typename o3tl::typed_flags<E>::Wrap operator ~(E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::typed_flags<E>::mask
        & ~o3tl::underlyingEnumValue(rhs));
}

template<typename E> constexpr typename o3tl::typed_flags<E>::Wrap operator ~(
    typename o3tl::typed_flags<E>::Wrap rhs)
{
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::typed_flags<E>::mask
        & ~o3tl::underlyingEnumValue<E>(rhs));
}

template<typename E> constexpr typename o3tl::typed_flags<E>::Wrap operator ^(
    E lhs, E rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue(lhs)
        ^ o3tl::underlyingEnumValue(rhs));
}

template<typename E> constexpr typename o3tl::typed_flags<E>::Wrap operator ^(
    E lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue(lhs)
        ^ o3tl::underlyingEnumValue<E>(rhs));
}

template<typename E> constexpr typename o3tl::typed_flags<E>::Wrap operator ^(
    typename o3tl::typed_flags<E>::Wrap lhs, E rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue<E>(lhs)
        ^ o3tl::underlyingEnumValue(rhs));
}

template<typename W> constexpr
typename o3tl::typed_flags<typename W::Unwrapped::Self>::Wrap operator ^(
    W lhs, W rhs)
{
    return static_cast<W>(
        o3tl::underlyingEnumValue<typename W::Unwrapped::Self>(lhs)
        ^ o3tl::underlyingEnumValue<typename W::Unwrapped::Self>(rhs));
}

template<typename E>
constexpr typename o3tl::typed_flags<E>::Wrap operator &(E lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue(lhs)
        & o3tl::underlyingEnumValue(rhs));
}

template<typename E> constexpr typename o3tl::typed_flags<E>::Wrap operator &(
    E lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue(lhs)
        & o3tl::underlyingEnumValue<E>(rhs));
}

template<typename E> constexpr typename o3tl::typed_flags<E>::Wrap operator &(
    typename o3tl::typed_flags<E>::Wrap lhs, E rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue<E>(lhs)
        & o3tl::underlyingEnumValue(rhs));
}

template<typename W> constexpr
typename o3tl::typed_flags<typename W::Unwrapped::Self>::Wrap operator &(
    W lhs, W rhs)
{
    return static_cast<W>(
        o3tl::underlyingEnumValue<typename W::Unwrapped::Self>(lhs)
        & o3tl::underlyingEnumValue<typename W::Unwrapped::Self>(rhs));
}

template<typename E>
constexpr typename o3tl::typed_flags<E>::Wrap operator |(E lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue(lhs)
        | o3tl::underlyingEnumValue(rhs));
}

template<typename E> constexpr typename o3tl::typed_flags<E>::Wrap operator |(
    E lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue(lhs)
        | o3tl::underlyingEnumValue<E>(rhs));
}

template<typename E> constexpr typename o3tl::typed_flags<E>::Wrap operator |(
    typename o3tl::typed_flags<E>::Wrap lhs, E rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    return static_cast<typename o3tl::typed_flags<E>::Wrap>(
        o3tl::underlyingEnumValue<E>(lhs)
        | o3tl::underlyingEnumValue(rhs));
}

template<typename W> constexpr
typename o3tl::typed_flags<typename W::Unwrapped::Self>::Wrap operator |(
    W lhs, W rhs)
{
    return static_cast<W>(
        o3tl::underlyingEnumValue<typename W::Unwrapped::Self>(lhs)
        | o3tl::underlyingEnumValue<typename W::Unwrapped::Self>(rhs));
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator &=(E & lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    lhs = lhs & rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator &=(
    E & lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    lhs = lhs & rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator |=(E & lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    lhs = lhs | rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator |=(
    E & lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    lhs = lhs | rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator ^=(E & lhs, E rhs) {
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(rhs)));
    lhs = lhs ^ rhs;
    return lhs;
}

template<typename E>
inline typename o3tl::typed_flags<E>::Self operator ^=(
    E & lhs, typename o3tl::typed_flags<E>::Wrap rhs)
{
    assert(
        o3tl::detail::isNonNegative(
            o3tl::underlyingEnumValue(lhs)));
    lhs = lhs ^ rhs;
    return lhs;
}

#endif /* INCLUDED_O3TL_TYPED_FLAGS_SET_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
