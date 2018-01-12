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
#ifndef INCLUDED_COM_SUN_STAR_UNO_ANY_HXX
#define INCLUDED_COM_SUN_STAR_UNO_ANY_HXX

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <ostream>
#include <utility>

#include "com/sun/star/uno/Any.h"
#include "uno/data.h"
#include "uno/sequence2.h"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/uno/genfunc.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppu/cppudllapi.h"
#include "cppu/unotype.hxx"

extern "C" CPPU_DLLPUBLIC rtl_uString * SAL_CALL cppu_Any_extraction_failure_msg(
    uno_Any const * pAny, typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C();

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{


inline Any::Any()
{
    ::uno_any_construct( this, NULL, NULL, cpp_acquire );
}


template <typename T>
inline Any::Any( T const & value )
{
    ::uno_type_any_construct(
        this, const_cast<T *>(&value),
        ::cppu::getTypeFavourUnsigned(&value).getTypeLibType(),
        cpp_acquire );
}

inline Any::Any( bool value )
{
    sal_Bool b = value;
    ::uno_type_any_construct(
        this, &b, cppu::UnoType<bool>::get().getTypeLibType(),
        cpp_acquire );
}

#if defined LIBO_INTERNAL_ONLY
template<typename T1, typename T2>
Any::Any(rtl::OUStringConcat<T1, T2> && value):
    Any(rtl::OUString(std::move(value)))
{}
#endif

inline Any::Any( const Any & rAny )
{
    ::uno_type_any_construct( this, rAny.pData, rAny.pType, cpp_acquire );
}

inline Any::Any( const void * pData_, const Type & rType )
{
    ::uno_type_any_construct(
        this, const_cast< void * >( pData_ ), rType.getTypeLibType(),
        cpp_acquire );
}

inline Any::Any( const void * pData_, typelib_TypeDescription * pTypeDescr )
{
    ::uno_any_construct(
        this, const_cast< void * >( pData_ ), pTypeDescr, cpp_acquire );
}

inline Any::Any( const void * pData_, typelib_TypeDescriptionReference * pType_ )
{
    ::uno_type_any_construct(
        this, const_cast< void * >( pData_ ), pType_, cpp_acquire );
}

inline Any::~Any()
{
    ::uno_any_destruct(
        this, cpp_release );
}

inline Any & Any::operator = ( const Any & rAny )
{
    if (this != &rAny)
    {
        ::uno_type_any_assign(
            this, rAny.pData, rAny.pType,
            cpp_acquire, cpp_release );
    }
    return *this;
}

#if defined LIBO_INTERNAL_ONLY

namespace detail {

inline void moveAnyInternals(Any & from, Any & to) {
    uno_any_construct(&to, nullptr, nullptr, &cpp_acquire);
    std::swap(from.pType, to.pType);
    std::swap(from.pData, to.pData);
    std::swap(from.pReserved, to.pReserved);
    if (to.pData == &from.pReserved) {
        to.pData = &to.pReserved;
    }
    // This leaves from.pData (where "from" is now VOID) dangling to somewhere (cf.
    // CONSTRUCT_EMPTY_ANY, cppu/source/uno/prim.hxx), but what's relevant is
    // only that it isn't a nullptr (as e.g. >>= -> uno_type_assignData ->
    // _assignData takes a null pSource to mean "construct a default value").
}

}

Any::Any(Any && other) {
    detail::moveAnyInternals(other, *this);
}

Any & Any::operator =(Any && other) {
    uno_any_destruct(this, &cpp_release);
    detail::moveAnyInternals(other, *this);
    return *this;
}

#endif

inline ::rtl::OUString Any::getValueTypeName() const
{
    return ::rtl::OUString( pType->pTypeName );
}

inline void Any::setValue( const void * pData_, const Type & rType )
{
    ::uno_type_any_assign(
        this, const_cast< void * >( pData_ ), rType.getTypeLibType(),
        cpp_acquire, cpp_release );
}

inline void Any::setValue( const void * pData_, typelib_TypeDescriptionReference * pType_ )
{
    ::uno_type_any_assign(
        this, const_cast< void * >( pData_ ), pType_,
        cpp_acquire, cpp_release );
}

inline void Any::setValue( const void * pData_, typelib_TypeDescription * pTypeDescr )
{
    ::uno_any_assign(
        this, const_cast< void * >( pData_ ), pTypeDescr,
        cpp_acquire, cpp_release );
}

inline void Any::clear()
{
    ::uno_any_clear(
        this, cpp_release );
}

inline bool Any::isExtractableTo( const Type & rType ) const
{
    return ::uno_type_isAssignableFromData(
        rType.getTypeLibType(), pData, pType,
        cpp_queryInterface, cpp_release );
}


template <typename T>
inline bool Any::has() const
{
    Type const & rType = ::cppu::getTypeFavourUnsigned(static_cast< T * >(0));
    return ::uno_type_isAssignableFromData(
        rType.getTypeLibType(), pData, pType,
        cpp_queryInterface,
        cpp_release );
}

#if defined LIBO_INTERNAL_ONLY
template<> bool Any::has<Any>() const = delete;
#endif

inline bool Any::operator == ( const Any & rAny ) const
{
    return ::uno_type_equalData(
        pData, pType, rAny.pData, rAny.pType,
        cpp_queryInterface, cpp_release );
}

inline bool Any::operator != ( const Any & rAny ) const
{
    return (! ::uno_type_equalData(
        pData, pType, rAny.pData, rAny.pType,
        cpp_queryInterface, cpp_release ));
}


template< class C >
inline Any SAL_CALL makeAny( const C & value )
{
    return Any(value);
}

#if !defined LIBO_INTERNAL_ONLY
template<> Any makeAny(sal_uInt16 const & value)
{ return Any(&value, cppu::UnoType<cppu::UnoUnsignedShortType>::get()); }
#endif

template<typename T> Any toAny(T const & value) { return makeAny(value); }

template<> Any toAny(Any const & value) { return value; }

#if defined LIBO_INTERNAL_ONLY

template<typename T1, typename T2>
Any makeAny(rtl::OUStringConcat<T1, T2> && value)
{ return Any(std::move(value)); }

template<typename T1, typename T2>
Any toAny(rtl::OUStringConcat<T1, T2> && value)
{ return makeAny(std::move(value)); }

template<typename T> bool fromAny(Any const & any, T * value) {
    assert(value != nullptr);
    return any >>= *value;
}

template<> bool fromAny(Any const & any, Any * value) {
    assert(value != nullptr);
    *value = any;
    return true;
}

#endif

template< class C >
inline void SAL_CALL operator <<= ( Any & rAny, const C & value )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned(&value);
    ::uno_type_any_assign(
        &rAny, const_cast< C * >( &value ), rType.getTypeLibType(),
        cpp_acquire, cpp_release );
}

// additionally for C++ bool:

template<>
inline void SAL_CALL operator <<= ( Any & rAny, bool const & value )
{
    sal_Bool b = value;
    ::uno_type_any_assign(
        &rAny, &b, cppu::UnoType<bool>::get().getTypeLibType(),
        cpp_acquire, cpp_release );
}


#ifdef LIBO_INTERNAL_ONLY // "RTL_FAST_STRING"
template< class C1, class C2 >
inline void SAL_CALL operator <<= ( Any & rAny, rtl::OUStringConcat< C1, C2 >&& value )
{
    const rtl::OUString str( std::move(value) );
    const Type & rType = ::cppu::getTypeFavourUnsigned(&str);
    ::uno_type_any_assign(
        &rAny, const_cast< rtl::OUString * >( &str ), rType.getTypeLibType(),
        cpp_acquire, cpp_release );
}
template<typename T1, typename T2>
void operator <<=(Any &, rtl::OUStringConcat<T1, T2> const &) = delete;
#endif

#if defined LIBO_INTERNAL_ONLY
template<> void SAL_CALL operator <<=(Any &, Any const &) = delete;
#endif

template< class C >
inline bool SAL_CALL operator >>= ( const Any & rAny, C & value )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned(&value);
    return ::uno_type_assignData(
        &value, rType.getTypeLibType(),
        rAny.pData, rAny.pType,
        cpp_queryInterface,
        cpp_acquire, cpp_release );
}

