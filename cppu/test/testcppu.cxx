/*************************************************************************
 *
 *  $RCSfile: testcppu.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-30 12:04:07 $
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
#include <stdio.h>

#include <uno/environment.h>
#include <uno/mapping.hxx>

#include <uno/dispatcher.h>
#include <rtl/alloc.h>

#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Uik.hpp>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <osl/interlck.h>


#include <test/Test1.hpp>
#include <test/Test2.hpp>
#include <test/TdTest1.hpp>
#include <test/Test3.hpp>
#include <test/Base.hpp>
#include <test/Base1.hpp>
#include <test/Base2.hpp>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <test/XSimpleInterface.hpp>
#include <test/XLanguageBindingTest.hpp>

using namespace test;
using namespace rtl;
using namespace osl;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace test;


sal_Int32 getSize( const Type & rT )
{
    sal_Int32 nSize;
    typelib_TypeDescription * pTD = 0;
    typelib_typedescriptionreference_getDescription( &pTD, rT.getTypeLibType() );
    nSize = pTD->nSize;
    typelib_typedescription_release( pTD );
    return nSize;
}
/*
 * main.
 */
void testCppu()
{
    {
    // test the size of types
    OSL_ENSURE( sizeof( Uik ) == getSize( getCppuType( (Uik *)0) ),
                "bad sizeof uik" );
    OSL_ENSURE( sizeof( Test1 ) == getSize( getCppuType( (Test1*)0).getTypeLibType() ),
                "bad sizeof test1" );
//      OSL_ENSURE( sizeof( TdTest1 ) == getSize( get_test_TdTest1_Type().getTypeLibType() ),
//                  "bad sizeof TypedefTest1" );
    OSL_ENSURE( sizeof( Test2 ) == getSize( getCppuType( (Test2*)0).getTypeLibType() ),
                "bad sizeof test2" );

/* find the error
sal_Int32 nPos;
nPos = (sal_Int32)&((Test3 *)0)->nInt8;
nPos = (sal_Int32)&((Test3 *)0)->nFloat;
nPos = (sal_Int32)&((Test3 *)0)->nuInt8;
nPos = (sal_Int32)&((Test3 *)0)->nDouble;
nPos = (sal_Int32)&((Test3 *)0)->nInt16;
nPos = (sal_Int32)&((Test3 *)0)->aString;
nPos = (sal_Int32)&((Test3 *)0)->nuInt16;
nPos = (sal_Int32)&((Test3 *)0)->nInt64;
nPos = (sal_Int32)&((Test3 *)0)->nInt32;
nPos = (sal_Int32)&((Test3 *)0)->nuInt64;
nPos = (sal_Int32)&((Test3 *)0)->nuInt32;
nPos = (sal_Int32)&((Test3 *)0)->eType;
nPos = (sal_Int32)&((Test3 *)0)->wChar;
nPos = (sal_Int32)&((Test3 *)0)->td;
nPos = (sal_Int32)&((Test3 *)0)->bBool;
nPos = (sal_Int32)&((Test3 *)0)->aAny;
*/
    OSL_ENSURE( sizeof( Test3 ) == getSize( getCppuType( (Test3*)0).getTypeLibType() ),
                "bad sizeof test3" );
    }

    {
    // test the default constructor
    Test1 a1;
    a1.nInt16 = 4;
    a1.dDouble = 3.6;
    a1.bBool = sal_True;
    uno_type_constructData( &a1, getCppuType( (Test1*)0).getTypeLibType() );
    OSL_ASSERT( a1.nInt16 == 0 && a1.dDouble == 0.0 && a1.bBool == sal_False);

    Test2 a2;
    a2.nInt16 = 2;
    a2.aTest1.nInt16 = 4;
    a2.aTest1.dDouble = 3.6;
    a2.aTest1.dDouble = sal_True;
    uno_type_constructData( &a2, getCppuType( (Test2*)0).getTypeLibType() );
    OSL_ASSERT( a2.nInt16 == 0 && a2.aTest1.nInt16 == 0 && a2.aTest1.dDouble == 0.0 && a2.aTest1.bBool == sal_False);

    Test3 * pa3 = (Test3 *)new char[ sizeof( Test3 ) ];
    Test3 & a3 = *pa3;
    a3.nInt8 = 2;
    a3.nFloat = (float)2;
    a3.nDouble = 2;
    a3.nInt16 = 2;
    a3.nuInt16 = 2;
    a3.nInt64 = 2;
    a3.nInt32 = 2;
    a3.nuInt64 = 2;
    a3.nuInt32 = 2;
    a3.eType = TypeClass_STRUCT;
    a3.wChar = L'g';
    a3.td.nInt16 = 2;
    a3.td.dDouble = 2;
    a3.bBool = sal_True;
    uno_type_constructData( &a3, getCppuType( (Test3*)0).getTypeLibType() );
    OSL_ASSERT( a3.nInt8 == 0 && a3.nFloat == (float)0
                && a3.nDouble == 0 && a3.nInt16 == 0 && a3.aString == OUString()
                && a3.nuInt16 == 0 && a3.nInt64 == 0 && a3.nInt32 == 0
                && a3.nuInt64 == 0 && a3.nuInt32 == 0 && a3.eType == TypeClass_VOID
                && a3.wChar == L'\0' && a3.td.nInt16 == 0 && a3.td.dDouble == 0
                && a3.bBool == sal_False );
    OSL_ASSERT( a3.aAny.getValueType() == getCppuVoidType() );
    delete pa3;
    }

    {
    // test the destructor
    long a1[ sizeof( Test1 ) / sizeof(long) +1 ];
    uno_type_constructData( &a1, getCppuType( (Test1*)0).getTypeLibType() );
    uno_type_destructData( &a1, getCppuType( (Test1*)0).getTypeLibType(), cpp_release );

    long a2[ sizeof( Test2 ) / sizeof(long) +1 ];
    uno_type_constructData( &a2, getCppuType( (Test2*)0).getTypeLibType() );
    uno_type_destructData( &a2, getCppuType( (Test2*)0).getTypeLibType(), cpp_release );

    long a3[ sizeof( Test3 ) / sizeof(long) +1 ];
    uno_type_constructData( &a3, getCppuType( (Test3*)0).getTypeLibType() );
    OUString aTestString( RTL_CONSTASCII_USTRINGPARAM("test") );
    ((Test3*)a3)->aString = aTestString;
    uno_type_destructData( &a3, getCppuType( (Test3*)0).getTypeLibType(), cpp_release );
    OSL_ASSERT( aTestString.pData->refCount == 1 );
    }

    {
    // test the copy constructor
    Test1 a1;
    a1.nInt16 = 4;
    a1.dDouble = 3.6;
    a1.bBool = sal_True;
    char sz1[sizeof( Test1 )];
    uno_type_copyData( sz1, &a1, getCppuType( (Test1*)0).getTypeLibType(), cpp_acquire );
    OSL_ASSERT( ((Test1*)sz1)->nInt16 == 4 && ((Test1*)sz1)->dDouble == 3.6 && ((Test1*)sz1)->bBool == sal_True);

    Test2 a2;
    a2.nInt16 = 2;
    a2.aTest1.nInt16 = 4;
    a2.aTest1.dDouble = 3.6;
    a2.aTest1.bBool = sal_True;
    char sz2[sizeof( Test2 )];
    uno_type_copyData( sz2, &a2, getCppuType( (Test2*)0).getTypeLibType(), cpp_acquire );
    OSL_ASSERT( ((Test2*)sz2)->nInt16 == 2 );
    OSL_ASSERT(((Test2*)sz2)->aTest1.nInt16 == 4 );
    OSL_ASSERT( ((Test2*)sz2)->aTest1.dDouble == 3.6 );
    OSL_ASSERT(((Test2*)sz2)->aTest1.bBool == sal_True);

    Test3 a3;
    a3.nInt8 = 2;
    a3.nFloat = (float)2;
    a3.nDouble = 2;
    a3.nInt16 = 2;
    a3.aString = OUString::createFromAscii("2");
    a3.nuInt16 = 2;
    a3.nInt64 = 2;
    a3.nInt32 = 2;
    a3.nuInt64 = 2;
    a3.nuInt32 = 2;
    a3.eType = TypeClass_STRUCT;
    a3.wChar = L'2';
    a3.td.nInt16 = 2;
    a3.td.dDouble = 2;
    a3.bBool = sal_True;
    a3.aAny = makeAny( (sal_Int32)2 );
    char sz3[sizeof( Test3 )];
    uno_type_copyData( sz3, &a3, getCppuType( (Test3*)0).getTypeLibType(), cpp_acquire );
    OSL_ASSERT( ((Test3*)sz3)->nInt8 == 2 );
    OSL_ASSERT( ((Test3*)sz3)->nFloat == (float)2 );
    OSL_ASSERT( ((Test3*)sz3)->nDouble == 2 );
    OSL_ASSERT( ((Test3*)sz3)->nInt16 == 2 );
    OSL_ASSERT( ((Test3*)sz3)->aString == OUString::createFromAscii("2") );
    OSL_ASSERT( ((Test3*)sz3)->nuInt16 == 2 );
    OSL_ASSERT( ((Test3*)sz3)->nInt64 == 2 );
    OSL_ASSERT( ((Test3*)sz3)->nInt32 == 2 );
    OSL_ASSERT( ((Test3*)sz3)->nuInt64 == 2 );
    OSL_ASSERT( ((Test3*)sz3)->nuInt32 == 2 );
    OSL_ASSERT( ((Test3*)sz3)->eType == TypeClass_STRUCT );
    OSL_ASSERT( ((Test3*)sz3)->wChar == L'2' );
    OSL_ASSERT( ((Test3*)sz3)->td.nInt16 == 2 );
    OSL_ASSERT( ((Test3*)sz3)->td.dDouble == 2 );
    OSL_ASSERT( ((Test3*)sz3)->bBool == sal_True );
    OSL_ASSERT( ((Test3*)sz3)->aAny.getValueType() == getCppuType( (sal_Int32 *)0 ) );
    OSL_ASSERT( *(sal_Int32*)((Test3*)sz3)->aAny.getValue() == 2 );
    ((Test3 *)sz3)->~Test3();
    }

    {
    sal_Bool bAssignable;
    // test assignment
    Test1 a1;
    a1.nInt16 = 4;
    a1.dDouble = 3.6;
    a1.bBool = sal_True;
    Test1 sz1;
    bAssignable = uno_type_assignData(
        &sz1, getCppuType( (Test1*)0).getTypeLibType(),
        &a1, getCppuType( (Test1*)0).getTypeLibType(),
        cpp_queryInterface, cpp_acquire, cpp_release );
    OSL_ASSERT( bAssignable );
    OSL_ASSERT( sz1.nInt16 == 4 && sz1.dDouble == 3.6 && sz1.bBool == sal_True);

    Test2 a2;
    a2.nInt16 = 2;
    a2.aTest1.nInt16 = 4;
    a2.aTest1.dDouble = 3.6;
    a2.aTest1.bBool = sal_True;
    Test2 sz2;
    bAssignable = uno_type_assignData(
        &sz2, getCppuType( (Test2*)0).getTypeLibType(),
        &a2, getCppuType( (Test2*)0).getTypeLibType(),
        cpp_queryInterface, cpp_acquire, cpp_release );
    OSL_ASSERT( bAssignable );
    OSL_ASSERT( sz2.nInt16 == 2 && sz2.aTest1.nInt16 == 4
                && sz2.aTest1.dDouble == 3.6 && sz2.aTest1.bBool == sal_True);

    Test3 a3;
    Test3 sz3;
    a3.nInt8 = 2;
    a3.nFloat = (float)2;
    a3.nDouble = 2;
    a3.nInt16 = 2;
    a3.aString = OUString::createFromAscii("2");
    a3.nuInt16 = 2;
    a3.nInt64 = 2;
    a3.nInt32 = 2;
    a3.nuInt64 = 2;
    a3.nuInt32 = 2;
    a3.eType = TypeClass_STRUCT;
    a3.wChar = L'2';
    a3.td.nInt16 = 2;
    a3.td.dDouble = 2;
    a3.bBool = sal_True;
    a3.aAny = makeAny( (sal_Int32)2 );
    bAssignable = uno_type_assignData(
        &sz3, getCppuType( (Test3*)0).getTypeLibType(),
        &a3, getCppuType( (Test3*)0).getTypeLibType(),
        cpp_queryInterface, cpp_acquire, cpp_release );
    OSL_ASSERT( bAssignable );
    OSL_ASSERT( sz3.nInt8 == 2 );
    OSL_ASSERT( sz3.nFloat == (float)2 );
    OSL_ASSERT( sz3.nDouble == 2 );
    OSL_ASSERT( sz3.nInt16 == 2 );
    OSL_ASSERT( sz3.aString == OUString::createFromAscii("2") );
    OSL_ASSERT( sz3.nuInt16 == 2 );
    OSL_ASSERT( sz3.nInt64 == 2 );
    OSL_ASSERT( sz3.nInt32 == 2 );
    OSL_ASSERT( sz3.nuInt64 == 2 );
    OSL_ASSERT( sz3.nuInt32 == 2 );
    OSL_ASSERT( sz3.eType == TypeClass_STRUCT );
    OSL_ASSERT( sz3.wChar == L'2' );
    OSL_ASSERT( sz3.td.nInt16 == 2 );
    OSL_ASSERT( sz3.td.dDouble == 2 );
    OSL_ASSERT( sz3.bBool == sal_True );
    OSL_ASSERT( sz3.aAny.getValueType() == getCppuType( (sal_Int32 *)0 ) );
    OSL_ASSERT( *(sal_Int32*)sz3.aAny.getValue() == 2 );

    // test not assigneable values
    bAssignable = uno_type_assignData(
        &a1, getCppuType( (Test1*)0).getTypeLibType(),
        &a2, getCppuType( (Test2*)0).getTypeLibType(),
        cpp_queryInterface, cpp_acquire, cpp_release );
    OSL_ASSERT( !bAssignable );
    }

    {
    // test any
    Any aAny = makeAny( (sal_Int8)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int8 *)0 ) );
    OSL_ASSERT( *(sal_Int8*)aAny.getValue() == 2 );
    aAny = makeAny( (float)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (float *)0 ) );
    OSL_ASSERT( *(float*)aAny.getValue() == (float)2 );
    aAny = makeAny( (sal_Int8)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int8 *)0 ) );
    OSL_ASSERT( *(sal_Int8*)aAny.getValue() == 2 );
    aAny = makeAny( (double)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (double *)0 ) );
    OSL_ASSERT( *(double*)aAny.getValue() == (double)2 );
    aAny = makeAny( (sal_Int16)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int16 *)0 ) );
    OSL_ASSERT( *(sal_Int16*)aAny.getValue() == 2 );
    aAny = makeAny( OUString( RTL_CONSTASCII_USTRINGPARAM("test") ) );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (OUString *)0 ) );
    OSL_ASSERT( *(OUString*)aAny.getValue() == OUString::createFromAscii("test") );
    aAny = makeAny( (sal_uInt16)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_uInt16 *)0 ) );
    OSL_ASSERT( *(sal_Int16*)aAny.getValue() == 2 );
    sal_Int64 aInt64 = 0x200000000;
    aAny = makeAny( aInt64 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int64 *)0 ) );
    OSL_ASSERT( *(sal_Int64*)aAny.getValue() == 0x200000000 );
    aAny = makeAny( (sal_Int32)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int32 *)0 ) );
    OSL_ASSERT( *(sal_Int32*)aAny.getValue() == 2 );
    sal_uInt64 auInt64 = 0x200000000;
    aAny = makeAny( auInt64 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_uInt64 *)0 ) );
    OSL_ASSERT( *(sal_uInt64*)aAny.getValue() == 0x200000000 );
    aAny = makeAny( (sal_uInt32)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_uInt32 *)0 ) );
    OSL_ASSERT( *(sal_uInt32*)aAny.getValue() == 2 );
    aAny = makeAny( TypeClass_STRUCT );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (TypeClass *)0 ) );
    OSL_ASSERT( *(TypeClass*)aAny.getValue() == TypeClass_STRUCT );
    sal_Unicode c = L'2';
    aAny.setValue( &c, getCppuCharType() );
    OSL_ASSERT( aAny.getValueType() == getCppuCharType() );
    OSL_ASSERT( *(sal_Unicode*)aAny.getValue() == L'2' );
    sal_Bool b = sal_True;
    aAny.setValue( &b, getCppuBooleanType() );
    OSL_ASSERT( aAny.getValueType() == getCppuBooleanType() );
    OSL_ASSERT( *(sal_Bool*)aAny.getValue() == sal_True );
    }

    {
    // test: operator <<=( any, value )
    Any aAny;
    aAny <<= (sal_Int8)2;
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int8 *)0 ) );
    OSL_ASSERT( *(sal_Int8*)aAny.getValue() == 2 );
    aAny <<=( (float)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (float *)0 ) );
    OSL_ASSERT( *(float*)aAny.getValue() == (float)2 );
