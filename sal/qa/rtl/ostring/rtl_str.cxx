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
#include <rtl/string.hxx>

namespace rtl_str
{

    class compare : public ::testing::Test
    {
    public:
    }; // class compare

    TEST_F(compare, compare_000)
    {
        rtl_str_compare( NULL, NULL);
    }

    TEST_F(compare, compare_000_1)
    {
        rtl::OString aStr1 = "Line must be equal.";
        rtl_str_compare( aStr1.getStr(), NULL);
    }
    TEST_F(compare, compare_001)
    {
        rtl::OString aStr1 = "";
        rtl::OString aStr2 = "";

        sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(compare, compare_002)
    {
        rtl::OString aStr1 = "Line must be equal.";
        rtl::OString aStr2 = "Line must be equal.";

        sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(compare, compare_003)
    {
        rtl::OString aStr1 = "Line must differ.";
        rtl::OString aStr2 = "Line foo bar, ok, differ.";

        sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }

    class compareIgnoreAsciiCase : public ::testing::Test
    {
    public:
    }; // class compareIgnoreAsciiCase

    TEST_F(compareIgnoreAsciiCase, compare_000)
    {
        rtl_str_compareIgnoreAsciiCase( NULL, NULL);
    }

    TEST_F(compareIgnoreAsciiCase, compare_000_1)
    {
        rtl::OString aStr1 = "Line must be equal.";
        rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), NULL);
    }
    TEST_F(compareIgnoreAsciiCase, compare_001)
    {
        rtl::OString aStr1 = "";
        rtl::OString aStr2 = "";

        sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(compareIgnoreAsciiCase, compare_002)
    {
        rtl::OString aStr1 = "Line must be equal.";
        rtl::OString aStr2 = "Line must be equal.";

        sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(compareIgnoreAsciiCase, compare_002_1)
    {
        rtl::OString aStr1 = "Line must be equal.";
        rtl::OString aStr2 = "LINE MUST BE EQUAL.";

        sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal (if case insensitve).";
    }

    TEST_F(compareIgnoreAsciiCase, compare_003)
    {
        rtl::OString aStr1 = "Line must differ.";
        rtl::OString aStr2 = "Line foo bar, ok, differ.";

        sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }
// -----------------------------------------------------------------------------

    class shortenedCompareIgnoreAsciiCase_WithLength : public ::testing::Test
    {
    public:
    }; // class compare

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_000)
    {
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( NULL, 0, NULL, 0, 0);
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_000_1)
    {
        rtl::OString aStr1 = "Line must be equal.";
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), NULL, 0, 1);
    }
    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_001)
    {
        rtl::OString aStr1 = "";
        rtl::OString aStr2 = "";

        sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), aStr2.getStr(), aStr2.getLength(), aStr1.getLength());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_002)
    {
        rtl::OString aStr1 = "Line must be equal.";
        rtl::OString aStr2 = "Line must be equal.";

        sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                               aStr2.getStr(), aStr2.getLength(),
                                                                               aStr1.getLength());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal.";
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_002_1)
    {
        rtl::OString aStr1 = "Line must be equal.";
        rtl::OString aStr2 = "LINE MUST BE EQUAL.";

        sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                               aStr2.getStr(), aStr2.getLength(),
                                                                               aStr1.getLength());
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal (if case insensitve).";
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_003)
    {
        rtl::OString aStr1 = "Line must differ.";
        rtl::OString aStr2 = "Line foo bar, ok, differ.";

        sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                               aStr2.getStr(), aStr2.getLength(),
                                                                               5);
        ASSERT_TRUE(nValue == 0) << "compare failed, strings are equal first 5 characters.";
    }

    TEST_F(shortenedCompareIgnoreAsciiCase_WithLength, compare_004)
    {
        rtl::OString aStr1 = "Line must differ.";
        rtl::OString aStr2 = "Line foo bar, ok, differ.";

        sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                               aStr2.getStr(), aStr2.getLength(),
                                                                               aStr1.getLength());
        ASSERT_TRUE(nValue != 0) << "compare failed, strings differ.";
    }