// bool

template<>
inline bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Bool & value )
{
    if (typelib_TypeClass_BOOLEAN == rAny.pType->eTypeClass)
    {
        value = bool(* static_cast< const sal_Bool * >( rAny.pData ));
        return true;
    }
    return false;
}

template<>
inline bool SAL_CALL operator == ( const Any & rAny, const sal_Bool & value )
{
    return (typelib_TypeClass_BOOLEAN == rAny.pType->eTypeClass &&
            bool(value) == bool(* static_cast< const sal_Bool * >( rAny.pData )));
}


template<>
inline bool SAL_CALL operator >>= ( Any const & rAny, bool & value )
{
    if (rAny.pType->eTypeClass == typelib_TypeClass_BOOLEAN)
    {
        value = *static_cast< sal_Bool const * >( rAny.pData );
        return true;
    }
    return false;
}


template<>
inline bool SAL_CALL operator == ( Any const & rAny, bool const & value )
{
    return (rAny.pType->eTypeClass == typelib_TypeClass_BOOLEAN &&
            (value ==
             bool(*static_cast< sal_Bool const * >( rAny.pData ))));
}

// byte

template<>
inline bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int8 & value )
{
    if (typelib_TypeClass_BYTE == rAny.pType->eTypeClass)
    {
        value = * static_cast< const sal_Int8 * >( rAny.pData );
        return true;
    }
    return false;
}
// short

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_Int16 & value )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * static_cast< const sal_Int8 * >( rAny.pData );
        return true;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * static_cast< const sal_Int16 * >( rAny.pData );
        return true;
    default:
        return false;
    }
}

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt16 & value )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = static_cast<sal_uInt16>( * static_cast< const sal_Int8 * >( rAny.pData ) );
        return true;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * static_cast< const sal_uInt16 * >( rAny.pData );
        return true;
    default:
        return false;
    }
}
// long

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_Int32 & value )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * static_cast< const sal_Int8 * >( rAny.pData );
        return true;
    case typelib_TypeClass_SHORT:
        value = * static_cast< const sal_Int16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * static_cast< const sal_uInt16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * static_cast< const sal_Int32 * >( rAny.pData );
        return true;
    default:
        return false;
    }
}

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt32 & value )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = static_cast<sal_uInt32>( * static_cast< const sal_Int8 * >( rAny.pData ) );
        return true;
    case typelib_TypeClass_SHORT:
        value = static_cast<sal_uInt32>( * static_cast< const sal_Int16 * >( rAny.pData ) );
        return true;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * static_cast< const sal_uInt16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * static_cast< const sal_uInt32 * >( rAny.pData );
        return true;
    default:
        return false;
    }
}
// hyper

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_Int64 & value )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * static_cast< const sal_Int8 * >( rAny.pData );
        return true;
    case typelib_TypeClass_SHORT:
        value = * static_cast< const sal_Int16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * static_cast< const sal_uInt16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_LONG:
        value = * static_cast< const sal_Int32 * >( rAny.pData );
        return true;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * static_cast< const sal_uInt32 * >( rAny.pData );
        return true;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        value = * static_cast< const sal_Int64 * >( rAny.pData );
        return true;
    default:
        return false;
    }
}

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt64 & value )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = static_cast<sal_uInt64>( * static_cast< const sal_Int8 * >( rAny.pData ) );
        return true;
    case typelib_TypeClass_SHORT:
        value = static_cast<sal_uInt64>( * static_cast< const sal_Int16 * >( rAny.pData ) );
        return true;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * static_cast< const sal_uInt16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_LONG:
        value = static_cast<sal_uInt64>( * static_cast< const sal_Int32 * >( rAny.pData ) );
        return true;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * static_cast< const sal_uInt32 * >( rAny.pData );
        return true;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        value = * static_cast< const sal_uInt64 * >( rAny.pData );
        return true;
    default:
        return false;
    }
}
// float

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, float & value )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * static_cast< const sal_Int8 * >( rAny.pData );
        return true;
    case typelib_TypeClass_SHORT:
        value = * static_cast< const sal_Int16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * static_cast< const sal_uInt16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_FLOAT:
        value = * static_cast< const float * >( rAny.pData );
        return true;
    default:
        return false;
    }
}
// double

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, double & value )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * static_cast< const sal_Int8 * >( rAny.pData );
        return true;
    case typelib_TypeClass_SHORT:
        value = * static_cast< const sal_Int16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * static_cast< const sal_uInt16 * >( rAny.pData );
        return true;
    case typelib_TypeClass_LONG:
        value = * static_cast< const sal_Int32 * >( rAny.pData );
        return true;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * static_cast< const sal_uInt32 * >( rAny.pData );
        return true;
    case typelib_TypeClass_FLOAT:
        value = * static_cast< const float * >( rAny.pData );
        return true;
    case typelib_TypeClass_DOUBLE:
        value = * static_cast< const double * >( rAny.pData );
        return true;
    default:
        return false;
    }
}
// string

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, ::rtl::OUString & value )
{
    if (typelib_TypeClass_STRING == rAny.pType->eTypeClass)
    {
        value = * static_cast< const ::rtl::OUString * >( rAny.pData );
        return true;
    }
    return false;
}

