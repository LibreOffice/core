/*************************************************************************
 *
 *  $RCSfile: rtl_OStringBuffer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lwang $ $Date: 2003-01-03 09:39:38 $
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

// -----------------------------------------------------------------------------

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _RTL_STRING_CONST_H_
#include <rtl_String_Const.h>
#endif

#ifndef _RTL_STRING_UTILS_HXX_
#include <rtl_String_Utils.hxx>
#endif

#include <rtl/strbuf.hxx>

#include <cppunit/simpleheader.hxx>

using namespace rtl;

//------------------------------------------------------------------------
// test classes
//------------------------------------------------------------------------
const MAXBUFLENGTH = 255;
//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// testing constructors
//------------------------------------------------------------------------

namespace rtl_OStringBuffer
{
    class  ctors : public CppUnit::TestFixture
    {
    public:

        void ctor_001()
        {
            ::rtl::OStringBuffer aStrBuf;

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer containing no characters",
                aStrBuf.getLength() == 0 &&
                ! *(aStrBuf.getStr()) && aStrBuf.getCapacity() == 16
            );
        }

        void ctor_002()
        {
            ::rtl::OString       aStrtmp( kTestStr1 );
            ::rtl::OStringBuffer aStrBuftmp( aStrtmp );
            ::rtl::OStringBuffer aStrBuf( aStrBuftmp );
            sal_Bool res = cmpstr(aStrBuftmp.getStr(),aStrBuf.getStr());

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer from another OStringBuffer",
                aStrBuf.getLength()==aStrBuftmp.getLength() &&
                aStrBuf.getCapacity() == aStrBuftmp.getCapacity() && res
            );

        }

        void ctor_003()
        {
            ::rtl::OStringBuffer aStrBuf1(kTestStr2Len);
    #ifdef WITH_CORE
            ::rtl::OStringBuffer aStrBuf2(kSInt32Max);     //will core dump
    #else
            ::rtl::OStringBuffer aStrBuf2(0);
    #endif
            ::rtl::OStringBuffer aStrBuf3(kNonSInt32Max);

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                aStrBuf1.getLength() == 0 &&
                ! *(aStrBuf1.getStr()) && aStrBuf1.getCapacity() == kTestStr2Len &&
    #ifdef WITH_CORE
                aStrBuf2.getLength() == 0 &&
                ! *(aStrBuf2.getStr()) && aStrBuf2.getCapacity() == kSInt32Max &&
    #else
                aStrBuf2.getLength() == 0 &&
                ! *(aStrBuf2.getStr()) && aStrBuf2.getCapacity() == 16 &&
    #endif
                aStrBuf3.getLength() == 0 &&
                ! *(aStrBuf3.getStr()) && aStrBuf3.getCapacity() == kNonSInt32Max

            );

        }

        void ctor_004()
        {
            ::rtl::OString aStrtmp( kTestStr1 );
            ::rtl::OStringBuffer aStrBuf( aStrtmp );
            sal_Int32 leg = aStrBuf.getLength();

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer from Ostring",
                aStrBuf.getStr() == aStrtmp &&
                leg == aStrtmp.pData->length &&
                aStrBuf.getCapacity() == leg+16

            );
        }

        CPPUNIT_TEST_SUITE(ctors);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST(ctor_003);
        CPPUNIT_TEST(ctor_004);
        CPPUNIT_TEST_SUITE_END();
    };



// -----------------------------------------------------------------------------

    class  makeStringAndClear : public CppUnit::TestFixture
    {
        OString* arrOUS[6];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( kTestStr14 );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( kTestStr27 );
            arrOUS[4] = new OString( kTestStr29 );
            arrOUS[5] = new OString( "\0" );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4]; delete arrOUS[5];
        }

        void makeStringAndClear_001()
        {
            ::rtl::OStringBuffer   aStrBuf1;
            ::rtl::OString         aStr1;

            sal_Bool lastRes = (aStrBuf1.makeStringAndClear() ==  aStr1 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "two empty strings(def. constructor)",
                lastRes && ( aStrBuf1.getCapacity() == 0 ) &&
                        ( *(aStrBuf1.getStr()) == '\0' )
            );

        }

        void makeStringAndClear_002()
        {
            ::rtl::OStringBuffer   aStrBuf2(26);
            ::rtl::OString         aStr2;

            sal_Bool lastRes = (aStrBuf2.makeStringAndClear() == aStr2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "two empty strings(with a argu)",
                lastRes && ( aStrBuf2.getCapacity() == 0 ) &&
                        ( *(aStrBuf2.getStr()) == '\0' )
            );

        }

        void makeStringAndClear_003()
        {
            ::rtl::OStringBuffer   aStrBuf3(*arrOUS[0]);
            ::rtl::OString        aStr3(*arrOUS[0]);

            sal_Bool lastRes = (aStrBuf3.makeStringAndClear() == aStr3 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "normal string",
                lastRes && ( aStrBuf3.getCapacity() == 0 ) &&
                        ( *(aStrBuf3.getStr()) == '\0' )
            );

        }

        void makeStringAndClear_004()
        {
            ::rtl::OStringBuffer   aStrBuf4(*arrOUS[1]);
            ::rtl::OString         aStr4(*arrOUS[1]);

            sal_Bool lastRes = (aStrBuf4.makeStringAndClear() ==  aStr4 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "string with space ",
                lastRes && ( aStrBuf4.getCapacity() == 0 ) &&
                        ( *(aStrBuf4.getStr()) == '\0' )
            );
        }

        void makeStringAndClear_005()
        {
            ::rtl::OStringBuffer   aStrBuf5(*arrOUS[2]);
            ::rtl::OString         aStr5(*arrOUS[2]);

            sal_Bool lastRes = (aStrBuf5.makeStringAndClear() ==  aStr5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "empty string",
                lastRes && ( aStrBuf5.getCapacity() == 0 ) &&
                        ( *(aStrBuf5.getStr()) == '\0' )
            );
        }

        void makeStringAndClear_006()
        {
            ::rtl::OStringBuffer   aStrBuf6(*arrOUS[3]);
            ::rtl::OString         aStr6(*arrOUS[3]);

            sal_Bool lastRes = (aStrBuf6.makeStringAndClear() == aStr6 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "string with a character",
                lastRes && ( aStrBuf6.getCapacity() == 0 ) &&
                        ( *(aStrBuf6.getStr()) == '\0' )
            );
        }

        void makeStringAndClear_007()
        {
            ::rtl::OStringBuffer   aStrBuf7(*arrOUS[4]);
            ::rtl::OString         aStr7(*arrOUS[4]);

            sal_Bool lastRes = (aStrBuf7.makeStringAndClear() == aStr7 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "string with special characters",
                lastRes && ( aStrBuf7.getCapacity() == 0 ) &&
                        ( *(aStrBuf7.getStr()) == '\0' )
            );
        }

        void makeStringAndClear_008()
        {
            ::rtl::OStringBuffer   aStrBuf8(*arrOUS[5]);
            ::rtl::OString         aStr8(*arrOUS[5]);

            sal_Bool lastRes = (aStrBuf8.makeStringAndClear() == aStr8 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "string only with (\0)",
                lastRes && ( aStrBuf8.getCapacity() == 0 ) &&
                        ( *(aStrBuf8.getStr()) == '\0' )
            );
        }

        CPPUNIT_TEST_SUITE(makeStringAndClear);
        CPPUNIT_TEST(makeStringAndClear_001);
        CPPUNIT_TEST(makeStringAndClear_002);
        CPPUNIT_TEST(makeStringAndClear_003);
        CPPUNIT_TEST(makeStringAndClear_004);
        CPPUNIT_TEST(makeStringAndClear_005);
        CPPUNIT_TEST(makeStringAndClear_006);
        CPPUNIT_TEST(makeStringAndClear_007);
        CPPUNIT_TEST(makeStringAndClear_008);
        CPPUNIT_TEST_SUITE_END();
    };

// -----------------------------------------------------------------------------

    class  getLength : public CppUnit::TestFixture
    {
        OString* arrOUS[6];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( "1" );
            arrOUS[2] = new OString( );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( "\0" );
            arrOUS[5] = new OString( kTestStr2 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4]; delete arrOUS[5];
        }

        void getLength_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              expVal = kTestStr1Len;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length of ascii string",
                aStrBuf.getLength() == expVal
            );

        }

        void getLength_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Int32              expVal = 1;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length of ascci string of size 1",
                aStrBuf.getLength() == expVal
            );
        }

        void getLength_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length of empty string",
                aStrBuf.getLength() == expVal
            );
        }

        void getLength_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length of empty string (empty ascii string arg)",
                aStrBuf.getLength() == expVal
            );
        }

        void getLength_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length of empty string (string arg = '\\0')",
                aStrBuf.getLength() == expVal
            );
        }

        void getLength_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
            sal_Int32              expVal = kTestStr2Len;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length(>16) of ascii string",
                aStrBuf.getLength() == expVal
            );
        }

        void getLength_007()
        {
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length of empty string (default constructor)",
                aStrBuf.getLength()== expVal
            );
        }

        void getLength_008()
        {
            ::rtl::OStringBuffer   aStrBuf( 26 );
            sal_Int32               expVal   = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length of empty string (with capacity)",
                aStrBuf.getLength()== expVal
            );
        }

        CPPUNIT_TEST_SUITE( getLength );
        CPPUNIT_TEST( getLength_001 );
        CPPUNIT_TEST( getLength_002 );
        CPPUNIT_TEST( getLength_003 );
        CPPUNIT_TEST( getLength_004 );
        CPPUNIT_TEST( getLength_005 );
        CPPUNIT_TEST( getLength_006 );
        CPPUNIT_TEST( getLength_007 );
        CPPUNIT_TEST( getLength_008 );
        CPPUNIT_TEST_SUITE_END();
    };

// -----------------------------------------------------------------------------

    class  getCapacity : public CppUnit::TestFixture
    {
        OString* arrOUS[6];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( "1" );
            arrOUS[2] = new OString( );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( "\0" );
            arrOUS[5] = new OString( kTestStr2 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4]; delete arrOUS[5];
        }

        void getCapacity_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              expVal = kTestStr1Len+16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of ascii string",
                aStrBuf.getCapacity()== expVal
            );

        }

        void getCapacity_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Int32              expVal = 1+16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of ascci string of size 1",
                aStrBuf.getCapacity() == expVal
            );
        }

        void getCapacity_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              expVal = 0+16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string",
                aStrBuf.getCapacity() == expVal
            );
        }

        void getCapacity_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              expVal = 0+16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (empty ascii string arg)",
                aStrBuf.getCapacity()== expVal
            );
        }

        void getCapacity_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Int32              expVal = 0+16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (string arg = '\\0')",
                aStrBuf.getCapacity() == expVal
            );
        }

        void getCapacity_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
            sal_Int32              expVal = kTestStr2Len+16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity(>16) of ascii string",
                aStrBuf.getCapacity() == expVal
            );
        }

        void getCapacity_007()
        {
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              expVal = 16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (default constructor)",
                aStrBuf.getCapacity() == expVal
            );
        }
#ifdef WITH_CORE
        void getCapacity_008()
        {
            ::rtl::OStringBuffer   aStrBuf ( kSInt32Max );
            sal_Int32              expVal = kSInt32Max;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (with capacity 2147483647)(code will core dump)",
                aStrBuf.getCapacity() == expVal
            );
        }
#endif
        void getCapacity_009()
        {
            ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
            sal_Int32              expVal = kNonSInt32Max;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (with capacity -2147483648)",
                aStrBuf.getCapacity() == expVal
            );
        }

        void getCapacity_010()
        {
            ::rtl::OStringBuffer   aStrBuf( 16 );
            sal_Int32              expVal = 16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (with capacity 16)",
                aStrBuf.getCapacity() == expVal
            );
        }

        void getCapacity_011()
        {
            ::rtl::OStringBuffer   aStrBuf( 6 );
            sal_Int32              expVal = 6;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (with capacity 6)",
                aStrBuf.getCapacity() == expVal
            );
        }

        void getCapacity_012()
        {
            ::rtl::OStringBuffer   aStrBuf( 0 );
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (with capacity 0)",
                aStrBuf.getCapacity() == expVal
            );
        }

        void getCapacity_013()
        {
            ::rtl::OStringBuffer   aStrBuf( -2 );
            sal_Int32              expVal = -2;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of empty string (with capacity -2)",
                aStrBuf.getCapacity() == expVal
            );
        }

        CPPUNIT_TEST_SUITE( getCapacity );
        CPPUNIT_TEST( getCapacity_001 );
        CPPUNIT_TEST( getCapacity_002 );
        CPPUNIT_TEST( getCapacity_003 );
        CPPUNIT_TEST( getCapacity_004 );
        CPPUNIT_TEST( getCapacity_005 );
        CPPUNIT_TEST( getCapacity_006 );
        CPPUNIT_TEST( getCapacity_007 );
#ifdef WITH_CORE
        CPPUNIT_TEST( getCapacity_008 );
#endif
        CPPUNIT_TEST( getCapacity_009 );
        CPPUNIT_TEST( getCapacity_010 );
        CPPUNIT_TEST( getCapacity_011 );
        CPPUNIT_TEST( getCapacity_012 );
        CPPUNIT_TEST( getCapacity_013 );
        CPPUNIT_TEST_SUITE_END();
    };
// -----------------------------------------------------------------------------

    class  ensureCapacity : public CppUnit::TestFixture
    {
        void ensureCapacity_001()
        {
            sal_Int32          expVal = 16;
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              input = 5;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 16, minimum is 5",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_002()
        {
            sal_Int32          expVal = 16;
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              input = -5;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 16, minimum is -5",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_003()
        {
            sal_Int32          expVal = 16;
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              input = 0;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 16, minimum is 0",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_004()           //the testcase is based on comments
        {
            sal_Int32          expVal = 34;
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              input = 20;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 16, minimum is 20",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_005()
        {
            sal_Int32          expVal = 50;
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              input = 50;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 16, minimum is 50",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_006()
        {
            sal_Int32          expVal = 20;
            ::rtl::OStringBuffer   aStrBuf( 6 );
            sal_Int32              input = 20;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 6, minimum is 20",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_007()
        {
            sal_Int32          expVal = 6;
            ::rtl::OStringBuffer   aStrBuf( 6 );
            sal_Int32              input = 2;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 6, minimum is 2",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_008()
        {
            sal_Int32          expVal = 6;
            ::rtl::OStringBuffer   aStrBuf( 6 );
            sal_Int32              input = -6;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 6, minimum is -6",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_009()      //the testcase is based on comments
        {
            sal_Int32          expVal = 14;
            ::rtl::OStringBuffer   aStrBuf( 6 );
            sal_Int32              input = 10;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 6, minimum is -6",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_010()
        {
            sal_Int32          expVal = 6;
            ::rtl::OStringBuffer   aStrBuf( 0 );
            sal_Int32              input = 6;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 0, minimum is 6",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_011()       //the testcase is based on comments
        {
            sal_Int32          expVal = 1;
            ::rtl::OStringBuffer   aStrBuf( 0 );
            sal_Int32              input = 1;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 0, minimum is 1",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_012()
        {
            sal_Int32          expVal = 0;
            ::rtl::OStringBuffer   aStrBuf( 0 );
            sal_Int32              input = -1;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 0, minimum is -1",
                aStrBuf.getCapacity() == expVal
            );

        }
#ifdef WITH_CORE
        void ensureCapacity_013()             //will core dump
        {
            sal_Int32          expVal = kSInt32Max;
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Int32              input = 65535;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 2147483647, minimum is 65535",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_014()             //will core dump
        {
            sal_Int32          expVal = kSInt32Max;
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Int32              input = kSInt32Max;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 2147483647, minimum is 2147483647",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_015()             //will core dump
        {
            sal_Int32          expVal = kSInt32Max;
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Int32              input = -1;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 2147483647, minimum is -1",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_016()             //will core dump
        {
            sal_Int32          expVal = kSInt32Max;
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Int32              input = 0;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 2147483647, minimum is 0",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_017()             //will core dump
        {
            sal_Int32          expVal = kSInt32Max;
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Int32              input = kNonSInt32Max;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to 2147483647, minimum is -2147483648",
                aStrBuf.getCapacity() == expVal
            );

        }
#endif
        void ensureCapacity_018()
        {
            sal_Int32          expVal = 65535;
            ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
            sal_Int32              input = 65535;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to -2147483648, minimum is 65535",
                aStrBuf.getCapacity() == expVal
            );

        }
#ifdef WITH_CORE
        void ensureCapacity_019()               //will core dump
        {
            sal_Int32          expVal = 2147483647;
            ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
            sal_Int32              input = 2147483647;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to -2147483648, minimum is 2147483647",
                aStrBuf.getCapacity() == expVal
            );

        }
#endif
        void ensureCapacity_020()
        {
            sal_Int32          expVal = 2;
            ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
            sal_Int32              input = -1;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to -2147483648, minimum is -1",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_021()
        {
            sal_Int32          expVal = 2;
            ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
            sal_Int32              input = 0;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to -2147483648, minimum is 0",
                aStrBuf.getCapacity() == expVal
            );

        }

        void ensureCapacity_022()
        {
            sal_Int32          expVal = kNonSInt32Max;
            ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
            sal_Int32              input = kNonSInt32Max;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity equal to -2147483648, minimum is -2147483648",
                aStrBuf.getCapacity() == expVal
            );

        }

        CPPUNIT_TEST_SUITE( ensureCapacity );
        CPPUNIT_TEST( ensureCapacity_001 );
        CPPUNIT_TEST( ensureCapacity_002 );
        CPPUNIT_TEST( ensureCapacity_003 );
        CPPUNIT_TEST( ensureCapacity_004 );
        CPPUNIT_TEST( ensureCapacity_005 );
        CPPUNIT_TEST( ensureCapacity_006 );
        CPPUNIT_TEST( ensureCapacity_007 );
        CPPUNIT_TEST( ensureCapacity_008 );
        CPPUNIT_TEST( ensureCapacity_009 );
        CPPUNIT_TEST( ensureCapacity_010 );
        CPPUNIT_TEST( ensureCapacity_011 );
        CPPUNIT_TEST( ensureCapacity_012 );
#ifdef WITH_CORE
        CPPUNIT_TEST( ensureCapacity_013 );
        CPPUNIT_TEST( ensureCapacity_014 );
        CPPUNIT_TEST( ensureCapacity_015 );
        CPPUNIT_TEST( ensureCapacity_016 );
        CPPUNIT_TEST( ensureCapacity_017 );
#endif
        CPPUNIT_TEST( ensureCapacity_018 );
#ifdef WITH_CORE
        CPPUNIT_TEST( ensureCapacity_019 );
#endif
        CPPUNIT_TEST( ensureCapacity_020 );
        CPPUNIT_TEST( ensureCapacity_021 );
        CPPUNIT_TEST( ensureCapacity_022 );
        CPPUNIT_TEST_SUITE_END();
    };

// -----------------------------------------------------------------------------

    class  setLength : public CppUnit::TestFixture
    {
        OString* arrOUS[6];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( "1" );
            arrOUS[2] = new OString( );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( "\0" );
            arrOUS[5] = new OString( kTestStr2 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4]; delete arrOUS[5];
        }

        void setLength_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              expVal1 = 50;
            ::rtl::OString         expVal2( kTestStr1 );
            sal_Int32              expVal3 = 50;
            sal_Int32              input   = 50;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(kTestStr1)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              expVal1 = kTestStr13Len;
            ::rtl::OString         expVal2( kTestStr1 );
            sal_Int32              expVal3 = 32;
            sal_Int32              input   = kTestStr13Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the length of OStringBuffer(kTestStr1)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              expVal1 = kTestStr1Len;
            ::rtl::OString         expVal2( kTestStr1 );
            sal_Int32              expVal3 = 32;
            sal_Int32              input   = kTestStr1Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength equal to the length of OStringBuffer(kTestStr1)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              expVal1 = kTestStr7Len;
            ::rtl::OString         expVal2( kTestStr7 );
            sal_Int32              expVal3 = 32;
            sal_Int32              input   = kTestStr7Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength less than the length of OStringBuffer(kTestStr1)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              expVal1 = 0;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 32;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength equal to 0",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Int32              expVal1 = 25;
            ::rtl::OString         expVal2( *arrOUS[1] );
            sal_Int32              expVal3 = 25;
            sal_Int32              input   = 25;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(1)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Int32              expVal1 = kTestStr27Len;
            ::rtl::OString         expVal2( *arrOUS[1] );
            sal_Int32              expVal3 = 17;
            sal_Int32              input   = kTestStr27Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength equal to the length of OStringBuffer(1)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Int32              expVal1 = 0;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 17;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength less than the length of OUStringBuffer(1)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              expVal1 = 20;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 20;
            sal_Int32              input   = 20;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer()",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              expVal1 = 3;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 3;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the length of OStringBuffer()",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              expVal1 = 0;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the length of OStringBuffer()",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              expVal1 = 20;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 20;
            sal_Int32              input   = 20;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer("")",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              expVal1 = 5;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 5;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the length of OStringBuffer("")",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              expVal1 = 0;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength less than the length of OStringBuffer("")",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Int32              expVal1 = 20;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 20;
            sal_Int32              input   = 20;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the length of OStringBuffer(\0)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Int32              expVal1 = 5;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 5;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the length of OStringBuffer(\0)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Int32              expVal1 = 0;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength less than the length of OStringBuffer(\0)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
            sal_Int32              expVal1 = 50;
            ::rtl::OString         expVal2( kTestStr2 );
            sal_Int32              expVal3 = 66;
            sal_Int32              input   = 50;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(kTestStr2)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
            sal_Int32              expVal1 = 40;
            ::rtl::OString         expVal2(kTestStr2);
            sal_Int32              expVal3 = 48;
            sal_Int32              input   = 40;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength more than the length of OStringBuffer(kTestStr2)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
            sal_Int32              expVal1 = kTestStr2Len;
            ::rtl::OString         expVal2(kTestStr2);
            sal_Int32              expVal3 = 48;
            sal_Int32              input   = kTestStr2Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength equal to the length of OUStringBuffer(kTestStr2)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
            sal_Int32              expVal1 = kTestStr7Len;
            ::rtl::OString         expVal2(kTestStr7);
            sal_Int32              expVal3 = 48;
            sal_Int32              input   = kTestStr7Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength less than the length of OUStringBuffer(TestStr2)",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }

        void setLength_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
            sal_Int32              expVal1 = 0;
            ::rtl::OString         expVal2;
            sal_Int32              expVal3 = 48;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "newLength equal to 0",
                aStrBuf.getStr() == expVal2 &&
                    aStrBuf.getLength() == expVal1 &&
                    aStrBuf.getCapacity() == expVal3
            );

        }



        CPPUNIT_TEST_SUITE( setLength );
        CPPUNIT_TEST( setLength_001 );
        CPPUNIT_TEST( setLength_002 );
        CPPUNIT_TEST( setLength_003 );
        CPPUNIT_TEST( setLength_004 );
        CPPUNIT_TEST( setLength_005 );
        CPPUNIT_TEST( setLength_006 );
        CPPUNIT_TEST( setLength_007 );
        CPPUNIT_TEST( setLength_008 );
        CPPUNIT_TEST( setLength_009 );
        CPPUNIT_TEST( setLength_010 );
        CPPUNIT_TEST( setLength_011 );
        CPPUNIT_TEST( setLength_012 );
        CPPUNIT_TEST( setLength_013 );
        CPPUNIT_TEST( setLength_014 );
        CPPUNIT_TEST( setLength_015 );
        CPPUNIT_TEST( setLength_016 );
        CPPUNIT_TEST( setLength_017 );
        CPPUNIT_TEST( setLength_018 );
        CPPUNIT_TEST( setLength_019 );
        CPPUNIT_TEST( setLength_020 );
        CPPUNIT_TEST( setLength_021 );
        CPPUNIT_TEST( setLength_022 );
        CPPUNIT_TEST_SUITE_END();
    };

// -----------------------------------------------------------------------------

    class  charAt : public CppUnit::TestFixture
    {
        OString* arrOUS[4];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( kTestStr27 );
            arrOUS[2] = new OString( kTestStr28 );
            arrOUS[3] = new OString(  );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3];
        }

        void charAt_001()
        {
            sal_Unicode            expVal = 83;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              input = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "return the first character of OStringBuffer(kTestStr1)",
                aStrBuf.charAt(input) == expVal
            );

        }

        void charAt_002()
        {
            sal_Unicode            expVal = 32;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              input = 3;

            CPPUNIT_ASSERT_MESSAGE
            (
                "return the middle character of OStringBuffer(kTestStr1)",
                aStrBuf.charAt(input) == expVal
            );

        }

        void charAt_003()
        {
            sal_Unicode            expVal = 115;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              input = 15;

            CPPUNIT_ASSERT_MESSAGE
            (
                "return the last character of OStringBuffer(kTestStr1)",
                aStrBuf.charAt(input) == expVal
            );

        }

        void charAt_004()
        {
            sal_Unicode            expVal = 115;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Int32              input = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "return the only character of OStringBuffer(kTestStr27)",
                aStrBuf.charAt(input) == expVal
            );

        }

        void charAt_005()
        {
            sal_Unicode            expVal = 40;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              input = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "return the first of OStringBuffer(kTestStr28) with special character",
                aStrBuf.charAt(input) == expVal
            );

        }

        void charAt_006()
        {
            sal_Unicode            expVal = 11;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              input = 5;

            CPPUNIT_ASSERT_MESSAGE
            (
                "return the mid of OStringBuffer(kTestStr28) with special character",
                aStrBuf.charAt(input) == expVal
            );

        }

        void charAt_007()
        {
            sal_Unicode            expVal = 0;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              input = 0;

            CPPUNIT_ASSERT_MESSAGE
            (
                "invalid character of OStringBuffer()",
                sal_True
            );

        }

        void charAt_008()
        {
            sal_Unicode            expVal = 0;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              input = -2;

            CPPUNIT_ASSERT_MESSAGE
            (
                "invalid character of OStringBuffer()",
                sal_True
            );

        }

        CPPUNIT_TEST_SUITE( charAt );
        CPPUNIT_TEST( charAt_001 );
        CPPUNIT_TEST( charAt_002 );
        CPPUNIT_TEST( charAt_003 );
        CPPUNIT_TEST( charAt_004 );
        CPPUNIT_TEST( charAt_005 );
        CPPUNIT_TEST( charAt_006 );
        CPPUNIT_TEST( charAt_007 );
        CPPUNIT_TEST( charAt_008 );
        CPPUNIT_TEST_SUITE_END();
    };
// -----------------------------------------------------------------------------

    class  csuc : public CppUnit::TestFixture
    {
        void csuc_001()
        {
            const sal_Char*        expVal = kTestStr1;
            ::rtl::OStringBuffer   aStrBuf( kTestStr1 );
            sal_Int32              cmpLen = kTestStr1Len;

            const sal_Char* pstr = *&aStrBuf;

            CPPUNIT_ASSERT_MESSAGE
            (
                "test normal string",
                cmpstr( pstr, expVal, cmpLen )
            );

        }

        void csuc_002()
        {
            const sal_Char         expVal=0x0;
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              cmpLen = 1;

            const sal_Char* pstr = *&aStrBuf;

            CPPUNIT_ASSERT_MESSAGE
            (
                "test empty string",
                cmpstr( pstr, &expVal, cmpLen )
            );

        }


        CPPUNIT_TEST_SUITE( csuc );
        CPPUNIT_TEST( csuc_001 );
        CPPUNIT_TEST( csuc_002 );
        CPPUNIT_TEST_SUITE_END();
    };


// -----------------------------------------------------------------------------

    class  getStr : public CppUnit::TestFixture
    {
        void getStr_001()
        {
            const sal_Char*        expVal = kTestStr1;
            ::rtl::OStringBuffer   aStrBuf( kTestStr1 );
            sal_Int32              cmpLen = kTestStr1Len;

            const sal_Char* pstr = aStrBuf.getStr();

            CPPUNIT_ASSERT_MESSAGE
            (
                "test normal string",
                cmpstr( pstr, expVal, cmpLen )
            );

        }

        void getStr_002()
        {
            const sal_Char         tmpUC=0x0;
            const sal_Char*        expVal=&tmpUC;
            ::rtl::OStringBuffer   aStrBuf;
            sal_Int32              cmpLen = 1;

            const sal_Char* pstr = aStrBuf.getStr();

            CPPUNIT_ASSERT_MESSAGE
            (
                "test empty string",
                cmpstr( pstr, expVal, cmpLen )
            );

        }


        CPPUNIT_TEST_SUITE( getStr );
        CPPUNIT_TEST( getStr_001 );
        CPPUNIT_TEST( getStr_002 );
        CPPUNIT_TEST_SUITE_END();
    };

// -----------------------------------------------------------------------------

    class  setCharAt : public CppUnit::TestFixture
    {
        OString* arrOUS[4];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( kTestStr27 );
            arrOUS[2] = new OString( kTestStr28 );
            arrOUS[3] = new OString(  );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3];
        }

        void setCharAt_001()
        {
            OString                expVal( kTestStr31 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              input1 = 0;
            sal_Char               input2 = 's';

            CPPUNIT_ASSERT_MESSAGE
            (
                "set the first character of OStringBuffer(kTestStr1) with s",
                (aStrBuf.setCharAt(input1, input2)).getStr() == expVal
            );

        }

        void setCharAt_002()
        {
            OString                expVal( kTestStr3 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              input1 = 4;
            sal_Char               input2 = 'm';

            CPPUNIT_ASSERT_MESSAGE
            (
                "set the middle character of OStringBuffer(kTestStr1) with m",
                (aStrBuf.setCharAt(input1, input2)).getStr() == expVal
            );

        }

        void setCharAt_003()
        {
            OString                expVal( kTestStr32 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Int32              input1 = 15;
            sal_Char               input2 = ' ';

            CPPUNIT_ASSERT_MESSAGE
            (
                "set the last character of OStringBuffer(kTestStr1) with ' '",
                (aStrBuf.setCharAt(input1, input2)).getStr() == expVal
            );

        }


        void setCharAt_004()
        {
            OString                expVal( kTestStr33 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Int32              input1 = 0;
            sal_Char               input2 = ' ';

            CPPUNIT_ASSERT_MESSAGE
            (
                "set the only character of OStringBuffer(kTestStr27) with ' '",
                (aStrBuf.setCharAt(input1, input2)).getStr() == expVal
            );

        }


        void setCharAt_005()
        {
            OString                expVal( kTestStr34 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              input1 = 1;
            sal_Char               input2 = (sal_Char)5;

            CPPUNIT_ASSERT_MESSAGE
            (
                "set the only of OStringBuffer(kTestStr28) with special character",
                (aStrBuf.setCharAt(input1, input2)).getStr() == expVal
            );

        }

        void setCharAt_006()
        {
            OString                expVal( kTestStr35 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Int32              input1 = 1;
            sal_Char               input2 = (sal_Char)-5;

            CPPUNIT_ASSERT_MESSAGE
            (
                "set the only of OStringBuffer(kTestStr28) with special character",
                (aStrBuf.setCharAt(input1, input2)).getStr() == expVal
            );

        }
#ifdef WITH_CORE
        void setCharAt_007()
        {
            OString*               expVal = 0;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              input1 = 0;
            sal_Char               input2 = (sal_Char)5;

            CPPUNIT_ASSERT_MESSAGE
            (
                "invalid character of OStringBuffer()",
                sal_True
            );

            delete expVal;

        }

        void setCharAt_008()
        {
            OString*               expVal = 0;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              input1 = -2;
            sal_Char               input2 = (sal_Char)5;

            CPPUNIT_ASSERT_MESSAGE
            (
                "invalid character of OStringBuffer()",
                sal_True
            );

            delete expVal;

        }

        void setCharAt_009()
        {
            OString*               expVal = 0;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Int32              input1 = 3;
            sal_Char               input2 = (sal_Char)5;

            CPPUNIT_ASSERT_MESSAGE
            (
                "invalid character of OStringBuffer()",
                sal_True
            );

            delete expVal;

        }
#endif
        CPPUNIT_TEST_SUITE( setCharAt );
        CPPUNIT_TEST( setCharAt_001 );
        CPPUNIT_TEST( setCharAt_002 );
        CPPUNIT_TEST( setCharAt_003 );
        CPPUNIT_TEST( setCharAt_004 );
        CPPUNIT_TEST( setCharAt_005 );
        CPPUNIT_TEST( setCharAt_006 );
#ifdef WITH_CORE
        CPPUNIT_TEST( setCharAt_007 );
        CPPUNIT_TEST( setCharAt_008 );
        CPPUNIT_TEST( setCharAt_009 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };

// -----------------------------------------------------------------------------

    class  append_001 : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001_001()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                input2( kTestStr8 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_001_002()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                input2( kTestStr36 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_003()
        {
            OString                expVal( kTestStr37 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                input2( kTestStr23 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_004()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                input2;

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_005()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                input2( kTestStr7 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_006()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                input2( kTestStr2 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_007()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                input2( kTestStr1 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_008()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                input2;

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_009()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                input2( kTestStr7 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_010()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                input2( kTestStr2 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_011()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                input2( kTestStr1 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_012()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                input2;

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_013()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                input2( kTestStr7 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_014()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                input2( kTestStr2 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_015()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                input2( kTestStr1 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_016()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                input2;

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_017()
        {
            OString                expVal( kTestStr29 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                input2( kTestStr38 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_018()
        {
            OString                expVal( kTestStr39 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                input2( kTestStr17 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_019()
        {
            OString                expVal( kTestStr40 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                input2( kTestStr31 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_001_020()
        {
            OString                expVal( kTestStr28 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                input2;

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

#ifdef WITH_CORE
        void append_001_021()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            OString                input2;

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_001 );
        CPPUNIT_TEST( append_001_001 );
        CPPUNIT_TEST( append_001_002 );
        CPPUNIT_TEST( append_001_003 );
        CPPUNIT_TEST( append_001_004 );
        CPPUNIT_TEST( append_001_005 );
        CPPUNIT_TEST( append_001_006 );
        CPPUNIT_TEST( append_001_007 );
        CPPUNIT_TEST( append_001_008 );
        CPPUNIT_TEST( append_001_009 );
        CPPUNIT_TEST( append_001_010 );
        CPPUNIT_TEST( append_001_011 );
        CPPUNIT_TEST( append_001_012 );
        CPPUNIT_TEST( append_001_013 );
        CPPUNIT_TEST( append_001_014 );
        CPPUNIT_TEST( append_001_015 );
        CPPUNIT_TEST( append_001_016 );
        CPPUNIT_TEST( append_001_017 );
        CPPUNIT_TEST( append_001_018 );
        CPPUNIT_TEST( append_001_019 );
        CPPUNIT_TEST( append_001_020 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_001_021 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };

// -----------------------------------------------------------------------------

    class  append_002 : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_002_001()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input = kTestStr8;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_002()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input = kTestStr36;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_003()
        {
            OString                expVal( kTestStr37 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input = kTestStr23;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_004()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_005()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input = kTestStr7;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_006()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input = kTestStr2;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_007()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input = kTestStr1;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_008()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_009()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input = kTestStr7;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_010()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input = kTestStr2;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_011()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input = kTestStr1;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_012()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_013()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input = kTestStr7;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_014()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input = kTestStr2;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_015()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input = kTestStr1;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_016()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_017()
        {
            OString                expVal( kTestStr29 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input = kTestStr38;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_018()
        {
            OString                expVal( kTestStr39 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input = kTestStr17;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_019()
        {
            OString                expVal( kTestStr40 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input = kTestStr31;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002_020()
        {
            OString                expVal( kTestStr28 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

#ifdef WITH_CORE
        void append_002_021()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            const sal_Char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer(with INT_MAX)",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_002 );
        CPPUNIT_TEST( append_002_001 );
        CPPUNIT_TEST( append_002_002 );
        CPPUNIT_TEST( append_002_003 );
        CPPUNIT_TEST( append_002_004 );
        CPPUNIT_TEST( append_002_005 );
        CPPUNIT_TEST( append_002_006 );
        CPPUNIT_TEST( append_002_007 );
        CPPUNIT_TEST( append_002_008 );
        CPPUNIT_TEST( append_002_009 );
        CPPUNIT_TEST( append_002_010 );
        CPPUNIT_TEST( append_002_011 );
        CPPUNIT_TEST( append_002_012 );
        CPPUNIT_TEST( append_002_013 );
        CPPUNIT_TEST( append_002_014 );
        CPPUNIT_TEST( append_002_015 );
        CPPUNIT_TEST( append_002_016 );
        CPPUNIT_TEST( append_002_017 );
        CPPUNIT_TEST( append_002_018 );
        CPPUNIT_TEST( append_002_019 );
        CPPUNIT_TEST( append_002_020 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_002_021 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };
// -----------------------------------------------------------------------------

    class  append_003 : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_003_001()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input1 = kTestStr36;
            sal_Int32              input2 = 12;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_002()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input1 = kTestStr36;
            sal_Int32              input2 = 28;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_003()
        {
            OString                expVal( kTestStr37 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input1 = kTestStr23;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_004()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_005()
        {
            OString                expVal( kTestStr41 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = -1;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 0) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_006()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 4;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_007()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 32;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_008()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_009()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_010()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = -1;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 0) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_011()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 4;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_012()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 32;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_013()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_014()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_015()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = -1;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 0) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_016()
        {
            OString                expVal( kTestStr7 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 4;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_017()
        {
            OString                expVal( kTestStr2 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 32;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_018()
        {
            OString                expVal( kTestStr1 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_019()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_020()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = -1;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 0) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_021()
        {
            OString                expVal( kTestStr29 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input1 = kTestStr38;
            sal_Int32              input2 = 7;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_022()
        {
            OString                expVal( kTestStr39 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input1 = kTestStr17;
            sal_Int32              input2 = 22;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_023()
        {
            OString                expVal( kTestStr40 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input1 = kTestStr31;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_024()
        {
            OString                expVal( kTestStr28 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003_025()
        {
            OString                expVal( kTestStr42 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            const sal_Char*        input1 = kTestStr2;
            sal_Int32              input2 = -1;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length less than 0) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

#ifdef WITH_CORE
        void append_003_026()
        {
            OString                expVal;
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            const sal_Char*        input1 = kTestStr25;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer(with INT_MAX)",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_003 );
        CPPUNIT_TEST( append_003_001 );
        CPPUNIT_TEST( append_003_002 );
        CPPUNIT_TEST( append_003_003 );
        CPPUNIT_TEST( append_003_004 );
        CPPUNIT_TEST( append_003_005 );
        CPPUNIT_TEST( append_003_006 );
        CPPUNIT_TEST( append_003_007 );
        CPPUNIT_TEST( append_003_008 );
        CPPUNIT_TEST( append_003_009 );
        CPPUNIT_TEST( append_003_010 );
        CPPUNIT_TEST( append_003_011 );
        CPPUNIT_TEST( append_003_012 );
        CPPUNIT_TEST( append_003_013 );
        CPPUNIT_TEST( append_003_014 );
        CPPUNIT_TEST( append_003_015 );
        CPPUNIT_TEST( append_003_016 );
        CPPUNIT_TEST( append_003_017 );
        CPPUNIT_TEST( append_003_018 );
        CPPUNIT_TEST( append_003_019 );
        CPPUNIT_TEST( append_003_020 );
        CPPUNIT_TEST( append_003_021 );
        CPPUNIT_TEST( append_003_022 );
        CPPUNIT_TEST( append_003_023 );
        CPPUNIT_TEST( append_003_024 );
        CPPUNIT_TEST( append_003_025 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_003_026 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };
// -----------------------------------------------------------------------------

    class  append_004 : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_004_001()
        {
            OString                expVal( kTestStr45 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Bool               input = sal_True;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_002()
        {
            OString                expVal( kTestStr46 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Bool               input = sal_False;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_003()
        {
            OString                expVal( kTestStr47 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Bool               input = sal_True;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_004()
        {
            OString                expVal( kTestStr48 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Bool               input = sal_False;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_005()
        {
            OString                expVal( kTestStr47 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Bool               input = sal_True;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_006()
        {
            OString                expVal( kTestStr48 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Bool               input = sal_False;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_007()
        {
            OString                expVal( kTestStr47 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Bool               input = sal_True;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_008()
        {
            OString                expVal( kTestStr48 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Bool               input = sal_False;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_009()
        {
            OString                expVal( kTestStr49 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Bool               input = sal_True;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_010()
        {
            OString                expVal( kTestStr50 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Bool               input = sal_False;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

#ifdef WITH_CORE
        void append_004_011()
        {
            OString                expVal( kTestStr47 );
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Bool               input = sal_True;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer(with INT_MAX)",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004_012()
        {
            OString                expVal( kTestStr48 );
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Bool               input = sal_False;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer(with INT_MAX)",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_004 );
        CPPUNIT_TEST( append_004_001 );
        CPPUNIT_TEST( append_004_002 );
        CPPUNIT_TEST( append_004_003 );
        CPPUNIT_TEST( append_004_004 );
        CPPUNIT_TEST( append_004_005 );
        CPPUNIT_TEST( append_004_006 );
        CPPUNIT_TEST( append_004_007 );
        CPPUNIT_TEST( append_004_008 );
        CPPUNIT_TEST( append_004_009 );
        CPPUNIT_TEST( append_004_010 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_004_011 );
        CPPUNIT_TEST( append_004_012 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };

} // namespace rtl_OStringBuffer


// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::ctors,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::makeStringAndClear,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::getLength,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::getCapacity,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::ensureCapacity,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::setLength,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::charAt,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::csuc,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::getStr,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::setCharAt,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::append_001,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::append_002,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::append_003,
                                                "rtl_OStringBuffer");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OStringBuffer::append_004,
                                                "rtl_OStringBuffer");


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;