//      aAny <<=( (sal_uInt8)2 );
//      OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_uInt8 *)0 ) );
//      OSL_ASSERT( *(sal_uInt8*)aAny.getValue() == 2 );
    aAny <<=( (double)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (double *)0 ) );
    OSL_ASSERT( *(double*)aAny.getValue() == (double)2 );
    aAny <<=( (sal_Int16)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int16 *)0 ) );
    OSL_ASSERT( *(sal_Int16*)aAny.getValue() == 2 );
    aAny <<=( OUString( RTL_CONSTASCII_USTRINGPARAM("test") ) );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (OUString *)0 ) );
    OSL_ASSERT( *(OUString*)aAny.getValue() == OUString::createFromAscii("test") );
    aAny <<=( (sal_uInt16)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_uInt16 *)0 ) );
    OSL_ASSERT( *(sal_Int16*)aAny.getValue() == 2 );
    sal_Int64 aInt64 = 0x200000000;
    aAny <<=( aInt64 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int64 *)0 ) );
    OSL_ASSERT( *(sal_Int64*)aAny.getValue() == 0x200000000 );
    aAny <<=( (sal_Int32)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_Int32 *)0 ) );
    OSL_ASSERT( *(sal_Int32*)aAny.getValue() == 2 );
    sal_uInt64 auInt64 = 0x200000000;
    aAny <<=( auInt64 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_uInt64 *)0 ) );
    OSL_ASSERT( *(sal_uInt64*)aAny.getValue() == 0x200000000 );
    aAny <<=( (sal_uInt32)2 );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (sal_uInt32 *)0 ) );
    OSL_ASSERT( *(sal_uInt32*)aAny.getValue() == 2 );
    aAny <<=( TypeClass_STRUCT );
    OSL_ASSERT( aAny.getValueType() == getCppuType( (TypeClass *)0 ) );
    OSL_ASSERT( *(TypeClass*)aAny.getValue() == TypeClass_STRUCT );
    }

    {
    // test: operator >>=( any, value )
    Test3 a3;
    makeAny( (sal_Int8)2) >>= a3.nInt8;
    OSL_ASSERT( (makeAny( (sal_Int8)2) >>= a3.nInt8) && a3.nInt8 == 2 );
    OSL_ASSERT( (makeAny( (float)2) >>= a3.nFloat) && a3.nFloat ==(float)2 );
    OSL_ASSERT( (makeAny( (double)2) >>= a3.nDouble) && a3.nDouble == 2 );
    OSL_ASSERT( (makeAny( (sal_Int16)2) >>= a3.nInt16) && a3.nInt16 == 2 );
    OSL_ASSERT( (makeAny( OUString( RTL_CONSTASCII_USTRINGPARAM("2") )) >>= a3.aString) &&
                a3.aString == OUString::createFromAscii("2") );
    OSL_ASSERT( (makeAny( (sal_uInt16)2) >>= a3.nuInt16) && a3.nuInt16 == 2 );
    sal_Int64 aInt64 = 0x200000000;
    OSL_ASSERT( makeAny( aInt64 ) >>= a3.nInt64 );
    OSL_ASSERT( a3.nInt64 == 0x200000000 );
    OSL_ASSERT( (makeAny( (sal_Int32)2) >>= a3.nInt32) && a3.nInt32 == 2 );
    sal_uInt64 auInt64 = 0x200000000;
    OSL_ASSERT( makeAny( auInt64 ) >>= a3.nuInt64 );
    OSL_ASSERT( a3.nuInt64 == 0x200000000 );
    OSL_ASSERT( (makeAny( (sal_uInt32)2) >>= a3.nuInt32) && a3.nuInt32 == 2 );
    OSL_ASSERT( (makeAny( TypeClass_STRUCT) >>= a3.eType) && a3.eType == TypeClass_STRUCT );
    //OSL_ASSERT( (makeAny( L'2' ) >>= a3.wChar) && a3.nInt8 ==L'2';
    OSL_ASSERT( (makeAny( (sal_Int16)2) >>= a3.td.nInt16) && a3.nInt16 == 2 );
    OSL_ASSERT( (makeAny( (double)2) >>= a3.td.dDouble) && a3.nDouble == 2 );
    //OSL_ASSERT( (makeAny( (sal_True)2) >>= a3.bBool) && a3.nInt8 ==sal_True;

    // Only one negative test, the implementation has only one if to test this
    OSL_ASSERT( (makeAny( (float)2) >>= a3.nFloat) && a3.nFloat ==(float)2 );
    }

    {
    // test: Sequence
    Sequence< Test1 > aTestSeq;
    OSL_ASSERT( aTestSeq.getLength() == 0 );
    sal_Int32 szInt32[2] = { 1, 2 };
    Sequence< sal_Int32 > aInt32Seq( szInt32, 2 );
    OSL_ASSERT( aInt32Seq.getLength() == 2 );
    OSL_ASSERT( aInt32Seq[0] == 1 && aInt32Seq[1] == 2 );
    OSL_ASSERT( aInt32Seq.getArray()[0] == 1 && aInt32Seq.getArray()[1] == 2 );
    Sequence< sal_Int32 > aNextInt32Seq( aInt32Seq );
    OSL_ASSERT( aNextInt32Seq[0] == 1 && aNextInt32Seq[1] == 2 );
    aInt32Seq[0] = 45;
    OSL_ASSERT( aInt32Seq[0] == 45 && aInt32Seq[1] == 2 );
    OSL_ASSERT( aNextInt32Seq[0] == 1 && aNextInt32Seq[1] == 2 );
    sal_Int32 * pArray = aNextInt32Seq.getArray();
    OSL_ASSERT( pArray[0] == 1 && pArray[1] == 2 );
    Sequence< double > aDoubleSeq( 5 );
    OSL_ASSERT( aDoubleSeq[4] == 0.0 );
    Sequence< OUString > aStringSeq( 5 );
    OSL_ASSERT( aStringSeq[4] == OUString() );
    }
    sal_Int32 szInt32[2] = { 1, 2 };
    Sequence<sal_Int32 > aInt32Seq( szInt32, 2 );
    Sequence<sal_Int32 > aNextInt32Seq( aInt32Seq );
    aNextInt32Seq.realloc( 1 ); // split of sequence
    const sal_Int32 * pArray = aNextInt32Seq.getConstArray();
    OSL_ASSERT( pArray[0] == 1 );
    aInt32Seq.realloc( 1 ); // reallocate mem
    pArray = aInt32Seq.getConstArray();
    OSL_ASSERT( pArray[0] == 1 );

    Sequence<sal_Int32 > aInt32Seq2( aInt32Seq );
    aInt32Seq.realloc( 0 );
    aInt32Seq.realloc( 1 );
    aInt32Seq.realloc( 0 );
}

