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

#include <sal/types.h>
#include <rtl/string.hxx>
#include "rtl_String_Const.h"
#include <rtl/strbuf.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <string.h>

// This file contains cppunit tests for the
// OString and OStringBuffer classes

// testing constructors
namespace rtl_OStringBuffer
{
    class  ctors : public CppUnit::TestFixture
    {
    public:

        void ctor_001()
        {
            OStringBuffer aStrBuf;
            const char* pStr = aStrBuf.getStr();

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer containing no characters",
                aStrBuf.isEmpty()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer containing no characters",
                '\0', *pStr
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer containing no characters",
                sal_Int32(16), aStrBuf.getCapacity()
            );
        }

        void ctor_002()
        {
            OString       aStrtmp( kTestStr1 );
            OStringBuffer aStrBuftmp( aStrtmp );
            OStringBuffer aStrBuf( aStrBuftmp );
            // sal_Bool res = cmpstr(aStrBuftmp.getStr(),aStrBuf.getStr());

            sal_Int32 nLenStrBuftmp = aStrBuftmp.getLength();

            OString sStr(aStrBuftmp.getStr());
            bool res = aStrtmp == sStr;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer from another OStringBuffer",
                nLenStrBuftmp, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer from another OStringBuffer",
                aStrBuftmp.getCapacity(), aStrBuf.getCapacity()
            );
            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer from another OStringBuffer",
                res
            );

        }

        void ctor_003()
        {
            OStringBuffer aStrBuf1(kTestStr2Len);
            OStringBuffer aStrBuf2(0);

            const char* pStr1 = aStrBuf1.getStr();
            const char* pStr2 = aStrBuf2.getStr();

            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                aStrBuf1.isEmpty()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                '\0', *pStr1
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                kTestStr2Len, aStrBuf1.getCapacity()
            );
            CPPUNIT_ASSERT_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                aStrBuf2.isEmpty()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                '\0', *pStr2
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer containing no characters and contain assigned capacity",
                sal_Int32(0), aStrBuf2.getCapacity()
            );

        }

        void ctor_004()
        {
            OString aStrtmp( kTestStr1 );
            OStringBuffer aStrBuf( aStrtmp );
            sal_Int32 leg = aStrBuf.getLength();

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer from OString",
                aStrtmp, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer from OString",
                aStrtmp.pData->length, leg
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer from OString",
                leg+16, aStrBuf.getCapacity()

            );
        }

        void ctor_005() {
            OStringBuffer b1;
            auto dummy = b1.makeStringAndClear();
            (void)dummy;
            OStringBuffer b2(b1);
            (void)b2;
        }

        void ctor_006()
        {
            //pass in a const char*, get a temp
            //OString
            OStringBuffer aStrBuf(kTestStr1);
            sal_Int32 leg = aStrBuf.getLength();

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer from const char*",
                rtl_str_getLength(kTestStr1), leg
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "New OStringBuffer from const char*",
                leg+16, aStrBuf.getCapacity()
            );
        }

        CPPUNIT_TEST_SUITE(ctors);
        CPPUNIT_TEST(ctor_001);
        CPPUNIT_TEST(ctor_002);
        CPPUNIT_TEST(ctor_003);
        CPPUNIT_TEST(ctor_004);
        CPPUNIT_TEST(ctor_005);
        CPPUNIT_TEST(ctor_006);
        CPPUNIT_TEST_SUITE_END();
    };

    class  makeStringAndClear : public CppUnit::TestFixture
    {
        OString arrOUS[6];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr1 );
            arrOUS[1] = OString( kTestStr14 );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( kTestStr27 );
            arrOUS[4] = OString( kTestStr29 );
            arrOUS[5] = OString( "\0", 1 );

        }

        void makeStringAndClear_001()
        {
            OStringBuffer   aStrBuf1;

            bool lastRes = aStrBuf1.makeStringAndClear().isEmpty();

            CPPUNIT_ASSERT_MESSAGE
            (
                "two empty strings(def. constructor)",
                lastRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "two empty strings(def. constructor)",
                sal_Int32(0), aStrBuf1.getCapacity()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "two empty strings(def. constructor)",
                '\0', *(aStrBuf1.getStr())
            );

        }

        void makeStringAndClear_002()
        {
            OStringBuffer   aStrBuf2(26);

            bool lastRes = aStrBuf2.makeStringAndClear().isEmpty();

            CPPUNIT_ASSERT_MESSAGE
            (
                "two empty strings(with an argu)",
                lastRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "two empty strings(with an argu)",
                sal_Int32(0), aStrBuf2.getCapacity()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "two empty strings(with an argu)",
                '\0', *(aStrBuf2.getStr())
            );

        }

        void makeStringAndClear_003()
        {
            OStringBuffer   aStrBuf3(arrOUS[0]);
            OString        aStr3(arrOUS[0]);

            bool lastRes = (aStrBuf3.makeStringAndClear() == aStr3 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "normal string",
                lastRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "normal string",
                sal_Int32(0), aStrBuf3.getCapacity()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "normal string",
                '\0', *(aStrBuf3.getStr())
            );

        }

        void makeStringAndClear_004()
        {
            OStringBuffer   aStrBuf4(arrOUS[1]);
            OString         aStr4(arrOUS[1]);

            bool lastRes = (aStrBuf4.makeStringAndClear() ==  aStr4 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "string with space ",
                lastRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "string with space ",
                sal_Int32(0), aStrBuf4.getCapacity()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "string with space ",
                '\0', *(aStrBuf4.getStr())
            );
        }

        void makeStringAndClear_005()
        {
            OStringBuffer   aStrBuf5(arrOUS[2]);
            OString         aStr5(arrOUS[2]);

            bool lastRes = (aStrBuf5.makeStringAndClear() ==  aStr5 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "empty string",
                lastRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "empty string",
                sal_Int32(0), aStrBuf5.getCapacity()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "empty string",
                '\0', *(aStrBuf5.getStr())
            );
        }

        void makeStringAndClear_006()
        {
            OStringBuffer   aStrBuf6(arrOUS[3]);
            OString         aStr6(arrOUS[3]);

            bool lastRes = (aStrBuf6.makeStringAndClear() == aStr6 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "string with a character",
                lastRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "string with a character",
                sal_Int32(0), aStrBuf6.getCapacity()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "string with a character",
                '\0', *(aStrBuf6.getStr())
            );
        }

        void makeStringAndClear_007()
        {
            OStringBuffer   aStrBuf7(arrOUS[4]);
            OString         aStr7(arrOUS[4]);

            bool lastRes = (aStrBuf7.makeStringAndClear() == aStr7 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "string with special characters",
                lastRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "string with special characters",
                sal_Int32(0), aStrBuf7.getCapacity()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "string with special characters",
                '\0', *(aStrBuf7.getStr())
            );
        }

        void makeStringAndClear_008()
        {
            OStringBuffer   aStrBuf8(arrOUS[5]);
            OString         aStr8(arrOUS[5]);

            bool lastRes = (aStrBuf8.makeStringAndClear() == aStr8 );

            CPPUNIT_ASSERT_MESSAGE
            (
                "string only with (\0)",
                lastRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "string only with (\0)",
                sal_Int32(0), aStrBuf8.getCapacity()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "string only with (\0)",
                '\0', *(aStrBuf8.getStr())
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
        void remove_001()
        {
            OStringBuffer sb(
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

    class  getLength : public CppUnit::TestFixture
    {
        OString arrOUS[6];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr1 );
            arrOUS[1] = OString( "1" );
            arrOUS[2] = OString( );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( "\0", 1 );
            arrOUS[5] = OString( kTestStr2 );

        }

        void getLength_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            sal_Int32              expVal = kTestStr1Len;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "length of ascii string",
                expVal, aStrBuf.getLength()
            );

        }

        void getLength_002()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            sal_Int32              expVal = 1;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "length of ascii string of size 1",
                expVal, aStrBuf.getLength()
            );
        }

        void getLength_003()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "length of empty string",
                expVal, aStrBuf.getLength()
            );
        }

        void getLength_004()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "length of empty string (empty ascii string arg)",
                expVal, aStrBuf.getLength()
            );
        }

        void getLength_005()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            sal_Int32              expVal = 1;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "length of string with \\0 embedded",
                expVal, aStrBuf.getLength()
            );
        }

        void getLength_006()
        {
            OStringBuffer   aStrBuf( arrOUS[5] );
            sal_Int32              expVal = kTestStr2Len;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "length(>16) of ascii string",
                expVal, aStrBuf.getLength()
            );
        }

        void getLength_007()
        {
            OStringBuffer   aStrBuf;
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "length of empty string (default constructor)",
                expVal, aStrBuf.getLength()
            );
        }

        void getLength_008()
        {
            OStringBuffer   aStrBuf( 26 );
            sal_Int32               expVal   = 0;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "length of empty string (with capacity)",
                expVal, aStrBuf.getLength()
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

    class  getCapacity : public CppUnit::TestFixture
    {
        OString arrOUS[6];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr1 );
            arrOUS[1] = OString( "1" );
            arrOUS[2] = OString( );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( "\0", 1 );
            arrOUS[5] = OString( kTestStr2 );

        }

        void getCapacity_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            sal_Int32              expVal = kTestStr1Len+16;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of ascii string",
                expVal, aStrBuf.getCapacity()
            );

        }

        void getCapacity_002()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            sal_Int32              expVal = 1+16;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of ascii string of size 1",
                expVal, aStrBuf.getCapacity()
            );
        }

        void getCapacity_003()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            sal_Int32              expVal = 0+16;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of empty string",
                expVal, aStrBuf.getCapacity()
            );
        }

        void getCapacity_004()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            sal_Int32              expVal = 0+16;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of empty string (empty ascii string arg)",
                expVal, aStrBuf.getCapacity()
            );
        }

        void getCapacity_005()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            sal_Int32              expVal = 1+16;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of string with \\0 embedded",
                expVal, aStrBuf.getCapacity()
            );
        }

        void getCapacity_006()
        {
            OStringBuffer   aStrBuf( arrOUS[5] );
            sal_Int32              expVal = kTestStr2Len+16;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity(>16) of ascii string",
                expVal, aStrBuf.getCapacity()
            );
        }

        void getCapacity_007()
        {
            OStringBuffer   aStrBuf;
            sal_Int32              expVal = 16;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of empty string (default constructor)",
                expVal, aStrBuf.getCapacity()
            );
        }

        void getCapacity_010()
        {
            OStringBuffer   aStrBuf( 16 );
            sal_Int32              expVal = 16;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of empty string (with capacity 16)",
                expVal, aStrBuf.getCapacity()
            );
        }

        void getCapacity_011()
        {
            OStringBuffer   aStrBuf( 6 );
            sal_Int32              expVal = 6;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of empty string (with capacity 6)",
                expVal, aStrBuf.getCapacity()
            );
        }

        void getCapacity_012()
        {
            OStringBuffer   aStrBuf( 0 );
            sal_Int32              expVal = 0;

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity of empty string (with capacity 0)",
                expVal, aStrBuf.getCapacity()
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
        CPPUNIT_TEST( getCapacity_010 );
        CPPUNIT_TEST( getCapacity_011 );
        CPPUNIT_TEST( getCapacity_012 );
        CPPUNIT_TEST_SUITE_END();
    };

    class  ensureCapacity : public CppUnit::TestFixture
    {
        void ensureCapacity_001()
        {
            sal_Int32          expVal = 16;
            OStringBuffer   aStrBuf;
            sal_Int32              input = 5;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 16, minimum is 5",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_002()
        {
            sal_Int32          expVal = 16;
            OStringBuffer   aStrBuf;
            sal_Int32              input = -5;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 16, minimum is -5",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_003()
        {
            sal_Int32          expVal = 16;
            OStringBuffer   aStrBuf;
            sal_Int32              input = 0;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 16, minimum is 0",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_004()           //the testcase is based on comments
        {
            sal_Int32          expVal = 20;
            OStringBuffer   aStrBuf;
            sal_Int32              input = 20;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 16, minimum is 20",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_005()
        {
            sal_Int32          expVal = 50;
            OStringBuffer   aStrBuf;
            sal_Int32              input = 50;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 16, minimum is 50",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_006()
        {
            sal_Int32          expVal = 20;
            OStringBuffer   aStrBuf( 6 );
            sal_Int32              input = 20;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 6, minimum is 20",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_007()
        {
            sal_Int32          expVal = 6;
            OStringBuffer   aStrBuf( 6 );
            sal_Int32              input = 2;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 6, minimum is 2",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_008()
        {
            sal_Int32          expVal = 6;
            OStringBuffer   aStrBuf( 6 );
            sal_Int32              input = -6;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 6, minimum is -6",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_009()      //the testcase is based on comments
        {
            sal_Int32          expVal = 10;
            OStringBuffer   aStrBuf( 6 );
            sal_Int32              input = 10;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 6, minimum is -6",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_010()
        {
            sal_Int32          expVal = 6;
            OStringBuffer   aStrBuf( 0 );
            sal_Int32              input = 6;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 0, minimum is 6",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_011()       //the testcase is based on comments
        {
            sal_Int32          expVal = 2;  // capacity is x = (str->length + 1) * 2; minimum < x ? x : minimum
            OStringBuffer   aStrBuf( 0 );
            sal_Int32              input = 1;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 0, minimum is 1",
                expVal, aStrBuf.getCapacity()
            );

        }

        void ensureCapacity_012()
        {
            sal_Int32          expVal = 0;
            OStringBuffer   aStrBuf( 0 );
            sal_Int32              input = -1;

            aStrBuf.ensureCapacity( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "capacity equal to 0, minimum is -1",
                expVal, aStrBuf.getCapacity()
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
        CPPUNIT_TEST_SUITE_END();
    };

    class  setLength : public CppUnit::TestFixture
    {
        OString arrOUS[6];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr1 );
            arrOUS[1] = OString( "1" );
            arrOUS[2] = OString( );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( "\0", 1 );
            arrOUS[5] = OString( kTestStr2 );

        }

        void setLength_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            sal_Int32              expVal1 = 50;
            OString         expVal2( kTestStr1 );
            sal_Int32              expVal3 = 50;
            sal_Int32              input   = 50;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(kTestStr1)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(kTestStr1)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(kTestStr1)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            sal_Int32              expVal1 = kTestStr13Len;
            OString         expVal2( kTestStr1 );
            sal_Int32              expVal3 = 32;
            sal_Int32              input   = kTestStr13Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(kTestStr1)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(kTestStr1)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(kTestStr1)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            sal_Int32              expVal1 = kTestStr1Len;
            OString         expVal2( kTestStr1 );
            sal_Int32              expVal3 = 32;
            sal_Int32              input   = kTestStr1Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OStringBuffer(kTestStr1)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OStringBuffer(kTestStr1)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OStringBuffer(kTestStr1)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            sal_Int32              expVal1 = kTestStr7Len;
            OString         expVal2( kTestStr7 );
            sal_Int32              expVal3 = 32;
            sal_Int32              input   = kTestStr7Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer(kTestStr1)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer(kTestStr1)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer(kTestStr1)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            sal_Int32              expVal1 = 0;
            sal_Int32              expVal3 = 32;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to 0",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to 0",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to 0",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_006()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            sal_Int32              expVal1 = 25;
            OString         expVal2( arrOUS[1] );
            sal_Int32              expVal3 = 25;
            sal_Int32              input   = 25;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(1)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(1)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(1)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_007()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            sal_Int32              expVal1 = kTestStr27Len;
            OString         expVal2( arrOUS[1] );
            sal_Int32              expVal3 = 17;
            sal_Int32              input   = kTestStr27Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OStringBuffer(1)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OStringBuffer(1)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OStringBuffer(1)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_008()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            sal_Int32              expVal1 = 0;
            sal_Int32              expVal3 = 17;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OUStringBuffer(1)",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OUStringBuffer(1)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OUStringBuffer(1)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_009()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            sal_Int32              expVal1 = 20;
            sal_Int32              expVal3 = 20;
            sal_Int32              input   = 20;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer()",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer()",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer()",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_010()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            sal_Int32              expVal1 = 3;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 3;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer()",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer()",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer()",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_011()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            sal_Int32              expVal1 = 0;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer()",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer()",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer()",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_012()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            sal_Int32              expVal1 = 20;
            sal_Int32              expVal3 = 20;
            sal_Int32              input   = 20;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer("")",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer("")",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer("")",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_013()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            sal_Int32              expVal1 = 5;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 5;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer("")",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer("")",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer("")",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_014()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            sal_Int32              expVal1 = 0;
            sal_Int32              expVal3 = 16;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer("")",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer("")",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer("")",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_015()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            sal_Int32              expVal1 = 20;
            sal_Int32              expVal3 = 20;
            sal_Int32              input   = 20;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(\0)",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(\0)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(\0)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_016()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            sal_Int32              expVal1 = 5;
            sal_Int32              expVal3 = 17;
            sal_Int32              input   = 5;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(\0)",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(\0)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(\0)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_017()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            sal_Int32              expVal1 = 0;
            sal_Int32              expVal3 = 17;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer(\0)",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer(\0)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OStringBuffer(\0)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_018()
        {
            OStringBuffer   aStrBuf( arrOUS[5] );
            sal_Int32              expVal1 = 50;
            OString         expVal2( kTestStr2 );
            sal_Int32              expVal3 = 66;
            sal_Int32              input   = 50;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(kTestStr2)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(kTestStr2)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the capacity of OStringBuffer(kTestStr2)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_019()
        {
            OStringBuffer   aStrBuf( arrOUS[5] );
            sal_Int32              expVal1 = 40;
            OString         expVal2(kTestStr2);
            sal_Int32              expVal3 = 48;
            sal_Int32              input   = 40;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(kTestStr2)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(kTestStr2)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength more than the length of OStringBuffer(kTestStr2)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_020()
        {
            OStringBuffer   aStrBuf( arrOUS[5] );
            sal_Int32              expVal1 = kTestStr2Len;
            OString         expVal2(kTestStr2);
            sal_Int32              expVal3 = 48;
            sal_Int32              input   = kTestStr2Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OUStringBuffer(kTestStr2)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OUStringBuffer(kTestStr2)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to the length of OUStringBuffer(kTestStr2)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_021()
        {
            OStringBuffer   aStrBuf( arrOUS[5] );
            sal_Int32              expVal1 = kTestStr7Len;
            OString         expVal2(kTestStr7);
            sal_Int32              expVal3 = 48;
            sal_Int32              input   = kTestStr7Len;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OUStringBuffer(TestStr2)",
                expVal2, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OUStringBuffer(TestStr2)",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength less than the length of OUStringBuffer(TestStr2)",
                expVal3, aStrBuf.getCapacity()
            );

        }

        void setLength_022()
        {
            OStringBuffer   aStrBuf( arrOUS[5] );
            sal_Int32              expVal1 = 0;
            sal_Int32              expVal3 = 48;
            sal_Int32              input   = 0;

            aStrBuf.setLength( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to 0",
                '\0', aStrBuf.getStr()[0]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to 0",
                expVal1, aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "newLength equal to 0",
                expVal3, aStrBuf.getCapacity()
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

    class  csuc : public CppUnit::TestFixture
    {
        void csuc_001()
        {
            const char*        expVal = kTestStr1;
            OStringBuffer   aStrBuf( kTestStr1 );
            sal_Int32              cmpLen = kTestStr1Len;

            // LLA: wrong access! const char* pstr = *&aStrBuf;
            const char* pstr = aStrBuf.getStr();
            int nEqual = strncmp(pstr, expVal, cmpLen);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test normal string",
                /* cmpstr( pstr, expVal, cmpLen ) */
                0, nEqual
            );

        }

        void csuc_002()
        {
            OStringBuffer   aStrBuf;

            // LLA: wrong access! const char* pstr = *&aStrBuf;
            const char* pstr = aStrBuf.getStr();
            sal_Int32 nLen = strlen(pstr);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test empty string",
                // cmpstr( pstr, &expVal, cmpLen )
                static_cast<sal_Int32>(0), nLen
                );

        }

        CPPUNIT_TEST_SUITE( csuc );
        CPPUNIT_TEST( csuc_001 );
        CPPUNIT_TEST( csuc_002 );
        CPPUNIT_TEST_SUITE_END();
    };

    class  getStr : public CppUnit::TestFixture
    {
        void getStr_001()
        {
            const char*        expVal = kTestStr1;
            OStringBuffer   aStrBuf( kTestStr1 );
            sal_Int32              cmpLen = kTestStr1Len;

            const char* pstr = aStrBuf.getStr();
            int nEqual = strncmp(pstr, expVal, cmpLen);

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test normal string",
                0, nEqual
            );

        }

        void getStr_002()
        {
            OStringBuffer   aStrBuf;
            const char* pstr = aStrBuf.getStr();
            CPPUNIT_ASSERT_MESSAGE
            (
                "test empty string",
                pstr != nullptr
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "test empty string",
                '\0', pstr[0]
            );

        }

        CPPUNIT_TEST_SUITE( getStr );
        CPPUNIT_TEST( getStr_001 );
        CPPUNIT_TEST( getStr_002 );
        CPPUNIT_TEST_SUITE_END();
    };

    class  append_001 : public CppUnit::TestFixture
    {
        OString arrOUS[5];

        OString empty; // silence loplugin

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );

        }

        void append_001_001()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                input2( kTestStr8 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_002()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                input2( kTestStr36 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_003()
        {
            OString                expVal( kTestStr37 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                input2( kTestStr23 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_004()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[0] );

            aStrBuf.append( empty );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_005()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                input2( kTestStr7 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_006()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                input2( kTestStr2 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_007()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                input2( kTestStr1 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_008()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[1] );

            aStrBuf.append( empty );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_009()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                input2( kTestStr7 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_010()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                input2( kTestStr2 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_011()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                input2( kTestStr1 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_012()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[2] );

            aStrBuf.append( empty );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_013()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                input2( kTestStr7 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_014()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                input2( kTestStr2 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_015()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                input2( kTestStr1 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_016()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[3] );

            aStrBuf.append( empty );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_017()
        {
            OString                expVal( kTestStr29 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                input2( kTestStr38 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_018()
        {
            OString                expVal( kTestStr39 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                input2( kTestStr17 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_019()
        {
            OString                expVal( kTestStr40 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                input2( kTestStr31 );

            aStrBuf.append( input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_001_020()
        {
            OString                expVal( kTestStr28 );
            OStringBuffer   aStrBuf( arrOUS[4] );

            aStrBuf.append( empty );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_null()
        {
            OStringBuffer   aStrBuf("hello world");

            aStrBuf.append('\0');
            aStrBuf.append('\1');
            aStrBuf.append('\2');

            aStrBuf.append("hello world");

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "should be able to append nulls",
                sal_Int32(2 * RTL_CONSTASCII_LENGTH("hello world") + 3), aStrBuf.getLength()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "should be able to append nulls",
                '\0', aStrBuf[RTL_CONSTASCII_LENGTH("hello world")]
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "should be able to append nulls",
                1, aStrBuf[RTL_CONSTASCII_LENGTH("hello world")]+1
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "should be able to append nulls",
                2, aStrBuf[RTL_CONSTASCII_LENGTH("hello world")]+2
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

    class  append_002 : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );

        }

        void append_002_001()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            const char*        input = kTestStr8;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_002()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            const char*        input = kTestStr36;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_003()
        {
            OString                expVal( kTestStr37 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            const char*        input = kTestStr23;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_004()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            const char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_005()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            const char*        input = kTestStr7;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_006()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            const char*        input = kTestStr2;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_007()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            const char*        input = kTestStr1;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_008()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[1] );
            const char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_009()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            const char*        input = kTestStr7;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_010()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            const char*        input = kTestStr2;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_011()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            const char*        input = kTestStr1;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_012()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[2] );
            const char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_013()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            const char*        input = kTestStr7;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_014()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            const char*        input = kTestStr2;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_015()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            const char*        input = kTestStr1;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_016()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[3] );
            const char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_017()
        {
            OString                expVal( kTestStr29 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            const char*        input = kTestStr38;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_018()
        {
            OString                expVal( kTestStr39 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            const char*        input = kTestStr17;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_019()
        {
            OString                expVal( kTestStr40 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            const char*        input = kTestStr31;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002_020()
        {
            OString                expVal( kTestStr28 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            const char*        input = kTestStr25;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

#ifdef WITH_CORE
        void append_002_021()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( kSInt32Max );
            const char*        input = kTestStr25;

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

    class  append_003 : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );

        }

        void append_003_001()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            const char*        input1 = kTestStr36;
            sal_Int32              input2 = 12;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_002()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            const char*        input1 = kTestStr36;
            sal_Int32              input2 = 28;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_003()
        {
            OString                expVal( kTestStr37 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            const char*        input1 = kTestStr23;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_004()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_006()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 4;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_007()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 32;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_008()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_009()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[1] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_011()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 4;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_012()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 32;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_013()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_014()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[2] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_016()
        {
            OString                expVal( kTestStr7 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 4;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_017()
        {
            OString                expVal( kTestStr2 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 32;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_018()
        {
            OString                expVal( kTestStr1 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_019()
        {
            OString                expVal;
            OStringBuffer   aStrBuf( arrOUS[3] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_021()
        {
            OString                expVal( kTestStr29 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            const char*        input1 = kTestStr38;
            sal_Int32              input2 = 7;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length less than 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_022()
        {
            OString                expVal( kTestStr39 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            const char*        input1 = kTestStr17;
            sal_Int32              input2 = 22;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length more than 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_023()
        {
            OString                expVal( kTestStr40 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            const char*        input1 = kTestStr31;
            sal_Int32              input2 = 16;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003_024()
        {
            OString                expVal( kTestStr28 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            const char*        input1 = kTestStr2;
            sal_Int32              input2 = 0;

            aStrBuf.append( input1, input2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
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

    class  append_004 : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );

        }

        void append_004_001()
        {
            OString                expVal( kTestStr45 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            bool               input = true;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_002()
        {
            OString                expVal( kTestStr46 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            bool               input = false;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_003()
        {
            OString                expVal( kTestStr47 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            bool               input = true;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_004()
        {
            OString                expVal( kTestStr48 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            bool               input = false;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_005()
        {
            OString                expVal( kTestStr47 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            bool               input = true;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_006()
        {
            OString                expVal( kTestStr48 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            bool               input = false;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_007()
        {
            OString                expVal( kTestStr47 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            bool               input = true;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_008()
        {
            OString                expVal( kTestStr48 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            bool               input = false;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_009()
        {
            OString                expVal( kTestStr49 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            bool               input = true;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_True) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004_010()
        {
            OString                expVal( kTestStr50 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            bool               input = false;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Bool(sal_False) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

#ifdef WITH_CORE
        void append_004_011()
        {
            OString                expVal( kTestStr47 );
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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

// testing the method append(char c)

    class  append_005 : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );

        }

        void append_001()
        {
            OString                expVal( kTestStr51 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            char               input = 'M';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(M) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(M) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OString                expVal( kTestStr143 );
            OStringBuffer   aStrBuf( arrOUS[0] );
            char               input = static_cast<char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Unicode(kSInt8Max) to the string buffer arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the sal_Unicode(kSInt8Max) to the string buffer arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OString                expVal( kTestStr27 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(s) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(s) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OString                expVal( kTestStr144 );
            OStringBuffer   aStrBuf( arrOUS[1] );
            char               input = static_cast<char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005_005()
        {
            OString                expVal( kTestStr27 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(s) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(s) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OString                expVal( kTestStr144 );
            OStringBuffer   aStrBuf( arrOUS[2] );
            char               input = static_cast<char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OString                expVal( kTestStr27 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(s) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(s) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OString                expVal( kTestStr144 );
            OStringBuffer   aStrBuf( arrOUS[3] );
            char               input = static_cast<char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OString                expVal( kTestStr56 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(s) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(s) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OString                expVal( kTestStr145 );
            OStringBuffer   aStrBuf( arrOUS[4] );
            char               input = static_cast<char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

#ifdef WITH_CORE
        void append_011()
        {
            OString                expVal( kTestStr27 );
            OStringBuffer   aStrBuf( kSInt32Max );
            char               input = 's';

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the char(s) to the string buffer(with INT_MAX)",
                ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
            );

        }

        void append_012()
        {
            OString                expVal( kTestStr144 );
            OStringBuffer   aStrBuf( kSInt32Max );
            char               input = static_cast<char>(SAL_MAX_UINT8);

            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "Appends the char(kSInt8Max) to the string buffer with INT_MAX)",
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
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );

        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_011()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_012()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_013()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_014()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_015()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_016()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_017()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_018()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_019()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_020()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_021()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_022()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_023()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_024()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_025()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_026()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_027()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_028()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_029()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_030()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_031()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_032()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_033()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_034()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_035()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_036()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_037()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_038()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_039()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_040()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_041()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_042()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_043()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_044()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_045()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_046()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_047()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_048()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_049()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_050()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_051()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_052()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_053()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_054()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_055()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_056()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_057()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_058()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_059()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_060()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_061()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_062()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_063()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_064()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_065()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_066()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_067()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_068()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_069()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_070()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_071()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_072()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_073()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_074()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_075()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_076()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_077()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_078()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_079()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_080()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_081()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_082()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_083()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_084()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_085()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_086()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_087()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_088()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_089()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_090()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_091()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_092()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_093()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_094()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_095()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_096()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_097()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_098()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_099()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_100()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
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

    class  append_006_Int32_Bounderies : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_011()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_012()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_013()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_014()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_015()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_016()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_017()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_018()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_019()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_020()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_021()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_022()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_023()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_024()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_025()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_026()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_027()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_028()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_029()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_030()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_031()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_032()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_033()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_034()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_035()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_036()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_037()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_038()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_039()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_040()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_041()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_042()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_043()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_044()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 8;

            expVal += OString( "17777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_045()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_046()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 10;

            expVal += OString( "2147483647" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_047()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_048()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_049()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_050()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = kSInt32Max;
            sal_Int16              radix = 36;

            expVal += OString( "zik0zj" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
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

    class  append_006_Int32_Negative : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_011()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_012()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_013()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_014()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_015()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_016()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_017()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_018()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_019()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_020()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_021()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_022()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_023()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_024()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_025()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_026()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_027()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_028()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_029()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_030()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_031()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_032()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_033()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_034()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_035()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_036()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_037()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_038()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_039()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_040()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_041()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_042()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_043()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_044()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_045()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_046()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_047()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_048()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_049()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_050()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_051()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_052()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_053()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_054()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_055()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_056()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_057()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_058()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_059()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_060()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_061()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_062()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_063()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_064()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_065()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_066()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_067()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_068()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_069()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_070()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_071()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_072()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_073()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_074()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_075()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_076()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_077()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_078()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_079()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_080()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_081()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_082()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_083()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_084()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_085()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_086()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_087()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_088()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_089()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_090()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_091()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_092()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_093()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_094()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_095()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_096()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_097()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_098()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );
        }

        void append_099()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );
        }

        void append_100()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int32              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
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

    class  append_006_Int32_defaultParam : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr59 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[0]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[0]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr62 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[0]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[0]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr63 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[0]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[0]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr64 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[0]+2147483647",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[0]+2147483647",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr65 );
            sal_Int32              input = kNonSInt32Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[0]+(-2147483648)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[0]+(-2147483648)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr60 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[1]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[1]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr66 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[1]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[1]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr67 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[1]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[1]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr68 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[1]+2147483647",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[1]+2147483647",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr69 );
            sal_Int32              input = SAL_MIN_INT32 /*-2147483648*/;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[1]+(-2147483648)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[1]+(-2147483648)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_011()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr60 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[2]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[2]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_012()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr66 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OUStringBuffer[2]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OUStringBuffer[2]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_013()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr67 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OUStringBuffer[2]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OUStringBuffer[2]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_014()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr68 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[2]+2147483647",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[2]+2147483647",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_015()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr69 );
            sal_Int32              input = SAL_MIN_INT32;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[2]+(-2147483648)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[2]+(-2147483648)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_016()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr60 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[3]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[3]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_017()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr66 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[3]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[3]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_018()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr67 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[3]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[3]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_019()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr68 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[3]+2147483647",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[3]+2147483647",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_020()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr69 );
            sal_Int32              input = SAL_MIN_INT32;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[3]+(-2147483648)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[3]+(-2147483648)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_021()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr61 );
            sal_Int32              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[4]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 11 and return OStringBuffer[4]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_022()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr70 );
            sal_Int32              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[4]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 0 and return OStringBuffer[4]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_023()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr71 );
            sal_Int32              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[4]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -11 and return OStringBuffer[4]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_024()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr72 );
            sal_Int32              input = 2147483647;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[4]+2147483647",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 2147483647 and return OStringBuffer[4]+2147483647",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_025()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr73 );
            sal_Int32              input = SAL_MIN_INT32;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[4]+(-2147483648)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int32 -2147483648 and return OStringBuffer[4]+(-2147483648)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }
#ifdef WITH_CORE
        void append_026()
        {
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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

// testing the method append( sal_Int64 l, sal_Int16 radix=2 )
// testing the method append( sal_Int64 l, sal_Int16 radix=8 )
// testing the method append( sal_Int64 l, sal_Int16 radix=10 )
// testing the method append( sal_Int64 l, sal_Int16 radix=16 )
// testing the method append( sal_Int64 l, sal_Int16 radix=36 )

    class  append_007_Int64 : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64             input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_011()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_012()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_013()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_014()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_015()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_016()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_017()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_018()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_019()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_020()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_021()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_022()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_023()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_024()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_025()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_026()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_027()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_028()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_029()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_030()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_031()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_032()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_033()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_034()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_035()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_036()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_037()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_038()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_039()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_040()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_041()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_042()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_043()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_044()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_045()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_046()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_047()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_048()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_049()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_050()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_051()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_052()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_053()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_054()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_055()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_056()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_057()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_058()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_059()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_060()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_061()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_062()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_063()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_064()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_065()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_066()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_067()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_068()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_069()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_070()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_071()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_072()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_073()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_074()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_075()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_076()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_077()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_078()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_079()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_080()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_081()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_082()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 2;

            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_083()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 2;

            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_084()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 2;

            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_085()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_086()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 8;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_087()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 8;

            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_088()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 8;

            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_089()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_090()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 10;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_091()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 10;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_092()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 10;

            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_093()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_094()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 16;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_095()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 16;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_096()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 15;
            sal_Int16              radix = 16;

            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_097()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_098()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 4;
            sal_Int16              radix = 36;

            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_099()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 8;
            sal_Int16              radix = 36;

            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_100()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = 35;
            sal_Int16              radix = 36;

            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
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

    class  append_007_Int64_Bounderies : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64             input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_011()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_012()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_013()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_014()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_015()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_016()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_017()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_018()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_019()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_020()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_021()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_022()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_023()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_024()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_025()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_026()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_027()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_028()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_029()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_030()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_031()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_032()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_033()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_034()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_035()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_036()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_037()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_038()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_039()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_040()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_041()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 2;

            expVal += OString( "1111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_042()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 2;

            expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_043()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 8;

            expVal += OString( "177" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_044()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 8;

            expVal += OString( "777777777777777777777" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_045()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 10;

            expVal += OString( "127" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_046()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 10;

            expVal += OString( "9223372036854775807" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_047()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 16;

            expVal += OString( "7f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_048()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 16;

            expVal += OString( "7fffffffffffffff" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_049()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt8Max;
            sal_Int16              radix = 36;

            expVal += OString( "3j" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_050()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = kSInt64Max;
            sal_Int16              radix = 36;

            expVal += OString( "1y2p0ij32e8e7" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
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

    class  append_007_Int64_Negative : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_011()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_012()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_013()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_014()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_015()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_016()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_017()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_018()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_019()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_020()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_021()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_022()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_023()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_024()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_025()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_026()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_027()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_028()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_029()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_030()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_031()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_032()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_033()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_034()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_035()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_036()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_037()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_038()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_039()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_040()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_041()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_042()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_043()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_044()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_045()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_046()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_047()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_048()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_049()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_050()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_051()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_052()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_053()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_054()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_055()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_056()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_057()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_058()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_059()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_060()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_061()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_062()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_063()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_064()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_065()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_066()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_067()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_068()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_069()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_070()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_071()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_072()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_073()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_074()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_075()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_076()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_077()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_078()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_079()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_080()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_081()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 2;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_082()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "100" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_083()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1000" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_084()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 2;

            expVal += OString( "-" );
            expVal += OString( "1111" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_085()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 8;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_086()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_087()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "10" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_088()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 8;

            expVal += OString( "-" );
            expVal += OString( "17" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_089()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 10;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_090()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_091()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_092()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 10;

            expVal += OString( "-" );
            expVal += OString( "15" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_093()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 16;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_094()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_095()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_096()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -15;
            sal_Int16              radix = 16;

            expVal += OString( "-" );
            expVal += OString( "f" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_097()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -0;
            sal_Int16              radix = 36;

            expVal += OString( "0" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_098()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -4;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "4" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_099()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -8;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "8" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_100()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( aStrBuf.getStr() );
            sal_Int64              input = -35;
            sal_Int16              radix = 36;

            expVal += OString( "-" );
            expVal += OString( "z" );
            aStrBuf.append( input, radix );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal, OString(aStrBuf.getStr())
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]",
                expVal.getLength(), aStrBuf.getLength()
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

    class  append_007_Int64_defaultParam : public CppUnit::TestFixture
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr59 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[0]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[0]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_002()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr62 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[0]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[0]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_003()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr63 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[0]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[0]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_004()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr116 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[0]+9223372036854775807",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[0]+9223372036854775807",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_005()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            OString                expVal( kTestStr117 );
            sal_Int64              input = SAL_MIN_INT64/*-9223372036854775808*/; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            bool bRes = expVal == aStrBuf.getStr();
            CPPUNIT_ASSERT_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[0]+(-9223372036854775808)",
                bRes
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[0]+(-9223372036854775808)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_006()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr60 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[1]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[1]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_007()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr66 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[1]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[1]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_008()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr67 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[1]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[1]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_009()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr118 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[1]+9223372036854775807",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[1]+9223372036854775807",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_010()
        {
            OStringBuffer   aStrBuf( arrOUS[1] );
            OString                expVal( kTestStr119 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[1]+(-9223372036854775808)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[1]+(-9223372036854775808)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_011()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr60 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[2]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[2]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_012()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr66 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OUStringBuffer[2]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OUStringBuffer[2]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_013()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr67 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OUStringBuffer[2]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OUStringBuffer[2]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_014()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr118 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[2]+9223372036854775807",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[2]+9223372036854775807",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_015()
        {
            OStringBuffer   aStrBuf( arrOUS[2] );
            OString                expVal( kTestStr119 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[2]+(-9223372036854775808)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[2]+(-9223372036854775808)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_016()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr60 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[3]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[3]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_017()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr66 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[3]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[3]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_018()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr67 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[3]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[3]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_019()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr118 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[3]+9223372036854775807",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[3]+9223372036854775807",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_020()
        {
            OStringBuffer   aStrBuf( arrOUS[3] );
            OString                expVal( kTestStr119 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[3]+(-9223372036854775808)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[3]+(-9223372036854775808)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_021()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr61 );
            sal_Int64              input = 11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[4]+11",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 11 and return OStringBuffer[4]+11",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_022()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr70 );
            sal_Int64              input = 0;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[4]+0",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 0 and return OStringBuffer[4]+0",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_023()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr71 );
            sal_Int64              input = -11;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[4]+(-11)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -11 and return OStringBuffer[4]+(-11)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_024()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr120 );
            sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[4]+9223372036854775807",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 9223372036854775807 and return OStringBuffer[4]+9223372036854775807",
                expVal.getLength(), aStrBuf.getLength()
            );

        }

        void append_025()
        {
            OStringBuffer   aStrBuf( arrOUS[4] );
            OString                expVal( kTestStr121 );
            sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

            aStrBuf.append( input );

            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[4]+(-9223372036854775808)",
                expVal, aStrBuf.toString()
            );
            CPPUNIT_ASSERT_EQUAL_MESSAGE
            (
                "input Int64 -9223372036854775808 and return OStringBuffer[4]+(-9223372036854775808)",
                expVal.getLength(), aStrBuf.getLength()
            );

        }
#ifdef WITH_CORE
        void append_026()
        {
            OStringBuffer   aStrBuf( kSInt64Max );
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
            OStringBuffer   aStrBuf( kSInt64Max );
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
            OStringBuffer   aStrBuf( kSInt64Max );
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
            OStringBuffer   aStrBuf( kSInt64Max );
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
            OStringBuffer   aStrBuf( kSInt64Max );
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

// testing the method append( float f )

    class checkfloat : public CppUnit::TestFixture
    {
    public:
        bool checkIfStrBufContainAtPosTheFloat(OStringBuffer const& _sStrBuf, sal_Int32 _nLen, float _nFloat)
            {
                OString sFloatValue = OString::number(_nFloat);

                OString sBufferString(_sStrBuf.getStr());
                sal_Int32 nPos = sBufferString.indexOf(sFloatValue);
                return nPos >= 0 && nPos == _nLen;
            }
    };

    class  append_008_float : public checkfloat
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("3.0"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("3.5"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("3.0625"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("3.502525"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("3.141592"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("3.5025255"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("3.00390625"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("3.0"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("3.5"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("3.0625"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("3.502525"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("3.141592"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("3.5025255"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("3.00390625"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("3.0"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("3.5"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("3.0625"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("3.502525"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("3.141592"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("3.5025255"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("3.00390625"));

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
            OStringBuffer   aStrBuf( arrOUS[3] );
            float                  input = static_cast<float>(atof("3.0"));

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
            OStringBuffer   aStrBuf( arrOUS[3] );
            float                  input = static_cast<float>(atof("3.5"));

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
            OStringBuffer   aStrBuf( arrOUS[3] );
            float                  input = static_cast<float>(atof("3.0625"));

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
            OStringBuffer   aStrBuf( arrOUS[3] );
            float                  input = static_cast<float>(atof("3.502525"));

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append 3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

#ifdef WITH_CORE
        void append_036()
        {
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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

// testing the method append( float f ) for negative value

    class  append_008_Float_Negative : public checkfloat
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("-3.0"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("-3.5"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("-3.0625"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("-3.502525"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("-3.141592"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("-3.5025255"));

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
            OStringBuffer   aStrBuf( arrOUS[0] );
            float                  input = static_cast<float>(atof("-3.00390625"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("-3.0"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("-3.5"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("-3.0625"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("-3.502525"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("-3.141592"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("-3.5025255"));

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
            OStringBuffer   aStrBuf( arrOUS[1] );
            float                  input = static_cast<float>(atof("-3.00390625"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("-3.0"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("-3.5"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("-3.0625"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("-3.502525"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("-3.141592"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("-3.5025255"));

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
            OStringBuffer   aStrBuf( arrOUS[2] );
            float                  input = static_cast<float>(atof("-3.00390625"));

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
            OStringBuffer   aStrBuf( arrOUS[3] );
            float                  input = static_cast<float>(atof("-3.0"));

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
            OStringBuffer   aStrBuf( arrOUS[3] );
            float                  input = static_cast<float>(atof("-3.5"));

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
            OStringBuffer   aStrBuf( arrOUS[3] );
            float                  input = static_cast<float>(atof("-3.0625"));

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
            OStringBuffer   aStrBuf( arrOUS[3] );
            float                  input = static_cast<float>(atof("-3.502525"));

            sal_Int32 nLen = aStrBuf.getLength();
            aStrBuf.append( input );

            CPPUNIT_ASSERT_MESSAGE
            (
                "arrOUS[3] append -3.502525",
                checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)
            );

        }

#ifdef WITH_CORE
        void append_036()
        {
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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
            OStringBuffer   aStrBuf( kSInt32Max );
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

// testing the method append( double d )

    class checkdouble : public CppUnit::TestFixture
    {
    public:
        bool checkIfStrBufContainAtPosTheDouble(OStringBuffer const& _sStrBuf, sal_Int32 _nLen, double _nDouble)
            {
                OString sDoubleValue = OString::number(_nDouble);

                OString sBufferString(_sStrBuf.getStr());
                sal_Int32 nPos = sBufferString.indexOf(sDoubleValue);
                return nPos >= 0 && nPos == _nLen;
            }
    };

    class  append_009_double : public checkdouble
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
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
            OStringBuffer   aStrBuf( arrOUS[4] );
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

// testing the method append( double f ) for negative value

    class  append_009_Double_Negative : public checkdouble
    {
        OString arrOUS[5];

    public:
        void setUp() override
        {
            arrOUS[0] = OString( kTestStr7 );
            arrOUS[1] = OString(  );
            arrOUS[2] = OString( kTestStr25 );
            arrOUS[3] = OString( "" );
            arrOUS[4] = OString( kTestStr28 );
        }

        void append_001()
        {
            OStringBuffer   aStrBuf( arrOUS[0] );
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
            OStringBuffer   aStrBuf( arrOUS[4] );
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

    class AppendUninitialized: public CppUnit::TestFixture {
    private:
        void testEmpty();

        void testNonEmpty();

        void testZero();

        CPPUNIT_TEST_SUITE(AppendUninitialized);
        CPPUNIT_TEST(testEmpty);
        CPPUNIT_TEST(testNonEmpty);
        CPPUNIT_TEST(testZero);
        CPPUNIT_TEST_SUITE_END();
    };

    void AppendUninitialized::testEmpty() {
        OStringBuffer s;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s.getLength());
        char * p = s.appendUninitialized(5);
        CPPUNIT_ASSERT_EQUAL(
            static_cast<void const *>(s.getStr()),
            static_cast<void const *>(p));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), s.getLength());
    }

    void AppendUninitialized::testNonEmpty() {
        OStringBuffer s("ab");
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), s.getLength());
        char * p = s.appendUninitialized(5);
        CPPUNIT_ASSERT_EQUAL(
            static_cast<void const *>(s.getStr() + 2),
            static_cast<void const *>(p));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), s.getLength());
    }

    void AppendUninitialized::testZero() {
        OStringBuffer s;
        char * p = s.appendUninitialized(0);
        CPPUNIT_ASSERT_EQUAL(
            static_cast<void const *>(s.getStr()),
            static_cast<void const *>(p));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s.getLength());
    }
} // namespace rtl_OStringBuffer

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
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_006_Int32_defaultParam);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64_Bounderies);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64_Negative);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_007_Int64_defaultParam);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_008_float);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_008_Float_Negative);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_009_double);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::append_009_Double_Negative);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::AppendUninitialized);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OStringBuffer::remove);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
