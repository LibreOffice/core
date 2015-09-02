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
#include "gtest/gtest.h"
#include <rtl/ustring.hxx>

/** print a UNI_CODE file name.
*/
inline void printOUString( ::rtl::OUString const & _suStr )
{
    rtl::OString aString;

    printf( "OUString: " );
    aString = ::rtl::OUStringToOString( _suStr, RTL_TEXTENCODING_ASCII_US );
    printf( "%s\n", aString.getStr( ) );
}


namespace rtl_ustr
{

    class compare : public ::testing::Test
    {
    public:
    }; // class compare

    TEST_F(compare, compare_000)
    {
        rtl_ustr_compare( NULL, NULL);
        // should not GPF
    }

    TEST_F(compare, compare_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl_ustr_compare( aStr1.getStr(), NULL);
        // should not GPF
    }
    TEST_F(compare, compare_001)
    {
        rtl::OUString aStr1;
        rtl::OUString aStr2;

        sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(compare, compare_002)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("Line must be equal.");

        sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(compare, compare_003)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("Line foo bar, ok, differ.");

        sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }

    class compareIgnoreAsciiCase : public ::testing::Test
    {
    public:
    }; // class compareIgnoreAsciiCase

    TEST_F(compareIgnoreAsciiCase, compare_000)
    {
        rtl_ustr_compareIgnoreAsciiCase( NULL, NULL);
    }

    TEST_F(compareIgnoreAsciiCase, compare_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), NULL);
    }
    TEST_F(compareIgnoreAsciiCase, compare_001)
    {
        rtl::OUString aStr1;
        rtl::OUString aStr2;

        sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(compareIgnoreAsciiCase, compare_002)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("Line must be equal.");

        sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(compareIgnoreAsciiCase, compare_002_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("LINE MUST BE EQUAL.");

        sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal (if case insensitve).";
    }

    TEST_F(compareIgnoreAsciiCase, compare_003)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("Line foo bar, ok, differ.");

        sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }
// -----------------------------------------------------------------------------

    class shortenedCompareIgnoreAsciiCase_WithLength : public ::testing::Test
    {
    public:
}; // class compare

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_000)
    {
        rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( NULL, 0, NULL, 0, 0);
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), NULL, 0, 1);
    }
    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_001)
    {
        rtl::OUString aStr1;
        rtl::OUString aStr2;

        sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), aStr2.getStr(), aStr2.getLength(), aStr1.getLength());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_002)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("Line must be equal.");

        sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                               aStr2.getStr(), aStr2.getLength(),
                                                                               aStr1.getLength());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_002_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("LINE MUST BE EQUAL.");

        sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                               aStr2.getStr(), aStr2.getLength(),
                                                                               aStr1.getLength());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal (if case insensitve).";
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_003)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("Line foo bar, ok, differ.");

        sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                               aStr2.getStr(), aStr2.getLength(),
                                                                               5);
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal first 5 characters.";
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_004)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OUString aStr2 = rtl::OUString::createFromAscii("Line foo bar, ok, differ.");

        sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                               aStr2.getStr(), aStr2.getLength(),
                                                                               aStr1.getLength());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }

// // -----------------------------------------------------------------------------
//
//     class hashCode : public ::testing::Test
//     {
//     public:
//     };
//
//     TEST_F(hashCode, hashCode_000)
//     {
//         sal_Int32 nHashCode = rtl_ustr_hashCode( NULL );
//         volatile int dummy = 0;
//     }
//
//     TEST_F(hashCode, hashCode_001)
//     {
//         rtl::OString aStr1 = "Line for a hashCode.";
//         sal_Int32 nHashCode = rtl_ustr_hashCode( aStr1.getStr() );
//         printf("hashcode: %d\n", nHashCode);
//         // ASSERT_TRUE(nValue == 0) << "failed.";
//     }
//
//     TEST_F(hashCode, hashCode_002)
//     {
//         rtl::OString aStr1 = "Line for a hashCode.";
//         sal_Int32 nHashCode1 = rtl_ustr_hashCode( aStr1.getStr() );
//
//         rtl::OString aStr2 = "Line for a hashCode.";
//         sal_Int32 nHashCode2 = rtl_ustr_hashCode( aStr2.getStr() );
//
//         ASSERT_TRUE(nHashCode1 == nHashCode2) << "hashcodes must be equal.";
//     }
//
//     TEST_F(hashCode, hashCode_003)
//     {
//         rtl::OString aStr1 = "Line for a hashCode.";
//         sal_Int32 nHashCode1 = rtl_ustr_hashCode( aStr1.getStr() );
//
//         rtl::OString aStr2 = "Line for an other hashcode.";
//         sal_Int32 nHashCode2 = rtl_ustr_hashCode( aStr2.getStr() );
//
//         ASSERT_TRUE(nHashCode1 != nHashCode2) << "hashcodes must differ.";
//     }
//
//
// // -----------------------------------------------------------------------------
//
    class indexOfChar : public ::testing::Test
    {
    public:
    }; // class indexOfChar

    TEST_F(indexOfChar, indexOfChar_000)
    {
        rtl_ustr_indexOfChar( NULL, 0 );
    }

    TEST_F(indexOfChar, indexOfChar_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a indexOfChar.");

        sal_Int32 nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'L' );
        ASSERT_TRUE(nIndex == 0) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'i' );
        ASSERT_TRUE(nIndex == 1) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'n' );
        ASSERT_TRUE(nIndex == 2) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'e' );
        ASSERT_TRUE(nIndex == 3) << "index is wrong.";
    }

    TEST_F(indexOfChar, indexOfChar_002)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a indexOfChar.");
        sal_Int32 nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'y' );

        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

// // -----------------------------------------------------------------------------
    class lastIndexOfChar : public ::testing::Test
    {
    public:
    }; // class lastIndexOfChar

    TEST_F(lastIndexOfChar, lastIndexOfChar_000)
    {
        rtl_ustr_lastIndexOfChar( NULL, 0 );
    }

    TEST_F(lastIndexOfChar, lastIndexOfChar_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a lastIndexOfChar.");

        sal_Int32 nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'C' );
        ASSERT_TRUE(nIndex == 22) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'h' );
        ASSERT_TRUE(nIndex == 23) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'a' );
        ASSERT_TRUE(nIndex == 24) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'r' );
        ASSERT_TRUE(nIndex == 25) << "index is wrong.";
    }

    TEST_F(lastIndexOfChar, lastIndexOfChar_002)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a lastIndexOfChar.");
        sal_Int32 nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'y' );

        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

// -----------------------------------------------------------------------------

    class indexOfStr : public ::testing::Test
    {
    public:
    }; // class compare

    TEST_F(indexOfStr, indexOfStr_000)
    {
        rtl_ustr_indexOfStr( NULL, 0 );
    }

    TEST_F(indexOfStr, indexOfStr_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a indexOfStr.");
        rtl_ustr_indexOfStr( aStr1.getStr(), 0 );
    }

    TEST_F(indexOfStr, indexOfStr_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a indexOfStr.");

        rtl::OUString suSearch = rtl::OUString::createFromAscii("Line");
        sal_Int32 nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );
        ASSERT_TRUE(nIndex == 0) << "index is wrong.";

        /* rtl::OUString */ suSearch = rtl::OUString::createFromAscii("for");
        /* sal_Int32 */ nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );
        ASSERT_TRUE(nIndex == 5) << "index is wrong.";

        /* rtl::OUString */ suSearch = rtl::OUString::createFromAscii("a");
        /* sal_Int32 */ nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );
        ASSERT_TRUE(nIndex == 9) << "index is wrong.";

        /* rtl::OUString */ suSearch = rtl::OUString::createFromAscii("a index");
        /* sal_Int32 */ nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );
        ASSERT_TRUE(nIndex ==9) << "index is wrong.";
    }

    TEST_F(indexOfStr, indexOfStr_002)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a indexOfStr.");
        rtl::OUString suSearch = rtl::OUString::createFromAscii("not exist");
        sal_Int32 nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );

        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