template<>
inline bool SAL_CALL operator == ( const Any & rAny, const ::rtl::OUString & value )
{
    return (typelib_TypeClass_STRING == rAny.pType->eTypeClass &&
            value == * static_cast< const ::rtl::OUString * >( rAny.pData ) );
}
// type

template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, Type & value )
{
    if (typelib_TypeClass_TYPE == rAny.pType->eTypeClass)
    {
        value = * static_cast< const Type * >( rAny.pData );
        return true;
    }
    return false;
}

template<>
inline bool SAL_CALL operator == ( const Any & rAny, const Type & value )
{
    return (typelib_TypeClass_TYPE == rAny.pType->eTypeClass &&
            value.equals( * static_cast< const Type * >( rAny.pData ) ));
}
// any

#if defined LIBO_INTERNAL_ONLY
template<> bool SAL_CALL operator >>=(Any const &, Any &) = delete;
#else
template<>
inline bool SAL_CALL operator >>= ( const Any & rAny, Any & value )
{
    if (&rAny != &value)
    {
        ::uno_type_any_assign(
            &value, rAny.pData, rAny.pType,
            cpp_acquire, cpp_release );
    }
    return true;
}
#endif
// interface

template<>
inline bool SAL_CALL operator == ( const Any & rAny, const BaseReference & value )
{
    if (typelib_TypeClass_INTERFACE == rAny.pType->eTypeClass)
    {
        return static_cast< const BaseReference * >( rAny.pData )->operator == ( value );
    }
    return false;
}

