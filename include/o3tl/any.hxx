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
#include <utility>

#include <boost/optional.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppu/unotype.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

// Some functionality related to css::uno::Any that would ideally be part of
// <com/sun/star/uno/Any.hxx>, but (for now) cannot be for some reason.

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
} } } }

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
template<typename T> struct Optional<css::uno::Reference<T>> {
    using type = boost::optional<css::uno::Reference<T>>;
};
template<> struct Optional<css::uno::Reference<css::uno::XInterface>> {
    using type = css::uno::Reference<css::uno::XInterface> const *;
};

template<typename> struct IsDerivedReference: std::false_type {};
template<typename T> struct IsDerivedReference<css::uno::Reference<T>>:
    std::true_type
{};
template<> struct IsDerivedReference<css::uno::Reference<css::uno::XInterface>>:
    std::false_type
{};

template<typename> struct IsUnoSequenceType: std::false_type {};
template<typename T> struct IsUnoSequenceType<cppu::UnoSequenceType<T>>:
    std::true_type
{};

template<typename T> inline boost::optional<T> tryGetConverted(
    css::uno::Any const & any)
{
    T v;
    return (any >>= v)
        ? boost::optional<T>(std::move(v)) : boost::optional<T>();
}

}

/** Try to get the value of a specific type from an Any.

    In trying to obtain a value, the same set of conversions as supported by
    ">>=" are considere.

    The returned object is a proxy.  Proxies can be either positive or negative.
    Each proxy can be contextually converted to bool, yielding true iff the
    proxy is positive.  For a positive proxy P representing a value of requested
    type T, for any T other than void, the expression *P yields that value of
    type T.  (Technically, the proxy is either a plain pointer or a
    boost::optional, depending on whether a plain pointer into the given Any can
    be returned for the specified type.)

    @note Ideally this would be a public member function of css::uno::Any (at
    least conditional on LIBO_INTERNAL_ONLY, as it requires C++11).  However, as
    std::optional (which would be needed to implement the proxies) is only
    available since C++14, we need to use boost::optional for now.  But To not
    make every entity that includes <com/sun/star/uno/Any.hxx> depend on
    boost_headers, keep this here for now.

    @tparam T  the C++ representation of a UNO type that can be contained in a
    UNO ANY (i.e., any UNO type other than ANY itself).  The legacy C++
    representations sal_Bool, cppu::UnoVoidType, cppu::UnoUnsignedShortType,
    cppu::UnoCharType, and cppu::UnoSequenceType are not supported.

    @param any  an Any value.

    @return a positive proxy for the value of the specfied type obtained from
    the given Any, or a negative proxy if no such value can be obtained.
*/
template<typename T> inline
typename std::enable_if<
    !(detail::IsDerivedReference<T>::value
      || detail::IsUnoSequenceType<T>::value),
    typename detail::Optional<T>::type>::type
tryGet(css::uno::Any const & any) {
    // CHAR, STRING, TYPE, sequence types, enum types, struct types, exception
    // types, and com.sun.star.uno.XInterface interface type:
    return cppu::UnoType<T>::get().isAssignableFrom(any.getValueType())
        ? static_cast<T const *>(any.getValue()) : nullptr;
}

template<> inline detail::Optional<void>::type tryGet<void>(
    css::uno::Any const & any)
{
    return any.hasValue()
        ? boost::optional<detail::Void>()
        : boost::optional<detail::Void>(detail::Void());
}

template<> inline detail::Optional<bool>::type tryGet<bool>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<bool>(any);
}

template<> inline detail::Optional<sal_Int8>::type tryGet<sal_Int8>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int8>(any);
}

template<> inline detail::Optional<sal_Int16>::type tryGet<sal_Int16>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int16>(any);
}

template<> inline detail::Optional<sal_uInt16>::type tryGet<sal_uInt16>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt16>(any);
}

template<> inline detail::Optional<sal_Int32>::type tryGet<sal_Int32>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int32>(any);
}

template<> inline detail::Optional<sal_uInt32>::type tryGet<sal_uInt32>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt32>(any);
}

template<> inline detail::Optional<sal_Int64>::type tryGet<sal_Int64>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int64>(any);
}

template<> inline detail::Optional<sal_uInt64>::type tryGet<sal_uInt64>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt64>(any);
}

template<> inline detail::Optional<float>::type tryGet<float>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<float>(any);
}

template<> inline detail::Optional<double>::type tryGet<double>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<double>(any);
}

template<> detail::Optional<css::uno::Any>::type tryGet<css::uno::Any>(
    css::uno::Any const &) = delete;

template<> detail::Optional<sal_Bool>::type tryGet<sal_Bool>(
    css::uno::Any const &) = delete;

template<> detail::Optional<cppu::UnoVoidType>::type tryGet<cppu::UnoVoidType>(
    css::uno::Any const &) = delete;

template<> detail::Optional<cppu::UnoUnsignedShortType>::type
tryGet<cppu::UnoUnsignedShortType>(css::uno::Any const &) = delete;

template<> detail::Optional<cppu::UnoCharType>::type tryGet<cppu::UnoCharType>(
    css::uno::Any const &) = delete;

template<typename T> inline
typename std::enable_if<
    detail::IsDerivedReference<T>::value,
    typename detail::Optional<T>::type>::type
tryGet(css::uno::Any const & any) {
    return detail::tryGetConverted<T>(any);
}

/** Get the value of a specific type from an Any, throwing an exception on
    failure.

    @note Ideally this would be a public member function of css::uno::Any.  See
    tryGet for details.

    @tparam T  the C++ representation of a UNO type that can be contained in a
    UNO ANY.  See tryGet for details.

    @param any  an Any value.

    @return a positive proxy for the value of the specfied type obtained from
    the given Any.  See tryGet for details.

    @throws css::uno::RuntimeException  when a value of the requested type
    cannot be obtained.
*/
template<typename T> inline typename detail::Optional<T>::type doGet(
    css::uno::Any const & any)
{
    auto opt = tryGet<T>(any);
    if (!opt) {
        throw css::uno::RuntimeException(
            OUString(
                cppu_Any_extraction_failure_msg(
                    &any, cppu::UnoType<T>::get().getTypeLibType()),
                SAL_NO_ACQUIRE));
    }
    return opt;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
