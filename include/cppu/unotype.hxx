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

#ifndef INCLUDED_CPPU_UNOTYPE_HXX
#define INCLUDED_CPPU_UNOTYPE_HXX

#include "sal/config.h"

#include <cstddef>

#if defined LIBO_INTERNAL_ONLY
#include <type_traits>
#endif

#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace com { namespace sun { namespace star { namespace uno {
    class Type;
    class Any;
    class Exception;
    template< typename > class Reference;
    template< typename > class Sequence;
    class XInterface;
} } } }
namespace rtl { class OUString; }

namespace cppu {

template< typename > class UnoType;

/**
   A unique C++ type representing the UNO type VOID in cppu::UnoType.

   This type is declared but not defined.  Its only use is as a template
   argument to cppu::UnoType.

   @since UDK 3.2.2
*/
struct UnoVoidType;

/**
   A unique C++ type representing the UNO type UNSIGNED SHORT in cppu::UnoType.

   The UNO types UNSIGNED SHORT and CHAR map to the same C++ type, so this C++
   type is needed to unambiguously specify UNO types in cppu::UnoType.

   This type is declared but not defined.  Its only use is as a template
   argument to cppu::UnoType.

   @since UDK 3.2.2
*/
struct UnoUnsignedShortType;

/**
   A unique C++ type representing the UNO type CHAR in cppu::UnoType.

   The UNO types UNSIGNED SHORT and CHAR map to the same C++ type, so this C++
   type is needed to unambiguously specify UNO types in cppu::UnoType.

   This type is declared but not defined.  Its only use is as a template
   argument to cppu::UnoType.

   @since UDK 3.2.2
*/
struct UnoCharType;

/**
   A unique C++ type template representing the UNO sequence types in
   cppu::UnoType.

   The UNO types UNSIGNED SHORT and CHAR map to the same C++ type, so this C++
   type is needed to unambiguously specify UNO types in cppu::UnoType.

   This type is declared but not defined.  Its only use is as a template
   argument to cppu::UnoType.

   @since UDK 3.2.2
*/
template< typename > struct UnoSequenceType;

namespace detail {

inline css::uno::Type const & getTypeFromTypeDescriptionReference(
    ::typelib_TypeDescriptionReference * const * tdr)
{
    return *reinterpret_cast< css::uno::Type const * >(tdr);
}

inline css::uno::Type const &
getTypeFromTypeClass(::typelib_TypeClass tc) {
    return getTypeFromTypeDescriptionReference(
        ::typelib_static_type_getByTypeClass(tc));
}

}

}

namespace cppu { namespace detail {

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::cppu::UnoVoidType const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_VOID);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER bool const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_BOOLEAN);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER sal_Bool const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_BOOLEAN);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::sal_Int8 const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_BYTE);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::sal_Int16 const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_SHORT);
}

inline css::uno::Type const &
cppu_detail_getUnoType(
    SAL_UNUSED_PARAMETER ::cppu::UnoUnsignedShortType const *)
{
    return ::cppu::detail::getTypeFromTypeClass(
        ::typelib_TypeClass_UNSIGNED_SHORT);
}

#if defined LIBO_INTERNAL_ONLY
    // cf. sal/types.h sal_Unicode
inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER sal_uInt16 const *) {
    return cppu::detail::getTypeFromTypeClass(typelib_TypeClass_UNSIGNED_SHORT);
}
#endif

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::sal_Int32 const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_LONG);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::sal_uInt32 const *) {
    return ::cppu::detail::getTypeFromTypeClass(
        ::typelib_TypeClass_UNSIGNED_LONG);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::sal_Int64 const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_HYPER);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::sal_uInt64 const *) {
    return ::cppu::detail::getTypeFromTypeClass(
        ::typelib_TypeClass_UNSIGNED_HYPER);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER float const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_FLOAT);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER double const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_DOUBLE);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::cppu::UnoCharType const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_CHAR);
}

#if defined LIBO_INTERNAL_ONLY
    // cf. sal/types.h sal_Unicode
inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER sal_Unicode const *) {
    return cppu::detail::getTypeFromTypeClass(typelib_TypeClass_CHAR);
}
#endif

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ::rtl::OUString const *) {
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_STRING);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER css::uno::Type const *)
{
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_TYPE);
}

inline css::uno::Type const &
cppu_detail_getUnoType(SAL_UNUSED_PARAMETER css::uno::Any const *)
{
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_ANY);
}

template< typename T > inline css::uno::Type const &
cppu_detail_getUnoType(
    SAL_UNUSED_PARAMETER ::cppu::UnoSequenceType< T > const *)
{
    //TODO: depending on memory model, the following might not work reliably
    static typelib_TypeDescriptionReference * p = NULL;
    if (p == NULL) {
        ::typelib_static_sequence_type_init(
            &p, ::cppu::UnoType< T >::get().getTypeLibType());
    }
    return ::cppu::detail::getTypeFromTypeDescriptionReference(&p);
}

template< typename T > inline css::uno::Type const &
cppu_detail_getUnoType(
    SAL_UNUSED_PARAMETER css::uno::Sequence< T > const *)
{
    return cppu_detail_getUnoType(
        static_cast< ::cppu::UnoSequenceType< T > * >(0));
}