// operator to compare to an any.

template< class C >
inline bool SAL_CALL operator == ( const Any & rAny, const C & value )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned(&value);
    return ::uno_type_equalData(
        rAny.pData, rAny.pType,
        const_cast< C * >( &value ), rType.getTypeLibType(),
        cpp_queryInterface, cpp_release );
}
// operator to compare to an any.  may use specialized operators ==.

template< class C >
inline bool SAL_CALL operator != ( const Any & rAny, const C & value )
{
    return (! operator == ( rAny, value ));
}

template <typename T>
T Any::get() const
{
    T value = T();
    if (! (*this >>= value)) {
        throw RuntimeException(
            ::rtl::OUString(
                cppu_Any_extraction_failure_msg(
                    this,
                    ::cppu::getTypeFavourUnsigned(&value).getTypeLibType() ),
                SAL_NO_ACQUIRE ) );
    }
    return value;
}

#if defined LIBO_INTERNAL_ONLY
template<> Any Any::get() const = delete;
#endif

/**
   Support for Any in std::ostream (and thus in CPPUNIT_ASSERT or SAL_INFO
   macros, for example).

   @since LibreOffice 4.2
*/
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> &operator<<(std::basic_ostream<charT, traits> &o, Any const &any) {
    o << "<Any: (" << any.getValueTypeName() << ')';
    switch(any.pType->eTypeClass) {
        case typelib_TypeClass_VOID:
            break;
        case typelib_TypeClass_BOOLEAN:
            o << ' ' << any.get<bool>();
            break;
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_HYPER:
            o << ' ' << any.get<sal_Int64>();
            break;
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_UNSIGNED_HYPER:
            o << ' ' << any.get<sal_uInt64>();
            break;
        case typelib_TypeClass_FLOAT:
        case typelib_TypeClass_DOUBLE:
            o << ' ' << any.get<double>();
            break;
        case typelib_TypeClass_CHAR: {
            std::ios_base::fmtflags flgs = o.setf(
                std::ios_base::hex, std::ios_base::basefield);
            charT fill = o.fill('0');
            o << " U+" << std::setw(4)
              << *static_cast<sal_Unicode const *>(any.getValue());
            o.setf(flgs);
            o.fill(fill);
            break;
        }
        case typelib_TypeClass_STRING:
            o << ' ' << any.get<rtl::OUString>();
            break;
        case typelib_TypeClass_TYPE:
            o << ' ' << any.get<css::uno::Type>().getTypeName();
            break;
        case typelib_TypeClass_SEQUENCE:
            o << " len "
              << ((*static_cast<uno_Sequence * const *>(any.getValue()))->
                  nElements);
            break;
        case typelib_TypeClass_ENUM:
            o << ' ' << *static_cast<sal_Int32 const *>(any.getValue());
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            o << ' ' << any.getValue();
            break;
        case typelib_TypeClass_INTERFACE:
            o << ' ' << *static_cast<void * const *>(any.getValue());
            break;
        default:
            assert(false); // this cannot happen
            break;
    }
    o << '>';
    return o;
}

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