// -----------------------------------------------------------------------------

    class hashCode : public ::testing::Test
    {
    public:
    }; // class compare

    TEST_F(hashCode, hashCode_000)
    {
        rtl_str_hashCode( NULL );
    }

    TEST_F(hashCode, hashCode_001)
    {
        rtl::OString aStr1 = "Line for a hashCode.";
        sal_Int32 nHashCode = rtl_str_hashCode( aStr1.getStr() );
        printf("hashcode: %d\n", nHashCode);
        // ASSERT_TRUE(nValue == 0) << "failed.";
    }

    TEST_F(hashCode, hashCode_002)
    {
        rtl::OString aStr1 = "Line for a hashCode.";
        sal_Int32 nHashCode1 = rtl_str_hashCode( aStr1.getStr() );

        rtl::OString aStr2 = "Line for a hashCode.";
        sal_Int32 nHashCode2 = rtl_str_hashCode( aStr2.getStr() );

        ASSERT_TRUE(nHashCode1 == nHashCode2) << "hashcodes must be equal.";
    }

    TEST_F(hashCode, hashCode_003)
    {
        rtl::OString aStr1 = "Line for a hashCode.";
        sal_Int32 nHashCode1 = rtl_str_hashCode( aStr1.getStr() );

        rtl::OString aStr2 = "Line for an other hashcode.";
        sal_Int32 nHashCode2 = rtl_str_hashCode( aStr2.getStr() );

        ASSERT_TRUE(nHashCode1 != nHashCode2) << "hashcodes must differ.";
    }

// -----------------------------------------------------------------------------

    class indexOfChar : public ::testing::Test
    {
    public:
    }; // class compare

    TEST_F(indexOfChar, indexOfChar_000)
    {
        rtl_str_indexOfChar( NULL, 0 );
    }

    TEST_F(indexOfChar, indexOfChar_001)
    {
        rtl::OString aStr1 = "Line for a indexOfChar.";

        sal_Int32 nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'L' );
        ASSERT_TRUE(nIndex == 0) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'i' );
        ASSERT_TRUE(nIndex == 1) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'n' );
        ASSERT_TRUE(nIndex == 2) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'e' );
        ASSERT_TRUE(nIndex == 3) << "index is wrong.";
    }

    TEST_F(indexOfChar, indexOfChar_002)
    {
        rtl::OString aStr1 = "Line for a indexOfChar.";
        sal_Int32 nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'y' );

        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

// -----------------------------------------------------------------------------
    class lastIndexOfChar : public ::testing::Test
    {
    public:
    }; // class lastIndexOfChar

    TEST_F(lastIndexOfChar, lastIndexOfChar_000)
    {
        rtl_str_lastIndexOfChar( NULL, 0 );
    }

    TEST_F(lastIndexOfChar, lastIndexOfChar_001)
    {
        rtl::OString aStr1 = "Line for a lastIndexOfChar.";

        sal_Int32 nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'C' );
        ASSERT_TRUE(nIndex == 22) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'h' );
        ASSERT_TRUE(nIndex == 23) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'a' );
        ASSERT_TRUE(nIndex == 24) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'r' );
        ASSERT_TRUE(nIndex == 25) << "index is wrong.";
    }

    TEST_F(lastIndexOfChar, lastIndexOfChar_002)
    {
        rtl::OString aStr1 = "Line for a lastIndexOfChar.";
        sal_Int32 nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'y' );

        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

