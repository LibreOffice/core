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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

// -----------------------------------------------------------------------------
#include <rtl/string.hxx>
#include <rtl_String_Const.h>
#include <rtl_String_Utils.hxx>

#include <rtl/strbuf.hxx>

#include "gtest/gtest.h"

#include <string.h>

using namespace rtl;

//------------------------------------------------------------------------
// test classes
//------------------------------------------------------------------------
// const MAXBUFLENGTH = 255;
//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// testing constructors
//------------------------------------------------------------------------

// LLA: there exist some #if WITH_CORE #endif envelopes, which contain test code, which will core dump
//      due to the fact, that we can't handle MAXINT32 right.

namespace rtl_OStringBuffer
{
    class  ctors : public ::testing::Test
    {
    public:
    };

    TEST_F(ctors, ctor_001)
    {
        ::rtl::OStringBuffer aStrBuf;
        const sal_Char* pStr = aStrBuf.getStr();

        ASSERT_TRUE(aStrBuf.getLength() == 0 &&
            *pStr == '\0' && aStrBuf.getCapacity() == 16) << "New OStringBuffer containing no characters";
    }

    TEST_F(ctors, ctor_002)
    {
        ::rtl::OString       aStrtmp( kTestStr1 );
        ::rtl::OStringBuffer aStrBuftmp( aStrtmp );
        ::rtl::OStringBuffer aStrBuf( aStrBuftmp );
        // sal_Bool res = cmpstr(aStrBuftmp.getStr(),aStrBuf.getStr());

        sal_Int32 nLenStrBuftmp = aStrBuftmp.getLength();

        rtl::OString sStr(aStrBuftmp.getStr());
        sal_Bool res = aStrtmp.equals( sStr );

        ASSERT_TRUE(aStrBuf.getLength() == nLenStrBuftmp &&
            aStrBuf.getCapacity() == aStrBuftmp.getCapacity() &&
            res) << "New OStringBuffer from another OStringBuffer";

    }

    TEST_F(ctors, ctor_003)
    {
        ::rtl::OStringBuffer aStrBuf1(kTestStr2Len);
#ifdef WITH_CORE
        ::rtl::OStringBuffer aStrBuf2(kSInt32Max);     //will core dump
        // LLA: will core, due to the fact, that ksint32max is too big, the max length can't
        //      use, because there are some internal bytes, which we can't calculate.

#else
        ::rtl::OStringBuffer aStrBuf2(0);
#endif

        const sal_Char* pStr1 = aStrBuf1.getStr();
        const sal_Char* pStr2 = aStrBuf2.getStr();

#ifdef WITH_CORE
        ASSERT_TRUE(aStrBuf1.getLength() == 0 &&
            ! *(aStrBuf1.getStr()) && aStrBuf1.getCapacity() == kTestStr2Len &&
            aStrBuf2.getLength() == 0 &&
            ! *(aStrBuf2.getStr()) && aStrBuf2.getCapacity() == kSInt32Max) << "New OStringBuffer containing no characters and contain assigned capacity";
#else
        ASSERT_TRUE(aStrBuf1.getLength() == 0 &&
            *pStr1 == '\0' &&
            aStrBuf1.getCapacity() == kTestStr2Len &&
            aStrBuf2.getLength() == 0 &&
            *pStr2 == '\0' &&
            aStrBuf2.getCapacity() == 0) << "New OStringBuffer containing no characters and contain assigned capacity";
#endif

    }

    TEST_F(ctors, ctor_003_1)
    {
        // LLA: StringBuffer with created negativ size are the same as empty StringBuffers
        ::rtl::OStringBuffer aStrBuf3(kNonSInt32Max);

        const sal_Char* pStr = aStrBuf3.getStr();

        ASSERT_TRUE(aStrBuf3.getLength() == 0 &&
            *pStr == '\0' &&
            aStrBuf3.getCapacity() == kNonSInt32Max) << "New OStringBuffer containing no characters and contain assigned capacity";
    }

    TEST_F(ctors, ctor_004)
    {
        ::rtl::OString aStrtmp( kTestStr1 );
        ::rtl::OStringBuffer aStrBuf( aStrtmp );
        sal_Int32 leg = aStrBuf.getLength();

        ASSERT_TRUE(aStrBuf.getStr() == aStrtmp &&
            leg == aStrtmp.pData->length &&
            aStrBuf.getCapacity() == leg+16) << "New OStringBuffer from Ostring";
    }

    TEST_F(ctors, ctor_005) {
        rtl::OStringBuffer b1;
        b1.makeStringAndClear();
        rtl::OStringBuffer b2(b1);
    }

// -----------------------------------------------------------------------------

    class  makeStringAndClear : public ::testing::Test
    {
    protected:
        OString* arrOUS[6];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( kTestStr14 );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( kTestStr27 );
            arrOUS[4] = new OString( kTestStr29 );
            arrOUS[5] = new OString( "\0" );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4]; delete arrOUS[5];
        }
    };

    TEST_F(makeStringAndClear, makeStringAndClear_001)
    {
        ::rtl::OStringBuffer   aStrBuf1;
        ::rtl::OString         aStr1;

        sal_Bool lastRes = (aStrBuf1.makeStringAndClear() ==  aStr1 );

        ASSERT_TRUE(lastRes && ( aStrBuf1.getCapacity() == 0 ) &&
                    ( *(aStrBuf1.getStr()) == '\0' )) << "two empty strings(def. constructor)";

    }

    TEST_F(makeStringAndClear, makeStringAndClear_002)
    {
        ::rtl::OStringBuffer   aStrBuf2(26);
        ::rtl::OString         aStr2;

        sal_Bool lastRes = (aStrBuf2.makeStringAndClear() == aStr2 );

        ASSERT_TRUE(lastRes && ( aStrBuf2.getCapacity() == 0 ) &&
                    ( *(aStrBuf2.getStr()) == '\0' )) << "two empty strings(with a argu)";

    }

    TEST_F(makeStringAndClear, makeStringAndClear_003)
    {
        ::rtl::OStringBuffer   aStrBuf3(*arrOUS[0]);
        ::rtl::OString        aStr3(*arrOUS[0]);

        sal_Bool lastRes = (aStrBuf3.makeStringAndClear() == aStr3 );

        ASSERT_TRUE(lastRes && ( aStrBuf3.getCapacity() == 0 ) &&
                    ( *(aStrBuf3.getStr()) == '\0' )) << "normal string";

    }

    TEST_F(makeStringAndClear, makeStringAndClear_004)
    {
        ::rtl::OStringBuffer   aStrBuf4(*arrOUS[1]);
        ::rtl::OString         aStr4(*arrOUS[1]);

        sal_Bool lastRes = (aStrBuf4.makeStringAndClear() ==  aStr4 );

        ASSERT_TRUE(lastRes && ( aStrBuf4.getCapacity() == 0 ) &&
                    ( *(aStrBuf4.getStr()) == '\0' )) << "string with space ";
    }

    TEST_F(makeStringAndClear, makeStringAndClear_005)
    {
        ::rtl::OStringBuffer   aStrBuf5(*arrOUS[2]);
        ::rtl::OString         aStr5(*arrOUS[2]);

        sal_Bool lastRes = (aStrBuf5.makeStringAndClear() ==  aStr5 );

        ASSERT_TRUE(lastRes && ( aStrBuf5.getCapacity() == 0 ) &&
                    ( *(aStrBuf5.getStr()) == '\0' )) << "empty string";
    }

    TEST_F(makeStringAndClear, makeStringAndClear_006)
    {
        ::rtl::OStringBuffer   aStrBuf6(*arrOUS[3]);
        ::rtl::OString         aStr6(*arrOUS[3]);

        sal_Bool lastRes = (aStrBuf6.makeStringAndClear() == aStr6 );

        ASSERT_TRUE(lastRes && ( aStrBuf6.getCapacity() == 0 ) &&
                    ( *(aStrBuf6.getStr()) == '\0' )) << "string with a character";
    }

    TEST_F(makeStringAndClear, makeStringAndClear_007)
    {
        ::rtl::OStringBuffer   aStrBuf7(*arrOUS[4]);
        ::rtl::OString         aStr7(*arrOUS[4]);

        sal_Bool lastRes = (aStrBuf7.makeStringAndClear() == aStr7 );

        ASSERT_TRUE(lastRes && ( aStrBuf7.getCapacity() == 0 ) &&
                    ( *(aStrBuf7.getStr()) == '\0' )) << "string with special characters";
    }

    TEST_F(makeStringAndClear, makeStringAndClear_008)
    {
        ::rtl::OStringBuffer   aStrBuf8(*arrOUS[5]);
        ::rtl::OString         aStr8(*arrOUS[5]);

        sal_Bool lastRes = (aStrBuf8.makeStringAndClear() == aStr8 );

        ASSERT_TRUE(lastRes && ( aStrBuf8.getCapacity() == 0 ) &&
                    ( *(aStrBuf8.getStr()) == '\0' )) << "string only with (\0)";
    }

// -----------------------------------------------------------------------------

    class  getLength : public ::testing::Test
    {
    protected:
        OString* arrOUS[6];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( "1" );
            arrOUS[2] = new OString( );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( "\0" );
            arrOUS[5] = new OString( kTestStr2 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4]; delete arrOUS[5];
        }
    };

    TEST_F(getLength, getLength_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              expVal = kTestStr1Len;

        ASSERT_TRUE(aStrBuf.getLength() == expVal) << "length of ascii string";

    }

    TEST_F(getLength, getLength_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Int32              expVal = 1;

        ASSERT_TRUE(aStrBuf.getLength() == expVal) << "length of ascci string of size 1";
    }

    TEST_F(getLength, getLength_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              expVal = 0;

        ASSERT_TRUE(aStrBuf.getLength() == expVal) << "length of empty string";
    }

    TEST_F(getLength, getLength_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Int32              expVal = 0;

        ASSERT_TRUE(aStrBuf.getLength() == expVal) << "length of empty string (empty ascii string arg)";
    }

    TEST_F(getLength, getLength_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Int32              expVal = 0;

        ASSERT_TRUE(aStrBuf.getLength() == expVal) << "length of empty string (string arg = '\\0')";
    }

    TEST_F(getLength, getLength_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
        sal_Int32              expVal = kTestStr2Len;

        ASSERT_TRUE(aStrBuf.getLength() == expVal) << "length(>16) of ascii string";
    }

    TEST_F(getLength, getLength_007)
    {
        ::rtl::OStringBuffer   aStrBuf;
        sal_Int32              expVal = 0;

        ASSERT_TRUE(aStrBuf.getLength()== expVal) << "length of empty string (default constructor)";
    }

    TEST_F(getLength, getLength_008)
    {
        ::rtl::OStringBuffer   aStrBuf( 26 );
        sal_Int32               expVal   = 0;

        ASSERT_TRUE(aStrBuf.getLength()== expVal) << "length of empty string (with capacity)";
    }

// -----------------------------------------------------------------------------

    class  getCapacity : public ::testing::Test
    {
    protected:
        OString* arrOUS[6];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( "1" );
            arrOUS[2] = new OString( );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( "\0" );
            arrOUS[5] = new OString( kTestStr2 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4]; delete arrOUS[5];
        }
    };

    TEST_F(getCapacity, getCapacity_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              expVal = kTestStr1Len+16;

        ASSERT_TRUE(aStrBuf.getCapacity()== expVal) << "capacity of ascii string";

    }

    TEST_F(getCapacity, getCapacity_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Int32              expVal = 1+16;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of ascci string of size 1";
    }

    TEST_F(getCapacity, getCapacity_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              expVal = 0+16;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string";
    }

    TEST_F(getCapacity, getCapacity_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Int32              expVal = 0+16;

        ASSERT_TRUE(aStrBuf.getCapacity()== expVal) << "capacity of empty string (empty ascii string arg)";
    }

    TEST_F(getCapacity, getCapacity_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Int32              expVal = 0+16;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string (string arg = '\\0')";
    }

    TEST_F(getCapacity, getCapacity_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
        sal_Int32              expVal = kTestStr2Len+16;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity(>16) of ascii string";
    }

    TEST_F(getCapacity, getCapacity_007)
    {
        ::rtl::OStringBuffer   aStrBuf;
        sal_Int32              expVal = 16;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string (default constructor)";
    }
