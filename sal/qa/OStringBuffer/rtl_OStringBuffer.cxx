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

#include <sal/types.h>
#include <rtl/string.hxx>
#include <rtl_String_Const.h>
#include <rtl_String_Utils.hxx>
#include <rtl/strbuf.hxx>

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <string.h>

using ::rtl::OStringBuffer;
using ::rtl::OString;
// This file contains cppunit tests for the
// OString and OStringBuffer classes

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
            const sal_Char* pStr = aStrBuf.getStr();

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer containing no characters",
                aStrBuf.getLength() == 0 &&
                *pStr == '\0' && aStrBuf.getCapacity() == 16
            );
        }

        void ctor_002()
        {
            ::rtl::OString       aStrtmp( kTestStr1 );
            ::rtl::OStringBuffer aStrBuftmp( aStrtmp );
            ::rtl::OStringBuffer aStrBuf( aStrBuftmp );
            // sal_Bool res = cmpstr(aStrBuftmp.getStr(),aStrBuf.getStr());

            sal_Int32 nLenStrBuftmp = aStrBuftmp.getLength();

            rtl::OString sStr(aStrBuftmp.getStr());
            sal_Bool res = aStrtmp.equals( sStr );

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer from another OStringBuffer",
                aStrBuf.getLength() == nLenStrBuftmp &&
                aStrBuf.getCapacity() == aStrBuftmp.getCapacity() &&
                res
            );

        }

        void ctor_003()
        {
            ::rtl::OStringBuffer aStrBuf1(kTestStr2Len);
            ::rtl::OStringBuffer aStrBuf2(0);

            const sal_Char* pStr1 = aStrBuf1.getStr();
            const sal_Char* pStr2 = aStrBuf2.getStr();

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                aStrBuf1.getLength() == 0 &&
                *pStr1 == '\0' &&
                aStrBuf1.getCapacity() == kTestStr2Len &&
                aStrBuf2.getLength() == 0 &&
                *pStr2 == '\0' &&
                aStrBuf2.getCapacity() == 0
            );

        }

        void ctor_003_1()
        {
            // StringBuffer with created negative size are the
            // same as empty StringBuffers
            ::rtl::OStringBuffer aStrBuf3(kNonSInt32Max);

            const sal_Char* pStr = aStrBuf3.getStr();

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                aStrBuf3.getLength() == 0 &&
                *pStr == '\0' &&
                aStrBuf3.getCapacity() == kNonSInt32Max
            );
        }

        void ctor_004()
        {
            ::rtl::OString aStrtmp( kTestStr1 );
            ::rtl::OStringBuffer aStrBuf( aStrtmp );
            sal_Int32 leg = aStrBuf.getLength();

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer from OString",
                aStrBuf.getStr() == aStrtmp &&
                leg == aStrtmp.pData->length &&
                aStrBuf.getCapacity() == leg+16

            );
        }

        void ctor_005() {
            rtl::OStringBuffer b1;
            b1.makeStringAndClear();
            rtl::OStringBuffer b2(b1);
        }

        void ctor_006()
        {
            //pass in a const char*, get a temp
            //OString
            ::rtl::OStringBuffer aStrBuf(kTestStr1);
            sal_Int32 leg = aStrBuf.getLength();

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer from const char*",
                leg == rtl_str_getLength(kTestStr1) &&
                aStrBuf.getCapacity() == leg+16
            );
        }

        CPPUNIT_TEST_SUITE(ctors);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST(ctor_003);
        CPPUNIT_TEST(ctor_003_1);
        CPPUNIT_TEST(ctor_004);
        CPPUNIT_TEST(ctor_005);
        CPPUNIT_TEST(ctor_006);
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
            arrOUS[5] = new OString( "\0", 1 );

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


    class  remove : public CppUnit::TestFixture
    {
    public:
        void setUp()
        {
        }

        void tearDown()
        {
        }

        void remove_001()
        {
            ::rtl::OStringBuffer sb(
                RTL_CONSTASCII_STRINGPARAM("Red Hat, Inc."));

            sb.remove(0, 4);
            CPPUNIT_ASSERT(sb.toString().equalsL(
                RTL_CONSTASCII_STRINGPARAM("Hat, Inc.")));

            sb.remove(3, 6);
            CPPUNIT_ASSERT(sb.toString().equalsL(
                RTL_CONSTASCII_STRINGPARAM("Hat")));

            sb.remove(0, 100);

            CPPUNIT_ASSERT(sb.toString().isEmpty());

            sb.append(RTL_CONSTASCII_STRINGPARAM("Red Hat, Inc."));

            sb.remove(3, 100);

            CPPUNIT_ASSERT(sb.toString().equalsL(
                RTL_CONSTASCII_STRINGPARAM("Red")));

            sb.remove(0, sb.getLength());

            CPPUNIT_ASSERT(sb.toString().isEmpty());
        }

        CPPUNIT_TEST_SUITE(remove);
        CPPUNIT_TEST(remove_001);
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
            arrOUS[4] = new OString( "\0", 1 );
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
            sal_Int32              expVal = 1;

            CPPUNIT_ASSERT_MESSAGE
            (
                "length of string with \\0 embedded",
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
            arrOUS[4] = new OString( "\0", 1 );
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
            sal_Int32              expVal = 1+16;

            CPPUNIT_ASSERT_MESSAGE
            (
                "capacity of string with \\0 embedded",
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
            sal_Int32          expVal = 20;
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
            sal_Int32          expVal = 10;
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
            sal_Int32          expVal = 2;  // capacity is x = (str->length + 1) * 2; minimum < x ? x : minimum
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
        CPPUNIT_TEST( ensureCapacity_018 );
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
            arrOUS[4] = new OString( "\0", 1 );
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
            sal_Int32              expVal3 = 17;
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
            sal_Int32              expVal3 = 17;
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

    class  csuc : public CppUnit::TestFixture
    {
        void csuc_001()
        {
            const sal_Char*        expVal = kTestStr1;
            ::rtl::OStringBuffer   aStrBuf( kTestStr1 );
            sal_Int32              cmpLen = kTestStr1Len;

            // LLA: wrong access! const sal_Char* pstr = *&aStrBuf;
            const sal_Char* pstr = aStrBuf.getStr();
            int nEqual = strncmp(pstr, expVal, cmpLen);

            CPPUNIT_ASSERT_MESSAGE
            (
                "test normal string",
                /* cmpstr( pstr, expVal, cmpLen ) */
                nEqual == 0
            );

        }

        void csuc_002()
        {
            ::rtl::OStringBuffer   aStrBuf;

            // LLA: wrong access! const sal_Char* pstr = *&aStrBuf;
            const sal_Char* pstr = aStrBuf.getStr();
            sal_Int32 nLen = strlen(pstr);

            CPPUNIT_ASSERT_MESSAGE
            (
                "test empty string",
                // cmpstr( pstr, &expVal, cmpLen )
                nLen == 0
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
            int nEqual = strncmp(pstr, expVal, cmpLen);

            CPPUNIT_ASSERT_MESSAGE
            (
                "test normal string",
                nEqual == 0
            );

        }

        void getStr_002()
        {
            // const sal_Char         tmpUC=0x0;
            // const sal_Char*        expVal=&tmpUC;
            ::rtl::OStringBuffer   aStrBuf;
            // sal_Int32              cmpLen = 1;

            const sal_Char* pstr = aStrBuf.getStr();
            sal_Int32 nLen = strlen(pstr);

            CPPUNIT_ASSERT_MESSAGE
            (
                "test empty string",
                pstr != 0 &&
                nLen == 0
            );

        }

        CPPUNIT_TEST_SUITE( getStr );
        CPPUNIT_TEST( getStr_001 );
        CPPUNIT_TEST( getStr_002 );
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
            arrOUS[3] = new OString( "" );
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

        void append_null()
        {
            ::rtl::OStringBuffer   aStrBuf("hello world");

            aStrBuf.append('\0');
            aStrBuf.append('\1');
            aStrBuf.append('\2');

            aStrBuf.append("hello world");

            CPPUNIT_ASSERT_MESSAGE
            (
                "should be able to append nulls",
                aStrBuf.getLength() ==
                    2 * RTL_CONSTASCII_LENGTH("hello world") + 3 &&
                aStrBuf[RTL_CONSTASCII_LENGTH("hello world")] == 0 &&
                aStrBuf[RTL_CONSTASCII_LENGTH("hello world")]+1 == 1 &&
                aStrBuf[RTL_CONSTASCII_LENGTH("hello world")]+2 == 2
            );

        }

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
        CPPUNIT_TEST( append_null );
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
            arrOUS[3] = new OString( "" );
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
            arrOUS[3] = new OString( "" );
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

        CPPUNIT_TEST_SUITE( append_003 );
        CPPUNIT_TEST( append_003_001 );
        CPPUNIT_TEST( append_003_002 );
        CPPUNIT_TEST( append_003_003 );
        CPPUNIT_TEST( append_003_004 );
        CPPUNIT_TEST( append_003_006 );
        CPPUNIT_TEST( append_003_007 );
        CPPUNIT_TEST( append_003_008 );
        CPPUNIT_TEST( append_003_009 );
        CPPUNIT_TEST( append_003_011 );
        CPPUNIT_TEST( append_003_012 );
        CPPUNIT_TEST( append_003_013 );
        CPPUNIT_TEST( append_003_014 );
        CPPUNIT_TEST( append_003_016 );
        CPPUNIT_TEST( append_003_017 );
        CPPUNIT_TEST( append_003_018 );
        CPPUNIT_TEST( append_003_019 );
        CPPUNIT_TEST( append_003_021 );
        CPPUNIT_TEST( append_003_022 );
        CPPUNIT_TEST( append_003_023 );
        CPPUNIT_TEST( append_003_024 );
        CPPUNIT_TEST_SUITE_END();
    };
//-----------------------------------------------------------------------------

    class  append_004 : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
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
//------------------------------------------------------------------------
// testing the method append(sal_Char c)
//------------------------------------------------------------------------
    class  append_005 : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            OString                expVal( kTestStr51 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Char               input = 'M';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(M) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_002()
        {
            OString                expVal( kTestStr143 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Unicode(kSInt8Max) to the string buffer arrOUS[0]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_003()
        {
            OString                expVal( kTestStr27 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(s) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_004()
        {
            OString                expVal( kTestStr144 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(kSInt8Max) to the string buffer arrOUS[1]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_005_005()
        {
            OString                expVal( kTestStr27 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(s) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_006()
        {
            OString                expVal( kTestStr144 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(kSInt8Max) to the string buffer arrOUS[2]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_007()
        {
            OString                expVal( kTestStr27 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(s) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_008()
        {
            OString                expVal( kTestStr144 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(kSInt8Max) to the string buffer arrOUS[3]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_009()
        {
            OString                expVal( kTestStr56 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(s) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_010()
        {
            OString                expVal( kTestStr145 );
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(kSInt8Max) to the string buffer arrOUS[4]",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

#ifdef WITH_CORE
        void append_011()
        {
            OString                expVal( kTestStr27 );
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(s) to the string buffer(with INT_MAX)",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_012()
        {
            OString                expVal( kTestStr144 );
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the sal_Char(kSInt8Max) to the string buffer with INT_MAX)",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_005 );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 );
        CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005_005 );
        CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 );
        CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 );
        CPPUNIT_TEST( append_010 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_011 );
        CPPUNIT_TEST( append_012 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };

    class  append_006_Int32 : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_031()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_032()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_033()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_034()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_036()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_037()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_038()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_039()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_040()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_041()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_042()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_043()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_044()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_045()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_046()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_047()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_048()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_049()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_050()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_051()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_052()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_053()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_054()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_055()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_056()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_057()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_058()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_059()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_060()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_061()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_062()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_063()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_064()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_065()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_066()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_067()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_068()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_069()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_070()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_071()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_072()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_073()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_074()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_075()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_076()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_077()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_078()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_079()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_080()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_081()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_082()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_083()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_084()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_085()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_086()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_087()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_088()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_089()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_090()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_091()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_092()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_093()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_094()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_095()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_096()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_097()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_098()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_099()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_100()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        CPPUNIT_TEST_SUITE( append_006_Int32 );
        CPPUNIT_TEST( append_001 ); CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 ); CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 ); CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 ); CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 ); CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 ); CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 ); CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 ); CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 ); CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 ); CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 ); CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 ); CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 ); CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 ); CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 ); CPPUNIT_TEST( append_030 );
        CPPUNIT_TEST( append_031 ); CPPUNIT_TEST( append_032 );
        CPPUNIT_TEST( append_033 ); CPPUNIT_TEST( append_034 );
        CPPUNIT_TEST( append_035 ); CPPUNIT_TEST( append_036 );
        CPPUNIT_TEST( append_037 ); CPPUNIT_TEST( append_038 );
        CPPUNIT_TEST( append_039 ); CPPUNIT_TEST( append_040 );
        CPPUNIT_TEST( append_041 ); CPPUNIT_TEST( append_042 );
        CPPUNIT_TEST( append_043 ); CPPUNIT_TEST( append_044 );
        CPPUNIT_TEST( append_045 ); CPPUNIT_TEST( append_046 );
        CPPUNIT_TEST( append_047 ); CPPUNIT_TEST( append_048 );
        CPPUNIT_TEST( append_049 ); CPPUNIT_TEST( append_050 );
        CPPUNIT_TEST( append_051 ); CPPUNIT_TEST( append_052 );
        CPPUNIT_TEST( append_053 ); CPPUNIT_TEST( append_054 );
        CPPUNIT_TEST( append_055 ); CPPUNIT_TEST( append_056 );
        CPPUNIT_TEST( append_057 ); CPPUNIT_TEST( append_058 );
        CPPUNIT_TEST( append_059 ); CPPUNIT_TEST( append_060 );
        CPPUNIT_TEST( append_061 ); CPPUNIT_TEST( append_062 );
        CPPUNIT_TEST( append_063 ); CPPUNIT_TEST( append_064 );
        CPPUNIT_TEST( append_065 ); CPPUNIT_TEST( append_066 );
        CPPUNIT_TEST( append_067 ); CPPUNIT_TEST( append_068 );
        CPPUNIT_TEST( append_069 ); CPPUNIT_TEST( append_070 );
        CPPUNIT_TEST( append_071 ); CPPUNIT_TEST( append_072 );
        CPPUNIT_TEST( append_073 ); CPPUNIT_TEST( append_074 );
        CPPUNIT_TEST( append_075 ); CPPUNIT_TEST( append_076 );
        CPPUNIT_TEST( append_077 ); CPPUNIT_TEST( append_078 );
        CPPUNIT_TEST( append_079 ); CPPUNIT_TEST( append_080 );
        CPPUNIT_TEST( append_081 ); CPPUNIT_TEST( append_082 );
        CPPUNIT_TEST( append_083 ); CPPUNIT_TEST( append_084 );
        CPPUNIT_TEST( append_085 ); CPPUNIT_TEST( append_086 );
        CPPUNIT_TEST( append_087 ); CPPUNIT_TEST( append_088 );
        CPPUNIT_TEST( append_089 ); CPPUNIT_TEST( append_090 );
        CPPUNIT_TEST( append_091 ); CPPUNIT_TEST( append_092 );
        CPPUNIT_TEST( append_093 ); CPPUNIT_TEST( append_094 );
        CPPUNIT_TEST( append_095 ); CPPUNIT_TEST( append_096 );
        CPPUNIT_TEST( append_097 ); CPPUNIT_TEST( append_098 );
        CPPUNIT_TEST( append_099 ); CPPUNIT_TEST( append_100 );
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( sal_Int32 i, sal_Int16 radix=2 )
// where i = large constants
// testing the method append( sal_Int32 i, sal_Int16 radix=8 )
// where i = large constants
// testing the method append( sal_Int32 i, sal_Int16 radix=10 )
// where i = large constants
// testing the method append( sal_Int32 i, sal_Int16 radix=16 )
// where i = large constants
// testing the method append( sal_Int32 i, sal_Int16 radix=36 )
// where i = large constants
//------------------------------------------------------------------------
    class  append_006_Int32_Bounderies : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_031()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_032()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_033()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_034()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_036()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_037()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_038()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_039()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_040()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_041()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_042()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_043()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_044()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_045()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_046()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_047()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_048()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_049()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_050()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        CPPUNIT_TEST_SUITE( append_006_Int32_Bounderies );
        CPPUNIT_TEST( append_001 ); CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 ); CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 ); CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 ); CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 ); CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 ); CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 ); CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 ); CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 ); CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 ); CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 ); CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 ); CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 ); CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 ); CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 ); CPPUNIT_TEST( append_030 );
        CPPUNIT_TEST( append_031 ); CPPUNIT_TEST( append_032 );
        CPPUNIT_TEST( append_033 ); CPPUNIT_TEST( append_034 );
        CPPUNIT_TEST( append_035 ); CPPUNIT_TEST( append_036 );
        CPPUNIT_TEST( append_037 ); CPPUNIT_TEST( append_038 );
        CPPUNIT_TEST( append_039 ); CPPUNIT_TEST( append_040 );
        CPPUNIT_TEST( append_041 ); CPPUNIT_TEST( append_042 );
        CPPUNIT_TEST( append_043 ); CPPUNIT_TEST( append_044 );
        CPPUNIT_TEST( append_045 ); CPPUNIT_TEST( append_046 );
        CPPUNIT_TEST( append_047 ); CPPUNIT_TEST( append_048 );
        CPPUNIT_TEST( append_049 ); CPPUNIT_TEST( append_050 );
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( sal_Int32 i, sal_Int16 radix=2 )
// for negative value
// testing the method append( sal_Int32 i, sal_Int16 radix=8 )
// for negative value
// testing the method append( sal_Int32 i, sal_Int16 radix=10 )
// for negative value
// testing the method append( sal_Int32 i, sal_Int16 radix=16 )
// for negative value
// testing the method append( sal_Int32 i, sal_Int16 radix=36 )
// for negative value
//------------------------------------------------------------------------
    class  append_006_Int32_Negative : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_031()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_032()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_033()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_034()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_036()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_037()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_038()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_039()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_040()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_041()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_042()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_043()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_044()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_045()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_046()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_047()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_048()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_049()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_050()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_051()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_052()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_053()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_054()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_055()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_056()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_057()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_058()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_059()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_060()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_061()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_062()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_063()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_064()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_065()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_066()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_067()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_068()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_069()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_070()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_071()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_072()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_073()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_074()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_075()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_076()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_077()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_078()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_079()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_080()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_081()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_082()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_083()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_084()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_085()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_086()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_087()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_088()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_089()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_090()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_091()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_092()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_093()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_094()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_095()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_096()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_097()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_098()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );
        }

        void append_099()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );
        }

        void append_100()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );
        }

        CPPUNIT_TEST_SUITE( append_006_Int32_Negative );
        CPPUNIT_TEST( append_001 ); CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 ); CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 ); CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 ); CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 ); CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 ); CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 ); CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 ); CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 ); CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 ); CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 ); CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 ); CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 ); CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 ); CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 ); CPPUNIT_TEST( append_030 );
        CPPUNIT_TEST( append_031 ); CPPUNIT_TEST( append_032 );
        CPPUNIT_TEST( append_033 ); CPPUNIT_TEST( append_034 );
        CPPUNIT_TEST( append_035 ); CPPUNIT_TEST( append_036 );
        CPPUNIT_TEST( append_037 ); CPPUNIT_TEST( append_038 );
        CPPUNIT_TEST( append_039 ); CPPUNIT_TEST( append_040 );
        CPPUNIT_TEST( append_041 ); CPPUNIT_TEST( append_042 );
        CPPUNIT_TEST( append_043 ); CPPUNIT_TEST( append_044 );
        CPPUNIT_TEST( append_045 ); CPPUNIT_TEST( append_046 );
        CPPUNIT_TEST( append_047 ); CPPUNIT_TEST( append_048 );
        CPPUNIT_TEST( append_049 ); CPPUNIT_TEST( append_050 );
        CPPUNIT_TEST( append_051 ); CPPUNIT_TEST( append_052 );
        CPPUNIT_TEST( append_053 ); CPPUNIT_TEST( append_054 );
        CPPUNIT_TEST( append_055 ); CPPUNIT_TEST( append_056 );
        CPPUNIT_TEST( append_057 ); CPPUNIT_TEST( append_058 );
        CPPUNIT_TEST( append_059 ); CPPUNIT_TEST( append_060 );
        CPPUNIT_TEST( append_061 ); CPPUNIT_TEST( append_062 );
        CPPUNIT_TEST( append_063 ); CPPUNIT_TEST( append_064 );
        CPPUNIT_TEST( append_065 ); CPPUNIT_TEST( append_066 );
        CPPUNIT_TEST( append_067 ); CPPUNIT_TEST( append_068 );
        CPPUNIT_TEST( append_069 ); CPPUNIT_TEST( append_070 );
        CPPUNIT_TEST( append_071 ); CPPUNIT_TEST( append_072 );
        CPPUNIT_TEST( append_073 ); CPPUNIT_TEST( append_074 );
        CPPUNIT_TEST( append_075 ); CPPUNIT_TEST( append_076 );
        CPPUNIT_TEST( append_077 ); CPPUNIT_TEST( append_078 );
        CPPUNIT_TEST( append_079 ); CPPUNIT_TEST( append_080 );
        CPPUNIT_TEST( append_081 ); CPPUNIT_TEST( append_082 );
        CPPUNIT_TEST( append_083 ); CPPUNIT_TEST( append_084 );
        CPPUNIT_TEST( append_085 ); CPPUNIT_TEST( append_086 );
        CPPUNIT_TEST( append_087 ); CPPUNIT_TEST( append_088 );
        CPPUNIT_TEST( append_089 ); CPPUNIT_TEST( append_090 );
        CPPUNIT_TEST( append_091 ); CPPUNIT_TEST( append_092 );
        CPPUNIT_TEST( append_093 ); CPPUNIT_TEST( append_094 );
        CPPUNIT_TEST( append_095 ); CPPUNIT_TEST( append_096 );
        CPPUNIT_TEST( append_097 ); CPPUNIT_TEST( append_098 );
        CPPUNIT_TEST( append_099 ); CPPUNIT_TEST( append_100 );
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( sal_Int32 i, sal_Int16 radix ) where radix = -5
//------------------------------------------------------------------------
    class  append_006_Int32_WrongRadix : public CppUnit::TestFixture
    {
        OString* arrOUS[5];
        sal_Int32 intVal;

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );
            intVal = 11;

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr59 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );
        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr60 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );
        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr60 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr60 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr61 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[4]",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );
        }