// -----------------------------------------------------------------------------

    class indexOfStr : public ::testing::Test
    {
    public:
    }; // class compare

    TEST_F(indexOfStr, indexOfStr_000)
    {
        rtl_str_indexOfStr( NULL, 0 );
    }

    TEST_F(indexOfStr, indexOfStr_000_1)
    {
        rtl::OString aStr1 = "Line for a indexOfStr.";
        rtl_str_indexOfStr( aStr1.getStr(), 0 );
    }

    TEST_F(indexOfStr, indexOfStr_001)
    {
        rtl::OString aStr1 = "Line for a indexOfStr.";

        sal_Int32 nIndex = rtl_str_indexOfStr( aStr1.getStr(), "Line" );
        ASSERT_TRUE(nIndex == 0) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "for" );
        ASSERT_TRUE(nIndex == 5) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "a" );
        ASSERT_TRUE(nIndex == 9) << "index is wrong.";

        /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "a index" );
        ASSERT_TRUE(nIndex ==9) << "index is wrong.";
    }

    TEST_F(indexOfStr, indexOfStr_002)
    {
        rtl::OString aStr1 = "Line for a indexOfStr.";
        sal_Int32 nIndex = rtl_str_indexOfStr( aStr1.getStr(), "not exist" );

        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

// -----------------------------------------------------------------------------


    class lastIndexOfStr : public ::testing::Test
    {
    public:
    }; // class lastIndexOfStr

    TEST_F(lastIndexOfStr, lastIndexOfStr_000)
    {
        rtl_str_lastIndexOfStr( NULL, NULL );
    }

    TEST_F(lastIndexOfStr, lastIndexOfStr_000_1)
    {
        rtl::OString aStr1 = "Line for a lastIndexOfStr.";
        rtl_str_lastIndexOfStr( aStr1.getStr(), NULL );
    }

    TEST_F(lastIndexOfStr, lastIndexOfStr_001)
    {
        rtl::OString aStr1 = "Line for a lastIndexOfStr.";
        rtl::OString aSearchStr = "Index";

        sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
        ASSERT_TRUE(nIndex == 15) << "index is wrong.";

        /* rtl::OString */ aSearchStr = "Line";
        /* sal_Int32 */ nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
        ASSERT_TRUE(nIndex == 0) << "index is wrong.";

        /* rtl::OString */ aSearchStr = "";
        /* sal_Int32 */ nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

    TEST_F(lastIndexOfStr, lastIndexOfStr_002)
    {
        rtl::OString aStr1 = "Line for a lastIndexOfStr.";
        rtl::OString aSearchStr = "foo";
        sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

        ASSERT_TRUE(nIndex == -1) << "index is wrong.";
    }

    TEST_F(lastIndexOfStr, lastIndexOfStr_003)
    {
        rtl::OString aStr1 = "Line for a lastIndexOfStr.";
        rtl::OString aSearchStr = "O";
        sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

        ASSERT_TRUE(nIndex == 20) << "index is wrong.";
    }

// -----------------------------------------------------------------------------

    class replaceChar : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(replaceChar, replaceChar_000)
    {
        rtl_str_replaceChar( NULL, 0, 0 );
    }

    TEST_F(replaceChar, replaceChar_001)
    {
        rtl::OString aStr1 = "replace char.";
        rtl::OString aShouldStr1 = "ruplacu char.";

        sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        strcpy(pStr, aStr1.getStr());

        rtl_str_replaceChar( pStr, 'e', 'u' );

        ASSERT_TRUE(aShouldStr1.equals(rtl::OString(pStr)) == sal_True) << "replace failed";
        free(pStr);
    }

// -----------------------------------------------------------------------------

    class replaceChar_WithLength : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(replaceChar_WithLength, replaceChar_WithLength_000)
    {
        rtl_str_replaceChar_WithLength( NULL, 0, 0, 0 );
    }

    TEST_F(replaceChar_WithLength, replaceChar_WithLength_000_1)
    {
        rtl_str_replaceChar_WithLength( NULL, 1, 0, 0 );
    }
    TEST_F(replaceChar_WithLength, replaceChar_WithLength_001)
    {
        rtl::OString aStr1 = "replace char.";
        rtl::OString aShouldStr1 = "ruplace char.";

        sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        strcpy(pStr, aStr1.getStr());

        rtl_str_replaceChar_WithLength( pStr, 6, 'e', 'u' );

        ASSERT_TRUE(aShouldStr1.equals(rtl::OString(pStr)) == sal_True) << "replace failed";
        free(pStr);
    }

// -----------------------------------------------------------------------------

    class toAsciiLowerCase : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(toAsciiLowerCase, toAsciiLowerCase_000)
    {
        rtl_str_toAsciiLowerCase( NULL );
    }

    TEST_F(toAsciiLowerCase, toAsciiLowerCase_001)
    {
        rtl::OString aStr1 = "CHANGE THIS TO ASCII LOWER CASE.";
        rtl::OString aShouldStr1 = "change this to ascii lower case.";

        sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        strcpy(pStr, aStr1.getStr());

        rtl_str_toAsciiLowerCase( pStr );

        ASSERT_TRUE(aShouldStr1.equals(rtl::OString(pStr)) == sal_True) << "failed";
        free(pStr);
    }

    class toAsciiLowerCase_WithLength : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(toAsciiLowerCase_WithLength, toAsciiLowerCase_WithLength_000)
    {
        rtl_str_toAsciiLowerCase_WithLength( NULL, 0 );
    }

    TEST_F(toAsciiLowerCase_WithLength, toAsciiLowerCase_WithLength_001)
    {
        rtl::OString aStr1 = "CHANGE THIS TO ASCII LOWER CASE.";
        rtl::OString aShouldStr1 = "change thiS TO ASCII LOWER CASE.";

        sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        strcpy(pStr, aStr1.getStr());

        rtl_str_toAsciiLowerCase_WithLength( pStr, 10 );

        printf("Lowercase with length: '%s'\n", pStr);
        ASSERT_TRUE(aShouldStr1.equals(rtl::OString(pStr)) == sal_True) << "failed";
        free(pStr);
    }

// -----------------------------------------------------------------------------

    class toAsciiUpperCase : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(toAsciiUpperCase, toAsciiUpperCase_000)
    {
        rtl_str_toAsciiUpperCase( NULL );
    }

    TEST_F(toAsciiUpperCase, toAsciiUpperCase_001)
    {
        rtl::OString aStr1 = "change this to ascii upper case.";
        rtl::OString aShouldStr1 = "CHANGE THIS TO ASCII UPPER CASE.";

        sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";
        strcpy(pStr, aStr1.getStr());

        rtl_str_toAsciiUpperCase( pStr );

        ASSERT_TRUE(aShouldStr1.equals(rtl::OString(pStr)) == sal_True) << "failed";
        free(pStr);
    }

    class toAsciiUpperCase_WithLength : public ::testing::Test
    {
    public:
    }; // class replaceChar

    TEST_F(toAsciiUpperCase_WithLength, toAsciiUpperCase_WithLength_000)
    {
        rtl_str_toAsciiUpperCase_WithLength( NULL, 0 );
    }

    TEST_F(toAsciiUpperCase_WithLength, toAsciiUpperCase_WithLength_001)
    {
        rtl::OString aStr1 = "change this to ascii lower case.";
        rtl::OString aShouldStr1 = "CHANGE THIs to ascii lower case.";

        sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
        ASSERT_TRUE(pStr != NULL) << "can't get memory for test";

        strcpy(pStr, aStr1.getStr());
        rtl_str_toAsciiUpperCase_WithLength( pStr, 10 );

        printf("Uppercase with length: '%s'\n", aStr1.getStr());
        ASSERT_TRUE(aShouldStr1.equals(rtl::OString(pStr)) == sal_True) << "failed";
        free(pStr);
    }

    // -----------------------------------------------------------------------------

    class trim_WithLength : public ::testing::Test
    {
      public:
    };

    TEST_F(trim_WithLength, trim_WithLength_000)
    {
        rtl_str_trim_WithLength(NULL, 0);
        // should not GPF
    }

    TEST_F(trim_WithLength, trim_WithLength_000_1)
    {
        char pStr[] = { "  trim this" };
        rtl_str_trim_WithLength( pStr, 0 );
    }

    TEST_F(trim_WithLength, trim_WithLength_001)
    {
        char const *pStr = "  trim this";
        sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
        if (pStr2)
        {
            strcpy(pStr2, pStr);
            rtl_str_trim_WithLength( pStr2, 2 );

            ASSERT_TRUE(strlen(pStr2) == 0) << "string should be empty";
            free(pStr2);
        }
    }

    TEST_F(trim_WithLength, trim_WithLength_002)
    {
        char const *pStr = "trim this";
        sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
        if (pStr2)
        {
            strcpy(pStr2, pStr);
            rtl_str_trim_WithLength( pStr2, 5 );

            ASSERT_TRUE(strlen(pStr2) == 4) << "string should contain 'trim'";
            free(pStr2);
        }
    }

    TEST_F(trim_WithLength, trim_WithLength_003)
    {
        char const *pStr = "     trim   this";
        sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
        if (pStr2)
        {
            strcpy(pStr2, pStr);
            rtl_str_trim_WithLength( pStr2, 11 );

            ASSERT_TRUE(strlen(pStr2) == 4) << "string should contain 'trim'";
            free(pStr2);
        }
    }

    TEST_F(trim_WithLength, trim_WithLength_004)
    {
        char const *pStr = "\r\n\t \n\r    trim  \n this";
        sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
        if (pStr2)
        {
            strcpy(pStr2, pStr);
            rtl_str_trim_WithLength( pStr2, 17 );

            ASSERT_TRUE(strlen(pStr2) == 4) << "string should contain 'trim'";
            free(pStr2);
        }
    }

    TEST_F(trim_WithLength, trim_WithLength_005)
    {
        char const *pStr = "\r\n\t \n\r    trim \t this \n\r\t\t     ";
        sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
        if (pStr2)
        {
            strcpy(pStr2, pStr);
            rtl_str_trim_WithLength( pStr2, strlen(pStr2) );
            ASSERT_TRUE(strlen(pStr2) == 11) << "string should contain 'trim'";
            free(pStr2);
        }
    }

    // -----------------------------------------------------------------------------

    class valueOfChar : public ::testing::Test
    {
      public:
    };

    TEST_F(valueOfChar, valueOfChar_000)
    {
        rtl_str_valueOfChar(NULL, 0);
        // should not GPF
    }
    TEST_F(valueOfChar, valueOfChar_001)
    {
        sal_Char *pStr = (sal_Char*)malloc(RTL_STR_MAX_VALUEOFCHAR);
        if (pStr)
        {
            rtl_str_valueOfChar(pStr, 'A');

            ASSERT_TRUE(pStr[0] == 'A') << "string should contain 'A'";
            free(pStr);
        }
    }

} // namespace rtl_str

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
