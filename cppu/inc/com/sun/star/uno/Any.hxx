/*************************************************************************
 *
 *  $RCSfile: Any.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:02:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#define _COM_SUN_STAR_UNO_ANY_HXX_

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_GENFUNC_HXX_
#include <com/sun/star/uno/genfunc.hxx>
#endif


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
    return Any( &value, getCppuType( &value ) );
}

// additionally specialized for C++ bool
//______________________________________________________________________________
template<>
inline Any SAL_CALL makeAny( bool const & value ) SAL_THROW( () )
{
    sal_Bool b = value;
    return Any( &b, ::getCppuBooleanType() );
}

//__________________________________________________________________________________________________
template< class C >
inline void SAL_CALL operator <<= ( Any & rAny, const C & value ) SAL_THROW( () )
{
    const Type & rType = getCppuType( &value );
    ::uno_type_any_assign(
        &rAny, const_cast< C * >( &value ), rType.getTypeLibType(),
        (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release );
}
//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, C & value ) SAL_THROW( () )
{
    const Type & rType = getCppuType( &value );
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
        value = (* reinterpret_cast< const sal_Bool * >( &rAny.pReserved ) != sal_False);
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const sal_Bool & value ) SAL_THROW( () )
{
    return (typelib_TypeClass_BOOLEAN == rAny.pType->eTypeClass &&
            (value != sal_False) == (* reinterpret_cast< const sal_Bool * >( &rAny.pReserved ) != sal_False));
}

//______________________________________________________________________________
template<>
inline sal_Bool SAL_CALL operator >>= ( Any const & rAny, bool & value )
    SAL_THROW( () )
{
    if (rAny.pType->eTypeClass == typelib_TypeClass_BOOLEAN)
    {
        value = *reinterpret_cast< sal_Bool const * >(
            &rAny.pReserved ) != sal_False;
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
             (*reinterpret_cast< sal_Bool const * >( &rAny.pReserved )
              != sal_False)));
}

// byte
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int8 & value ) SAL_THROW( () )
{
    if (typelib_TypeClass_BYTE == rAny.pType->eTypeClass)
    {
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
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
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( &rAny.pReserved );
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
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( &rAny.pReserved );
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
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( &rAny.pReserved );
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
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( &rAny.pReserved );
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
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        value = * reinterpret_cast< const sal_Int64 * >(
            (sizeof(void *) >= sizeof(sal_Int64)) ? (void *)&rAny.pReserved : rAny.pData );
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
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        value = * reinterpret_cast< const sal_uInt64 * >(
            (sizeof(void *) >= sizeof(sal_uInt64)) ? (void *)&rAny.pReserved : rAny.pData );
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
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_FLOAT:
        value = * reinterpret_cast< const float * >(
            (sizeof(void *) >= sizeof(float)) ? (void *)&rAny.pReserved : rAny.pData );
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
        value = * reinterpret_cast< const sal_Int8 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( &rAny.pReserved );
        return sal_True;
    case typelib_TypeClass_FLOAT:
        value = * reinterpret_cast< const float * >(
            (sizeof(void *) >= sizeof(float)) ? (void *)&rAny.pReserved : rAny.pData );
        return sal_True;
    case typelib_TypeClass_DOUBLE:
        value = * reinterpret_cast< const double * >(
            (sizeof(void *) >= sizeof(double)) ? (void *)&rAny.pReserved : rAny.pData );
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
        value = * reinterpret_cast< const ::rtl::OUString * >( &rAny.pReserved );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const ::rtl::OUString & value ) SAL_THROW( () )
{
    return (typelib_TypeClass_STRING == rAny.pType->eTypeClass &&
            value.equals( * reinterpret_cast< const ::rtl::OUString * >( &rAny.pReserved ) ));
}
// type
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Type & value ) SAL_THROW( () )
{
    if (typelib_TypeClass_TYPE == rAny.pType->eTypeClass)
    {
        value = * reinterpret_cast< const Type * >( &rAny.pReserved );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const Type & value ) SAL_THROW( () )
{
    return (typelib_TypeClass_TYPE == rAny.pType->eTypeClass &&
            value.equals( * reinterpret_cast< const Type * >( &rAny.pReserved ) ));
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
        return reinterpret_cast< const BaseReference * >( &rAny.pReserved )->operator == ( value );
    }
    return sal_False;
}

// operator to compare to an any.
//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const C & value ) SAL_THROW( () )
{
    const Type & rType = getCppuType( &value );
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

}
}
}
}

#endif
