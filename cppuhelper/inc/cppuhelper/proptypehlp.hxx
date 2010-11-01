/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CPPUHELPER_PROPTYPEHLP_HXX
#define _CPPUHELPER_PROPTYPEHLP_HXX

#include <cppuhelper/proptypehlp.h>

namespace cppu
{

/** Converts the value stored in an any to a concrete C++ type.
    The function does the same as the operator >>= () at the
    Any class, except that it throws an IllegalArgumentException in case of
    failures (the value cannot be extracted without data loss )

   @exception com::sun::star::lang::IllegalArgumentException when the type could not be converted.
 */
template < class target >
inline void SAL_CALL convertPropertyValue( target &value , const  ::com::sun::star::uno::Any & a)
{

    if( !( a >>= value ) ) {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}


// This template is needed at least for msci4 compiler
template < class target >
inline void SAL_CALL convertPropertyValue( target &value ,  ::com::sun::star::uno::Any & a)
{
    convertPropertyValue( value ,  (const ::com::sun::star::uno::Any & )  a );
}

/**
  conversion of basic types
*/
inline void SAL_CALL convertPropertyValue( sal_Bool & b   , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        sal_Int32 i32 = 0;
        a >>= i32;
        b = ( sal_Bool )i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c = *(sal_Unicode*) a.getValue();
        b = ( sal_Bool ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16 = 0;
        a >>= i16;
        b = ( sal_Bool ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        b = *((sal_Bool*)a.getValue());
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8 = 0;
        a >>= i8;
        b = ( sal_Bool ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16 = 0;
        a >>= i16;
        b = ( sal_Bool  ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32 = 0;
        a >>= i32;
        b = ( sal_Bool ) i32;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}

inline void SAL_CALL convertPropertyValue( sal_Int64 & i  , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if( ::com::sun::star::uno::TypeClass_HYPER == tc ) {
        a >>= i;
    }
    else if( ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER == tc ) {
        sal_uInt64 i64 = 0;
        a >>= i64;
        i = ( sal_Int64 ) i64;
    }
    else if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        sal_Int32 i32 = 0;
        a >>= i32;
        i = ( sal_Int64 )i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode *)a.getValue();
        i = ( sal_Int64 ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16 = 0;
        a >>= i16;
        i = ( sal_Int64 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_Int64 ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8 = 0;
        a >>= i8;
        i = ( sal_Int64 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16 = 0;
        a >>= i16;
        i = ( sal_Int64 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32 = 0;
        a >>= i32;
        i = ( sal_Int64 ) i32;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}


inline void SAL_CALL convertPropertyValue( sal_uInt64 & i  , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if( ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER == tc ) {
        a >>= i;
    }
    if( ::com::sun::star::uno::TypeClass_HYPER == tc ) {
        sal_Int64 i64;
        a >>= i64;
        i = ( sal_uInt64 ) i64;
    }
    else if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        sal_Int32 i32;
        a >>= i32;
        i = ( sal_uInt64 )i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *( sal_Unicode * ) a.getValue() ;
        i = ( sal_uInt64 ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16;
        a >>= i16;
        i = ( sal_uInt64 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_uInt64 ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8;
        a >>= i8;
        i = ( sal_uInt64 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16;
        a >>= i16;
        i = ( sal_uInt64    ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32;
        a >>= i32;
        i = ( sal_uInt64 ) i32;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}

// the basic types
// sal_Int32
inline void SAL_CALL convertPropertyValue( sal_Int32 & i  , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        a >>= i;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode*) a.getValue();
        i = ( sal_Int32 ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16 = 0;
        a >>= i16;
        i = ( sal_Int32 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_Int32 ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8 = 0;
        a >>= i8;
        i = ( sal_Int32 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16 = 0;
        a >>= i16;
        i = ( sal_Int32 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32 = 0;
        a >>= i32;
        i = ( sal_Int32 ) i32;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}

inline void SAL_CALL convertPropertyValue( sal_uInt32 & i  , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        a >>= i;
    }
    else if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        sal_Int32 i32;
        a >>= i32;
        i = (sal_uInt32 ) i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode*) a.getValue();
        i = ( sal_uInt32 ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16;
        a >>= i16;
        i = ( sal_uInt32 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_uInt32 ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8;
        a >>= i8;
        i = ( sal_uInt32 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16;
        a >>= i16;
        i = ( sal_uInt32    ) i16;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}


inline void SAL_CALL convertPropertyValue( sal_Int16 & i  , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        a >>= i;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode*) a.getValue();
        i = ( sal_Int16 ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_Int16 ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8 = 0;
        a >>= i8;
        i = ( sal_Int16 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16 = 0;
        a >>= i16;
        i = ( sal_Int16 ) i16;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}

inline void SAL_CALL convertPropertyValue( sal_uInt16 & i  , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        a >>= i;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode *) a.getValue();
        i = ( sal_Int16 ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_Int16 ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8 = 0;
        a >>= i8;
        i = ( sal_Int16 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16 = 0;
        a >>= i16;
        i = ( sal_Int16 ) i16;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}

inline void SAL_CALL convertPropertyValue( sal_Int8 & i  , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        a >>= i;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_Int8 ) b;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}

inline void SAL_CALL convertPropertyValue( float &f , const ::com::sun::star::uno::Any &a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if ( ::com::sun::star::uno::TypeClass_FLOAT == tc ) {
        a >>= f;
    }
    else if( ::com::sun::star::uno::TypeClass_DOUBLE == tc ) {
         double d = 0;
         a >>= d;
         f = ( float ) d;
    }
    else if( ::com::sun::star::uno::TypeClass_HYPER == tc ) {
        sal_Int64 i64 = 0;
        a >>= i64;
        f = ( float ) i64;
    }
    // msci 4 does not support this conversion
/*  else if( ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER == tc ) {
        sal_uInt64 i64;
        a >>= i64;
        f = ( float ) i64;
    }
*/  else if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        sal_Int32 i32 = 0;
        a >>= i32;
        f = ( float )i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode*) a.getValue();
        f = ( float ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16 = 0;
        a >>= i16;
        f = ( float ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        f = ( float ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8 = 0;
        a >>= i8;
        f = ( float ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16 = 0;
        a >>= i16;
        f = ( float ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32 = 0;
        a >>= i32;
        f = ( float ) i32;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}


inline void SAL_CALL convertPropertyValue( double &d , const ::com::sun::star::uno::Any &a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if( ::com::sun::star::uno::TypeClass_DOUBLE == tc ) {
         float f;
         a >>= f;
         d = ( double ) f;
    }
    else if ( ::com::sun::star::uno::TypeClass_FLOAT == tc ) {
        float f;
        a >>= f;
        d = (double) f;
    }
    else if( ::com::sun::star::uno::TypeClass_HYPER == tc ) {
        sal_Int64 i64;
        a >>= i64;
        d = (double) i64;
    }
    // msci 4 does not support this
/*  else if( ::com::sun::star::uno::TypeClass_UNSIGNED_HYPER == tc ) {
        sal_uInt64 i64;
        a >>= i64;
        d = (double) i64;
    }
*/  else if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        sal_Int32 i32;
        a >>= i32;
        d = (double)i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode*) a.getValue();
        d = (double) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16;
        a >>= i16;
        d = (double) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        d = (double) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8;
        a >>= i8;
        d = (double) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16;
        a >>= i16;
        d = (double) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32;
        a >>= i32;
        d = (double) i32;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}

inline void SAL_CALL convertPropertyValue( ::rtl::OUString &ow , const ::com::sun::star::uno::Any &a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    if( ::com::sun::star::uno::TypeClass_STRING == a.getValueType().getTypeClass() ) {
        a >>= ow;
    }
    else {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}

}     // end namespace cppu

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