#ifdef WITH_CORE
    TEST_F(getCapacity, getCapacity_008)
    {
        ::rtl::OStringBuffer   aStrBuf ( kSInt32Max );
        sal_Int32              expVal = kSInt32Max;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string (with capacity 2147483647)(code will core dump)";
    }
#endif
    TEST_F(getCapacity, getCapacity_009)
    {
        ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
        sal_Int32              expVal = kNonSInt32Max;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string (with capacity -2147483648)";
    }

    TEST_F(getCapacity, getCapacity_010)
    {
        ::rtl::OStringBuffer   aStrBuf( 16 );
        sal_Int32              expVal = 16;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string (with capacity 16)";
    }

    TEST_F(getCapacity, getCapacity_011)
    {
        ::rtl::OStringBuffer   aStrBuf( 6 );
        sal_Int32              expVal = 6;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string (with capacity 6)";
    }

    TEST_F(getCapacity, getCapacity_012)
    {
        ::rtl::OStringBuffer   aStrBuf( 0 );
        sal_Int32              expVal = 0;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string (with capacity 0)";
    }

    TEST_F(getCapacity, getCapacity_013)
    {
        ::rtl::OStringBuffer   aStrBuf( -2 );
        sal_Int32              expVal = -2;

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity of empty string (with capacity -2)";
    }

// -----------------------------------------------------------------------------

    class  ensureCapacity : public ::testing::Test
    {
    };

    TEST_F(ensureCapacity, ensureCapacity_001)
    {
        sal_Int32          expVal = 16;
        ::rtl::OStringBuffer   aStrBuf;
        sal_Int32              input = 5;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 16, minimum is 5";

    }

    TEST_F(ensureCapacity, ensureCapacity_002)
    {
        sal_Int32          expVal = 16;
        ::rtl::OStringBuffer   aStrBuf;
        sal_Int32              input = -5;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 16, minimum is -5";

    }

    TEST_F(ensureCapacity, ensureCapacity_003)
    {
        sal_Int32          expVal = 16;
        ::rtl::OStringBuffer   aStrBuf;
        sal_Int32              input = 0;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 16, minimum is 0";

    }

    TEST_F(ensureCapacity, ensureCapacity_004)           //the testcase is based on comments
    {
        sal_Int32          expVal = 20;
        ::rtl::OStringBuffer   aStrBuf;
        sal_Int32              input = 20;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 16, minimum is 20";

    }

    TEST_F(ensureCapacity, ensureCapacity_005)
    {
        sal_Int32          expVal = 50;
        ::rtl::OStringBuffer   aStrBuf;
        sal_Int32              input = 50;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 16, minimum is 50";

    }

    TEST_F(ensureCapacity, ensureCapacity_006)
    {
        sal_Int32          expVal = 20;
        ::rtl::OStringBuffer   aStrBuf( 6 );
        sal_Int32              input = 20;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 6, minimum is 20";

    }

    TEST_F(ensureCapacity, ensureCapacity_007)
    {
        sal_Int32          expVal = 6;
        ::rtl::OStringBuffer   aStrBuf( 6 );
        sal_Int32              input = 2;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 6, minimum is 2";

    }

    TEST_F(ensureCapacity, ensureCapacity_008)
    {
        sal_Int32          expVal = 6;
        ::rtl::OStringBuffer   aStrBuf( 6 );
        sal_Int32              input = -6;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 6, minimum is -6";

    }

    TEST_F(ensureCapacity, ensureCapacity_009)      //the testcase is based on comments
    {
        sal_Int32          expVal = 10;
        ::rtl::OStringBuffer   aStrBuf( 6 );
        sal_Int32              input = 10;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 6, minimum is -6";

    }

    TEST_F(ensureCapacity, ensureCapacity_010)
    {
        sal_Int32          expVal = 6;
        ::rtl::OStringBuffer   aStrBuf( 0 );
        sal_Int32              input = 6;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 0, minimum is 6";

    }

    TEST_F(ensureCapacity, ensureCapacity_011)       //the testcase is based on comments
    {
        sal_Int32          expVal = 2;  // capacity is x = (str->length + 1) * 2; minimum < x ? x : minimum
        ::rtl::OStringBuffer   aStrBuf( 0 );
        sal_Int32              input = 1;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 0, minimum is 1";

    }

    TEST_F(ensureCapacity, ensureCapacity_012)
    {
        sal_Int32          expVal = 0;
        ::rtl::OStringBuffer   aStrBuf( 0 );
        sal_Int32              input = -1;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 0, minimum is -1";

    }
#ifdef WITH_CORE
    TEST_F(ensureCapacity, ensureCapacity_013)             //will core dump
    {
        sal_Int32          expVal = kSInt32Max;
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Int32              input = 65535;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 2147483647, minimum is 65535";

    }

    TEST_F(ensureCapacity, ensureCapacity_014)             //will core dump
    {
        sal_Int32          expVal = kSInt32Max;
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Int32              input = kSInt32Max;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 2147483647, minimum is 2147483647";

    }

    TEST_F(ensureCapacity, ensureCapacity_015)             //will core dump
    {
        sal_Int32          expVal = kSInt32Max;
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Int32              input = -1;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 2147483647, minimum is -1";

    }

    TEST_F(ensureCapacity, ensureCapacity_016)             //will core dump
    {
        sal_Int32          expVal = kSInt32Max;
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Int32              input = 0;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 2147483647, minimum is 0";

    }

    TEST_F(ensureCapacity, ensureCapacity_017)             //will core dump
    {
        sal_Int32          expVal = kSInt32Max;
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Int32              input = kNonSInt32Max;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to 2147483647, minimum is -2147483648";

    }
#endif
    TEST_F(ensureCapacity, ensureCapacity_018)
    {
        sal_Int32          expVal = 65535;
        ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
        sal_Int32              input = 65535;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to -2147483648, minimum is 65535";

    }
#ifdef WITH_CORE
    TEST_F(ensureCapacity, ensureCapacity_019)               //will core dump
    {
        sal_Int32          expVal = 2147483647;
        ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
        sal_Int32              input = 2147483647;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to -2147483648, minimum is 2147483647";

    }
#endif
    TEST_F(ensureCapacity, ensureCapacity_020)
    {
        sal_Int32          expVal = 2;
        ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
        sal_Int32              input = -1;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to -2147483648, minimum is -1";

    }

    TEST_F(ensureCapacity, ensureCapacity_021)
    {
        sal_Int32          expVal = 2;
        ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
        sal_Int32              input = 0;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to -2147483648, minimum is 0";

    }

    TEST_F(ensureCapacity, ensureCapacity_022)
    {
        sal_Int32          expVal = kNonSInt32Max;
        ::rtl::OStringBuffer   aStrBuf( kNonSInt32Max );
        sal_Int32              input = kNonSInt32Max;

        aStrBuf.ensureCapacity( input );

        ASSERT_TRUE(aStrBuf.getCapacity() == expVal) << "capacity equal to -2147483648, minimum is -2147483648";

    }

// -----------------------------------------------------------------------------

    class  setLength : public ::testing::Test
    {
    protected:
        OString* arrOUS[6];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( "1" );
            arrOUS[2] = new OString( );
            arrOUS[3] = new OString( "" );
            arrOUS[4] = new OString( "\0" );
            arrOUS[5] = new OString( kTestStr2 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4]; delete arrOUS[5];
        }
    };

    TEST_F(setLength, setLength_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              expVal1 = 50;
        ::rtl::OString         expVal2( kTestStr1 );
        sal_Int32              expVal3 = 50;
        sal_Int32              input   = 50;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the capacity of OStringBuffer(kTestStr1)";

    }

    TEST_F(setLength, setLength_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              expVal1 = kTestStr13Len;
        ::rtl::OString         expVal2( kTestStr1 );
        sal_Int32              expVal3 = 32;
        sal_Int32              input   = kTestStr13Len;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the length of OStringBuffer(kTestStr1)";

    }

    TEST_F(setLength, setLength_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              expVal1 = kTestStr1Len;
        ::rtl::OString         expVal2( kTestStr1 );
        sal_Int32              expVal3 = 32;
        sal_Int32              input   = kTestStr1Len;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength equal to the length of OStringBuffer(kTestStr1)";

    }

    TEST_F(setLength, setLength_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              expVal1 = kTestStr7Len;
        ::rtl::OString         expVal2( kTestStr7 );
        sal_Int32              expVal3 = 32;
        sal_Int32              input   = kTestStr7Len;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength less than the length of OStringBuffer(kTestStr1)";

    }

    TEST_F(setLength, setLength_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              expVal1 = 0;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 32;
        sal_Int32              input   = 0;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength equal to 0";

    }

    TEST_F(setLength, setLength_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Int32              expVal1 = 25;
        ::rtl::OString         expVal2( *arrOUS[1] );
        sal_Int32              expVal3 = 25;
        sal_Int32              input   = 25;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the capacity of OStringBuffer(1)";

    }

    TEST_F(setLength, setLength_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Int32              expVal1 = kTestStr27Len;
        ::rtl::OString         expVal2( *arrOUS[1] );
        sal_Int32              expVal3 = 17;
        sal_Int32              input   = kTestStr27Len;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength equal to the length of OStringBuffer(1)";

    }

    TEST_F(setLength, setLength_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Int32              expVal1 = 0;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 17;
        sal_Int32              input   = 0;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength less than the length of OUStringBuffer(1)";

    }

    TEST_F(setLength, setLength_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              expVal1 = 20;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 20;
        sal_Int32              input   = 20;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the capacity of OStringBuffer()";

    }

    TEST_F(setLength, setLength_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              expVal1 = 3;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 16;
        sal_Int32              input   = 3;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the length of OStringBuffer()";

    }

    TEST_F(setLength, setLength_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              expVal1 = 0;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 16;
        sal_Int32              input   = 0;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the length of OStringBuffer()";

    }

    TEST_F(setLength, setLength_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Int32              expVal1 = 20;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 20;
        sal_Int32              input   = 20;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the capacity of OStringBuffer("")";

    }

    TEST_F(setLength, setLength_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Int32              expVal1 = 5;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 16;
        sal_Int32              input   = 5;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the length of OStringBuffer("")";

    }

    TEST_F(setLength, setLength_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Int32              expVal1 = 0;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 16;
        sal_Int32              input   = 0;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength less than the length of OStringBuffer("")";

    }

    TEST_F(setLength, setLength_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Int32              expVal1 = 20;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 20;
        sal_Int32              input   = 20;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the length of OStringBuffer(\0)";

    }

    TEST_F(setLength, setLength_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Int32              expVal1 = 5;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 16;
        sal_Int32              input   = 5;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the length of OStringBuffer(\0)";

    }

    TEST_F(setLength, setLength_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Int32              expVal1 = 0;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 16;
        sal_Int32              input   = 0;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength less than the length of OStringBuffer(\0)";

    }

    TEST_F(setLength, setLength_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
        sal_Int32              expVal1 = 50;
        ::rtl::OString         expVal2( kTestStr2 );
        sal_Int32              expVal3 = 66;
        sal_Int32              input   = 50;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the capacity of OStringBuffer(kTestStr2)";

    }

    TEST_F(setLength, setLength_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
        sal_Int32              expVal1 = 40;
        ::rtl::OString         expVal2(kTestStr2);
        sal_Int32              expVal3 = 48;
        sal_Int32              input   = 40;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength more than the length of OStringBuffer(kTestStr2)";

    }

    TEST_F(setLength, setLength_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
        sal_Int32              expVal1 = kTestStr2Len;
        ::rtl::OString         expVal2(kTestStr2);
        sal_Int32              expVal3 = 48;
        sal_Int32              input   = kTestStr2Len;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength equal to the length of OUStringBuffer(kTestStr2)";

    }

    TEST_F(setLength, setLength_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
        sal_Int32              expVal1 = kTestStr7Len;
        ::rtl::OString         expVal2(kTestStr7);
        sal_Int32              expVal3 = 48;
        sal_Int32              input   = kTestStr7Len;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength less than the length of OUStringBuffer(TestStr2)";

    }

    TEST_F(setLength, setLength_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[5] );
        sal_Int32              expVal1 = 0;
        ::rtl::OString         expVal2;
        sal_Int32              expVal3 = 48;
        sal_Int32              input   = 0;

        aStrBuf.setLength( input );

        ASSERT_TRUE(aStrBuf.getStr() == expVal2 &&
                aStrBuf.getLength() == expVal1 &&
                aStrBuf.getCapacity() == expVal3) << "newLength equal to 0";

    }

