/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_ANY_HXX
#define INCLUDED_O3TL_ANY_HXX

#include <sal/config.h>

#include <type_traits>

#include <boost/optional.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace o3tl {

namespace detail {

struct Void {};

template<typename T> struct Optional { using type = T const *; };
template<> struct Optional<void> { using type = boost::optional<Void>; };
template<> struct Optional<bool> { using type = boost::optional<bool>; };
template<> struct Optional<sal_Int8> {
    using type = boost::optional<sal_Int8>;
};
template<> struct Optional<sal_Int16> {
    using type = boost::optional<sal_Int16>;
};
template<> struct Optional<sal_uInt16> {
    using type = boost::optional<sal_uInt16>;
};
template<> struct Optional<sal_Int32> {
    using type = boost::optional<sal_Int32>;
};
template<> struct Optional<sal_uInt32> {
    using type = boost::optional<sal_uInt32>;
};
template<> struct Optional<sal_Int64> {
    using type = boost::optional<sal_Int64>;
};
template<> struct Optional<sal_uInt64> {
    using type = boost::optional<sal_uInt64>;
};
template<> struct Optional<float> { using type = boost::optional<float>; };
template<> struct Optional<double> { using type = boost::optional<double>; };

template<typename T> boost::optional<T> tryGetConverted(
    css::uno::Any const & any)
{
    T v;
    return (any >>= v) ? boost::optional<T>(v) : boost::optional<T>();
}

template<typename T> T const * tryGetByType(css::uno::Any const & any) {
    return any.getValueType() == cppu::UnoType<T>::get()
        ? static_cast<T const *>(any.getValue()) : nullptr;
}

template<typename> struct IsSequence: std::false_type {};
template<typename T> struct IsSequence<css::uno::Sequence<T>>: std::true_type
{};

template<typename> struct IsReference: std::false_type {};
template<typename T> struct IsReference<css::uno::Reference<T>>: std::true_type
{};

}

template<typename T>
typename std::enable_if<
    !(detail::IsSequence<T>::value || std::is_enum<T>::value
      || detail::IsReference<T>::value),
    typename detail::Optional<T>::type>::type
tryGet(css::uno::Any const & any) { // struct and exception
    return detail::tryGetByType<T>(any); //TODO: derived
}

template<> typename detail::Optional<void>::type tryGet<void>(
    css::uno::Any const & any)
{
    return any.hasValue()
        ? boost::optional<detail::Void>()
        : boost::optional<detail::Void>(detail::Void());
}

template<> typename detail::Optional<bool>::type tryGet<bool>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<bool>(any);
}

template<> typename detail::Optional<sal_Int8>::type tryGet<sal_Int8>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int8>(any);
}

template<> typename detail::Optional<sal_Int16>::type tryGet<sal_Int16>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int16>(any);
}

template<> typename detail::Optional<sal_uInt16>::type tryGet<sal_uInt16>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt16>(any);
}

template<> typename detail::Optional<sal_Int32>::type tryGet<sal_Int32>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int32>(any);
}

template<> typename detail::Optional<sal_uInt32>::type tryGet<sal_uInt32>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt32>(any);
}

template<> typename detail::Optional<sal_Int64>::type tryGet<sal_Int64>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int64>(any);
}

template<> typename detail::Optional<sal_uInt64>::type tryGet<sal_uInt64>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt64>(any);
}

template<> typename detail::Optional<float>::type tryGet<float>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<float>(any);
}

template<> typename detail::Optional<double>::type tryGet<double>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<double>(any);
}

template<> typename detail::Optional<sal_Unicode>::type tryGet<sal_Unicode>(
    css::uno::Any const & any)
{
    return detail::tryGetByType<sal_Unicode>(any);
}

template<> typename detail::Optional<OUString>::type tryGet<OUString>(
    css::uno::Any const & any)
{
    return detail::tryGetByType<OUString>(any);
}

template<> typename detail::Optional<css::uno::Type>::type
tryGet<css::uno::Type>(css::uno::Any const & any)
{
    return detail::tryGetByType<css::uno::Type>(any);
}

template<typename T>
typename std::enable_if<
    detail::IsSequence<T>::value, typename detail::Optional<T>::type>::type
tryGet(css::uno::Any const & any)
{
    return detail::tryGetByType<T>(any);
}

template<typename T>
typename std::enable_if<
    std::is_enum<T>::value, typename detail::Optional<T>::type>::type
tryGet(css::uno::Any const & any)
{
    return detail::tryGetByType<T>(any);
}

template<typename T>
typename std::enable_if<
    detail::IsReference<T>::value, typename detail::Optional<T>::type>::type
tryGet(css::uno::Any const &) = delete; //TODO

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