#ifdef WITH_CORE
        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            OString                expVal( kTestStr60 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer(with INT_MAX)",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_006_Int32_WrongRadix );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 );
        CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_006 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
    class  append_006_Int32_defaultParam : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr59 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[0]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr62 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[0]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr63 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[0]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr64 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[0]+2147483647",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr65 );
            sal_Int32              input = kNonSInt32Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[0]+(-2147483648)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr60 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[1]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr66 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[1]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr67 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[1]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr68 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[1]+2147483647",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr69 );
            sal_Int32              input = SAL_MIN_INT32 /*-2147483648*/;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[1]+(-2147483648)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr60 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[2]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr66 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 0 and return OUStringBuffer[2]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr67 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -11 and return OUStringBuffer[2]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr68 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[2]+2147483647",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr69 );
            sal_Int32              input = SAL_MIN_INT32;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[2]+(-2147483648)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr60 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[3]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr66 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[3]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr67 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[3]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr68 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[3]+2147483647",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr69 );
            sal_Int32              input = SAL_MIN_INT32;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[3]+(-2147483648)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr61 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[4]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr70 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[4]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr71 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[4]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr72 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[4]+2147483647",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr73 );
            sal_Int32              input = SAL_MIN_INT32;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[4]+(-2147483648)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }
#ifdef WITH_CORE
        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            OString                expVal( kTestStr60 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 11 and return OStringBuffer(kSInt32Max)+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            OString                expVal( kTestStr66 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 0 and return OStringBuffer(kSInt32Max)+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            OString                expVal( kTestStr67 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer(kSInt32Max)+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            OString                expVal( kTestStr68 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer(kSInt32Max)+2147483647",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            OString                expVal( kTestStr69 );
            sal_Int32              input = SAL_MIN_INT32;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer(kSInt32Max)+(-2147483648)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_006_Int32_defaultParam );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 );
        CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 );
        CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 );
        CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 );
        CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 );
        CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 );
        CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 );
        CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 );
        CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 );
        CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 );
        CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 );
        CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 );
        CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 );
        CPPUNIT_TEST( append_030 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( sal_Int64 l, sal_Int16 radix=2 )
// testing the method append( sal_Int64 l, sal_Int16 radix=8 )
// testing the method append( sal_Int64 l, sal_Int16 radix=10 )
// testing the method append( sal_Int64 l, sal_Int16 radix=16 )
// testing the method append( sal_Int64 l, sal_Int16 radix=36 )
//------------------------------------------------------------------------
    class  append_007_Int64 : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64             input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_031()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_032()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_033()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_034()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_036()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_037()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_038()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_039()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_040()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_041()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_042()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_043()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_044()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_045()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_046()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_047()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_048()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_049()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_050()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_051()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_052()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_053()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_054()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_055()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_056()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_057()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_058()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_059()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_060()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_061()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_062()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_063()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_064()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_065()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_066()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_067()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_068()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_069()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_070()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_071()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_072()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_073()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_074()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_075()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_076()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_077()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_078()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_079()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_080()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_081()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_082()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_083()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_084()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_085()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_086()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_087()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_088()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_089()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_090()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_091()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_092()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_093()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_094()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_095()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_096()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_097()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_098()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_099()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_100()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        CPPUNIT_TEST_SUITE( append_007_Int64 );
        CPPUNIT_TEST( append_001 ); CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 ); CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 ); CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 ); CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 ); CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 ); CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 ); CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 ); CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 ); CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 ); CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 ); CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 ); CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 ); CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 ); CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 ); CPPUNIT_TEST( append_030 );
        CPPUNIT_TEST( append_031 ); CPPUNIT_TEST( append_032 );
        CPPUNIT_TEST( append_033 ); CPPUNIT_TEST( append_034 );
        CPPUNIT_TEST( append_035 ); CPPUNIT_TEST( append_036 );
        CPPUNIT_TEST( append_037 ); CPPUNIT_TEST( append_038 );
        CPPUNIT_TEST( append_039 ); CPPUNIT_TEST( append_040 );
        CPPUNIT_TEST( append_041 ); CPPUNIT_TEST( append_042 );
        CPPUNIT_TEST( append_043 ); CPPUNIT_TEST( append_044 );
        CPPUNIT_TEST( append_045 ); CPPUNIT_TEST( append_046 );
        CPPUNIT_TEST( append_047 ); CPPUNIT_TEST( append_048 );
        CPPUNIT_TEST( append_049 ); CPPUNIT_TEST( append_050 );
        CPPUNIT_TEST( append_051 ); CPPUNIT_TEST( append_052 );
        CPPUNIT_TEST( append_053 ); CPPUNIT_TEST( append_054 );
        CPPUNIT_TEST( append_055 ); CPPUNIT_TEST( append_056 );
        CPPUNIT_TEST( append_057 ); CPPUNIT_TEST( append_058 );
        CPPUNIT_TEST( append_059 ); CPPUNIT_TEST( append_060 );
        CPPUNIT_TEST( append_061 ); CPPUNIT_TEST( append_062 );
        CPPUNIT_TEST( append_063 ); CPPUNIT_TEST( append_064 );
        CPPUNIT_TEST( append_065 ); CPPUNIT_TEST( append_066 );
        CPPUNIT_TEST( append_067 ); CPPUNIT_TEST( append_068 );
        CPPUNIT_TEST( append_069 ); CPPUNIT_TEST( append_070 );
        CPPUNIT_TEST( append_071 ); CPPUNIT_TEST( append_072 );
        CPPUNIT_TEST( append_073 ); CPPUNIT_TEST( append_074 );
        CPPUNIT_TEST( append_075 ); CPPUNIT_TEST( append_076 );
        CPPUNIT_TEST( append_077 ); CPPUNIT_TEST( append_078 );
        CPPUNIT_TEST( append_079 ); CPPUNIT_TEST( append_080 );
        CPPUNIT_TEST( append_081 ); CPPUNIT_TEST( append_082 );
        CPPUNIT_TEST( append_083 ); CPPUNIT_TEST( append_084 );
        CPPUNIT_TEST( append_085 ); CPPUNIT_TEST( append_086 );
        CPPUNIT_TEST( append_087 ); CPPUNIT_TEST( append_088 );
        CPPUNIT_TEST( append_089 ); CPPUNIT_TEST( append_090 );
        CPPUNIT_TEST( append_091 ); CPPUNIT_TEST( append_092 );
        CPPUNIT_TEST( append_093 ); CPPUNIT_TEST( append_094 );
        CPPUNIT_TEST( append_095 ); CPPUNIT_TEST( append_096 );
        CPPUNIT_TEST( append_097 ); CPPUNIT_TEST( append_098 );
        CPPUNIT_TEST( append_099 ); CPPUNIT_TEST( append_100 );
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( sal_Int64 i, sal_Int16 radix=2 )
// where i = large constants
// testing the method append( sal_Int64 i, sal_Int16 radix=8 )
// where i = large constants
// testing the method append( sal_Int64 i, sal_Int16 radix=10 )
// where i = large constants
// testing the method append( sal_Int64 i, sal_Int16 radix=16 )
// where i = large constants
// testing the method append( sal_Int64 i, sal_Int16 radix=36 )
// where i = large constants
//------------------------------------------------------------------------
    class  append_007_Int64_Bounderies : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64             input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_031()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_032()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_033()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_034()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_036()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_037()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_038()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_039()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_040()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_041()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_042()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_043()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_044()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_045()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_046()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_047()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_048()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_049()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_050()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        CPPUNIT_TEST_SUITE( append_007_Int64_Bounderies );
        CPPUNIT_TEST( append_001 ); CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 ); CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 ); CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 ); CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 ); CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 ); CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 ); CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 ); CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 ); CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 ); CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 ); CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 ); CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 ); CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 ); CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 ); CPPUNIT_TEST( append_030 );
        CPPUNIT_TEST( append_031 ); CPPUNIT_TEST( append_032 );
        CPPUNIT_TEST( append_033 ); CPPUNIT_TEST( append_034 );
        CPPUNIT_TEST( append_035 ); CPPUNIT_TEST( append_036 );
        CPPUNIT_TEST( append_037 ); CPPUNIT_TEST( append_038 );
        CPPUNIT_TEST( append_039 ); CPPUNIT_TEST( append_040 );
        CPPUNIT_TEST( append_041 ); CPPUNIT_TEST( append_042 );
        CPPUNIT_TEST( append_043 ); CPPUNIT_TEST( append_044 );
        CPPUNIT_TEST( append_045 ); CPPUNIT_TEST( append_046 );
        CPPUNIT_TEST( append_047 ); CPPUNIT_TEST( append_048 );
        CPPUNIT_TEST( append_049 ); CPPUNIT_TEST( append_050 );
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( sal_Int64 i, sal_Int16 radix=2 )
// for negative value
// testing the method append( sal_Int64 i, sal_Int16 radix=8 )
// for negative value
// testing the method append( sal_Int64 i, sal_Int16 radix=10 )
// for negative value
// testing the method append( sal_Int64 i, sal_Int16 radix=16 )
// for negative value
// testing the method append( sal_Int64 i, sal_Int16 radix=36 )
// for negative value
//------------------------------------------------------------------------
    class  append_007_Int64_Negative : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_031()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_032()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_033()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_034()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_036()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_037()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_038()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_039()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_040()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_041()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_042()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_043()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_044()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_045()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_046()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_047()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_048()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_049()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_050()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_051()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_052()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_053()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_054()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_055()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_056()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_057()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_058()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_059()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_060()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_061()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_062()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_063()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_064()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_065()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_066()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_067()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_068()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_069()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_070()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_071()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_072()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_073()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_074()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_075()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_076()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_077()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_078()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_079()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_080()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_081()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_082()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_083()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_084()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_085()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_086()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_087()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_088()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_089()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_090()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_091()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_092()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_093()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_094()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_095()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_096()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_097()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_098()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_099()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_100()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                aStrBuf.getStr()== expVal &&
                    aStrBuf.getLength() == expVal.getLength()
            );

        }

        CPPUNIT_TEST_SUITE( append_007_Int64_Negative );
        CPPUNIT_TEST( append_001 ); CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 ); CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 ); CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 ); CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 ); CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 ); CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 ); CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 ); CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 ); CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 ); CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 ); CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 ); CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 ); CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 ); CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 ); CPPUNIT_TEST( append_030 );
        CPPUNIT_TEST( append_031 ); CPPUNIT_TEST( append_032 );
        CPPUNIT_TEST( append_033 ); CPPUNIT_TEST( append_034 );
        CPPUNIT_TEST( append_035 ); CPPUNIT_TEST( append_036 );
        CPPUNIT_TEST( append_037 ); CPPUNIT_TEST( append_038 );
        CPPUNIT_TEST( append_039 ); CPPUNIT_TEST( append_040 );
        CPPUNIT_TEST( append_041 ); CPPUNIT_TEST( append_042 );
        CPPUNIT_TEST( append_043 ); CPPUNIT_TEST( append_044 );
        CPPUNIT_TEST( append_045 ); CPPUNIT_TEST( append_046 );
        CPPUNIT_TEST( append_047 ); CPPUNIT_TEST( append_048 );
        CPPUNIT_TEST( append_049 ); CPPUNIT_TEST( append_050 );
        CPPUNIT_TEST( append_051 ); CPPUNIT_TEST( append_052 );
        CPPUNIT_TEST( append_053 ); CPPUNIT_TEST( append_054 );
        CPPUNIT_TEST( append_055 ); CPPUNIT_TEST( append_056 );
        CPPUNIT_TEST( append_057 ); CPPUNIT_TEST( append_058 );
        CPPUNIT_TEST( append_059 ); CPPUNIT_TEST( append_060 );
        CPPUNIT_TEST( append_061 ); CPPUNIT_TEST( append_062 );
        CPPUNIT_TEST( append_063 ); CPPUNIT_TEST( append_064 );
        CPPUNIT_TEST( append_065 ); CPPUNIT_TEST( append_066 );
        CPPUNIT_TEST( append_067 ); CPPUNIT_TEST( append_068 );
        CPPUNIT_TEST( append_069 ); CPPUNIT_TEST( append_070 );
        CPPUNIT_TEST( append_071 ); CPPUNIT_TEST( append_072 );
        CPPUNIT_TEST( append_073 ); CPPUNIT_TEST( append_074 );
        CPPUNIT_TEST( append_075 ); CPPUNIT_TEST( append_076 );
        CPPUNIT_TEST( append_077 ); CPPUNIT_TEST( append_078 );
        CPPUNIT_TEST( append_079 ); CPPUNIT_TEST( append_080 );
        CPPUNIT_TEST( append_081 ); CPPUNIT_TEST( append_082 );
        CPPUNIT_TEST( append_083 ); CPPUNIT_TEST( append_084 );
        CPPUNIT_TEST( append_085 ); CPPUNIT_TEST( append_086 );
        CPPUNIT_TEST( append_087 ); CPPUNIT_TEST( append_088 );
        CPPUNIT_TEST( append_089 ); CPPUNIT_TEST( append_090 );
        CPPUNIT_TEST( append_091 ); CPPUNIT_TEST( append_092 );
        CPPUNIT_TEST( append_093 ); CPPUNIT_TEST( append_094 );
        CPPUNIT_TEST( append_095 ); CPPUNIT_TEST( append_096 );
        CPPUNIT_TEST( append_097 ); CPPUNIT_TEST( append_098 );
        CPPUNIT_TEST( append_099 ); CPPUNIT_TEST( append_100 );
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( sal_Int64 i, sal_Int16 radix ) where radix = -5
//------------------------------------------------------------------------
    class  append_007_Int64_WrongRadix : public CppUnit::TestFixture
    {
        OString* arrOUS[5];
        sal_Int64 intVal;

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );
            intVal = 11;

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr59 );

            aStrBuf.append( intVal, -5 );


            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[0]",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );
        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr60 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[1]",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );
        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr60 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[2]",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );
        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr60 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[3]",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );
        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr61 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer arrOUS[4]",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );
        }