// -----------------------------------------------------------------------------

    class  charAt : public ::testing::Test
    {
    protected:
        OString* arrOUS[4];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( kTestStr27 );
            arrOUS[2] = new OString( kTestStr28 );
            arrOUS[3] = new OString(  );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3];
        }
    };

    TEST_F(charAt, charAt_001)
    {
        sal_Unicode            expVal = 83;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              input = 0;

        ASSERT_TRUE(aStrBuf.charAt(input) == expVal) << "return the first character of OStringBuffer(kTestStr1)";

    }

    TEST_F(charAt, charAt_002)
    {
        sal_Unicode            expVal = 32;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              input = 3;

        ASSERT_TRUE(aStrBuf.charAt(input) == expVal) << "return the middle character of OStringBuffer(kTestStr1)";

    }

    TEST_F(charAt, charAt_003)
    {
        sal_Unicode            expVal = 115;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              input = 15;

        ASSERT_TRUE(aStrBuf.charAt(input) == expVal) << "return the last character of OStringBuffer(kTestStr1)";

    }

    TEST_F(charAt, charAt_004)
    {
        sal_Unicode            expVal = 115;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Int32              input = 0;

        ASSERT_TRUE(aStrBuf.charAt(input) == expVal) << "return the only character of OStringBuffer(kTestStr27)";

    }

    TEST_F(charAt, charAt_005)
    {
        sal_Unicode            expVal = 40;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              input = 0;

        ASSERT_TRUE(aStrBuf.charAt(input) == expVal) << "return the first of OStringBuffer(kTestStr28) with special character";

    }

    TEST_F(charAt, charAt_006)
    {
        sal_Unicode            expVal = 11;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              input = 5;

        ASSERT_TRUE(aStrBuf.charAt(input) == expVal) << "return the mid of OStringBuffer(kTestStr28) with special character";

    }

    TEST_F(charAt, charAt_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );

        ASSERT_TRUE(sal_True) << "invalid character of OStringBuffer()";

    }

    TEST_F(charAt, charAt_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );

        ASSERT_TRUE(sal_True) << "invalid character of OStringBuffer()";

    }

// -----------------------------------------------------------------------------


    class  csuc : public ::testing::Test
    {
    };

    TEST_F(csuc, csuc_001)
    {
        const sal_Char*        expVal = kTestStr1;
        ::rtl::OStringBuffer   aStrBuf( kTestStr1 );
        sal_Int32              cmpLen = kTestStr1Len;

        // LLA: wrong access! const sal_Char* pstr = *&aStrBuf;
        const sal_Char* pstr = aStrBuf.getStr();
        int nEqual = strncmp(pstr, expVal, cmpLen);

        ASSERT_TRUE(/* cmpstr( pstr, expVal, cmpLen ) */
            nEqual == 0) << "test normal string";

    }

    TEST_F(csuc, csuc_002)
    {
        ::rtl::OStringBuffer   aStrBuf;

        // LLA: wrong access! const sal_Char* pstr = *&aStrBuf;
        const sal_Char* pstr = aStrBuf.getStr();
        sal_Int32 nLen = strlen(pstr);

        ASSERT_TRUE(// cmpstr( pstr, &expVal, cmpLen )
            nLen == 0) << "test empty string";

    }

// -----------------------------------------------------------------------------

    class  getStr : public ::testing::Test
    {
    };

    TEST_F(getStr, getStr_001)
    {
        const sal_Char*        expVal = kTestStr1;
        ::rtl::OStringBuffer   aStrBuf( kTestStr1 );
        sal_Int32              cmpLen = kTestStr1Len;

        const sal_Char* pstr = aStrBuf.getStr();
        int nEqual = strncmp(pstr, expVal, cmpLen);

        ASSERT_TRUE(nEqual == 0) << "test normal string";

    }

    TEST_F(getStr, getStr_002)
    {
        // const sal_Char         tmpUC=0x0;
        // const sal_Char*        expVal=&tmpUC;
        ::rtl::OStringBuffer   aStrBuf;
        // sal_Int32              cmpLen = 1;

        const sal_Char* pstr = aStrBuf.getStr();
        sal_Int32 nLen = strlen(pstr);

        ASSERT_TRUE(pstr != 0 &&
            nLen == 0) << "test empty string";

    }