// -----------------------------------------------------------------------------


    class lastIndexOfStr : public ::testing::Test
    {
    public:
    }; // class lastIndexOfStr

    TEST_F(lastIndexOfStr, lastIndexOfStr_000)
    {
        rtl_ustr_lastIndexOfStr( NULL, NULL );
    }

    TEST_F(lastIndexOfStr, lastIndexOfStr_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a lastIndexOfStr.");
        rtl_ustr_lastIndexOfStr( aStr1.getStr(), NULL );
    }

    TEST_F(lastIndexOfStr, lastIndexOfStr_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a lastIndexOfStr.");
        rtl::OUString aSearchStr = rtl::OUString::createFromAscii("Index");

        sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
        ASSERT_TRUE(nIndex == 15) << "index is wrong.";

        /* rtl::OString */ aSearchStr = rtl::OUString::createFromAscii("Line");
        /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
        ASSERT_TRUE(nIndex == 0) << "index is wrong.";

        /* rtl::OString */ aSearchStr = rtl::OUString::createFromAscii("");
        /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

    TEST_F(lastIndexOfStr, lastIndexOfStr_002)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a lastIndexOfStr.");
        rtl::OUString aSearchStr = rtl::OUString::createFromAscii("foo");
        sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

    TEST_F(lastIndexOfStr, lastIndexOfStr_003)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line for a lastIndexOfStr.");
        rtl::OUString aSearchStr = rtl::OUString::createFromAscii("O");
        sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

        ASSERT_TRUE(nIndex == 20) << "index is wrong.";
    }

// -----------------------------------------------------------------------------

    class replaceChar : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(replaceChar, replaceChar_000)
    {
        rtl_ustr_replaceChar( NULL, 0, 0 );
    }

    TEST_F(replaceChar, replaceChar_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("replace char.");
        rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("ruplacu char.");

        sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
        sal_Unicode* pStr = (sal_Unicode*) malloc( nLength + sizeof(sal_Unicode)); // length + 1 (null terminator)
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        memset(pStr, 0, nLength + sizeof(sal_Unicode));
        memcpy(pStr, aStr1.getStr(), nLength);

        rtl_ustr_replaceChar( pStr, 'e', 'u' );
        rtl::OUString suStr(pStr, aStr1.getLength());

        ASSERT_TRUE(aShouldStr1.equals(suStr) == sal_True) << "replace failed";
        free(pStr);
    }

// -----------------------------------------------------------------------------

    class replaceChar_WithLength : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(replaceChar_WithLength, replaceChar_WithLength_000)
    {
        rtl_ustr_replaceChar_WithLength( NULL, 0, 0, 0 );
    }

    TEST_F(replaceChar_WithLength, replaceChar_WithLength_000_1)
    {
        rtl_ustr_replaceChar_WithLength( NULL, 1, 0, 0 );
    }
    TEST_F(replaceChar_WithLength, replaceChar_WithLength_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("replace char.");
        rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("ruplace char.");

        sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
        sal_Unicode* pStr = (sal_Unicode*) malloc(nLength);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        memcpy(pStr, aStr1.getStr(), nLength);

        rtl_ustr_replaceChar_WithLength( pStr, 6, 'e', 'u' );
        rtl::OUString suStr(pStr, aStr1.getLength());

        ASSERT_TRUE(aShouldStr1.equals(suStr) == sal_True) << "replace failed";
        free(pStr);
    }

    TEST_F(replaceChar_WithLength, replaceChar_WithLength_002)
    {
        rtl::OUString aStr1       = rtl::OUString::createFromAscii("eeeeeeeeeeeee");
        rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("uuuuuueeeeeee");

        sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
        sal_Unicode* pStr = (sal_Unicode*) malloc(nLength);                 // no null terminator is need
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        memcpy(pStr, aStr1.getStr(), nLength);

        rtl_ustr_replaceChar_WithLength( pStr, 6, 'e', 'u' );
        rtl::OUString suStr(pStr, aStr1.getLength());

        ASSERT_TRUE(aShouldStr1.equals(suStr) == sal_True) << "replace failed";
        free(pStr);
    }

