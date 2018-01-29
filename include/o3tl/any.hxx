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

#include <cassert>
#include <type_traits>
#include <utility>

#include <boost/optional.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppu/unotype.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

// Some functionality related to css::uno::Any that would ideally be part of
// <com/sun/star/uno/Any.hxx>, but (for now) cannot be for some reason.

namespace o3tl {

namespace detail {

struct Void {};

template<typename T> struct Optional { using type = T const *; };
template<> struct Optional<void> { using type = boost::optional<Void const>; };
template<> struct Optional<bool> { using type = boost::optional<bool const>; };
template<> struct Optional<sal_Int8> {
    using type = boost::optional<sal_Int8 const>;
};
template<> struct Optional<sal_Int16> {
    using type = boost::optional<sal_Int16 const>;
};
template<> struct Optional<sal_uInt16> {
    using type = boost::optional<sal_uInt16 const>;
};
template<> struct Optional<sal_Int32> {
    using type = boost::optional<sal_Int32 const>;
};
template<> struct Optional<sal_uInt32> {
    using type = boost::optional<sal_uInt32 const>;
};
template<> struct Optional<sal_Int64> {
    using type = boost::optional<sal_Int64 const>;
};
template<> struct Optional<sal_uInt64> {
    using type = boost::optional<sal_uInt64 const>;
};
template<> struct Optional<float> {
    using type = boost::optional<float const>;
};
template<> struct Optional<double> {
    using type = boost::optional<double const>;
};
template<typename T> struct Optional<css::uno::Reference<T>> {
    using type = boost::optional<css::uno::Reference<T> const>;
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

template<typename T> inline boost::optional<T const> tryGetConverted(
    css::uno::Any const & any)
{
    T v;
    return (any >>= v)
        ? boost::optional<T const>(std::move(v)) : boost::optional<T const>();
}

}

/** Try to access the value of a specific type stored in an Any.

    In trying to obtain a value, the same set of conversions as supported by
    ">>=" are considered.

    The returned object is a proxy.  Proxies can be either positive or negative.
    Each proxy can be contextually converted to bool, yielding true iff the
    proxy is positive.  For a positive proxy P representing a value of requested
    type T, for any T other than void, the expression *P yields that value of
    type T.  (Technically, the proxy is either a plain pointer or a
    boost::optional, depending on whether a plain pointer into the given Any can
    be returned for the specified type.)

    @attention A proxy returned from this function must not outlive the
    corresponding Any passed into this function (as it may constitute a pointer
    into the Any's internals).  That is the reason why this function is
    restricted to lvalue arguments (i.e., to non-temporary Any objects), to
    avoid misuses like
    @code
      css::uno::Any f();

      if (auto p = o3tl::tryAccess<css::beans::NamedValue>(f())) {
        return p->Name;
      }
    @endcode

    @note Ideally this would be a public member function of css::uno::Any (at
    least conditional on LIBO_INTERNAL_ONLY, as it requires C++11).  However, as
    std::optional (which would be needed to implement the proxies) is only
    available since C++14, we need to use boost::optional for now.  But To not
    make every entity that includes <com/sun/star/uno/Any.hxx> depend on
    boost_headers, keep this here for now.

    @tparam T  the C++ representation of a UNO type that can be contained in a
    UNO ANY (i.e., any UNO type other than ANY itself).  The legacy C++
    representations sal_Bool, cppu::UnoVoidType, cppu::UnoUnsignedShortType,
    cppu::UnoCharType, and cppu::UnoSequenceType are not supported.  Must be a
    complete type or void.

    @param any  an Any value.

    @return a positive proxy for the value of the specified type obtained from
    the given Any, or a negative proxy if no such value can be obtained.
*/
template<typename T> inline
typename std::enable_if<
    !(detail::IsDerivedReference<T>::value
      || detail::IsUnoSequenceType<T>::value
      || std::is_base_of<css::uno::XInterface, T>::value),
    typename detail::Optional<T>::type>::type
tryAccess(css::uno::Any const & any) {
    // CHAR, STRING, TYPE, sequence types, enum types, struct types, exception
    // types, and com.sun.star.uno.XInterface interface type:
    return cppu::UnoType<T>::get().isAssignableFrom(any.getValueType())
        ? static_cast<T const *>(any.getValue()) : nullptr;
}

template<> inline detail::Optional<void>::type tryAccess<void>(
    css::uno::Any const & any)
{
    return any.hasValue()
        ? boost::optional<detail::Void const>()
        : boost::optional<detail::Void const>(detail::Void());
}

template<> inline detail::Optional<bool>::type tryAccess<bool>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<bool>(any);
}

template<> inline detail::Optional<sal_Int8>::type tryAccess<sal_Int8>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int8>(any);
}