// -----------------------------------------------------------------------------

    class  setCharAt : public ::testing::Test
    {
    protected:
        OString* arrOUS[4];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr1 );
            arrOUS[1] = new OString( kTestStr27 );
            arrOUS[2] = new OString( kTestStr28 );
            arrOUS[3] = new OString(  );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3];
        }
    };

    TEST_F(setCharAt, setCharAt_001)
    {
        OString                expVal( kTestStr31 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              input1 = 0;
        sal_Char               input2 = 's';

        ASSERT_TRUE((aStrBuf.setCharAt(input1, input2)).getStr() == expVal) << "set the first character of OStringBuffer(kTestStr1) with s";

    }

    TEST_F(setCharAt, setCharAt_002)
    {
        OString                expVal( kTestStr3 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              input1 = 4;
        sal_Char               input2 = 'm';

        ASSERT_TRUE((aStrBuf.setCharAt(input1, input2)).getStr() == expVal) << "set the middle character of OStringBuffer(kTestStr1) with m";

    }

    TEST_F(setCharAt, setCharAt_003)
    {
        OString                expVal( kTestStr32 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Int32              input1 = 15;
        sal_Char               input2 = ' ';

        ASSERT_TRUE((aStrBuf.setCharAt(input1, input2)).getStr() == expVal) << "set the last character of OStringBuffer(kTestStr1) with ' '";

    }


    TEST_F(setCharAt, setCharAt_004)
    {
        OString                expVal( kTestStr33 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Int32              input1 = 0;
        sal_Char               input2 = ' ';

        ASSERT_TRUE((aStrBuf.setCharAt(input1, input2)).getStr() == expVal) << "set the only character of OStringBuffer(kTestStr27) with ' '";

    }


    TEST_F(setCharAt, setCharAt_005)
    {
        OString                expVal( kTestStr34 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              input1 = 1;
        sal_Char               input2 = (sal_Char)5;

        ASSERT_TRUE((aStrBuf.setCharAt(input1, input2)).getStr() == expVal) << "set the only of OStringBuffer(kTestStr28) with special character";

    }

    TEST_F(setCharAt, setCharAt_006)
    {
        OString                expVal( kTestStr35 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Int32              input1 = 1;
        sal_Char               input2 = (sal_Char)-5;

        ASSERT_TRUE((aStrBuf.setCharAt(input1, input2)).getStr() == expVal) << "set the only of OStringBuffer(kTestStr28) with special character";

    }
#ifdef WITH_CORE
    TEST_F(setCharAt, setCharAt_007)
    {
        OString*               expVal = 0;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Int32              input1 = 0;
        sal_Char               input2 = (sal_Char)5;

        ASSERT_TRUE(sal_True) << "invalid character of OStringBuffer()";

        delete expVal;

    }

    TEST_F(setCharAt, setCharAt_008)
    {
        OString*               expVal = 0;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Int32              input1 = -2;
        sal_Char               input2 = (sal_Char)5;

        ASSERT_TRUE(sal_True) << "invalid character of OStringBuffer()";

        delete expVal;

    }

    TEST_F(setCharAt, setCharAt_009)
    {
        OString*               expVal = 0;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Int32              input1 = 3;
        sal_Char               input2 = (sal_Char)5;

        ASSERT_TRUE(sal_True) << "invalid character of OStringBuffer()";

        delete expVal;

    }
#endif

// -----------------------------------------------------------------------------

    class  append_001 : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_001, append_001_001)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                input2( kTestStr8 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_001, append_001_002)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                input2( kTestStr36 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length more than 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_001, append_001_003)
    {
        OString                expVal( kTestStr37 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                input2( kTestStr23 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_001, append_001_004)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                input2;

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 0) to the string buffer arrOUS[0]";

    }

    TEST_F(append_001, append_001_005)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                input2( kTestStr7 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length less than 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_001, append_001_006)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                input2( kTestStr2 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length more than 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_001, append_001_007)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                input2( kTestStr1 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_001, append_001_008)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                input2;

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 0) to the string buffer arrOUS[1]";

    }

    TEST_F(append_001, append_001_009)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                input2( kTestStr7 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length less than 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_001, append_001_010)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                input2( kTestStr2 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length more than 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_001, append_001_011)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                input2( kTestStr1 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_001, append_001_012)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                input2;

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 0) to the string buffer arrOUS[2]";

    }

    TEST_F(append_001, append_001_013)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                input2( kTestStr7 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length less than 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_001, append_001_014)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                input2( kTestStr2 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length more than 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_001, append_001_015)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                input2( kTestStr1 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_001, append_001_016)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                input2;

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 0) to the string buffer arrOUS[3]";

    }

    TEST_F(append_001, append_001_017)
    {
        OString                expVal( kTestStr29 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                input2( kTestStr38 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length less than 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_001, append_001_018)
    {
        OString                expVal( kTestStr39 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                input2( kTestStr17 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length more than 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_001, append_001_019)
    {
        OString                expVal( kTestStr40 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                input2( kTestStr31 );

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_001, append_001_020)
    {
        OString                expVal( kTestStr28 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                input2;

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 0) to the string buffer arrOUS[4]";

    }

#ifdef WITH_CORE
    TEST_F(append_001, append_001_021)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                input2;

        aStrBuf.append( input2 );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "Appends the string(length equal to 0) to the string buffer arrOUS[4]";

    }
#endif

// -----------------------------------------------------------------------------

    class  append_002 : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_002, append_002_001)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        const sal_Char*        input = kTestStr8;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_002, append_002_002)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        const sal_Char*        input = kTestStr36;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_002, append_002_003)
    {
        OString                expVal( kTestStr37 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        const sal_Char*        input = kTestStr23;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_002, append_002_004)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        const sal_Char*        input = kTestStr25;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[0]";

    }

    TEST_F(append_002, append_002_005)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        const sal_Char*        input = kTestStr7;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_002, append_002_006)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        const sal_Char*        input = kTestStr2;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_002, append_002_007)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        const sal_Char*        input = kTestStr1;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_002, append_002_008)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        const sal_Char*        input = kTestStr25;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[1]";

    }

    TEST_F(append_002, append_002_009)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        const sal_Char*        input = kTestStr7;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_002, append_002_010)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        const sal_Char*        input = kTestStr2;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_002, append_002_011)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        const sal_Char*        input = kTestStr1;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_002, append_002_012)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        const sal_Char*        input = kTestStr25;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[2]";

    }

    TEST_F(append_002, append_002_013)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        const sal_Char*        input = kTestStr7;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_002, append_002_014)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        const sal_Char*        input = kTestStr2;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_002, append_002_015)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        const sal_Char*        input = kTestStr1;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_002, append_002_016)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        const sal_Char*        input = kTestStr25;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[3]";

    }

    TEST_F(append_002, append_002_017)
    {
        OString                expVal( kTestStr29 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        const sal_Char*        input = kTestStr38;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_002, append_002_018)
    {
        OString                expVal( kTestStr39 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        const sal_Char*        input = kTestStr17;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_002, append_002_019)
    {
        OString                expVal( kTestStr40 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        const sal_Char*        input = kTestStr31;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_002, append_002_020)
    {
        OString                expVal( kTestStr28 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        const sal_Char*        input = kTestStr25;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[4]";

    }

#ifdef WITH_CORE
    TEST_F(append_002, append_002_021)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        const sal_Char*        input = kTestStr25;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer(with INT_MAX)";

    }
#endif

// -----------------------------------------------------------------------------

    class  append_003 : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_003, append_003_001)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        const sal_Char*        input1 = kTestStr36;
        sal_Int32              input2 = 12;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_003, append_003_002)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        const sal_Char*        input1 = kTestStr36;
        sal_Int32              input2 = 28;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_003, append_003_003)
    {
        OString                expVal( kTestStr37 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        const sal_Char*        input1 = kTestStr23;
        sal_Int32              input2 = 16;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[0]";

    }

    TEST_F(append_003, append_003_004)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 0;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[0]";

    }

    TEST_F(append_003, append_003_005)
    {
        // LLA: this is an illegal test, the input2 value must non-negative
        // LLA: OString                expVal( kTestStr41 );
        // LLA: ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: const sal_Char*        input1 = kTestStr2;
        // LLA: sal_Int32              input2 = -1;
        // LLA:
        // LLA: aStrBuf.append( input1, input2 );
        // LLA:
        // LLA: ASSERT_TRUE(// LLA:     ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
        // LLA:) << // LLA:     "Appends the string(length less than 0) to the string buffer arrOUS[0]";

    }

    TEST_F(append_003, append_003_006)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 4;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_003, append_003_007)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 32;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_003, append_003_008)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 16;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[1]";

    }

    TEST_F(append_003, append_003_009)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 0;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[1]";

    }

    TEST_F(append_003, append_003_010)
    {
        // LLA: this is an illegal test, the input2 value must non-negative
        // LLA: OString                expVal;
        // LLA: ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: const sal_Char*        input1 = kTestStr2;
        // LLA: sal_Int32              input2 = -1;
        // LLA:
        // LLA: aStrBuf.append( input1, input2 );
        // LLA:
        // LLA: ASSERT_TRUE(// LLA:     ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
        // LLA:) << // LLA:     "Appends the string(length less than 0) to the string buffer arrOUS[1]";
    }

    TEST_F(append_003, append_003_011)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 4;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_003, append_003_012)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 32;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_003, append_003_013)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 16;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[2]";

    }

    TEST_F(append_003, append_003_014)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 0;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[2]";

    }

    TEST_F(append_003, append_003_015)
    {
        // LLA: this is an illegal test, the input2 value must non-negative
        // LLA: OString                expVal;
        // LLA: ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: const sal_Char*        input1 = kTestStr2;
        // LLA: sal_Int32              input2 = -1;
        // LLA:
        // LLA: aStrBuf.append( input1, input2 );
        // LLA:
        // LLA: ASSERT_TRUE(// LLA:     ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
        // LLA:) << // LLA:     "Appends the string(length less than 0) to the string buffer arrOUS[2]";

    }

    TEST_F(append_003, append_003_016)
    {
        OString                expVal( kTestStr7 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 4;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_003, append_003_017)
    {
        OString                expVal( kTestStr2 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 32;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_003, append_003_018)
    {
        OString                expVal( kTestStr1 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 16;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[3]";

    }

    TEST_F(append_003, append_003_019)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 0;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[3]";

    }

    TEST_F(append_003, append_003_020)
    {
        // LLA: this is an illegal test, the input2 value must non-negative
        // LLA: OString                expVal;
        // LLA: ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: const sal_Char*        input1 = kTestStr2;
        // LLA: sal_Int32              input2 = -1;
        // LLA:
        // LLA: aStrBuf.append( input1, input2 );
        // LLA:
        // LLA: ASSERT_TRUE(// LLA:     ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
        // LLA:) << // LLA:     "Appends the string(length less than 0) to the string buffer arrOUS[3]";

    }

    TEST_F(append_003, append_003_021)
    {
        OString                expVal( kTestStr29 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        const sal_Char*        input1 = kTestStr38;
        sal_Int32              input2 = 7;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length less than 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_003, append_003_022)
    {
        OString                expVal( kTestStr39 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        const sal_Char*        input1 = kTestStr17;
        sal_Int32              input2 = 22;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length more than 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_003, append_003_023)
    {
        OString                expVal( kTestStr40 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        const sal_Char*        input1 = kTestStr31;
        sal_Int32              input2 = 16;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 16) to the string buffer arrOUS[4]";

    }

    TEST_F(append_003, append_003_024)
    {
        OString                expVal( kTestStr28 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        const sal_Char*        input1 = kTestStr2;
        sal_Int32              input2 = 0;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer arrOUS[4]";

    }

    TEST_F(append_003, append_003_025)
    {
        // LLA: this is an illegal test, the input2 value must non-negative
        // LLA: OString                expVal( kTestStr42 );
        // LLA: ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: const sal_Char*        input1 = kTestStr2;
        // LLA: sal_Int32              input2 = -1;
        // LLA:
        // LLA: aStrBuf.append( input1, input2 );
        // LLA:
        // LLA: ASSERT_TRUE(// LLA:     ( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )
        // LLA:) << // LLA:     "Appends the string(length less than 0) to the string buffer arrOUS[4]";

    }

#ifdef WITH_CORE
    TEST_F(append_003, append_003_026)
    {
        OString                expVal;
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        const sal_Char*        input1 = kTestStr25;
        sal_Int32              input2 = 0;

        aStrBuf.append( input1, input2 );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the string(length equal to 0) to the string buffer(with INT_MAX)";

    }
#endif

// -----------------------------------------------------------------------------

    class  append_004 : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_004, append_004_001)
    {
        OString                expVal( kTestStr45 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Bool               input = sal_True;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_True) to the string buffer arrOUS[0]";

    }

    TEST_F(append_004, append_004_002)
    {
        OString                expVal( kTestStr46 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Bool               input = sal_False;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_False) to the string buffer arrOUS[0]";

    }

    TEST_F(append_004, append_004_003)
    {
        OString                expVal( kTestStr47 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Bool               input = sal_True;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_True) to the string buffer arrOUS[1]";

    }

    TEST_F(append_004, append_004_004)
    {
        OString                expVal( kTestStr48 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Bool               input = sal_False;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_False) to the string buffer arrOUS[1]";

    }

    TEST_F(append_004, append_004_005)
    {
        OString                expVal( kTestStr47 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Bool               input = sal_True;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_True) to the string buffer arrOUS[2]";

    }

    TEST_F(append_004, append_004_006)
    {
        OString                expVal( kTestStr48 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Bool               input = sal_False;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_False) to the string buffer arrOUS[2]";

    }

    TEST_F(append_004, append_004_007)
    {
        OString                expVal( kTestStr47 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Bool               input = sal_True;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_True) to the string buffer arrOUS[3]";

    }

    TEST_F(append_004, append_004_008)
    {
        OString                expVal( kTestStr48 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Bool               input = sal_False;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_False) to the string buffer arrOUS[3]";

    }

    TEST_F(append_004, append_004_009)
    {
        OString                expVal( kTestStr49 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Bool               input = sal_True;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_True) to the string buffer arrOUS[4]";

    }

    TEST_F(append_004, append_004_010)
    {
        OString                expVal( kTestStr50 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Bool               input = sal_False;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_False) to the string buffer arrOUS[4]";

    }

#ifdef WITH_CORE
    TEST_F(append_004, append_004_011)
    {
        OString                expVal( kTestStr47 );
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Bool               input = sal_True;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_True) to the string buffer(with INT_MAX)";

    }

    TEST_F(append_004, append_004_012)
    {
        OString                expVal( kTestStr48 );
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Bool               input = sal_False;

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Bool(sal_False) to the string buffer(with INT_MAX)";

    }
#endif

//------------------------------------------------------------------------
// testing the method append(sal_Char c)
//------------------------------------------------------------------------
    class  append_005 : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_005, append_001)
    {
        OString                expVal( kTestStr51 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Char               input = 'M';

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(M) to the string buffer arrOUS[0]";

    }

    TEST_F(append_005, append_002)
    {
        OString                expVal( kTestStr143 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Unicode(kSInt8Max) to the string buffer arrOUS[0]";

    }

    TEST_F(append_005, append_003)
    {
        OString                expVal( kTestStr27 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Char               input = 's';

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(s) to the string buffer arrOUS[1]";

    }

    TEST_F(append_005, append_004)
    {
        OString                expVal( kTestStr144 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(kSInt8Max) to the string buffer arrOUS[1]";

    }

    TEST_F(append_005, append_005_005)
    {
        OString                expVal( kTestStr27 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Char               input = 's';

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(s) to the string buffer arrOUS[2]";

    }

    TEST_F(append_005, append_006)
    {
        OString                expVal( kTestStr144 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(kSInt8Max) to the string buffer arrOUS[2]";

    }

    TEST_F(append_005, append_007)
    {
        OString                expVal( kTestStr27 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Char               input = 's';

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(s) to the string buffer arrOUS[3]";

    }

    TEST_F(append_005, append_008)
    {
        OString                expVal( kTestStr144 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(kSInt8Max) to the string buffer arrOUS[3]";

    }

    TEST_F(append_005, append_009)
    {
        OString                expVal( kTestStr56 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Char               input = 's';

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(s) to the string buffer arrOUS[4]";

    }

    TEST_F(append_005, append_010)
    {
        OString                expVal( kTestStr145 );
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(kSInt8Max) to the string buffer arrOUS[4]";

    }

#ifdef WITH_CORE
    TEST_F(append_005, append_011)
    {
        OString                expVal( kTestStr27 );
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Char               input = 's';

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(s) to the string buffer(with INT_MAX)";

    }

    TEST_F(append_005, append_012)
    {
        OString                expVal( kTestStr144 );
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        sal_Char               input = static_cast<sal_Char>(SAL_MAX_UINT8);

        aStrBuf.append( input );

        ASSERT_TRUE(( aStrBuf.getStr()== expVal) &&( aStrBuf.getLength() == expVal.getLength() )) << "Appends the sal_Char(kSInt8Max) to the string buffer with INT_MAX)";

    }
#endif

/**
 * Calls the method append(T, radix) and compares
 * returned OUString with OUString that passed in the array resArray.
 *
 * @param T, type of argument, passed to append
 * @param resArray, array of result ustrings to compare to
 * @param n the number of elements in the array resArray (testcases)
 * @param pTestResult the instance of the class TestResult
 * @param inArray [optional], array of value that is passed as first argument
 *                            to append
 *
 * @return true, if all returned OUString are equal to corresponding OUString in
 *               resArray else, false.
 */
/*template <class T>
sal_Bool test_append( const char** resArray, int n, sal_Int16 radix,
                            const T *inArray, OStringBuffer &aStr1 )
{
    sal_Bool    bRes = sal_True;

    //sal_Char    methName[MAXBUFLENGTH];
    //sal_Char*   pMeth = methName;
    sal_Int32   i;
//    static sal_Unicode aUchar[80]={0x12};

    for (i = 0; i < n; i++)
    {

    OSL_ENSURE( i < 80, "ERROR: leave aUchar bound");

//        AStringToUStringCopy(aUchar,resArray[i]);

        ::rtl::OString aStr2(aStr1.getStr());
        ::rtl::OString aStr3( "-" );

        if (inArray == 0)
    {
            aStr2 += OString(resArray[i]);
            aStr1.append((T)i, radix);
    }
        else
        {
 //           sal_Unicode   aStr4[100];
            if ( inArray[i] < 0 )
            {
                aStr2 += aStr3;

            }
//          if(AStringToUStringCopy(aStr4,resArray[i]))
//            {
                aStr2 += OString(resArray[i]);
//            }
            aStr1.append((T)inArray[i], radix);
        }

        ASSERT_TRUE(aStr1.getStr()== aStr2 &&
                    aStr1.getLength() == aStr2.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]";
    }

    return (bRes);
}
#define test_append_Int32      test_append<sal_Int32>
#define test_append_Int64      test_append<sal_Int64>
#define test_append_float      test_append<float>
#define test_append_double     test_append<double>*/
//------------------------------------------------------------------------
// testing the method append( sal_Int32 i, sal_Int16 radix=2 )
// testing the method append( sal_Int32 i, sal_Int16 radix=8 )
// testing the method append( sal_Int32 i, sal_Int16 radix=10 )
// testing the method append( sal_Int32 i, sal_Int16 radix=16 )
// testing the method append( sal_Int32 i, sal_Int16 radix=36 )
//------------------------------------------------------------------------
    class  append_006_Int32 : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }

        // For some bizarre reason, only odd numbered tests were set up to run
    };

    TEST_F(append_006_Int32, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        /*test_append_Int32((const char**)kBinaryNumsStr,
                            kBinaryNumsCount, kRadixBinary,
                            0, aStrBuf );*/

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_006_Int32, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_006_Int32, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_043)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_044)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_045)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_046)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_047)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_048)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_049)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_050)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_051)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_052)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_053)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_054)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_055)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_056)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_057)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_058)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_059)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_060)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_006_Int32, append_061)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_062)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_063)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_064)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_065)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_066)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_067)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_068)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_069)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_070)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_071)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_072)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_073)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_074)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_075)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_076)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_077)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_078)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_079)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_080)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_006_Int32, append_081)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_082)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_083)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_084)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_085)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_086)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_087)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_088)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_089)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_090)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_091)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_092)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_093)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_094)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_095)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_096)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_097)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_098)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_099)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_006_Int32, append_100)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_kRadixBase36 for arrOUS[4]";

    }

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
    class  append_006_Int32_Bounderies : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_006_Int32_Bounderies, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 8;

        expVal += OString( "17777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 10;

        expVal += OString( "2147483647" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 36;

        expVal += OString( "zik0zj" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Bounderies, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 8;

        expVal += OString( "17777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 10;

        expVal += OString( "2147483647" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 36;

        expVal += OString( "zik0zj" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Bounderies, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 8;

        expVal += OString( "17777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 10;

        expVal += OString( "2147483647" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 36;

        expVal += OString( "zik0zj" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Bounderies, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 8;

        expVal += OString( "17777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 10;

        expVal += OString( "2147483647" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 36;

        expVal += OString( "zik0zj" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Bounderies, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_043)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_044)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 8;

        expVal += OString( "17777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_045)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_046)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 10;

        expVal += OString( "2147483647" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_047)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_048)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_049)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Bounderies, append_050)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = kSInt32Max;
        sal_Int16              radix = 36;

        expVal += OString( "zik0zj" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

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
    class  append_006_Int32_Negative : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_006_Int32_Negative, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_006_Int32_Negative, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_006_Int32_Negative, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_043)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_044)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_045)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_046)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_047)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_048)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_049)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_050)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_051)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_052)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_053)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_054)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_055)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_056)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_057)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_058)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_059)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_060)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_006_Int32_Negative, append_061)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_062)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_063)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_064)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_065)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_066)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_067)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_068)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_069)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_070)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_071)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_072)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_073)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_074)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_075)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_076)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_077)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_078)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_079)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_080)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_006_Int32_Negative, append_081)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_082)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_083)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_084)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 2)_006_negative_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_085)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_086)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_087)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_088)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 8)_006_negative_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_089)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_090)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_091)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_092)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 10)_006_negative_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_093)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_094)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_095)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_096)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_097)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_098)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_099)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_006_Int32_Negative, append_100)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int32              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int32, radix 36)_006_negative_kRadixBase36 for arrOUS[4]";

    }