// -----------------------------------------------------------------------------

    class toAsciiLowerCase : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(toAsciiLowerCase, toAsciiLowerCase_000)
    {
        rtl_ustr_toAsciiLowerCase( NULL );
    }

    TEST_F(toAsciiLowerCase, toAsciiLowerCase_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("CHANGE THIS TO ASCII LOWER CASE.");
        rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("change this to ascii lower case.");

        sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
        sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode) );  // we need to add '\0' so one more
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        memset(pStr, 0, nLength + sizeof(sal_Unicode));                             // empty the sal_Unicode array
        memcpy(pStr, aStr1.getStr(), nLength);

        rtl_ustr_toAsciiLowerCase( pStr );
        rtl::OUString suStr(pStr, aStr1.getLength());

        ASSERT_TRUE(aShouldStr1.equals(suStr) == sal_True) << "failed";
        free(pStr);
    }

    class toAsciiLowerCase_WithLength : public ::testing::Test
    {
    }; // class replaceChar

    TEST_F(toAsciiLowerCase, toAsciiLowerCase_WithLength_000)
    {
        rtl_ustr_toAsciiLowerCase_WithLength( NULL, 0 );
    }

    TEST_F(toAsciiLowerCase, toAsciiLowerCase_WithLength_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("CHANGE THIS TO ASCII LOWER CASE.");
        rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("change thiS TO ASCII LOWER CASE.");

        sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
        sal_Unicode* pStr = (sal_Unicode*) malloc(nLength);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        memcpy(pStr, aStr1.getStr(), nLength);

        rtl_ustr_toAsciiLowerCase_WithLength( pStr, 10 );

        rtl::OUString suStr(pStr, aStr1.getLength());
        sal_Bool bResult = aShouldStr1.equals(suStr);

        printOUString(suStr);
        printf("Result length: %d\n", suStr.getLength() );
        printf("Result: %d\n", bResult);

        ASSERT_TRUE(bResult == sal_True) << "failed";
        free(pStr);
    }

