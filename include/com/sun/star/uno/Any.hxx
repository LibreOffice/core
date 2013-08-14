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
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#define _COM_SUN_STAR_UNO_ANY_HXX_

#include "sal/config.h"

#include <cassert>
#include <iomanip>
#include <ostream>

#include <com/sun/star/uno/Any.h>
#include <uno/data.h>
#include <uno/sequence2.h>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include "cppu/unotype.hxx"

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

//__________________________________________________________________________________________________
inline Any::Any() SAL_THROW(())
{
    ::uno_any_construct( this, 0, 0, (uno_AcquireFunc)cpp_acquire );
}

//______________________________________________________________________________
template <typename T>
inline Any::Any( T const & value )
{
    ::uno_type_any_construct(
        this, const_cast<T *>(&value),
        ::cppu::getTypeFavourUnsigned(&value).getTypeLibType(),
        (uno_AcquireFunc) cpp_acquire );
}
//______________________________________________________________________________
inline Any::Any( bool value )
{
    sal_Bool b = value;
    ::uno_type_any_construct(
        this, &b, ::getCppuBooleanType().getTypeLibType(),
        (uno_AcquireFunc) cpp_acquire );
}

//__________________________________________________________________________________________________
inline Any::Any( const Any & rAny ) SAL_THROW(())
{
    ::uno_type_any_construct( this, rAny.pData, rAny.pType, (uno_AcquireFunc)cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData_, const Type & rType ) SAL_THROW(())
{
    ::uno_type_any_construct(
        this, const_cast< void * >( pData_ ), rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData_, typelib_TypeDescription * pTypeDescr ) SAL_THROW(())
{
    ::uno_any_construct(
        this, const_cast< void * >( pData_ ), pTypeDescr, (uno_AcquireFunc)cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData_, typelib_TypeDescriptionReference * pType_ ) SAL_THROW(())
{
    ::uno_type_any_construct(
        this, const_cast< void * >( pData_ ), pType_, (uno_AcquireFunc)cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::~Any() SAL_THROW(())
{
    ::uno_any_destruct(
        this, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline Any & Any::operator = ( const Any & rAny ) SAL_THROW(())
{
    if (this != &rAny)
    {
        ::uno_type_any_assign(
            this, rAny.pData, rAny.pType,
            (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
    }
    return *this;
}
//__________________________________________________________________________________________________
inline ::rtl::OUString Any::getValueTypeName() const SAL_THROW(())
{
    return ::rtl::OUString( pType->pTypeName );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData_, const Type & rType ) SAL_THROW(())
{
    ::uno_type_any_assign(
        this, const_cast< void * >( pData_ ), rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData_, typelib_TypeDescriptionReference * pType_ ) SAL_THROW(())
{
    ::uno_type_any_assign(
        this, const_cast< void * >( pData_ ), pType_,
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData_, typelib_TypeDescription * pTypeDescr ) SAL_THROW(())
{
    ::uno_any_assign(
        this, const_cast< void * >( pData_ ), pTypeDescr,
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::clear() SAL_THROW(())
{
    ::uno_any_clear(
        this, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline sal_Bool Any::isExtractableTo( const Type & rType ) const SAL_THROW(())
{
    return ::uno_type_isAssignableFromData(
        rType.getTypeLibType(), pData, pType,
        (uno_QueryInterfaceFunc)cpp_queryInterface, (uno_ReleaseFunc)cpp_release );
}

//______________________________________________________________________________
template <typename T>
inline bool Any::has() const
{
    Type const & rType = ::cppu::getTypeFavourUnsigned(static_cast< T * >(0));
    return ::uno_type_isAssignableFromData(
        rType.getTypeLibType(), pData, pType,
        (uno_QueryInterfaceFunc) cpp_queryInterface,
        (uno_ReleaseFunc) cpp_release );
}
#if ! defined(__SUNPRO_CC)
// not impl: forbid use with ambiguous type (sal_Unicode, sal_uInt16)
template <>
bool Any::has<sal_uInt16>() const;
#endif // ! defined(__SUNPRO_CC)

//__________________________________________________________________________________________________
inline sal_Bool Any::operator == ( const Any & rAny ) const SAL_THROW(())
{
    return ::uno_type_equalData(
        pData, pType, rAny.pData, rAny.pType,
        (uno_QueryInterfaceFunc)cpp_queryInterface, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline sal_Bool Any::operator != ( const Any & rAny ) const SAL_THROW(())
{
    return (! ::uno_type_equalData(
        pData, pType, rAny.pData, rAny.pType,
        (uno_QueryInterfaceFunc)cpp_queryInterface, (uno_ReleaseFunc)cpp_release ));
}

//__________________________________________________________________________________________________
template< class C >
inline Any SAL_CALL makeAny( const C & value ) SAL_THROW(())
{
    return Any( &value, ::cppu::getTypeFavourUnsigned(&value) );
}

// additionally specialized for C++ bool
//______________________________________________________________________________
template<>
inline Any SAL_CALL makeAny( bool const & value ) SAL_THROW(())
{
    const sal_Bool b = value;
    return Any( &b, ::getCppuBooleanType() );
}

//__________________________________________________________________________________________________
#ifdef RTL_FAST_STRING
template< class C1, class C2 >
inline Any SAL_CALL makeAny( const rtl::OUStringConcat< C1, C2 >& value ) SAL_THROW(())
{
    const rtl::OUString str( value );
    return Any( &str, ::cppu::getTypeFavourUnsigned(&str) );
}
#endif
//__________________________________________________________________________________________________
template< class C >
inline void SAL_CALL operator <<= ( Any & rAny, const C & value ) SAL_THROW(())
{
    const Type & rType = ::cppu::getTypeFavourUnsigned(&value);
    ::uno_type_any_assign(
        &rAny, const_cast< C * >( &value ), rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}

// additionally for C++ bool:
//______________________________________________________________________________
inline void SAL_CALL operator <<= ( Any & rAny, bool const & value )
    SAL_THROW(())
{
    sal_Bool b = value;
    ::uno_type_any_assign(
        &rAny, &b, ::getCppuBooleanType().getTypeLibType(),
        (uno_AcquireFunc) cpp_acquire, (uno_ReleaseFunc) cpp_release );
}

//______________________________________________________________________________
#ifdef RTL_FAST_STRING
template< class C1, class C2 >
inline void SAL_CALL operator <<= ( Any & rAny, const rtl::OUStringConcat< C1, C2 >& value )
    SAL_THROW(())
{
    const rtl::OUString str( value );
    const Type & rType = ::cppu::getTypeFavourUnsigned(&str);
    ::uno_type_any_assign(
        &rAny, const_cast< rtl::OUString * >( &str ), rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}
#endif
//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, C & value ) SAL_THROW(())
{
    const Type & rType = ::cppu::getTypeFavourUnsigned(&value);
    return ::uno_type_assignData(
        &value, rType.getTypeLibType(),
        rAny.pData, rAny.pType,
        (uno_QueryInterfaceFunc)cpp_queryInterface,
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}

// bool
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Bool & value ) SAL_THROW(())
{
    if (typelib_TypeClass_BOOLEAN == rAny.pType->eTypeClass)
    {
        value = (* reinterpret_cast< const sal_Bool * >( rAny.pData ) != sal_False);
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const sal_Bool & value ) SAL_THROW(())
{
    return (typelib_TypeClass_BOOLEAN == rAny.pType->eTypeClass &&
            (value != sal_False) == (* reinterpret_cast< const sal_Bool * >( rAny.pData ) != sal_False));
}

//______________________________________________________________________________
template<>
inline sal_Bool SAL_CALL operator >>= ( Any const & rAny, bool & value )
    SAL_THROW(())
{
    if (rAny.pType->eTypeClass == typelib_TypeClass_BOOLEAN)
    {
        value = *reinterpret_cast< sal_Bool const * >(
            rAny.pData ) != sal_False;
        return true;
    }
    return false;
}

//______________________________________________________________________________
template<>
inline sal_Bool SAL_CALL operator == ( Any const & rAny, bool const & value )
    SAL_THROW(())
{
    return (rAny.pType->eTypeClass == typelib_TypeClass_BOOLEAN &&
            (value ==
             (*reinterpret_cast< sal_Bool const * >( rAny.pData )
              != sal_False)));
}

// byte
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int8 & value ) SAL_THROW(())
{
    if (typelib_TypeClass_BYTE == rAny.pType->eTypeClass)
    {
        value = * reinterpret_cast< const sal_Int8 * >( rAny.pData );
        return sal_True;
    }
    return sal_False;
}
// short
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int16 & value ) SAL_THROW(())
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt16 & value ) SAL_THROW(())
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = (sal_uInt16)( * reinterpret_cast< const sal_Int8 * >( rAny.pData ) );
        return sal_True;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
// long
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int32 & value ) SAL_THROW(())
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt32 & value ) SAL_THROW(())
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = (sal_uInt32)( * reinterpret_cast< const sal_Int8 * >( rAny.pData ) );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = (sal_uInt32)( * reinterpret_cast< const sal_Int16 * >( rAny.pData ) );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
// hyper
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int64 & value ) SAL_THROW(())
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        value = * reinterpret_cast< const sal_Int64 * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt64 & value ) SAL_THROW(())
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = (sal_uInt64)( * reinterpret_cast< const sal_Int8 * >( rAny.pData ) );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = (sal_uInt64)( * reinterpret_cast< const sal_Int16 * >( rAny.pData ) );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_LONG:
        value = (sal_uInt64)( * reinterpret_cast< const sal_Int32 * >( rAny.pData ) );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        value = * reinterpret_cast< const sal_uInt64 * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
// float
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, float & value ) SAL_THROW(())
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_FLOAT:
        value = * reinterpret_cast< const float * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
// double
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, double & value ) SAL_THROW(())
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_FLOAT:
        value = * reinterpret_cast< const float * >( rAny.pData );
        return sal_True;
    case typelib_TypeClass_DOUBLE:
        value = * reinterpret_cast< const double * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
// string
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, ::rtl::OUString & value ) SAL_THROW(())
{
    if (typelib_TypeClass_STRING == rAny.pType->eTypeClass)
    {
        value = * reinterpret_cast< const ::rtl::OUString * >( rAny.pData );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const ::rtl::OUString & value ) SAL_THROW(())
{
    return (typelib_TypeClass_STRING == rAny.pType->eTypeClass &&
            value.equals( * reinterpret_cast< const ::rtl::OUString * >( rAny.pData ) ));
}
// type
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Type & value ) SAL_THROW(())
{
    if (typelib_TypeClass_TYPE == rAny.pType->eTypeClass)
    {
        value = * reinterpret_cast< const Type * >( rAny.pData );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const Type & value ) SAL_THROW(())
{
    return (typelib_TypeClass_TYPE == rAny.pType->eTypeClass &&
            value.equals( * reinterpret_cast< const Type * >( rAny.pData ) ));
}
// any
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Any & value ) SAL_THROW(())
{
    if (&rAny != &value)
    {
        ::uno_type_any_assign(
            &value, rAny.pData, rAny.pType,
            (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
    }
    return sal_True;
}
// interface
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const BaseReference & value ) SAL_THROW(())
{
    if (typelib_TypeClass_INTERFACE == rAny.pType->eTypeClass)
    {
        return reinterpret_cast< const BaseReference * >( rAny.pData )->operator == ( value );
    }
    return sal_False;
}

// operator to compare to an any.
//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const C & value ) SAL_THROW(())
{
    const Type & rType = ::cppu::getTypeFavourUnsigned(&value);
    return ::uno_type_equalData(
        rAny.pData, rAny.pType,
        const_cast< C * >( &value ), rType.getTypeLibType(),
        (uno_QueryInterfaceFunc)cpp_queryInterface, (uno_ReleaseFunc)cpp_release );
}
// operator to compare to an any.  may use specialized operators ==.
//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator != ( const Any & rAny, const C & value ) SAL_THROW(())
{
    return (! operator == ( rAny, value ));
}

#if ! defined(EXCEPTIONS_OFF)
extern "C" rtl_uString * SAL_CALL cppu_Any_extraction_failure_msg(
    uno_Any const * pAny, typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C();

//______________________________________________________________________________
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
                SAL_NO_ACQUIRE ),
            Reference<XInterface>() );
    }
    return value;
}
// not impl: forbid use with ambiguous type (sal_Unicode, sal_uInt16)
template <>
sal_uInt16 Any::get<sal_uInt16>() const;
#endif // ! defined(EXCEPTIONS_OFF)

/**
   Support for Any in std::ostream (and thus in CPPUNIT_ASSERT or SAL_INFO
   macros, for example).

   @since LibreOffice 4.2
*/
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> &operator<<(std::basic_ostream<charT, traits> &o, Any &any) {
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