//------------------------------------------------------------------------
// testing the method append( sal_Int32 i, sal_Int16 radix ) where radix = -5
//------------------------------------------------------------------------
    class  append_006_Int32_WrongRadix : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];
        sal_Int32 intVal;

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );
            intVal = 11;

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_006_Int32_WrongRadix, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr59 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[0]";

    }

    TEST_F(append_006_Int32_WrongRadix, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr60 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[1]";

    }

    TEST_F(append_006_Int32_WrongRadix, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr60 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[2]";

    }

    TEST_F(append_006_Int32_WrongRadix, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr60 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[3]";

    }

    TEST_F(append_006_Int32_WrongRadix, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr61 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[4]";

    }
#ifdef WITH_CORE
    TEST_F(append_006_Int32_WrongRadix, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr60 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer(with INT_MAX)";

    }
#endif

//------------------------------------------------------------------------
    class  append_006_Int32_defaultParam : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_006_Int32_defaultParam, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr59 );
        sal_Int32              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 11 and return OStringBuffer[0]+11";

    }

    TEST_F(append_006_Int32_defaultParam, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr62 );
        sal_Int32              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 0 and return OStringBuffer[0]+0";

    }

    TEST_F(append_006_Int32_defaultParam, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr63 );
        sal_Int32              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -11 and return OStringBuffer[0]+(-11)";

    }

    TEST_F(append_006_Int32_defaultParam, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr64 );
        sal_Int32              input = 2147483647;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 2147483647 and return OStringBuffer[0]+2147483647";

    }

    TEST_F(append_006_Int32_defaultParam, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr65 );
        sal_Int32              input = kNonSInt32Max;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -2147483648 and return OStringBuffer[0]+(-2147483648)";

    }

    TEST_F(append_006_Int32_defaultParam, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr60 );
        sal_Int32              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 11 and return OStringBuffer[1]+11";

    }

    TEST_F(append_006_Int32_defaultParam, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr66 );
        sal_Int32              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 0 and return OStringBuffer[1]+0";

    }

    TEST_F(append_006_Int32_defaultParam, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr67 );
        sal_Int32              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -11 and return OStringBuffer[1]+(-11)";

    }

    TEST_F(append_006_Int32_defaultParam, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr68 );
        sal_Int32              input = 2147483647;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 2147483647 and return OStringBuffer[1]+2147483647";

    }

    TEST_F(append_006_Int32_defaultParam, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr69 );
        sal_Int32              input = SAL_MIN_INT32 /*-2147483648*/;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -2147483648 and return OStringBuffer[1]+(-2147483648)";

    }

    TEST_F(append_006_Int32_defaultParam, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr60 );
        sal_Int32              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 11 and return OStringBuffer[2]+11";

    }

    TEST_F(append_006_Int32_defaultParam, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr66 );
        sal_Int32              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 0 and return OUStringBuffer[2]+0";

    }

    TEST_F(append_006_Int32_defaultParam, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr67 );
        sal_Int32              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -11 and return OUStringBuffer[2]+(-11)";

    }

    TEST_F(append_006_Int32_defaultParam, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr68 );
        sal_Int32              input = 2147483647;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 2147483647 and return OStringBuffer[2]+2147483647";

    }

    TEST_F(append_006_Int32_defaultParam, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr69 );
        sal_Int32              input = SAL_MIN_INT32;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -2147483648 and return OStringBuffer[2]+(-2147483648)";

    }

    TEST_F(append_006_Int32_defaultParam, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr60 );
        sal_Int32              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 11 and return OStringBuffer[3]+11";

    }

    TEST_F(append_006_Int32_defaultParam, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr66 );
        sal_Int32              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 0 and return OStringBuffer[3]+0";

    }

    TEST_F(append_006_Int32_defaultParam, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr67 );
        sal_Int32              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -11 and return OStringBuffer[3]+(-11)";

    }

    TEST_F(append_006_Int32_defaultParam, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr68 );
        sal_Int32              input = 2147483647;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 2147483647 and return OStringBuffer[3]+2147483647";

    }

    TEST_F(append_006_Int32_defaultParam, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr69 );
        sal_Int32              input = SAL_MIN_INT32;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -2147483648 and return OStringBuffer[3]+(-2147483648)";

    }

    TEST_F(append_006_Int32_defaultParam, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr61 );
        sal_Int32              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 11 and return OStringBuffer[4]+11";

    }

    TEST_F(append_006_Int32_defaultParam, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr70 );
        sal_Int32              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 0 and return OStringBuffer[4]+0";

    }

    TEST_F(append_006_Int32_defaultParam, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr71 );
        sal_Int32              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -11 and return OStringBuffer[4]+(-11)";

    }

    TEST_F(append_006_Int32_defaultParam, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr72 );
        sal_Int32              input = 2147483647;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 2147483647 and return OStringBuffer[4]+2147483647";

    }

    TEST_F(append_006_Int32_defaultParam, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr73 );
        sal_Int32              input = SAL_MIN_INT32;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -2147483648 and return OStringBuffer[4]+(-2147483648)";

    }
#ifdef WITH_CORE
    TEST_F(append_006_Int32_defaultParam, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr60 );
        sal_Int32              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 11 and return OStringBuffer(kSInt32Max)+11";

    }

    TEST_F(append_006_Int32_defaultParam, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr66 );
        sal_Int32              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 0 and return OStringBuffer(kSInt32Max)+0";

    }

    TEST_F(append_006_Int32_defaultParam, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr67 );
        sal_Int32              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -11 and return OStringBuffer(kSInt32Max)+(-11)";

    }

    TEST_F(append_006_Int32_defaultParam, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr68 );
        sal_Int32              input = 2147483647;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 2147483647 and return OStringBuffer(kSInt32Max)+2147483647";

    }

    TEST_F(append_006_Int32_defaultParam, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr69 );
        sal_Int32              input = SAL_MIN_INT32;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int32 -2147483648 and return OStringBuffer(kSInt32Max)+(-2147483648)";

    }
#endif