template<> inline detail::Optional<sal_Int16>::type tryAccess<sal_Int16>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int16>(any);
}

template<> inline detail::Optional<sal_uInt16>::type tryAccess<sal_uInt16>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt16>(any);
}

template<> inline detail::Optional<sal_Int32>::type tryAccess<sal_Int32>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int32>(any);
}

template<> inline detail::Optional<sal_uInt32>::type tryAccess<sal_uInt32>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt32>(any);
}

template<> inline detail::Optional<sal_Int64>::type tryAccess<sal_Int64>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_Int64>(any);
}

template<> inline detail::Optional<sal_uInt64>::type tryAccess<sal_uInt64>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<sal_uInt64>(any);
}

template<> inline detail::Optional<float>::type tryAccess<float>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<float>(any);
}

template<> inline detail::Optional<double>::type tryAccess<double>(
    css::uno::Any const & any)
{
    return detail::tryGetConverted<double>(any);
}

template<> detail::Optional<css::uno::Any>::type tryAccess<css::uno::Any>(
    css::uno::Any const &) = delete;

template<> detail::Optional<sal_Bool>::type tryAccess<sal_Bool>(
    css::uno::Any const &) = delete;

/*

// Already prevented by std::is_base_of<css::uno::XInterface, T> requiring T to
// be complete:

template<> detail::Optional<cppu::UnoVoidType>::type
tryAccess<cppu::UnoVoidType>(css::uno::Any const &) = delete;

template<> detail::Optional<cppu::UnoUnsignedShortType>::type
tryAccess<cppu::UnoUnsignedShortType>(css::uno::Any const &) = delete;

template<> detail::Optional<cppu::UnoCharType>::type
tryAccess<cppu::UnoCharType>(css::uno::Any const &) = delete;

*/

template<typename T> inline
typename std::enable_if<
    detail::IsDerivedReference<T>::value,
    typename detail::Optional<T>::type>::type
tryAccess(css::uno::Any const & any) {
    return detail::tryGetConverted<T>(any);
}

template<typename T> typename detail::Optional<T>::type tryAccess(
    css::uno::Any const volatile &&) = delete;

/** Access the value of a specific type stored in an Any, throwing an exception
    on failure.

    @attention A proxy returned from this function must not outlive the
    corresponding Any passed into this function (as it may constitute a pointer
    into the Any's internals).  However, unlike with tryAccess, the benefit of
    allowing this function to operate on temporaries appears to outweigh its
    dangers.

    @note Ideally this would be a public member function of css::uno::Any.  See
    tryAccess for details.

    @tparam T  the C++ representation of a UNO type that can be contained in a
    UNO ANY.  See tryAccess for details.

    @param any  an Any value.

    @return a positive proxy for the value of the specified type obtained from
    the given Any.  See tryAccess for details.

    @throws css::uno::RuntimeException  when a value of the requested type
    cannot be obtained.
*/
template<typename T> inline typename detail::Optional<T>::type doAccess(
    css::uno::Any const & any)
{
    auto opt = tryAccess<T>(any);
    if (!opt) {
        throw css::uno::RuntimeException(
            OUString(
                cppu_Any_extraction_failure_msg(
                    &any, cppu::UnoType<T>::get().getTypeLibType()),
                SAL_NO_ACQUIRE));
    }
    return opt;
}

/** Access the value of a specific type stored in an Any, knowing the Any
    contains a value of a matching type.

    @attention A proxy returned from this function must not outlive the
    corresponding Any passed into this function (as it may constitute a pointer
    into the Any's internals).  However, unlike with tryAccess, the benefit of
    allowing this function to operate on temporaries appears to outweigh its
    dangers.

    @note Ideally this would be a public member function of css::uno::Any.  See
    tryAccess for details.

    @tparam T  the C++ representation of a UNO type that can be contained in a
    UNO ANY.  See tryAccess for details.

    @param any  an Any value.

    @return a positive proxy for the value of the specified type obtained from
    the given Any.  See tryAccess for details.
*/
template<typename T> inline typename detail::Optional<T>::type forceAccess(
    css::uno::Any const & any)
{
    auto opt = tryAccess<T>(any);
    assert(opt);
    return opt;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
