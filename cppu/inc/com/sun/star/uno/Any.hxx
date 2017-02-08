/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#define _COM_SUN_STAR_UNO_ANY_HXX_

#include <com/sun/star/uno/Any.h>
#include <uno/data.h>
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
inline Any::Any() SAL_THROW( () )
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
inline Any::Any( const Any & rAny ) SAL_THROW( () )
{
    ::uno_type_any_construct( this, rAny.pData, rAny.pType, (uno_AcquireFunc)cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData_, const Type & rType ) SAL_THROW( () )
{
    ::uno_type_any_construct(
        this, const_cast< void * >( pData_ ), rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData_, typelib_TypeDescription * pTypeDescr ) SAL_THROW( () )
{
    ::uno_any_construct(
        this, const_cast< void * >( pData_ ), pTypeDescr, (uno_AcquireFunc)cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData_, typelib_TypeDescriptionReference * pType_ ) SAL_THROW( () )
{
    ::uno_type_any_construct(
        this, const_cast< void * >( pData_ ), pType_, (uno_AcquireFunc)cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::~Any() SAL_THROW( () )
{
    ::uno_any_destruct(
        this, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline Any & Any::operator = ( const Any & rAny ) SAL_THROW( () )
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
inline ::rtl::OUString Any::getValueTypeName() const SAL_THROW( () )
{
    return ::rtl::OUString( pType->pTypeName );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData_, const Type & rType ) SAL_THROW( () )
{
    ::uno_type_any_assign(
        this, const_cast< void * >( pData_ ), rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData_, typelib_TypeDescriptionReference * pType_ ) SAL_THROW( () )
{
    ::uno_type_any_assign(
        this, const_cast< void * >( pData_ ), pType_,
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData_, typelib_TypeDescription * pTypeDescr ) SAL_THROW( () )
{
    ::uno_any_assign(
        this, const_cast< void * >( pData_ ), pTypeDescr,
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::clear() SAL_THROW( () )
{
    ::uno_any_clear(
        this, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline sal_Bool Any::isExtractableTo( const Type & rType ) const SAL_THROW( () )
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
inline sal_Bool Any::operator == ( const Any & rAny ) const SAL_THROW( () )
{
    return ::uno_type_equalData(
        pData, pType, rAny.pData, rAny.pType,
        (uno_QueryInterfaceFunc)cpp_queryInterface, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
inline sal_Bool Any::operator != ( const Any & rAny ) const SAL_THROW( () )
{
    return (! ::uno_type_equalData(
        pData, pType, rAny.pData, rAny.pType,
        (uno_QueryInterfaceFunc)cpp_queryInterface, (uno_ReleaseFunc)cpp_release ));
}

//__________________________________________________________________________________________________
template< class C >
inline Any SAL_CALL makeAny( const C & value ) SAL_THROW( () )
{
    return Any( &value, ::cppu::getTypeFavourUnsigned(&value) );
}

// additionally specialized for C++ bool
//______________________________________________________________________________
template<>
inline Any SAL_CALL makeAny( bool const & value ) SAL_THROW( () )
{
    const sal_Bool b = value;
    return Any( &b, ::getCppuBooleanType() );
}

//__________________________________________________________________________________________________
template< class C >
inline void SAL_CALL operator <<= ( Any & rAny, const C & value ) SAL_THROW( () )
{
    const Type & rType = ::cppu::getTypeFavourUnsigned(&value);
    ::uno_type_any_assign(
        &rAny, const_cast< C * >( &value ), rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}

// additionally for C++ bool:
//______________________________________________________________________________
inline void SAL_CALL operator <<= ( Any & rAny, bool const & value )
    SAL_THROW( () )
{
    sal_Bool b = value;
    ::uno_type_any_assign(
        &rAny, &b, ::getCppuBooleanType().getTypeLibType(),
        (uno_AcquireFunc) cpp_acquire, (uno_ReleaseFunc) cpp_release );
}

//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, C & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Bool & value ) SAL_THROW( () )
{
    if (typelib_TypeClass_BOOLEAN == rAny.pType->eTypeClass)
    {
        value = (* reinterpret_cast< const sal_Bool * >( rAny.pData ) != sal_False);
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const sal_Bool & value ) SAL_THROW( () )
{
    return (typelib_TypeClass_BOOLEAN == rAny.pType->eTypeClass &&
            (value != sal_False) == (* reinterpret_cast< const sal_Bool * >( rAny.pData ) != sal_False));
}

//______________________________________________________________________________
template<>
inline sal_Bool SAL_CALL operator >>= ( Any const & rAny, bool & value )
    SAL_THROW( () )
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
    SAL_THROW( () )
{
    return (rAny.pType->eTypeClass == typelib_TypeClass_BOOLEAN &&
            (value ==
             (*reinterpret_cast< sal_Bool const * >( rAny.pData )
              != sal_False)));
}

// byte
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int8 & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int16 & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt16 & value ) SAL_THROW( () )
{
    switch (rAny.pType->eTypeClass)
    {
    case typelib_TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.pData );
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
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int32 & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt32 & value ) SAL_THROW( () )
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
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.pData );
        return sal_True;
    default:
        return sal_False;
    }
}
// hyper
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int64 & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt64 & value ) SAL_THROW( () )
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
        value = * reinterpret_cast< const sal_uInt64 * >( rAny.pData );
        return sal_True;

    default:
        return sal_False;
    }
}
// float
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, float & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, double & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, ::rtl::OUString & value ) SAL_THROW( () )
{
    if (typelib_TypeClass_STRING == rAny.pType->eTypeClass)
    {
        value = * reinterpret_cast< const ::rtl::OUString * >( rAny.pData );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const ::rtl::OUString & value ) SAL_THROW( () )
{
    return (typelib_TypeClass_STRING == rAny.pType->eTypeClass &&
            value.equals( * reinterpret_cast< const ::rtl::OUString * >( rAny.pData ) ));
}
// type
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Type & value ) SAL_THROW( () )
{
    if (typelib_TypeClass_TYPE == rAny.pType->eTypeClass)
    {
        value = * reinterpret_cast< const Type * >( rAny.pData );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const Type & value ) SAL_THROW( () )
{
    return (typelib_TypeClass_TYPE == rAny.pType->eTypeClass &&
            value.equals( * reinterpret_cast< const Type * >( rAny.pData ) ));
}
// any
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Any & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const BaseReference & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const C & value ) SAL_THROW( () )
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
inline sal_Bool SAL_CALL operator != ( const Any & rAny, const C & value ) SAL_THROW( () )
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

}
}
}
}

#endif