//------------------------------------------------------------------------
// testing the method append( sal_Int64 l, sal_Int16 radix=2 )
// testing the method append( sal_Int64 l, sal_Int16 radix=8 )
// testing the method append( sal_Int64 l, sal_Int16 radix=10 )
// testing the method append( sal_Int64 l, sal_Int16 radix=16 )
// testing the method append( sal_Int64 l, sal_Int16 radix=36 )
//------------------------------------------------------------------------
    class  append_007_Int64 : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_007_Int64, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64             input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_007_Int64, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_007_Int64, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_043)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_044)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_045)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_046)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_047)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_048)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_049)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_050)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_051)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_052)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_053)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_054)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_055)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_056)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_057)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_058)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_059)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_060)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_007_Int64, append_061)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_062)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_063)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_064)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_065)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_066)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_067)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_068)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_069)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_070)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_071)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_072)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_073)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_074)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_075)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_076)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_077)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_078)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_079)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_080)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_007_Int64, append_081)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_082)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 2;

        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_083)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 2;

        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_084)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 2;

        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_085)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_086)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 8;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_087)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 8;

        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_088)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 8;

        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_089)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_090)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 10;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_091)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 10;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_092)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 10;

        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_093)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_094)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 16;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_095)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 16;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_096)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 15;
        sal_Int16              radix = 16;

        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_097)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_098)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 4;
        sal_Int16              radix = 36;

        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_099)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 8;
        sal_Int16              radix = 36;

        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_007_Int64, append_100)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = 35;
        sal_Int16              radix = 36;

        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_kRadixBase36 for arrOUS[4]";

    }

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
    class  append_007_Int64_Bounderies : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_007_Int64_Bounderies, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 2;

        expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64             input = kSInt64Max;
        sal_Int16              radix = 8;

        expVal += OString( "777777777777777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 10;

        expVal += OString( "9223372036854775807" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffffffffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 36;

        expVal += OString( "1y2p0ij32e8e7" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Bounderies, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 2;

        expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 8;

        expVal += OString( "777777777777777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 10;

        expVal += OString( "9223372036854775807" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffffffffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 36;

        expVal += OString( "1y2p0ij32e8e7" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Bounderies, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 2;

        expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 8;

        expVal += OString( "777777777777777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 10;

        expVal += OString( "9223372036854775807" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffffffffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 36;

        expVal += OString( "1y2p0ij32e8e7" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Bounderies, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 2;

        expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 8;

        expVal += OString( "777777777777777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 10;

        expVal += OString( "9223372036854775807" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffffffffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 36;

        expVal += OString( "1y2p0ij32e8e7" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Bounderies, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 2;

        expVal += OString( "1111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 2;

        expVal += OString( "111111111111111111111111111111111111111111111111111111111111111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_043)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 8;

        expVal += OString( "177" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_044)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 8;

        expVal += OString( "777777777777777777777" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_045)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 10;

        expVal += OString( "127" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_046)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 10;

        expVal += OString( "9223372036854775807" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_047)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 16;

        expVal += OString( "7f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_048)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 16;

        expVal += OString( "7fffffffffffffff" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_049)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt8Max;
        sal_Int16              radix = 36;

        expVal += OString( "3j" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Bounderies, append_050)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = kSInt64Max;
        sal_Int16              radix = 36;

        expVal += OString( "1y2p0ij32e8e7" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_Bounderies_kRadixBinary for arrOUS[4]";

    }

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
    class  append_007_Int64_Negative : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_007_Int64_Negative, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[0]";

    }

    TEST_F(append_007_Int64_Negative, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[1]";

    }

    TEST_F(append_007_Int64_Negative, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_043)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_044)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_045)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_046)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_047)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_048)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_049)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_050)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_051)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_052)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_053)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_054)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_055)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_056)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_057)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_058)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_059)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_060)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[2]";

    }

    TEST_F(append_007_Int64_Negative, append_061)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_062)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_063)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_064)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_065)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_066)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_067)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_068)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_069)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_070)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_071)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_072)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_073)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_074)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_075)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_076)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_077)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_078)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_079)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_080)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[3]";

    }

    TEST_F(append_007_Int64_Negative, append_081)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 2;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_082)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "100" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_083)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1000" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_084)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 2;

        expVal += OString( "-" );
        expVal += OString( "1111" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 2)_006_negative_kRadixBinary for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_085)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 8;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_086)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_087)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "10" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_088)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 8;

        expVal += OString( "-" );
        expVal += OString( "17" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 8)_006_negative_kRadixOctol for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_089)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 10;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_090)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_091)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_092)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 10;

        expVal += OString( "-" );
        expVal += OString( "15" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 10)_006_negative_kRadixDecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_093)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 16;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_094)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_095)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_096)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -15;
        sal_Int16              radix = 16;

        expVal += OString( "-" );
        expVal += OString( "f" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 16)_006_negative_kRadixHexdecimal for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_097)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -0;
        sal_Int16              radix = 36;

        expVal += OString( "0" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_098)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -4;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "4" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_099)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -8;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "8" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]";

    }

    TEST_F(append_007_Int64_Negative, append_100)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( aStrBuf.getStr() );
        sal_Int64              input = -35;
        sal_Int16              radix = 36;

        expVal += OString( "-" );
        expVal += OString( "z" );
        aStrBuf.append( input, radix );

        ASSERT_TRUE(aStrBuf.getStr()== expVal &&
                aStrBuf.getLength() == expVal.getLength()) << "append(sal_Int64, radix 36)_006_negative_kRadixBase36 for arrOUS[4]";

    }

//------------------------------------------------------------------------
// testing the method append( sal_Int64 i, sal_Int16 radix ) where radix = -5
//------------------------------------------------------------------------
    class  append_007_Int64_WrongRadix : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];
        sal_Int64 intVal;

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );
            intVal = 11;

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_007_Int64_WrongRadix, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr59 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[0]";

    }

    TEST_F(append_007_Int64_WrongRadix, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr60 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[1]";

    }

    TEST_F(append_007_Int64_WrongRadix, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr60 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[2]";

    }

    TEST_F(append_007_Int64_WrongRadix, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr60 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[3]";

    }

    TEST_F(append_007_Int64_WrongRadix, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr61 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer arrOUS[4]";

    }
#ifdef WITH_CORE
    TEST_F(append_007_Int64_WrongRadix, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
        OString                expVal( kTestStr60 );

        aStrBuf.append( intVal, -5 );

        ASSERT_TRUE(sal_True) << "Appends the WrongRadix to the string buffer(with INT_MAX)";

    }
#endif

//------------------------------------------------------------------------
    class  append_007_Int64_defaultParam : public ::testing::Test
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_007_Int64_defaultParam, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr59 );
        sal_Int64              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 11 and return OStringBuffer[0]+11";

    }

    TEST_F(append_007_Int64_defaultParam, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr62 );
        sal_Int64              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 0 and return OStringBuffer[0]+0";

    }

    TEST_F(append_007_Int64_defaultParam, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr63 );
        sal_Int64              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -11 and return OStringBuffer[0]+(-11)";

    }

    TEST_F(append_007_Int64_defaultParam, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr116 );
        sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 9223372036854775807 and return OStringBuffer[0]+9223372036854775807";

    }

    TEST_F(append_007_Int64_defaultParam, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr117 );
        sal_Int64              input = SAL_MIN_INT64/*-9223372036854775808*/; // LLA: this is not the same :-( kNonSInt64Max;

        aStrBuf.append( input );

        sal_Bool bRes = expVal.equals( aStrBuf.getStr() );
        ASSERT_TRUE(bRes && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -9223372036854775808 and return OStringBuffer[0]+(-9223372036854775808)";

    }

    TEST_F(append_007_Int64_defaultParam, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr60 );
        sal_Int64              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 11 and return OStringBuffer[1]+11";

    }

    TEST_F(append_007_Int64_defaultParam, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr66 );
        sal_Int64              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 0 and return OStringBuffer[1]+0";

    }

    TEST_F(append_007_Int64_defaultParam, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr67 );
        sal_Int64              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -11 and return OStringBuffer[1]+(-11)";

    }

    TEST_F(append_007_Int64_defaultParam, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr118 );
        sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 9223372036854775807 and return OStringBuffer[1]+9223372036854775807";

    }

    TEST_F(append_007_Int64_defaultParam, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr119 );
        sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -9223372036854775808 and return OStringBuffer[1]+(-9223372036854775808)";

    }

    TEST_F(append_007_Int64_defaultParam, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr60 );
        sal_Int64              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 11 and return OStringBuffer[2]+11";

    }

    TEST_F(append_007_Int64_defaultParam, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr66 );
        sal_Int64              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 0 and return OUStringBuffer[2]+0";

    }

    TEST_F(append_007_Int64_defaultParam, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr67 );
        sal_Int64              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -11 and return OUStringBuffer[2]+(-11)";

    }

    TEST_F(append_007_Int64_defaultParam, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr118 );
        sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 9223372036854775807 and return OStringBuffer[2]+9223372036854775807";

    }

    TEST_F(append_007_Int64_defaultParam, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr119 );
        sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -9223372036854775808 and return OStringBuffer[2]+(-9223372036854775808)";

    }

    TEST_F(append_007_Int64_defaultParam, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr60 );
        sal_Int64              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 11 and return OStringBuffer[3]+11";

    }

    TEST_F(append_007_Int64_defaultParam, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr66 );
        sal_Int64              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 0 and return OStringBuffer[3]+0";

    }

    TEST_F(append_007_Int64_defaultParam, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr67 );
        sal_Int64              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -11 and return OStringBuffer[3]+(-11)";

    }

    TEST_F(append_007_Int64_defaultParam, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr118 );
        sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 9223372036854775807 and return OStringBuffer[3]+9223372036854775807";

    }

    TEST_F(append_007_Int64_defaultParam, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr119 );
        sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -9223372036854775808 and return OStringBuffer[3]+(-9223372036854775808)";

    }

    TEST_F(append_007_Int64_defaultParam, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr61 );
        sal_Int64              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 11 and return OStringBuffer[4]+11";

    }

    TEST_F(append_007_Int64_defaultParam, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr70 );
        sal_Int64              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 0 and return OStringBuffer[4]+0";

    }

    TEST_F(append_007_Int64_defaultParam, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr71 );
        sal_Int64              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -11 and return OStringBuffer[4]+(-11)";

    }

    TEST_F(append_007_Int64_defaultParam, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr120 );
        sal_Int64              input = SAL_CONST_INT64(9223372036854775807);
        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 9223372036854775807 and return OStringBuffer[4]+9223372036854775807";

    }

    TEST_F(append_007_Int64_defaultParam, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr121 );
        sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -9223372036854775808 and return OStringBuffer[4]+(-9223372036854775808)";

    }
#ifdef WITH_CORE
    TEST_F(append_007_Int64_defaultParam, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
        OString                expVal( kTestStr60 );
        sal_Int64              input = 11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 11 and return OStringBuffer(kSInt64Max)+11";

    }

    TEST_F(append_007_Int64_defaultParam, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
        OString                expVal( kTestStr66 );
        sal_Int64              input = 0;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 0 and return OStringBuffer(kSInt64Max)+0";

    }

    TEST_F(append_007_Int64_defaultParam, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
        OString                expVal( kTestStr67 );
        sal_Int64              input = -11;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -11 and return OStringBuffer(kSInt64Max)+(-11)";

    }

    TEST_F(append_007_Int64_defaultParam, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
        OString                expVal( kTestStr118 );
        sal_Int64              input = 9223372036854775807;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 9223372036854775807 and return OStringBuffer(kSInt64Max)+9223372036854775807";

    }

    TEST_F(append_007_Int64_defaultParam, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt64Max );
        OString                expVal( kTestStr119 );
        sal_Int64              input = SAL_MIN_INT64; // LLA: this is not the same :-( kNonSInt64Max;

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "input Int64 -9223372036854775808 and return OStringBuffer(kSInt64Max)+(-9223372036854775808)";

    }
#endif