class TestInterface : public XInterface
{
public:
    // XInterface
    void SAL_CALL   acquire() { osl_incrementInterlockedCount( &nRefCount ); }
    void SAL_CALL   release()
    {
        if( !osl_decrementInterlockedCount( &nRefCount ) )
            delete this;
    }
    Any SAL_CALL    queryInterface( const Type & rType )
    {
        return cppu::queryInterface( rType, static_cast< XInterface* >( this ) );
    }

    TestInterface() : nRefCount( 0 ) {}

    sal_Int32 nRefCount;
};

struct SimpleInterface : public TestInterface, public XSimpleInterface
{
    void SAL_CALL   acquire() { TestInterface::acquire(); }
    void SAL_CALL   release() { TestInterface::release(); }
    Any SAL_CALL    queryInterface( const Type & rType )
    {
        Any aRet( cppu::queryInterface( rType, static_cast< XSimpleInterface * >( this ) ) );
        return (aRet.hasValue() ? aRet : TestInterface::queryInterface( rType ));
    }
    virtual void SAL_CALL method() throw(::com::sun::star::uno::RuntimeException)
        {}
};

static sal_Bool s_aAssignableFromTab[11][11] =
{
                         /* from CH,BO,BY,SH,US,LO,UL,HY,UH,FL,DO */
/* TypeClass_CHAR */            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_BOOLEAN */         { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_BYTE */            { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_SHORT */           { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
/* TypeClass_UNSIGNED_SHORT */  { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
/* TypeClass_LONG */            { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
/* TypeClass_UNSIGNED_LONG */   { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
/* TypeClass_HYPER */           { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
/* TypeClass_UNSIGNED_HYPER */  { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
/* TypeClass_FLOAT */           { 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0 },
/* TypeClass_DOUBLE */          { 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1 }
};
template < class T >
static void test_assignSimple( const T & rVal, /*const*/ Any /*&*/ rAny )
{
    typelib_TypeDescription * pTD = 0;
    ::getCppuType( &rVal ).getDescription( &pTD );
    sal_Bool bTable = s_aAssignableFromTab[pTD->eTypeClass-1][rAny.getValueTypeClass()-1];
    OSL_ASSERT(
        (bTable!=sal_False) ==
        (typelib_typedescriptionreference_isAssignableFrom(
            pTD->pWeakRef, rAny.getValueTypeRef() )!=sal_False) );
    T t;
    OSL_ASSERT( (bTable!=sal_False) == ((rAny >>= t)!=sal_False) );
    if (bTable)
        OSL_ASSERT( t == rVal );
    typelib_typedescription_release( pTD );
}
template < class T >
static void test_simple_assignment( const T & rVal )
{
    // bool
    sal_Bool tr = sal_True;
    typelib_TypeDescription * pBoolTD = 0;
    ::getCppuBooleanType().getDescription( &pBoolTD );
    Any a( &tr, pBoolTD );
    test_assignSimple( rVal, a );
    OSL_ASSERT( typelib_typedescriptionreference_isAssignableFrom( pBoolTD->pWeakRef, a.getValueTypeRef() ) );
    typelib_typedescription_release( pBoolTD );
    OSL_ASSERT( *(sal_Bool *)a.getValue() );
    // char
    sal_Unicode ch = 'a';
    typelib_TypeDescription * pCharTD = 0;
    ::getCppuCharType().getDescription( &pCharTD );
    a.setValue( &ch, pCharTD );
    test_assignSimple( rVal, a );
    OSL_ASSERT( typelib_typedescriptionreference_isAssignableFrom( pCharTD->pWeakRef, a.getValueTypeRef() ) );
    typelib_typedescription_release( pCharTD );
    OSL_ASSERT( *(sal_Unicode *)a.getValue() == 'a' );

    // rest by template
    a <<= (sal_Int8)5;
    test_assignSimple( rVal, a );
    a <<= (sal_Int16)5;
    test_assignSimple( rVal, a );
    a <<= (sal_uInt16)5;
    test_assignSimple( rVal, a );
    a <<= (sal_Int32)5;
    test_assignSimple( rVal, a );
    a <<= (sal_uInt32)5;
    test_assignSimple( rVal, a );
    a <<= (sal_Int64)5;
    test_assignSimple( rVal, a );
    a <<= (sal_uInt64)5;
    test_assignSimple( rVal, a );
    a <<= (float)5;
    test_assignSimple( rVal, a );
    a <<= (double)5;
    test_assignSimple( rVal, a );
}
static void testAssignment()
{
    // simple types
    test_simple_assignment( (sal_Int8)5 );
    test_simple_assignment( (sal_Int16)5 );
    test_simple_assignment( (sal_uInt16)5 );
    test_simple_assignment( (sal_Int32)5 );
    test_simple_assignment( (sal_uInt32)5 );
    test_simple_assignment( (sal_Int64)5 );
    test_simple_assignment( (sal_uInt64)5 );
    test_simple_assignment( (float)5 );
    test_simple_assignment( (double)5 );
    // some complex things
    Any a;
    TestSimple ts;
    TestElement te; // derived from simple
    a <<= ts;
    OSL_ASSERT( !(a >>= te) );
    OSL_ASSERT( a >>= ts );
    a <<= te;
    OSL_ASSERT( (a >>= te) && (a >>= ts) );
    // interface
    Reference< XSimpleInterface > xOriginal( new SimpleInterface() );
    a <<= xOriginal;
    Reference< XInterface > x;
    OSL_ASSERT( (a >>= x) && (a == xOriginal) && (xOriginal == x) && (x == xOriginal) );
    // sequence
    Sequence< TestElement > aSeq( 5 );
    Sequence< TestElement > aSeq2( 3 );
    aSeq[1].Byte = 17;
    a <<= aSeq;
    OSL_ASSERT( a >>= aSeq2 );
    OSL_ASSERT( aSeq2[1].Byte == 17 );
    aSeq2[1].Byte = 20;
    OSL_ASSERT( aSeq != aSeq2 );
    OSL_ASSERT( a != aSeq2 );
    a <<= aSeq2;
    OSL_ASSERT( a >>= aSeq );
    OSL_ASSERT( a == aSeq );
    OSL_ASSERT( !(a != aSeq) );
    OSL_ASSERT( aSeq == aSeq2 );
    OSL_ASSERT( aSeq[1].Byte == 20 );

    // equals...
    sal_uInt64 n = (sal_uInt64)(sal_Int64)-5;
    a.setValue( &n, getCppuType( (sal_uInt64 *)0 ) );
    Any b;
    sal_Int8 n2 = -5;
    b.setValue( &n2, getCppuType( (sal_Int8 *)0 ) );
    OSL_ASSERT( a != b );
}

void test_interface()
{
    {
    // test: Interface
    Reference< XInterface > xIFace;
    OSL_ASSERT( !xIFace.is() );
    xIFace.clear(); // do nothing
    }
}

void test_inheritance()
{
    OSL_ASSERT( sizeof( Base ) == getSize( getCppuType( (Base *)0).getTypeLibType() ) );
    OSL_ASSERT( sizeof( Base1 ) == getSize( getCppuType( (Base1 *)0).getTypeLibType() ) );
    OSL_ASSERT( sizeof( Base2 ) == getSize( getCppuType( (Base2 *)0).getTypeLibType() ) );
}

sal_Int32 nCallback_1;
sal_Int32 nCallback;
void SAL_CALL typedescription_Callback_1
(
    void * pContext,
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName
)
{
    OSL_ENSURE( pContext == (void *)1, "### unexpected context!" );
    if( *ppRet )
    {
        typelib_typedescription_release( *ppRet );
        *ppRet = 0;
    }

    OUString aTypeName( pTypeName );
    if( -1 != aTypeName.indexOf( OUString::createFromAscii("1_") ) )
    {
        nCallback_1++;
        OUString aName( RTL_CONSTASCII_USTRINGPARAM("unsigned short") );
        OUString empty;
        typelib_CompoundMember_Init aMember = { typelib_TypeClass_UNSIGNED_SHORT,
                                                aName.pData,
                                                empty.pData };
        typelib_typedescription_new(
            ppRet,
            typelib_TypeClass_STRUCT, pTypeName, 0,
            1,
            &aMember
        );
    }
}

void SAL_CALL typedescription_Callback
(
    void * pContext,
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName
)
{
    OSL_ENSURE( pContext == (void *)0, "### unexpected context!" );
    if( *ppRet )
    {
        typelib_typedescription_release( *ppRet );
        *ppRet = 0;
    }

    OUString aTypeName( pTypeName );
    if( -1 != aTypeName.indexOf( OUString::createFromAscii("cachetest") ) )
    {
        nCallback++;
        aTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM("1_") ) + aTypeName;
        OUString empty;
        typelib_CompoundMember_Init aMember = { typelib_TypeClass_STRUCT,
                                                aTypeName.pData,
                                                empty.pData };
        typelib_typedescription_new(
            ppRet,
            typelib_TypeClass_STRUCT, pTypeName, 0,
            1,
            &aMember
        );
    }
}

void test_cache()
{
    typelib_typedescription_registerCallback( (void *)1, typedescription_Callback_1 );
    typelib_typedescription_registerCallback( 0, typedescription_Callback );

    for( sal_Int32 i = 0; i < 300; i++ )
    {
        typelib_TypeDescription * pTD = 0;
        OUString aTypeName( RTL_CONSTASCII_USTRINGPARAM("cachetest") );
        aTypeName = aTypeName + OUString::valueOf( i );
        typelib_typedescription_getByName( &pTD, aTypeName.pData );
        typelib_typedescription_release( pTD );
    }
    OSL_ASSERT( nCallback_1 == 300 );
    OSL_ASSERT( nCallback == 300 );
    // The cache size is 200 so the description "cachetest200" is in the cache
    typelib_TypeDescription * pTD = 0;
    OUString aName200( RTL_CONSTASCII_USTRINGPARAM("cachetest200") );
    typelib_typedescription_getByName( &pTD, aName200.pData );
    OSL_ASSERT( nCallback_1 == 300 );
    OSL_ASSERT( nCallback == 300 );
    // The cache size is 200 so the description "cachetest199" is not in the cache
    // "1_cachetest199" is loaded too.
    OUString aName199( RTL_CONSTASCII_USTRINGPARAM("cachetest199") );
    typelib_typedescription_getByName( &pTD, aName199.pData );
    typelib_typedescription_release( pTD );
    OSL_ASSERT( nCallback_1 == 301 );
    OSL_ASSERT( nCallback == 301 );

    typelib_typedescription_revokeCallback( (void *)1, typedescription_Callback_1 );
    typelib_typedescription_revokeCallback( 0, typedescription_Callback );
}

static OUString s_aAddPurpose;

static void SAL_CALL getMappingCallback(
    uno_Mapping ** ppMapping,
    uno_Environment * pFrom, uno_Environment * pTo, rtl_uString * pAddPurpose )
{
    s_aAddPurpose = pAddPurpose;
}
static void testMappingCallback()
{
    uno_registerMappingCallback( getMappingCallback );
    OSL_ASSERT( ! s_aAddPurpose.getLength() );
    Mapping aTest(
        OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("test") ) );
    uno_revokeMappingCallback( getMappingCallback );
    OSL_ASSERT( s_aAddPurpose.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("test") ) );
    s_aAddPurpose = OUString();
    Mapping aTest2(
        OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ),
        OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("test") ) );
    OSL_ASSERT( ! s_aAddPurpose.getLength() );
}

static void testEnvironment(void)
{
    uno_Environment * pEnv = 0;
    void ** ppInterfaces = 0;
    sal_Int32 nLen;

    OUString aUnoEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );
    ::uno_getEnvironment( &pEnv, aUnoEnvTypeName.pData, 0 );
    ::uno_dumpEnvironment( stderr, pEnv, 0 );
    (*pEnv->pExtEnv->getRegisteredInterfaces)( pEnv->pExtEnv, &ppInterfaces, &nLen, rtl_allocateMemory );
    while (nLen--)
    {
        uno_Interface * pUnoI = (uno_Interface *)ppInterfaces[nLen];
        (*pUnoI->release)( pUnoI );
    }
    ::rtl_freeMemory( ppInterfaces );

    OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    ::uno_getEnvironment( &pEnv, aCppEnvTypeName.pData, 0 );
    ::uno_dumpEnvironment( stderr, pEnv, 0 );
    (*pEnv->pExtEnv->getRegisteredInterfaces)( pEnv->pExtEnv, &ppInterfaces, &nLen, rtl_allocateMemory );
    while (nLen--)
    {
        XInterface * p = (XInterface *)ppInterfaces[nLen];
        p->release();
    }
    ::rtl_freeMemory( ppInterfaces );
    (*pEnv->release)( pEnv );
}

/*
 * main.
 */
int SAL_CALL main(int argc, char **argv)
{
    typelib_setCacheSize( 200 );
#ifdef SAL_W32
    Reference< XMultiServiceFactory > xMgr( cppu::createRegistryServiceFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("testcppu.rdb") ) ) );
#endif
    testEnvironment();
    testMappingCallback();

    // security test
//  void test_security( const Reference< XMultiServiceFactory > & );
//      test_security( xMgr );
    // perform test
    void test_di(void);
    test_di();
    testAssignment();
    testCppu();
//      test_cache(); // cache test not possible if types are loaded dynamically...
    test_interface();
    test_inheritance();

      // shutdown
#ifdef SAL_W32
    Reference< XComponent > xComp( xMgr, UNO_QUERY );
    OSL_ENSURE( xComp.is(), "### serivce manager has to implement XComponent!" );
    xComp->dispose();
    xMgr.clear();
#endif
    typelib_setCacheSize( 0 );
    testEnvironment();

    return 0;
}
