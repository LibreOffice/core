/*************************************************************************
 *
 *  $RCSfile: proptypehlp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:15:26 $
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
#ifndef _CPPUHELPER_PROPTYPEHLP_HXX
#define _CPPUHELPER_PROPTYPEHLP_HXX

#include <cppuhelper/proptypehlp.h>

namespace cppu
{

template < class target >
inline void SAL_CALL convertPropertyValue( target &value , const  ::com::sun::star::uno::Any & a)
//      SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
//  any sense ?
//  const ::com::sun::star::uno::Type &t = ::getCppuType( &value );
//  enum TypeClass tc = t.getTypeClass();

    if( !( a >>= value ) ) {
        throw ::com::sun::star::lang::IllegalArgumentException();
    }
}


// This template is needed at least for msci4 compiler
template < class target >
inline void SAL_CALL convertPropertyValue( target &value ,  ::com::sun::star::uno::Any & a)
//      SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    convertPropertyValue( value ,  (const ::com::sun::star::uno::Any & )  a );
}

inline void SAL_CALL convertPropertyValue( sal_Bool & b   , const ::com::sun::star::uno::Any & a )
    SAL_THROW( (::com::sun::star::lang::IllegalArgumentException) )
{
    const enum ::com::sun::star::uno::TypeClass tc = a.getValueType().getTypeClass();

    if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        sal_Int32 i32;
        a >>= i32;
        b = ( sal_Bool )i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c = *(sal_Unicode*) a.getValue();
        b = ( sal_Bool ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16;
        a >>= i16;
        b = ( sal_Bool ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        b = *((sal_Bool*)a.getValue());
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8;
        a >>= i8;
        b = ( sal_Bool ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16;
        a >>= i16;
        b = ( sal_Bool  ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32;
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
        sal_uInt64 i64;
        a >>= i64;
        i = ( sal_Int64 ) i64;
    }
    else if( ::com::sun::star::uno::TypeClass_LONG == tc ) {
        sal_Int32 i32;
        a >>= i32;
        i = ( sal_Int64 )i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode *)a.getValue();
        i = ( sal_Int64 ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16;
        a >>= i16;
        i = ( sal_Int64 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_Int64 ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8;
        a >>= i8;
        i = ( sal_Int64 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16;
        a >>= i16;
        i = ( sal_Int64 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32;
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
        sal_Int16 i16;
        a >>= i16;
        i = ( sal_Int32 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        i = ( sal_Int32 ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8;
        a >>= i8;
        i = ( sal_Int32 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16;
        a >>= i16;
        i = ( sal_Int32 ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32;
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
        sal_Int8 i8;
        a >>= i8;
        i = ( sal_Int16 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16;
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
        sal_Int8 i8;
        a >>= i8;
        i = ( sal_Int16 ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16;
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
         double d;
         a >>= d;
         f = ( float ) d;
    }
    else if( ::com::sun::star::uno::TypeClass_HYPER == tc ) {
        sal_Int64 i64;
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
        sal_Int32 i32;
        a >>= i32;
        f = ( float )i32;
    }
    else if ( ::com::sun::star::uno::TypeClass_CHAR  == tc ) {
        sal_Unicode c;
        c = *(sal_Unicode*) a.getValue();
        f = ( float ) c;
    }
    else if ( ::com::sun::star::uno::TypeClass_SHORT == tc ) {
        sal_Int16 i16;
        a >>= i16;
        f = ( float ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_BOOLEAN == tc ) {
        sal_Bool b;
        b =  *((sal_Bool * )a.getValue());
        f = ( float ) b;
    }
    else if ( ::com::sun::star::uno::TypeClass_BYTE == tc ) {
        sal_Int8 i8;
        a >>= i8;
        f = ( float ) i8;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT == tc ) {
        sal_uInt16 i16;
        a >>= i16;
        f = ( float ) i16;
    }
    else if ( ::com::sun::star::uno::TypeClass_UNSIGNED_LONG == tc ) {
        sal_uInt32 i32;
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