//------------------------------------------------------------------------
// testing the method append( float f )
//------------------------------------------------------------------------
    class checkfloat : public ::testing::Test
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
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_008_float, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr74 );
        float                  input = (float)atof("3.0");

        // LLA:
        // the complex problem is here, that a float value is not really what we write.
        // So a 3.0 could also be 3 or 3.0 or 3.0000001 or 2.9999999
        // this has to be checked.
        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append 3.0";

    }

    TEST_F(append_008_float, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr75 );
        float                  input = (float)atof("3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append 3.5";

    }

    TEST_F(append_008_float, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr76 );
        float                  input = (float)atof("3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append 3.0625";

    }

    TEST_F(append_008_float, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr77 );
        float                  input = (float)atof("3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append 3.502525";

    }

    TEST_F(append_008_float, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr78 );
        float                  input = (float)atof("3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append 3.141592";

    }

    TEST_F(append_008_float, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr79 );
        float                  input = (float)atof("3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append 3.5025255";

    }

    TEST_F(append_008_float, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr80 );
        float                  input = (float)atof("3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append 3.0039062";

    }

    TEST_F(append_008_float, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr81 );
        float                  input = (float)atof("3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append 3.0";

    }

    TEST_F(append_008_float, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr82 );
        float                  input = (float)atof("3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append 3.5";

    }

    TEST_F(append_008_float, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr83 );
        float                  input = (float)atof("3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append 3.0625";

    }

    TEST_F(append_008_float, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr84 );
        float                  input = (float)atof("3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append 3.502525";

    }

    TEST_F(append_008_float, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr85 );
        float                  input = (float)atof("3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append 3.141592";

    }

    TEST_F(append_008_float, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr86 );
        float                  input = (float)atof("3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append 3.5025255";

    }

    TEST_F(append_008_float, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr87 );
        float                  input = (float)atof("3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append 3.0039062";

    }

    TEST_F(append_008_float, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr81 );
        float                  input = (float)atof("3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append 3.0";

    }

    TEST_F(append_008_float, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr82 );
        float                  input = (float)atof("3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append 3.5";

    }

    TEST_F(append_008_float, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr83 );
        float                  input = (float)atof("3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append 3.0625";

    }

    TEST_F(append_008_float, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr84 );
        float                  input = (float)atof("3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append 3.502525";

    }

    TEST_F(append_008_float, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr85 );
        float                  input = (float)atof("3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append 3.141592";

    }

    TEST_F(append_008_float, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr86 );
        float                  input = (float)atof("3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append 3.5025255";

    }

    TEST_F(append_008_float, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr87 );
        float                  input = (float)atof("3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append 3.0039062";

    }

    TEST_F(append_008_float, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr81 );
        float                  input = (float)atof("3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append 3.0";

    }

    TEST_F(append_008_float, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr82 );
        float                  input = (float)atof("3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append 3.5";

    }

    TEST_F(append_008_float, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr83 );
        float                  input = (float)atof("3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append 3.0625";

    }

    TEST_F(append_008_float, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr84 );
        float                  input = (float)atof("3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append 3.502525";

    }

    TEST_F(append_008_float, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr85 );
        float                  input = (float)atof("3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append 3.141592";

    }

    TEST_F(append_008_float, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr86 );
        float                  input = (float)atof("3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append 3.5025255";

    }

    TEST_F(append_008_float, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr87 );
        float                  input = (float)atof("3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append 3.0039062";

    }

    TEST_F(append_008_float, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr88 );
        float                  input = (float)atof("3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append 3.0";

    }

    TEST_F(append_008_float, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr89 );
        float                  input = (float)atof("3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append 3.5";

    }

    TEST_F(append_008_float, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr90 );
        float                  input = (float)atof("3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append 3.0625";

    }

    TEST_F(append_008_float, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr91 );
        float                  input = (float)atof("3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append 3.502525";

    }

    TEST_F(append_008_float, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr92 );
        float                  input = (float)atof("3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append 3.141592";

    }

    TEST_F(append_008_float, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr93 );
        float                  input = (float)atof("3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append 3.5025255";

    }

    TEST_F(append_008_float, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr94 );
        float                  input = (float)atof("3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append 3.0039062";

    }
#ifdef WITH_CORE
    TEST_F(append_008_float, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr81 );
        float                  input = (float)atof("3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append 3.0";

    }

    TEST_F(append_008_float, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr82 );
        float                  input = (float)atof("3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append 3.5";

    }

    TEST_F(append_008_float, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr83 );
        float                  input = (float)atof("3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append 3.0625";

    }

    TEST_F(append_008_float, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr84 );
        float                  input = (float)atof("3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append 3.502525";

    }

    TEST_F(append_008_float, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr85 );
        float                  input = (float)atof("3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append 3.141592";

    }

    TEST_F(append_008_float, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr86 );
        float                  input = (float)atof("3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append 3.5025255";

    }

    TEST_F(append_008_float, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr87 );
        float                  input = (float)atof("3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append 3.0039062";

    }
#endif

//------------------------------------------------------------------------
// testing the method append( float f ) for negative value
//------------------------------------------------------------------------
    class  append_008_Float_Negative : public checkfloat
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_008_Float_Negative, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr95 );
        float                  input = (float)atof("-3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append -3.0";

    }

    TEST_F(append_008_Float_Negative, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr96 );
        float                  input = (float)atof("-3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append -3.5";

    }

    TEST_F(append_008_Float_Negative, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr97 );
        float                  input = (float)atof("-3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append -3.0625";

    }

    TEST_F(append_008_Float_Negative, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr98 );
        float                  input = (float)atof("-3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append -3.502525";

    }

    TEST_F(append_008_Float_Negative, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr99 );
        float                  input = (float)atof("-3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append -3.141592";

    }

    TEST_F(append_008_Float_Negative, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr100 );
        float                  input = (float)atof("-3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append -3.5025255";

    }

    TEST_F(append_008_Float_Negative, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr101 );
        float                  input = (float)atof("-3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[0] append -3.0039062";

    }

    TEST_F(append_008_Float_Negative, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr102 );
        float                  input = (float)atof("-3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append -3.0";

    }

    TEST_F(append_008_Float_Negative, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr103 );
        float                  input = (float)atof("-3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append -3.5";

    }

    TEST_F(append_008_Float_Negative, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr104 );
        float                  input = (float)atof("-3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append -3.0625";

    }

    TEST_F(append_008_Float_Negative, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr105 );
        float                  input = (float)atof("-3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append -3.502525";

    }

    TEST_F(append_008_Float_Negative, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr106 );
        float                  input = (float)atof("-3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append -3.141592";

    }

    TEST_F(append_008_Float_Negative, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr107 );
        float                  input = (float)atof("-3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append -3.5025255";

    }

    TEST_F(append_008_Float_Negative, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        // LLA: OString                expVal( kTestStr108 );
        float                  input = (float)atof("-3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[1] append -3.0039062";

    }

    TEST_F(append_008_Float_Negative, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr102 );
        float                  input = (float)atof("-3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append -3.0";

    }

    TEST_F(append_008_Float_Negative, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr103 );
        float                  input = (float)atof("-3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append -3.5";

    }

    TEST_F(append_008_Float_Negative, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr104 );
        float                  input = (float)atof("-3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append -3.0625";

    }

    TEST_F(append_008_Float_Negative, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr105 );
        float                  input = (float)atof("-3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append -3.502525";

    }

    TEST_F(append_008_Float_Negative, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr106 );
        float                  input = (float)atof("-3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append -3.141592";

    }

    TEST_F(append_008_Float_Negative, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr107 );
        float                  input = (float)atof("-3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append -3.5025255";

    }

    TEST_F(append_008_Float_Negative, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        // LLA: OString                expVal( kTestStr108 );
        float                  input = (float)atof("-3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[2] append -3.0039062";

    }

    TEST_F(append_008_Float_Negative, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr102 );
        float                  input = (float)atof("-3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append -3.0";

    }

    TEST_F(append_008_Float_Negative, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr103 );
        float                  input = (float)atof("-3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append -3.5";

    }

    TEST_F(append_008_Float_Negative, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr104 );
        float                  input = (float)atof("-3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append -3.0625";

    }

    TEST_F(append_008_Float_Negative, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr105 );
        float                  input = (float)atof("-3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append -3.502525";

    }

    TEST_F(append_008_Float_Negative, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr106 );
        float                  input = (float)atof("-3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append -3.141592";

    }

    TEST_F(append_008_Float_Negative, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr107 );
        float                  input = (float)atof("-3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append -3.5025255";

    }

    TEST_F(append_008_Float_Negative, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        // LLA: OString                expVal( kTestStr108 );
        float                  input = (float)atof("-3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[3] append -3.0039062";

    }

    TEST_F(append_008_Float_Negative, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr109 );
        float                  input = (float)atof("-3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append -3.0";

    }

    TEST_F(append_008_Float_Negative, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr110 );
        float                  input = (float)atof("-3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append -3.5";

    }

    TEST_F(append_008_Float_Negative, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr111 );
        float                  input = (float)atof("-3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append -3.0625";

    }

    TEST_F(append_008_Float_Negative, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr112 );
        float                  input = (float)atof("-3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append -3.502525";

    }

    TEST_F(append_008_Float_Negative, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr113 );
        float                  input = (float)atof("-3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append -3.141592";

    }

    TEST_F(append_008_Float_Negative, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr114 );
        float                  input = (float)atof("-3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append -3.5025255";

    }

    TEST_F(append_008_Float_Negative, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr115 );
        float                  input = (float)atof("-3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "arrOUS[4] append -3.0039062";

    }
#ifdef WITH_CORE
    TEST_F(append_008_Float_Negative, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr102 );
        float                  input = (float)atof("-3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append -3.0";

    }

    TEST_F(append_008_Float_Negative, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr103 );
        float                  input = (float)atof("-3.5");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append -3.5";

    }

    TEST_F(append_008_Float_Negative, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr104 );
        float                  input = (float)atof("-3.0625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append -3.0625";

    }

    TEST_F(append_008_Float_Negative, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr105 );
        float                  input = (float)atof("-3.502525");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append -3.502525";

    }

    TEST_F(append_008_Float_Negative, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr106 );
        float                  input = (float)atof("-3.141592");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append -3.141592";

    }

    TEST_F(append_008_Float_Negative, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr107 );
        float                  input = (float)atof("-3.5025255");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append -3.5025255";

    }

    TEST_F(append_008_Float_Negative, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        // LLA: OString                expVal( kTestStr108 );
        float                  input = (float)atof("-3.00390625");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheFloat(aStrBuf, nLen, input)) << "OStringBuffer( kSInt32Max ) append -3.0039062";

    }
#endif

//------------------------------------------------------------------------
// testing the method append( double d )
//------------------------------------------------------------------------

    class checkdouble : public ::testing::Test
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
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_009_double, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr74 );
        double                 input = atof("3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheDouble(aStrBuf, nLen, input)) << "arrOUS[0] append 3.0";

    }

/*
    TEST_F(append_009_double, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr75 );
        double                 input = atof("3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append 3.5";

    }

    TEST_F(append_009_double, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr76 );
        double                 input = atof("3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append 3.0625";

    }

    TEST_F(append_009_double, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr122 );
        double                 input = atof("3.1415926535");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append 3.1415926535";

    }

    TEST_F(append_009_double, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr123 );
        double                 input = atof("3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append 3.141592653589793";

    }

    TEST_F(append_009_double, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr124 );
        double                  input = atof("3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append 3.14159265358979323";

    }

    TEST_F(append_009_double, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr125 );
        double                 input = atof("3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append 3.141592653589793238462643";

    }

    TEST_F(append_009_double, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr81 );
        double                 input = atof("3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append 3.0";

    }

    TEST_F(append_009_double, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr82 );
        double                 input = atof("3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append 3.5";

    }

    TEST_F(append_009_double, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr83 );
        double                 input = atof("3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append 3.0625";

    }

    TEST_F(append_009_double, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr126 );
        double                 input = atof("3.1415926535");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append 3.1415926535";

    }

    TEST_F(append_009_double, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr127 );
        double                 input = atof("3.141592653589793");

        aStrBuf.append( input );
        OString     *result = new OString( aStrBuf.getStr());
        double      output = result->toDouble();
        OString     *final = new OString();
        *final = final->valueOf(output);
t_print("the OStringvalus is:");
for(int m=0;m<final->getLength();m++)
{
t_print("%c",final->pData->buffer[m]);
}
t_print("\n");
t_print("the OStringBuffer is %d\n", aStrBuf.getLength());
t_print("the expVal is %d\n", expVal.getLength());
t_print("the OStringbuffervalus is:");
for(int j=0;j<aStrBuf.getLength();j++)
{
t_print("%c",*(aStrBuf.getStr()+j));
}
t_print("\n");
t_print("the expVlavalus is:");
for(int k=0;k<expVal.getLength();k++)
{
t_print("%c",expVal.pData->buffer[k]);
}
t_print("\n");
        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append 3.141592653589793";

    }

    TEST_F(append_009_double, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr128 );
        double                  input = atof("3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append 3.14159265358979323";

    }

    TEST_F(append_009_double, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr129 );
        double                 input = atof("3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append 3.141592653589793238462643";

    }

    TEST_F(append_009_double, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr81 );
        double                 input = atof("3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append 3.0";

    }

    TEST_F(append_009_double, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr82 );
        double                 input = atof("3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append 3.5";

    }

    TEST_F(append_009_double, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr83 );
        double                 input = atof("3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append 3.0625";

    }

    TEST_F(append_009_double, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr126 );
        double                 input = atof("3.1415926535");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append 3.1415926535";

    }

    TEST_F(append_009_double, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr127 );
        double                 input = atof("3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append 3.141592653589793";

    }

    TEST_F(append_009_double, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr128 );
        double                  input = atof("3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append 3.14159265358979323";

    }

    TEST_F(append_009_double, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr129 );
        double                 input = atof("3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append 3.141592653589793238462643";

    }

    TEST_F(append_009_double, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr81 );
        double                 input = atof("3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append 3.0";

    }

    TEST_F(append_009_double, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr82 );
        double                 input = atof("3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append 3.5";

    }

    TEST_F(append_009_double, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr83 );
        double                 input = atof("3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append 3.0625";

    }

    TEST_F(append_009_double, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr126 );
        double                 input = atof("3.1415926535");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append 3.1415926535";

    }

    TEST_F(append_009_double, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr127 );
        double                 input = atof("3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append 3.141592653589793";

    }

    TEST_F(append_009_double, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr128 );
        double                  input = atof("3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append 3.14159265358979323";

    }

    TEST_F(append_009_double, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr129 );
        double                 input = atof("3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append 3.141592653589793238462643";

    }

    TEST_F(append_009_double, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr88 );
        double                 input = atof("3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append 3.0";

    }

    TEST_F(append_009_double, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr89 );
        double                 input = atof("3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append 3.5";

    }

    TEST_F(append_009_double, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr90 );
        double                 input = atof("3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append 3.0625";

    }

    TEST_F(append_009_double, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr130 );
        double                 input = atof("3.1415926535");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append 3.1415926535";

    }

    TEST_F(append_009_double, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr131 );
        double                 input = atof("3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append 3.141592653589793";

    }

    TEST_F(append_009_double, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr132 );
        double                  input = atof("3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append 3.14159265358979323";

    }
*/
    TEST_F(append_009_double, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr133 );
        double                 input = atof("3.141592653589793238462643");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheDouble(aStrBuf, nLen, input)) << "arrOUS[4] append 3.141592653589793238462643";

    }