#ifdef WITH_CORE
        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
            OString                expVal( kTestStr60 );

            aStrBuf.append( intVal, -5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the WrongRadix to the string buffer(with INT_MAX)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );
        }
#endif

        CPPUNIT_TEST_SUITE( append_007_Int64_WrongRadix );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 );
        CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_006 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
    class  append_007_Int64_defaultParam : public CppUnit::TestFixture
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr59 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[0]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr62 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[0]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr63 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[0]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr116 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[0]+9223372036854775807",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            OString                expVal( kTestStr117 );
            sal_Int64              input = SAL_MIN_INT64/*-9223372036854775808*/; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            sal_Bool bRes = expVal.equals( aStrBuf.getStr() );
            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[0]+(-9223372036854775808)",
                bRes && aStrBuf.getLength() == expVal.getLength()
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr60 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[1]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr66 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[1]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr67 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[1]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr118 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[1]+9223372036854775807",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            OString                expVal( kTestStr119 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[1]+(-9223372036854775808)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr60 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[2]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr66 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 0 and return OUStringBuffer[2]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr67 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -11 and return OUStringBuffer[2]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr118 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[2]+9223372036854775807",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            OString                expVal( kTestStr119 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[2]+(-9223372036854775808)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr60 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[3]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr66 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[3]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr67 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[3]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr118 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[3]+9223372036854775807",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            OString                expVal( kTestStr119 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[3]+(-9223372036854775808)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr61 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[4]+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr70 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[4]+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr71 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[4]+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr120 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[4]+9223372036854775807",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            OString                expVal( kTestStr121 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[4]+(-9223372036854775808)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }
#ifdef WITH_CORE
        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
            OString                expVal( kTestStr60 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 11 and return OStringBuffer(kSInt64Max)+11",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
            OString                expVal( kTestStr66 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 0 and return OStringBuffer(kSInt64Max)+0",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
            OString                expVal( kTestStr67 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer(kSInt64Max)+(-11)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
            OString                expVal( kTestStr118 );
            sal_Int64              input = 9223372036854775807;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer(kSInt64Max)+9223372036854775807",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
            OString                expVal( kTestStr119 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer(kSInt64Max)+(-9223372036854775808)",
                (aStrBuf.toString() == expVal &&
                 aStrBuf.getLength() == expVal.getLength())
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_007_Int64_defaultParam );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 );
        CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 );
        CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 );
        CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 );
        CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 );
        CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 );
        CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 );
        CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 );
        CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 );
        CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 );
        CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 );
        CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 );
        CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 );
        CPPUNIT_TEST( append_030 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( float f )
//------------------------------------------------------------------------
    class checkfloat : public CppUnit::TestFixture
    {
    public:
        bool checkIfStrBufContainAtPosTheFloat(rtl::OStringBuffer const& _sStrBuf, sal_Int32 _nLen, float _nFloat)
            {
                OString sFloatValue;
                sFloatValue = rtl::OString::valueOf(_nFloat);

                OString sBufferString(_sStrBuf.getStr());
                sal_Int32 nPos = sBufferString.indexOf(sFloatValue);
                if ( nPos >= 0 && nPos == _nLen)
                {
                    return true;
                }
                return false;
            }
    };
// -----------------------------------------------------------------------------
    class  append_008_float : public checkfloat
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("3.0");

            // LLA:
            // the complex problem is here, that a float value is not really what we write.
            // So a 3.0 could also be 3 or 3.0 or 3.0000001 or 2.9999999
            // this has to be checked.
            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append 3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append 3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append 3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append 3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append 3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append 3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append 3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append 3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append 3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append 3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append 3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append 3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append 3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append 3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append 3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append 3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append 3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append 3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append 3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append 3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append 3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append 3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append 3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append 3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append 3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append 3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append 3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append 3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append 3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append 3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_031()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append 3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_032()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append 3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_033()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append 3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_034()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append 3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append 3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }
