/*************************************************************************
 *
 *  $RCSfile: Any.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:55 $
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
#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
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


/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{

//__________________________________________________________________________________________________
inline Any::Any() SAL_THROW( () )
{
    ::uno_any_construct( this, 0, 0, cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const Any & rAny ) SAL_THROW( () )
{
    ::uno_type_any_construct( this, rAny.pData, rAny.getValueTypeRef(), cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData, const Type & rType ) SAL_THROW( () )
{
    ::uno_type_any_construct(
        this, const_cast< void * >( pData ), rType.getTypeLibType(), cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData, typelib_TypeDescription * pTypeDescr ) SAL_THROW( () )
{
    ::uno_any_construct(
        this, const_cast< void * >( pData ), pTypeDescr, cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData, typelib_TypeDescriptionReference * pType ) SAL_THROW( () )
{
    ::uno_type_any_construct(
        this, const_cast< void * >( pData ), pType, cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::~Any() SAL_THROW( () )
{
    ::uno_any_destruct(
        this, cpp_release );
}
//__________________________________________________________________________________________________
inline Any & Any::operator = ( const Any & rAny ) SAL_THROW( () )
{
    if (this != &rAny)
        setValue( rAny.getValue(), rAny.getValueTypeRef() );
    return *this;
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData, const Type & rType ) SAL_THROW( () )
{
    ::uno_type_any_assign(
        this, const_cast< void * >( pData ), rType.getTypeLibType(), cpp_acquire, cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData, typelib_TypeDescriptionReference * pType ) SAL_THROW( () )
{
    ::uno_type_any_assign(
        this, const_cast< void * >( pData ), pType, cpp_acquire, cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData, typelib_TypeDescription * pTypeDescr ) SAL_THROW( () )
{
    ::uno_any_assign(
        this, const_cast< void * >( pData ), pTypeDescr, cpp_acquire, cpp_release );
}
//__________________________________________________________________________________________________
inline void Any::clear() SAL_THROW( () )
{
    ::uno_any_assign(
        this, 0, 0, cpp_acquire, cpp_release );
}
//__________________________________________________________________________________________________
inline sal_Bool Any::operator == ( const Any & rAny ) const SAL_THROW( () )
{
    const Type & rType = ::getCppuType( this );
    return ::uno_type_equalData(
        const_cast< void * >( getValue() ), getValueTypeRef(),
        const_cast< void * >( rAny.getValue() ), rAny.getValueTypeRef(),
        cpp_queryInterface, cpp_release );
}

//__________________________________________________________________________________________________
template< class C >
inline Any SAL_CALL makeAny( const C & value ) SAL_THROW( () )
{
    return Any( &value, ::getCppuType( &value ) );
}

//__________________________________________________________________________________________________
template< class C >
inline void SAL_CALL operator <<= ( Any & rAny, const C & value ) SAL_THROW( () )
{
    rAny.setValue( &value, ::getCppuType( &value ) );
}

//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, C & value ) SAL_THROW( () )
{
    const Type & rType = ::getCppuType( &value );
    return ::uno_type_assignData(
        &value, rType.getTypeLibType(),
        const_cast< void * >( rAny.getValue() ), rAny.getValueTypeRef(),
        cpp_queryInterface, cpp_acquire, cpp_release );
}

// bool
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Bool & value ) SAL_THROW( () )
{
    if (TypeClass_BOOLEAN == rAny.getValueTypeClass())
    {
        value = (* reinterpret_cast< const sal_Bool * >( rAny.getValue() ) != sal_False);
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const sal_Bool & value ) SAL_THROW( () )
{
    return (TypeClass_BOOLEAN == rAny.getValueTypeClass() &&
            (value != sal_False) == (* reinterpret_cast< const sal_Bool * >( rAny.getValue() ) != sal_False));
}
// byte
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int8 & value ) SAL_THROW( () )
{
    if (TypeClass_BYTE == rAny.getValueTypeClass())
    {
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// short
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int16 & value ) SAL_THROW( () )
{
    switch (rAny.getValueTypeClass())
    {
    case TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_SHORT:
    case TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt16 & value ) SAL_THROW( () )
{
    switch (rAny.getValueTypeClass())
    {
    case TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_SHORT:
    case TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// long
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int32 & value ) SAL_THROW( () )
{
    switch (rAny.getValueTypeClass())
    {
    case TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_LONG:
    case TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt32 & value ) SAL_THROW( () )
{
    switch (rAny.getValueTypeClass())
    {
    case TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_LONG:
    case TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// hyper
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_Int64 & value ) SAL_THROW( () )
{
    switch (rAny.getValueTypeClass())
    {
    case TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_HYPER:
    case TypeClass_UNSIGNED_HYPER:
        value = * reinterpret_cast< const sal_Int64 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, sal_uInt64 & value ) SAL_THROW( () )
{
    switch (rAny.getValueTypeClass())
    {
    case TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_HYPER:
    case TypeClass_UNSIGNED_HYPER:
        value = * reinterpret_cast< const sal_uInt64 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// float
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, float & value ) SAL_THROW( () )
{
    switch (rAny.getValueTypeClass())
    {
    case TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_FLOAT:
        value = * reinterpret_cast< const float * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// double
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, double & value ) SAL_THROW( () )
{
    switch (rAny.getValueTypeClass())
    {
    case TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.getValue() );
        return sal_True;
    case TypeClass_FLOAT:
        value = * reinterpret_cast< const float * >( rAny.getValue() );
        return sal_True;
    case TypeClass_DOUBLE:
        value = * reinterpret_cast< const double * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// string
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, ::rtl::OUString & value ) SAL_THROW( () )
{
    if (TypeClass_STRING == rAny.getValueTypeClass())
    {
        value = * reinterpret_cast< const ::rtl::OUString * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const ::rtl::OUString & value ) SAL_THROW( () )
{
    return (TypeClass_STRING == rAny.getValueTypeClass() &&
            value == * reinterpret_cast< const ::rtl::OUString * >( rAny.getValue() ));
}
// type
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Type & value ) SAL_THROW( () )
{
    if (TypeClass_TYPE == rAny.getValueTypeClass())
    {
        value = * reinterpret_cast< const Type * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const Type & value ) SAL_THROW( () )
{
    return (TypeClass_TYPE == rAny.getValueTypeClass() &&
            value == * reinterpret_cast< const Type * >( rAny.getValue() ));
}
// any
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const Any & rAny, Any & value ) SAL_THROW( () )
{
    value = rAny;
    return sal_True;
}
// interface
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const BaseReference & value ) SAL_THROW( () )
{
    if (TypeClass_INTERFACE == rAny.getValueTypeClass())
    {
        XInterface * p1 = * reinterpret_cast< XInterface * const * >( rAny.getValue() );
        if (p1 && p1 == value.get())
            return sal_True;
        Reference< XInterface > xI1( p1, UNO_QUERY );
        Reference< XInterface > xI2( value.get(), UNO_QUERY );
        return (xI1.is() && xI1.get() == xI2.get());
    }
    return sal_False;
}

// operator to compare to an any.
//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator == ( const Any & rAny, const C & value ) SAL_THROW( () )
{
    const Type & rType = ::getCppuType( &value );
    return ::uno_type_equalData(
        const_cast< void * >( rAny.getValue() ), rAny.getValueTypeRef(),
        const_cast< C * >( &value ), rType.getTypeLibType(),
        cpp_queryInterface, cpp_release );
}

}
}
}
}

#endif