/*
#ifdef WITH_CORE
    TEST_F(append_009_double, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr81 );
        double                 input = atof("3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append 3.0";

    }

    TEST_F(append_009_double, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr82 );
        double                 input = atof("3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append 3.5";

    }

    TEST_F(append_009_double, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr83 );
        double                 input = atof("3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append 3.0625";

    }

    TEST_F(append_009_double, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr126 );
        double                 input = atof("3.1415926535");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append 3.1415926535";

    }

    TEST_F(append_009_double, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr127 );
        double                 input = atof("3.141592653589793";

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append 3.141592653589793";

    }

    TEST_F(append_009_double, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr128 );
        double                 input = atof("3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append 3.14159265358979323";

    }

    TEST_F(append_009_double, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr129 );
        double                 input = atof("3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append 3.141592653589793238462643";

    }
#endif
*/

//------------------------------------------------------------------------
// testing the method append( double f ) for negative value
//------------------------------------------------------------------------
    class  append_009_Double_Negative : public checkdouble
    {
    protected:
        OString* arrOUS[5];

    public:
        void SetUp()
        {
            arrOUS[0] = new OString( kTestStr7 );
            arrOUS[1] = new OString(  );
            arrOUS[2] = new OString( kTestStr25 );
            arrOUS[3] = new OString( "\0"  );
            arrOUS[4] = new OString( kTestStr28 );

        }

        void TearDown()
        {
            delete arrOUS[0]; delete arrOUS[1]; delete arrOUS[2];
            delete arrOUS[3]; delete arrOUS[4];
        }
    };

    TEST_F(append_009_Double_Negative, append_001)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        // LLA: OString                expVal( kTestStr95 );
        double                 input = atof("-3.0");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheDouble(aStrBuf, nLen, input)) << "arrOUS[0] append -3.0";

    }
/*
    TEST_F(append_009_Double_Negative, append_002)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr96 );
        double                 input = atof("-3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append -3.5";

    }

    TEST_F(append_009_Double_Negative, append_003)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr97 );
        double                 input = atof("-3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append -3.0625";

    }

    TEST_F(append_009_Double_Negative, append_004)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr98 );
        double                 input = atof("-3.502525");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append -3.502525";

    }

    TEST_F(append_009_Double_Negative, append_005)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr134 );
        double                 input = atof("-3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append -3.141592653589793";

    }

    TEST_F(append_009_Double_Negative, append_006)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr135 );
        double                  input = atof("-3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append -3.14159265358979323";

    }

    TEST_F(append_009_Double_Negative, append_007)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[0] );
        OString                expVal( kTestStr136 );
        double                 input = atof("-3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[0] append -3.141592653589793238462643";

    }

    TEST_F(append_009_Double_Negative, append_008)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr102 );
        double                 input = atof("-3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append -3.0";

    }

    TEST_F(append_009_Double_Negative, append_009)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr103 );
        double                 input = atof("-3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append -3.5";

    }

    TEST_F(append_009_Double_Negative, append_010)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr104 );
        double                 input = atof("-3.0625");

        aStrBuf.append( input );
        OString     *result = new OString( aStrBuf.getStr());
        double      output = result->toDouble();
        OString     *final = new OString();
        *final = final->valueOf(output);
t_print("the OStringvalus is:");
for(int m=0;m<final->getLength();m++)
{
t_print("%c",final->pData->buffer[m]);
}
t_print("\n");
t_print("the OStringBuffer is %d\n", aStrBuf.getLength());
t_print("the expVal is %d\n", expVal.getLength());
t_print("the OStringbuffervalus is:");
for(int j=0;j<aStrBuf.getLength();j++)
{
t_print("%c",*(aStrBuf.getStr()+j));
}
t_print("\n");
t_print("the expVlavalus is:");
for(int k=0;k<expVal.getLength();k++)
{
t_print("%c",expVal.pData->buffer[k]);
}
t_print("\n");
        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append -3.0625";

    }

    TEST_F(append_009_Double_Negative, append_011)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr105 );
        double                 input = atof("-3.502525");

        aStrBuf.append( input );

        double      output = atof("-3.50252");
        OString     *final = new OString();
        *final = final->valueOf(output);
t_print("the OStringvalus is:");
for(int m=0;m<final->getLength();m++)
{
t_print("%c",final->pData->buffer[m]);
}
t_print("\n");
t_print("the OStringBuffer is %d\n", aStrBuf.getLength());
t_print("the expVal is %d\n", expVal.getLength());
t_print("the OStringbuffervalus is:");
for(int j=0;j<aStrBuf.getLength();j++)
{
t_print("%c",*(aStrBuf.getStr()+j));
}
t_print("\n");
t_print("the expVlavalus is:");
for(int k=0;k<expVal.getLength();k++)
{
t_print("%c",expVal.pData->buffer[k]);
}
t_print("\n");
        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append -3.502525";

    }

    TEST_F(append_009_Double_Negative, append_012)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr137 );
        double                 input = atof("-3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append -3.141592653589793";

    }

    TEST_F(append_009_Double_Negative, append_013)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr138 );
        double                  input = atof("-3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append -3.14159265358979323";

    }

    TEST_F(append_009_Double_Negative, append_014)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[1] );
        OString                expVal( kTestStr139 );
        double                 input = atof("-3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append -3.141592653589793238462643";

    }

    TEST_F(append_009_Double_Negative, append_015)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr102 );
        double                 input = atof("-3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append -3.0";

    }

    TEST_F(append_009_Double_Negative, append_016)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr103 );
        double                 input = atof("-3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append -3.5";

    }

    TEST_F(append_009_Double_Negative, append_017)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr104 );
        double                 input = atof("-3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append -3.0625";

    }

    TEST_F(append_009_Double_Negative, append_018)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr105 );
        double                 input = atof("-3.502525");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append -3.502525";

    }

    TEST_F(append_009_Double_Negative, append_019)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr137 );
        double                 input = atof("-3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append -3.141592653589793";

    }

    TEST_F(append_009_Double_Negative, append_020)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr138 );
        double                  input = atof("-3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append -3.14159265358979323";

    }

    TEST_F(append_009_Double_Negative, append_021)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[2] );
        OString                expVal( kTestStr139 );
        double                 input = atof("-3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[2] append -3.141592653589793238462643";

    }

    TEST_F(append_009_Double_Negative, append_022)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr102 );
        double                 input = atof("-3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append -3.0";

    }

    TEST_F(append_009_Double_Negative, append_023)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr103 );
        double                 input = atof("-3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append -3.5";

    }

    TEST_F(append_009_Double_Negative, append_024)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr104 );
        double                 input = atof("-3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append -3.0625";

    }

    TEST_F(append_009_Double_Negative, append_025)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr105 );
        double                 input = atof("-3.502525");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append -3.502525";

    }

    TEST_F(append_009_Double_Negative, append_026)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr137 );
        double                 input = atof("-3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append -3.141592653589793";

    }

    TEST_F(append_009_Double_Negative, append_027)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr138 );
        double                  input = atof("-3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[1] append -3.14159265358979323";

    }

    TEST_F(append_009_Double_Negative, append_028)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[3] );
        OString                expVal( kTestStr139 );
        double                 input = atof("-3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[3] append -3.141592653589793238462643";

    }

    TEST_F(append_009_Double_Negative, append_029)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr109 );
        double                 input = atof("-3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append -3.0";

    }

    TEST_F(append_009_Double_Negative, append_030)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr110 );
        double                 input = atof("-3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append -3.5";

    }

    TEST_F(append_009_Double_Negative, append_031)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr111 );
        double                 input = atof("-3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append -3.0625";

    }

    TEST_F(append_009_Double_Negative, append_032)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr112 );
        double                 input = atof("-3.502525");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append -3.502525";

    }

    TEST_F(append_009_Double_Negative, append_033)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr140 );
        double                 input = atof("-3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append -3.141592653589793";

    }

    TEST_F(append_009_Double_Negative, append_034)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        OString                expVal( kTestStr141 );
        double                  input = atof("-3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "arrOUS[4] append -3.14159265358979323";

    }

*/
    TEST_F(append_009_Double_Negative, append_035)
    {
        ::rtl::OStringBuffer   aStrBuf( *arrOUS[4] );
        // LLA: OString                expVal( kTestStr142 );
        double                 input = atof("-3.141592653589793238462643");

        sal_Int32 nLen = aStrBuf.getLength();
        aStrBuf.append( input );

        ASSERT_TRUE(checkIfStrBufContainAtPosTheDouble(aStrBuf, nLen, input)) << "arrOUS[4] append -3.141592653589793238462643";

    }
/*
#ifdef WITH_CORE
    TEST_F(append_009_Double_Negative, append_036)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr102 );
        double                 input = atof("-3.0");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append -3.0";

    }

    TEST_F(append_009_Double_Negative, append_037)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr103 );
        double                 input = atof("-3.5");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append -3.5";

    }

    TEST_F(append_009_Double_Negative, append_038)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr104 );
        double                 input = atof("-3.0625");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append -3.0625";

    }

    TEST_F(append_009_Double_Negative, append_039)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr105 );
        double                 input = atof("-3.502525");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append -3.502525";

    }

    TEST_F(append_009_Double_Negative, append_040)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr137 );
        double                 input = atof("-3.141592653589793");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append -3.141592653589793";

    }

    TEST_F(append_009_Double_Negative, append_041)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr138 );
        double                 input = atof("-3.14159265358979323");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append -3.14159265358979323";

    }

    TEST_F(append_009_Double_Negative, append_042)
    {
        ::rtl::OStringBuffer   aStrBuf( kSInt32Max );
        OString                expVal( kTestStr139 );
        double                 input = atof("-3.141592653589793238462643");

        aStrBuf.append( input );

        ASSERT_TRUE(aStrBuf == expVal && aStrBuf.getLength() == expVal.getLength()) << "OStringBuffer( kSInt32Max ) append -3.141592653589793238462643";

    }
#endif
*/

} // namespace rtl_OStringBuffer


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