// -----------------------------------------------------------------------------

    class toAsciiUpperCase : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(toAsciiUpperCase, toAsciiUpperCase_000)
    {
        rtl_ustr_toAsciiUpperCase( NULL );
    }

    TEST_F(toAsciiUpperCase, toAsciiUpperCase_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("change this to ascii upper case.");
        rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("CHANGE THIS TO ASCII UPPER CASE.");

        sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
        sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode)); // length + null terminator
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        memset(pStr, 0, nLength + sizeof(sal_Unicode));
        memcpy(pStr, aStr1.getStr(), nLength);

        rtl_ustr_toAsciiUpperCase( pStr );
        rtl::OUString suStr(pStr, aStr1.getLength());

        ASSERT_TRUE(aShouldStr1.equals(suStr) == sal_True) << "failed";
        free(pStr);
    }

    class toAsciiUpperCase_WithLength : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(toAsciiUpperCase_WithLength, toAsciiUpperCase_WithLength_000)
    {
        rtl_ustr_toAsciiUpperCase_WithLength( NULL, 0 );
    }

    TEST_F(toAsciiUpperCase_WithLength, toAsciiUpperCase_WithLength_001)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("change this to ascii lower case.");
        rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("CHANGE THIs to ascii lower case.");

        sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
        sal_Unicode* pStr = (sal_Unicode*) malloc(nLength);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";

        memcpy(pStr, aStr1.getStr(), nLength);
        rtl_ustr_toAsciiUpperCase_WithLength( pStr, 10 );
        rtl::OUString suStr(pStr, aStr1.getLength());

        // printf("Uppercase with length: '%s'\n", aStr1.getStr());
        ASSERT_TRUE(aShouldStr1.equals(suStr) == sal_True) << "failed";
        free(pStr);
    }

    // -----------------------------------------------------------------------------

    class trim_WithLength : public ::testing::Test
    {
      public:
    };

    TEST_F(trim_WithLength, trim_WithLength_000)
    {
        rtl_ustr_trim_WithLength(NULL, 0);
        // should not GPF
    }

    TEST_F(trim_WithLength, trim_WithLength_000_1)
    {
        rtl::OUString suStr = rtl::OUString::createFromAscii("  trim this");

        sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
        sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
        memcpy(pStr, suStr.getStr(), nLength);

        rtl_ustr_trim_WithLength( pStr, 0 );
        free(pStr);
    }

    TEST_F(trim_WithLength, trim_WithLength_001)
    {
        rtl::OUString suStr = rtl::OUString::createFromAscii("  trim this");
        sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
        sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
        memcpy(pStr, suStr.getStr(), nLength);

        rtl_ustr_trim_WithLength( pStr, 2 );

        ASSERT_TRUE(rtl::OUString(pStr).getLength() == 0) << "string should be empty";
        free(pStr);
    }


    TEST_F(trim_WithLength, trim_WithLength_002)
    {
        rtl::OUString suStr = rtl::OUString::createFromAscii("trim this");

        sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
        sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
        memcpy(pStr, suStr.getStr(), nLength);

        rtl_ustr_trim_WithLength( pStr, 5 );

        ASSERT_TRUE(rtl::OUString(pStr).getLength() == 4) << "string should contain 'trim'";
        free(pStr);
    }


    TEST_F(trim_WithLength, trim_WithLength_003)
    {
        rtl::OUString suStr = rtl::OUString::createFromAscii("     trim   this");

        sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
        sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
        memcpy(pStr, suStr.getStr(), nLength);

        rtl_ustr_trim_WithLength( pStr, 11 );

        ASSERT_TRUE(rtl::OUString(pStr).getLength() == 4) << "string should contain 'trim'";
        free(pStr);
    }

    TEST_F(trim_WithLength, trim_WithLength_004)
    {
        rtl::OUString suStr = rtl::OUString::createFromAscii("\r\n\t \n\r    trim  \n this");

        sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
        sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
        memcpy(pStr, suStr.getStr(), nLength);

        rtl_ustr_trim_WithLength( pStr, 17 );

        ASSERT_TRUE(rtl::OUString(pStr).getLength() == 4) << "string should contain 'trim'";
        free(pStr);
    }

    TEST_F(trim_WithLength, trim_WithLength_005)
    {
        rtl::OUString suStr = rtl::OUString::createFromAscii("\r\n\t \n\r    trim \t this \n\r\t\t     ");

        sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
        sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
        memcpy(pStr, suStr.getStr(), nLength);

        rtl_ustr_trim_WithLength( pStr, suStr.getLength() );

        ASSERT_TRUE(rtl::OUString(pStr).getLength() == 11) << "string should contain 'trim \\t this'";
        free(pStr);
    }

    // -----------------------------------------------------------------------------

    class valueOfChar : public ::testing::Test
    {
      public:
    };

    TEST_F(valueOfChar, valueOfChar_000)
    {
        rtl_ustr_valueOfChar(NULL, 0);
        // should not GPF
    }
    TEST_F(valueOfChar, valueOfChar_001)
    {
        sal_Unicode *pStr = (sal_Unicode*)malloc(RTL_USTR_MAX_VALUEOFCHAR);
        if (pStr)
        {
            rtl_ustr_valueOfChar(pStr, 'A');

            ASSERT_TRUE(pStr[0] == L'A') << "string should contain 'A'";
            free(pStr);
        }
    }


    class ascii_compare_WithLength : public ::testing::Test
    {
    public:
    };

    TEST_F(ascii_compare_WithLength, zero_length)
    {
    sal_Unicode pUnicode[] = {0xffff, 0xffff};
    char const * pAscii = "reference";

    sal_Int32 value = rtl_ustr_ascii_compare_WithLength(pUnicode, 0, pAscii);
    ASSERT_TRUE(value < 0) << "ref string is empty, compare failed, needs to be <0.";
    }

    TEST_F(ascii_compare_WithLength, equal_ascii_shorter)
    {
    rtl::OUString refStr(RTL_CONSTASCII_USTRINGPARAM("referenceString"));
    char const * pAscii = "reference";

    sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
    ASSERT_TRUE(value > 0) << "ref string is bigger, compare failed, needs to be >0.";
    }

    TEST_F(ascii_compare_WithLength, equal_ascii_shorter_asciiLength)
    {
    rtl::OUString refStr(RTL_CONSTASCII_USTRINGPARAM("referenceString"));
    char const * pAscii = "reference";

    sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, rtl_str_getLength(pAscii), pAscii);
    ASSERT_TRUE(value == 0) << "ref string is bigger despite ascii length, compare failed, needs to be == 0.";
    }

    TEST_F(ascii_compare_WithLength, equal_ref_shorter)
    {
    rtl::OUString refStr(RTL_CONSTASCII_USTRINGPARAM("reference"));
    char const * pAscii = "referenceString";

    sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
    ASSERT_TRUE(value < 0) << "ascii string is bigger, but only compared to ref length, needs to be 0.";
    }

    TEST_F(ascii_compare_WithLength, equal)
    {
    rtl::OUString refStr(RTL_CONSTASCII_USTRINGPARAM("reference"));
    char const * pAscii = "reference";

    sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
    ASSERT_TRUE(value == 0) << "strings are equal, compare failed, needs to be 0.";
    }

    TEST_F(ascii_compare_WithLength, unequal_reference_bigger)
    {
    rtl::OUString refStr(RTL_CONSTASCII_USTRINGPARAM("defghi"));
    char const * pAscii = "abc";

    sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
    ASSERT_TRUE(value > 0) << "strings are unequal and ref is bigger, needs to be >0.";
    }

    TEST_F(ascii_compare_WithLength, unequal_ascii_bigger)
    {
    rtl::OUString refStr(RTL_CONSTASCII_USTRINGPARAM("abc"));
    char const * pAscii = "defghi";

    sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);

    ASSERT_TRUE(value < 0) << "strings are unequal and ascii is bigger, needs to be <0.";
    }


    class ascii_shortenedCompareIgnoreAsciiCase_WithLength : public ::testing::Test
    {
    public:
    }; // class ascii_shortenedCompareIgnoreAsciiCase_WithLength

    TEST_F(ascii_shortenedCompareIgnoreAsciiCase_WithLength, ascii_shortenedCompareIgnoreAsciiCase_WithLength_000)
    {
        rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( NULL, 0, NULL, 0);
        // should not GPF
    }

    TEST_F(ascii_shortenedCompareIgnoreAsciiCase_WithLength, ascii_shortenedCompareIgnoreAsciiCase_WithLength_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), NULL, 0);
        // should not GPF
    }
    TEST_F(ascii_shortenedCompareIgnoreAsciiCase_WithLength, ascii_shortenedCompareIgnoreAsciiCase_WithLength_000_2)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OString sStr2 =                                 "Line is shorter.";
        rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), sStr2.getLength(), sStr2.getStr(), 0);
        // should not GPF
    }
    TEST_F(ascii_shortenedCompareIgnoreAsciiCase_WithLength, ascii_shortenedCompareIgnoreAsciiCase_WithLength_001)
    {
        rtl::OUString suStr1;
        rtl::OString sStr2;

        sal_Int32 nValue = rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( suStr1.getStr(), 0, sStr2.getStr(), 0);
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(ascii_shortenedCompareIgnoreAsciiCase_WithLength, ascii_shortenedCompareIgnoreAsciiCase_WithLength_002)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OString sStr2 =                                  "Line must be equal.";

        sal_Int32 nValue = rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr(), sStr2.getLength());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(ascii_shortenedCompareIgnoreAsciiCase_WithLength, ascii_shortenedCompareIgnoreAsciiCase_WithLength_003)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OString sStr2 =                                  "Line must be differ and longer.";

        sal_Int32 nValue = rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr(), sStr2.getLength());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }

// -----------------------------------------------------------------------------

    class ascii_compareIgnoreAsciiCase_WithLength : public ::testing::Test
    {
    public:
    }; // class ascii_compareIgnoreAsciiCase_WithLength

    TEST_F(ascii_compareIgnoreAsciiCase_WithLength, ascii_compareIgnoreAsciiCase_WithLength_000)
    {
        rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( NULL, 0, NULL);
        // should not GPF
    }

    TEST_F(ascii_compareIgnoreAsciiCase_WithLength, ascii_compareIgnoreAsciiCase_WithLength_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( aStr1.getStr(), 0, NULL);
        // should not GPF
    }
    TEST_F(ascii_compareIgnoreAsciiCase_WithLength, ascii_compareIgnoreAsciiCase_WithLength_000_2)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OString sStr2 =                                 "Line is shorter.";
        rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( aStr1.getStr(), sStr2.getLength(), sStr2.getStr());
        // should not GPF
    }
    TEST_F(ascii_compareIgnoreAsciiCase_WithLength, ascii_compareIgnoreAsciiCase_WithLength_001)
    {
        rtl::OUString suStr1;
        rtl::OString sStr2;

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( suStr1.getStr(), 0, sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compareIgnoreAsciiCase_WithLength failed, strings are equal.";
    }

    TEST_F(ascii_compareIgnoreAsciiCase_WithLength, ascii_compareIgnoreAsciiCase_WithLength_002)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OString sStr2 =                                  "Line must be equal.";

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(ascii_compareIgnoreAsciiCase_WithLength, ascii_compareIgnoreAsciiCase_WithLength_003)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OString sStr2 =                                  "Line must be differ and longer.";

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }
// -----------------------------------------------------------------------------

    class ascii_compare : public ::testing::Test
    {
    public:
    }; // class ascii_compare

    TEST_F(ascii_compare, ascii_compare_000)
    {
        rtl_ustr_ascii_compare( NULL, NULL);
        // should not GPF
    }

    TEST_F(ascii_compare, ascii_compare_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl_ustr_ascii_compare( aStr1.getStr(), NULL);
        // should not GPF
    }
    TEST_F(ascii_compare, ascii_compare_001)
    {
        rtl::OUString suStr1;
        rtl::OString sStr2;

        sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(ascii_compare, ascii_compare_002)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OString sStr2 =                                  "Line must be equal.";

        sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(ascii_compare, ascii_compare_003)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OString sStr2 = "Line foo bar, ok, differ.";

        sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }

// -----------------------------------------------------------------------------

    class ascii_compareIgnoreAsciiCase : public ::testing::Test
    {
    public:
    }; // class ascii_compareIgnoreAsciiCase

    TEST_F(ascii_compareIgnoreAsciiCase, ascii_compareIgnoreAsciiCase_000)
    {
        rtl_ustr_ascii_compareIgnoreAsciiCase( NULL, NULL);
        // should not GPF
    }

    TEST_F(ascii_compareIgnoreAsciiCase, ascii_compareIgnoreAsciiCase_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl_ustr_ascii_compareIgnoreAsciiCase( aStr1.getStr(), NULL);
        // should not GPF
    }
    TEST_F(ascii_compareIgnoreAsciiCase, ascii_compareIgnoreAsciiCase_001)
    {
        rtl::OUString suStr1;
        rtl::OString sStr2;

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(ascii_compareIgnoreAsciiCase, ascii_compareIgnoreAsciiCase_002)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OString sStr2 =                                  "Line must be equal.";

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(ascii_compareIgnoreAsciiCase, ascii_compareIgnoreAsciiCase_002_1)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must be equal, when ignore case.");
        rtl::OString sStr2 =                                 "LINE MUST BE EQUAL, WHEN IGNORE CASE.";

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal (if case insensitve).";
    }

    TEST_F(ascii_compareIgnoreAsciiCase, ascii_compareIgnoreAsciiCase_003)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OString sStr2 = "Line foo bar, ok, differ.";

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }

