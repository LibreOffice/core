/*************************************************************************
 *
 *  $RCSfile: Any.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:50 $
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
inline Any::Any()
{
    ::uno_any_construct( this, 0, 0, cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const Any & rAny )
{
    ::uno_type_any_construct( this, rAny.pData, rAny.getValueTypeRef(), cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData, const Type & rType )
{
    ::uno_type_any_construct( this, const_cast< void * >( pData ), rType.getTypeLibType(), cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData, typelib_TypeDescription * pTypeDescr )
{
    ::uno_any_construct( this, const_cast< void * >( pData ), pTypeDescr, cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::Any( const void * pData, typelib_TypeDescriptionReference * pType )
{
    ::uno_type_any_construct( this, const_cast< void * >( pData ), pType, cpp_acquire );
}
//__________________________________________________________________________________________________
inline Any::~Any()
{
    ::uno_any_destruct( this, cpp_release );
}
//__________________________________________________________________________________________________
inline Any & Any::operator = ( const Any & rAny )
{
    if (this != &rAny)
        setValue( rAny.getValue(), rAny.getValueTypeRef() );
    return *this;
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData, const Type & rType )
{
    ::uno_any_destruct( this, cpp_release );
    ::uno_type_any_construct( this, const_cast< void * >( pData ), rType.getTypeLibType(), cpp_acquire );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData, typelib_TypeDescriptionReference * pType )
{
    ::uno_any_destruct( this, cpp_release );
    ::uno_type_any_construct( this, const_cast< void * >( pData ), pType, cpp_acquire );
}
//__________________________________________________________________________________________________
inline void Any::setValue( const void * pData, typelib_TypeDescription * pTypeDescr )
{
    ::uno_any_destruct( this, cpp_release );
    ::uno_any_construct( this, const_cast< void * >( pData ), pTypeDescr, cpp_acquire );
}
//__________________________________________________________________________________________________
inline void Any::clear()
{
    ::uno_any_destruct( this, cpp_release );
    ::uno_any_construct( this, 0, 0, cpp_acquire );
}
//__________________________________________________________________________________________________
inline sal_Bool Any::operator == ( const Any & rAny ) const
{
    const Type & rType = ::getCppuType( this );
    return ::uno_type_equalData(
        const_cast< void * >( getValue() ), getValueTypeRef(),
        const_cast< void * >( rAny.getValue() ), rAny.getValueTypeRef(),
        cpp_queryInterface, cpp_release );
}

//__________________________________________________________________________________________________
template< class C >
inline Any SAL_CALL makeAny( const C & value )
{
    return Any( &value, ::getCppuType( &value ) );
}

//__________________________________________________________________________________________________
template< class C >
inline void SAL_CALL operator <<= ( ::com::sun::star::uno::Any & rAny, const C & value )
{
    rAny.setValue( &value, ::getCppuType( &value ) );
}

//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, C & value )
{
    const ::com::sun::star::uno::Type & rType = ::getCppuType( &value );
    return ::uno_type_assignData(
        &value, rType.getTypeLibType(),
        const_cast< void * >( rAny.getValue() ), rAny.getValueTypeRef(),
        ::com::sun::star::uno::cpp_queryInterface,
        ::com::sun::star::uno::cpp_acquire,
        ::com::sun::star::uno::cpp_release );
}

// bool
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Bool & value )
{
    if (::com::sun::star::uno::TypeClass_BOOLEAN == rAny.getValueTypeClass())
    {
        value = (* reinterpret_cast< const sal_Bool * >( rAny.getValue() ) != sal_False);
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == ( const ::com::sun::star::uno::Any & rAny, const sal_Bool & value )
{
    return (::com::sun::star::uno::TypeClass_BOOLEAN == rAny.getValueTypeClass() &&
            (value != sal_False) == (* reinterpret_cast< const sal_Bool * >( rAny.getValue() ) != sal_False));
}
// byte
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int8 & value )
{
    if (::com::sun::star::uno::TypeClass_BYTE == rAny.getValueTypeClass())
    {
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// short
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int16 & value )
{
    switch (rAny.getValueTypeClass())
    {
    case ::com::sun::star::uno::TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_SHORT:
    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_uInt16 & value )
{
    switch (rAny.getValueTypeClass())
    {
    case ::com::sun::star::uno::TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_SHORT:
    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// long
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int32 & value )
{
    switch (rAny.getValueTypeClass())
    {
    case ::com::sun::star::uno::TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_LONG:
    case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_uInt32 & value )
{
    switch (rAny.getValueTypeClass())
    {
    case ::com::sun::star::uno::TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_LONG:
    case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// hyper
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_Int64 & value )
{
    switch (rAny.getValueTypeClass())
    {
    case ::com::sun::star::uno::TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_HYPER:
    case ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
        value = * reinterpret_cast< const sal_Int64 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, sal_uInt64 & value )
{
    switch (rAny.getValueTypeClass())
    {
    case ::com::sun::star::uno::TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_HYPER:
    case ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
        value = * reinterpret_cast< const sal_uInt64 * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// float
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, float & value )
{
    switch (rAny.getValueTypeClass())
    {
    case ::com::sun::star::uno::TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_FLOAT:
        value = * reinterpret_cast< const float * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// double
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= ( const ::com::sun::star::uno::Any & rAny, double & value )
{
    switch (rAny.getValueTypeClass())
    {
    case ::com::sun::star::uno::TypeClass_BYTE:
        value = * reinterpret_cast< const sal_Int8 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_SHORT:
        value = * reinterpret_cast< const sal_Int16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        value = * reinterpret_cast< const sal_uInt16 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_LONG:
        value = * reinterpret_cast< const sal_Int32 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        value = * reinterpret_cast< const sal_uInt32 * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_FLOAT:
        value = * reinterpret_cast< const float * >( rAny.getValue() );
        return sal_True;
    case ::com::sun::star::uno::TypeClass_DOUBLE:
        value = * reinterpret_cast< const double * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
// string
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= (
    const ::com::sun::star::uno::Any & rAny, ::rtl::OUString & value )
{
    if (::com::sun::star::uno::TypeClass_STRING == rAny.getValueTypeClass())
    {
        value = * reinterpret_cast< const ::rtl::OUString * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == (
    const ::com::sun::star::uno::Any & rAny, const ::rtl::OUString & value )
{
    return (::com::sun::star::uno::TypeClass_STRING == rAny.getValueTypeClass() &&
            value == * reinterpret_cast< const ::rtl::OUString * >( rAny.getValue() ));
}
// type
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= (
    const ::com::sun::star::uno::Any & rAny, ::com::sun::star::uno::Type & value )
{
    if (::com::sun::star::uno::TypeClass_TYPE == rAny.getValueTypeClass())
    {
        value = * reinterpret_cast< const ::com::sun::star::uno::Type * >( rAny.getValue() );
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == (
    const ::com::sun::star::uno::Any & rAny, const ::com::sun::star::uno::Type & value )
{
    return (::com::sun::star::uno::TypeClass_TYPE == rAny.getValueTypeClass() &&
            value == * reinterpret_cast< const ::com::sun::star::uno::Type * >( rAny.getValue() ));
}
// any
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator >>= (
    const ::com::sun::star::uno::Any & rAny, ::com::sun::star::uno::Any & value )
{
    value = rAny;
    return sal_True;
}
// interface
//__________________________________________________________________________________________________
inline sal_Bool SAL_CALL operator == (
    const ::com::sun::star::uno::Any & rAny, const ::com::sun::star::uno::BaseReference & value )
{
    if (::com::sun::star::uno::TypeClass_INTERFACE == rAny.getValueTypeClass())
    {
        ::com::sun::star::uno::XInterface * p1 =
              * reinterpret_cast< ::com::sun::star::uno::XInterface * const * >( rAny.getValue() );
        if (p1 && p1 == value.get())
            return sal_True;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xI1(
            p1, ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xI2(
            value.get(), ::com::sun::star::uno::UNO_QUERY );
        return (xI1.is() && xI1.get() == xI2.get());
    }
    return sal_False;
}

// operator to compare to an any.
//__________________________________________________________________________________________________
template< class C >
inline sal_Bool SAL_CALL operator == ( const ::com::sun::star::uno::Any & rAny, const C & value )
{
    const ::com::sun::star::uno::Type & rType = ::getCppuType( &value );
    return ::uno_type_equalData(
        const_cast< void * >( rAny.getValue() ), rAny.getValueTypeRef(),
        const_cast< C * >( &value ), rType.getTypeLibType(),
        ::com::sun::star::uno::cpp_queryInterface,
        ::com::sun::star::uno::cpp_release );
}

}
}
}
}

#endif