inline css::uno::Type const & cppu_detail_getUnoType(
    SAL_UNUSED_PARAMETER css::uno::Exception const *)
{
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_EXCEPTION);
}

inline css::uno::Type const & cppu_detail_getUnoType(
    SAL_UNUSED_PARAMETER css::uno::XInterface const *)
{
    return ::cppu::detail::getTypeFromTypeClass(::typelib_TypeClass_INTERFACE);
}

template< typename T > inline css::uno::Type const &
cppu_detail_getUnoType(
    SAL_UNUSED_PARAMETER css::uno::Reference< T > const *)
{
    return ::cppu::UnoType< T >::get();
}

} }

namespace cppu {

/**
   Get the css::uno::Type instance representing a certain UNO type.

   For each C++ type representing a UNO type, the corresponding instantiation of
   this template has a public static member function get().  (The template is
   specialized for C++ templates representing polymorphic struct type templates
   of UNO.  In those cases, it does not work to instantiate UnoType with a C++
   type that is derived from a C++ type that represents a UNO type, but does not
   itself represent a UNO type.  In all other cases, UnoType even works for such
   C++ types that are unambiguously derived from one C++ type that represents a
   UNO type.)  In addition to those C++ types that are mappings of UNO types
   (except for sal_uInt16 and sal_Unicode, see below), the following C++ types
   are appropriate as template arguments: void, cppu::UnoVoidType, bool,
   cppu::UnoUnsignedShortType, cppu::UnoCharType, cppu::UnoSequenceType with any
   appropriate template argument (the latter three to unambiguously specify UNO
   types, as the UNO types UNSIGNED SHORT and CHAR map to the same C++ type),
   and css::uno::Reference with any appropriate template argument.

   @since UDK 3.2.2
*/
template< typename T > class UnoType {
public:
    static css::uno::Type const & get() {
        using namespace ::cppu::detail;
#if defined LIBO_INTERNAL_ONLY
        typedef typename std::remove_reference<T>::type T1;
            // for certain uses of UnoType<decltype(x)>
#else
        typedef T T1;
#endif
        return cppu_detail_getUnoType(static_cast< T1 * >(0));
    }

private:
    UnoType(UnoType &) SAL_DELETED_FUNCTION;
    ~UnoType() SAL_DELETED_FUNCTION;
    void operator =(UnoType &) SAL_DELETED_FUNCTION;
};

template<> css::uno::Type inline const & UnoType<void>::get() {
    return cppu::UnoType<cppu::UnoVoidType>::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of getCppuType.  The replacement has exactly the same semantics as
   getCppuType, in that it returns correct results for the UNO type UNSIGNED
   SHORT but not for the UNO type CHAR.

   @since UDK 3.2.2
*/
template< typename T > inline css::uno::Type const &
getTypeFavourUnsigned(SAL_UNUSED_PARAMETER T const *) {
    return ::cppu::UnoType< T >::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of getCppuType.  The replacement has exactly the same semantics as
   getCppuType, in that it returns correct results for the UNO type UNSIGNED
   SHORT but not for the UNO type CHAR.

   @since UDK 3.2.2
*/
inline css::uno::Type const &
getTypeFavourUnsigned(SAL_UNUSED_PARAMETER ::sal_uInt16 const *) {
    return ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of getCppuType.  The replacement has exactly the same semantics as
   getCppuType, in that it returns correct results for the UNO type UNSIGNED
   SHORT but not for the UNO type CHAR.

   @since UDK 3.2.2
*/
template< typename T > inline css::uno::Type const &
getTypeFavourUnsigned(css::uno::Sequence< T > const *);
    // defined in com/sun/star/uno/Sequence.hxx

/// @cond INTERNAL

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of the getCppuType template.  The replacement has exactly the same semantics
   as the getCppuType template, in that it returns correct results for the UNO
   type CHAR but not for the UNO type UNSIGNED SHORT.  Additionally, it also
   returns the intended results for sequence types.

   @since UDK 3.2.3
*/
template< typename T > inline css::uno::Type const &
getTypeFavourChar(SAL_UNUSED_PARAMETER T const *) {
    return ::cppu::UnoType< T >::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of the getCppuType template.  The replacement has exactly the same semantics
   as the getCppuType template, in that it returns correct results for the UNO
   type CHAR but not for the UNO type UNSIGNED SHORT.  Additionally, it also
   returns the intended results for sequence types.

   @since UDK 3.2.3
*/
inline css::uno::Type const &
getTypeFavourChar(SAL_UNUSED_PARAMETER ::sal_Unicode const *) {
    return ::cppu::UnoType< ::cppu::UnoCharType >::get();
}

/**
   A working replacement for getCppuType (see there).

   There are three overloads of this function that together form the replacement
   of the getCppuType template.  The replacement has exactly the same semantics
   as the getCppuType template, in that it returns correct results for the UNO
   type CHAR but not for the UNO type UNSIGNED SHORT.  Additionally, it also
   returns the intended results for sequence types.

   @since UDK 3.2.3
*/
template< typename T > inline css::uno::Type const &
getTypeFavourChar(css::uno::Sequence< T > const *);
    // defined in com/sun/star/uno/Sequence.hxx

/// @endcond

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