//! LLA: some more tests with some high level strings

//     TEST_F(ascii_compareIgnoreAsciiCase, ascii_compareIgnoreAsciiCase_001()
//     {
//         rtl::OUString suStr1 = rtl::OUString::createFromAscii("change this to ascii upper case.");
//         rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("CHANGE THIS TO ASCII UPPER CASE.");
//
//         sal_uInt32 nLength = suStr1.getLength() * sizeof(sal_Unicode);
//         sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode)); // length + null terminator
//         ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
//         memset(pStr, 0, nLength + sizeof(sal_Unicode));
//         memcpy(pStr, suStr1.getStr(), nLength);
//
//         rtl_ustr_ascii_compareIgnoreAsciiCase( pStr );
//         rtl::OUString suStr(pStr, suStr1.getLength());
//
//         ASSERT_TRUE(aShouldStr1.equals(suStr) == sal_True) << "failed";
//         free(pStr);
//     }

    // sample out of inc/rtl/ustring.hxx
    // rtl_uString * pToken = NULL;
    // sal_Int32 nIndex = 0;
    // do
    // {
    //       ...
    //       nIndex = rtl_uString_getToken(&pToken, pStr, 0, ';', nIndex);
    //       ...
    // }
    // while (nIndex >= 0);

    class getToken : public ::testing::Test
    {
    public:
    }; // class ascii_compareIgnoreAsciiCase

    TEST_F(getToken, getToken_000)
    {
        rtl_ustr_ascii_compareIgnoreAsciiCase( NULL, NULL);
        // should not GPF
    }

    TEST_F(getToken, ascii_compareIgnoreAsciiCase_000_1)
    {
        rtl::OUString aStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl_ustr_ascii_compareIgnoreAsciiCase( aStr1.getStr(), NULL);
        // should not GPF
    }
    TEST_F(getToken, ascii_compareIgnoreAsciiCase_001)
    {
        rtl::OUString suStr1;
        rtl::OString sStr2;

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(getToken, ascii_compareIgnoreAsciiCase_002)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must be equal.");
        rtl::OString sStr2 =                                  "Line must be equal.";

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(getToken, ascii_compareIgnoreAsciiCase_002_1)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must be equal, when ignore case.");
        rtl::OString sStr2 =                                 "LINE MUST BE EQUAL, WHEN IGNORE CASE.";

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal (if case insensitve).";
    }

    TEST_F(getToken, ascii_compareIgnoreAsciiCase_003)
    {
        rtl::OUString suStr1 = rtl::OUString::createFromAscii("Line must differ.");
        rtl::OString sStr2 = "Line foo bar, ok, differ.";

        sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }

//! LLA: some more tests with some high level strings

//     TEST_F(getToken, ascii_compareIgnoreAsciiCase_001)
//     {
//         rtl::OUString suStr1 = rtl::OUString::createFromAscii("change this to ascii upper case.");
//         rtl::OUString aShouldStr1 = rtl::OUString::createFromAscii("CHANGE THIS TO ASCII UPPER CASE.");
//
//         sal_uInt32 nLength = suStr1.getLength() * sizeof(sal_Unicode);
//         sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode)); // length + null terminator
//         ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
//         memset(pStr, 0, nLength + sizeof(sal_Unicode));
//         memcpy(pStr, suStr1.getStr(), nLength);
//
//         rtl_ustr_ascii_compareIgnoreAsciiCase( pStr );
//         rtl::OUString suStr(pStr, suStr1.getLength());
//
//         ASSERT_TRUE(aShouldStr1.equals(suStr) == sal_True) << "failed";
//         free(pStr);
//     }

} // namespace rtl_ustr


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