#ifdef WITH_CORE
        void append_036()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append 3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_037()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append 3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_038()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append 3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_039()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append 3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_040()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append 3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_041()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append 3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_042()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append 3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_008_float );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 );
        CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 );
        CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 );
        CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 );
        CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 );
        CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 );
        CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 );
        CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 );
        CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 );
        CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 );
        CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 );
        CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 );
        CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 );
        CPPUNIT_TEST( append_030 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( float f ) for negative value
//------------------------------------------------------------------------
    class  append_008_Float_Negative : public checkfloat
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("-3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append -3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_002()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("-3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append -3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_003()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("-3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append -3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_004()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("-3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append -3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_005()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("-3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append -3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_006()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("-3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append -3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_007()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            float                  input = (float)atof("-3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append -3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_008()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("-3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append -3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_009()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("-3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append -3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_010()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("-3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append -3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_011()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("-3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append -3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_012()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("-3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append -3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_013()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("-3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append -3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_014()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
            float                  input = (float)atof("-3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[1] append -3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_015()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("-3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append -3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_016()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("-3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append -3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_017()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("-3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append -3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_018()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("-3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append -3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_019()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("-3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append -3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_020()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("-3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append -3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_021()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
            float                  input = (float)atof("-3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[2] append -3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_022()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("-3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append -3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_023()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("-3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append -3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_024()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("-3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append -3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_025()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("-3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append -3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_026()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("-3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append -3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_027()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("-3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append -3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_028()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
            float                  input = (float)atof("-3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append -3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_029()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("-3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append -3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_030()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("-3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append -3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_031()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("-3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append -3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_032()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("-3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append -3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_033()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("-3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append -3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_034()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("-3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append -3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            float                  input = (float)atof("-3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append -3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }
#ifdef WITH_CORE
        void append_036()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("-3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append -3.0",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_037()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("-3.5");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append -3.5",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_038()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("-3.0625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append -3.0625",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_039()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("-3.502525");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append -3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_040()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("-3.141592");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append -3.141592",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_041()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("-3.5025255");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append -3.5025255",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

        void append_042()
        {
            ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
            float                  input = (float)atof("-3.00390625");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "OStringBuffer( kSInt32Max ) append -3.0039062",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }
#endif

        CPPUNIT_TEST_SUITE( append_008_Float_Negative );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_002 );
        CPPUNIT_TEST( append_003 );
        CPPUNIT_TEST( append_004 );
        CPPUNIT_TEST( append_005 );
        CPPUNIT_TEST( append_006 );
        CPPUNIT_TEST( append_007 );
        CPPUNIT_TEST( append_008 );
        CPPUNIT_TEST( append_009 );
        CPPUNIT_TEST( append_010 );
        CPPUNIT_TEST( append_011 );
        CPPUNIT_TEST( append_012 );
        CPPUNIT_TEST( append_013 );
        CPPUNIT_TEST( append_014 );
        CPPUNIT_TEST( append_015 );
        CPPUNIT_TEST( append_016 );
        CPPUNIT_TEST( append_017 );
        CPPUNIT_TEST( append_018 );
        CPPUNIT_TEST( append_019 );
        CPPUNIT_TEST( append_020 );
        CPPUNIT_TEST( append_021 );
        CPPUNIT_TEST( append_022 );
        CPPUNIT_TEST( append_023 );
        CPPUNIT_TEST( append_024 );
        CPPUNIT_TEST( append_025 );
#ifdef WITH_CORE
        CPPUNIT_TEST( append_026 );
        CPPUNIT_TEST( append_027 );
        CPPUNIT_TEST( append_028 );
        CPPUNIT_TEST( append_029 );
        CPPUNIT_TEST( append_030 );
#endif
        CPPUNIT_TEST_SUITE_END();
    };
//------------------------------------------------------------------------
// testing the method append( double d )
//------------------------------------------------------------------------

    class checkdouble : public CppUnit::TestFixture
    {
    public:
        bool checkIfStrBufContainAtPosTheDouble(rtl::OStringBuffer const& _sStrBuf, sal_Int32 _nLen, double _nDouble)
            {
                OString sDoubleValue;
                sDoubleValue = rtl::OString::valueOf(_nDouble);

                OString sBufferString(_sStrBuf.getStr());
                sal_Int32 nPos = sBufferString.indexOf(sDoubleValue);
                if ( nPos >= 0 && nPos == _nLen)
                {
                    return true;
                }
                return false;
            }
    };

    class  append_009_double : public checkdouble
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            double                 input = atof("3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append 3.0",
                checkIfStrBufContainAtPosTheDouble(aStrBuf, nLen, input)
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            double                 input = atof("3.141592653589793238462643");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append 3.141592653589793238462643",
                checkIfStrBufContainAtPosTheDouble(aStrBuf, nLen, input)
            );

        }

        CPPUNIT_TEST_SUITE( append_009_double );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_035 );
        CPPUNIT_TEST_SUITE_END();
    };

//------------------------------------------------------------------------
// testing the method append( double f ) for negative value
//------------------------------------------------------------------------
    class  append_009_Double_Negative : public checkdouble
    {
        OString* arrOUS[5];

    public:
        void setUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void tearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        void append_001()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
            double                 input = atof("-3.0");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[0] append -3.0",
                checkIfStrBufContainAtPosTheDouble(aStrBuf, nLen, input)
            );

        }

        void append_035()
        {
            ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
            double                 input = atof("-3.141592653589793238462643");

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[4] append -3.141592653589793238462643",
                checkIfStrBufContainAtPosTheDouble(aStrBuf, nLen, input)
            );

        }

        CPPUNIT_TEST_SUITE( append_009_Double_Negative );
        CPPUNIT_TEST( append_001 );
        CPPUNIT_TEST( append_035 );
        CPPUNIT_TEST_SUITE_END();
    };
} // namespace rtl_OStringBuffer

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::makeStringAndClear);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::getLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::getCapacity);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::ensureCapacity);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::setLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::csuc);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::getStr);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_001);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_002);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_003);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_004);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_005);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_006_Int32);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_006_Int32_Bounderies);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_006_Int32_Negative);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_006_Int32_WrongRadix);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_006_Int32_defaultParam);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64_Bounderies);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64_Negative);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64_WrongRadix);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64_defaultParam);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_008_float);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_008_Float_Negative);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_009_double);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_009_Double_Negative);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::remove);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
