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
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/string.hxx>
#include <cstring>

namespace rtl_str
{

    class compare : public CppUnit::TestFixture
    {
        void compare_001()
            {
                OString aStr1 = ""_ostr;
                OString aStr2 = ""_ostr;

                sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002()
            {
                OString aStr1 = "Line must be equal."_ostr;
                OString aStr2 = "Line must be equal."_ostr;

                sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_003()
            {
                OString aStr1 = "Line must differ."_ostr;
                OString aStr2 = "Line foo bar, ok, differ."_ostr;

                sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(compare);
    CPPUNIT_TEST(compare_001);
    CPPUNIT_TEST(compare_002);
    CPPUNIT_TEST(compare_003);
    CPPUNIT_TEST_SUITE_END();
    }; // class compare

    class compareIgnoreAsciiCase : public CppUnit::TestFixture
    {
        void compare_001()
            {
                OString aStr1 = ""_ostr;
                OString aStr2 = ""_ostr;

                sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002()
            {
                OString aStr1 = "Line must be equal."_ostr;
                OString aStr2 = "Line must be equal."_ostr;

                sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002_1()
            {
                OString aStr1 = "Line must be equal."_ostr;
                OString aStr2 = "LINE MUST BE EQUAL."_ostr;

                sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal (if case insensitive).", sal_Int32(0), nValue);
            }

        void compare_003()
            {
                OString aStr1 = "Line must differ."_ostr;
                OString aStr2 = "Line foo bar, ok, differ."_ostr;

                sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(compareIgnoreAsciiCase);
    CPPUNIT_TEST(compare_001);
    CPPUNIT_TEST(compare_002);
    CPPUNIT_TEST(compare_002_1);
    CPPUNIT_TEST(compare_003);
    CPPUNIT_TEST_SUITE_END();
    }; // class compareIgnoreAsciiCase

    class shortenedCompareIgnoreAsciiCase_WithLength : public CppUnit::TestFixture
    {
        void compare_000()
            {
                rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( nullptr, 0, nullptr, 0, 0);
            }

        void compare_000_1()
            {
                OString aStr1 = "Line must be equal."_ostr;
                rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), nullptr, 0, 1);
            }
        void compare_001()
            {
                OString aStr1 = ""_ostr;
                OString aStr2 = ""_ostr;

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), aStr2.getStr(), aStr2.getLength(), aStr1.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002()
            {
                OString aStr1 = "Line must be equal."_ostr;
                OString aStr2 = "Line must be equal."_ostr;

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002_1()
            {
                OString aStr1 = "Line must be equal."_ostr;
                OString aStr2 = "LINE MUST BE EQUAL."_ostr;

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal (if case insensitive).", sal_Int32(0), nValue);
            }

        void compare_003()
            {
                OString aStr1 = "Line must differ."_ostr;
                OString aStr2 = "Line foo bar, ok, differ."_ostr;

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       5);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal first 5 characters.", sal_Int32(0), nValue);
            }

        void compare_004()
            {
                OString aStr1 = "Line must differ."_ostr;
                OString aStr2 = "Line foo bar, ok, differ."_ostr;

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(shortenedCompareIgnoreAsciiCase_WithLength);
    CPPUNIT_TEST(compare_000);
    CPPUNIT_TEST(compare_000_1);
    CPPUNIT_TEST(compare_001);
    CPPUNIT_TEST(compare_002);
    CPPUNIT_TEST(compare_002_1);
    CPPUNIT_TEST(compare_003);
    CPPUNIT_TEST(compare_004);
    CPPUNIT_TEST_SUITE_END();
    }; // class compare

    class hashCode : public CppUnit::TestFixture
    {
        void hashCode_001()
            {
                OString aStr1 = "Line for a hashCode."_ostr;
                sal_Int32 nHashCode = rtl_str_hashCode( aStr1.getStr() );
                printf("hashcode: %" SAL_PRIdINT32 "\n", nHashCode);
                // CPPUNIT_ASSERT_MESSAGE("failed.", nValue == 0);
            }

        void hashCode_002()
            {
                OString aStr1 = "Line for a hashCode."_ostr;
                sal_Int32 nHashCode1 = rtl_str_hashCode( aStr1.getStr() );

                OString aStr2 = "Line for a hashCode."_ostr;
                sal_Int32 nHashCode2 = rtl_str_hashCode( aStr2.getStr() );

                CPPUNIT_ASSERT_EQUAL_MESSAGE("hashcodes must be equal.", nHashCode1, nHashCode2 );
            }

        void hashCode_003()
            {
                OString aStr1 = "Line for a hashCode."_ostr;
                sal_Int32 nHashCode1 = rtl_str_hashCode( aStr1.getStr() );

                OString aStr2 = "Line for another hashcode."_ostr;
                sal_Int32 nHashCode2 = rtl_str_hashCode( aStr2.getStr() );

                CPPUNIT_ASSERT_MESSAGE("hashcodes must differ.", nHashCode1 != nHashCode2 );
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(hashCode);
        CPPUNIT_TEST(hashCode_001);
        CPPUNIT_TEST(hashCode_002);
        CPPUNIT_TEST(hashCode_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class compare

    class indexOfChar : public CppUnit::TestFixture
    {
        void indexOfChar_000()
            {
                sal_Int32 nIndex = rtl_str_indexOfChar("", 0);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Trailing zero character is not part of the string",
                                             sal_Int32(-1), nIndex);
            }

        void indexOfChar_001()
            {
                OString aStr1 = "Line for an indexOfChar."_ostr;

                sal_Int32 nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'L' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(0), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'i' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(1), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'n' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(2), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'e' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(3), nIndex);
            }

        void indexOfChar_002()
            {
                OString aStr1 = "Line for an indexOfChar."_ostr;
                sal_Int32 nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'y' );

                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(-1), nIndex);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(indexOfChar);
        CPPUNIT_TEST(indexOfChar_000);
        CPPUNIT_TEST(indexOfChar_001);
        CPPUNIT_TEST(indexOfChar_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class compare

    class lastIndexOfChar : public CppUnit::TestFixture
    {
        void lastIndexOfChar_000()
            {
                sal_Int32 nIndex = rtl_str_lastIndexOfChar("", 0);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Trailing zero character is not part of the string",
                                             sal_Int32(-1), nIndex);
            }

        void lastIndexOfChar_001()
            {
                OString aStr1 = "Line for a lastIndexOfChar."_ostr;

                sal_Int32 nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'C' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(22), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'h' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(23), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'a' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(24), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'r' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(25), nIndex);
            }

        void lastIndexOfChar_002()
            {
                OString aStr1 = "Line for a lastIndexOfChar."_ostr;
                sal_Int32 nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'y' );

                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(-1), nIndex);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(lastIndexOfChar);
        CPPUNIT_TEST(lastIndexOfChar_000);
        CPPUNIT_TEST(lastIndexOfChar_001);
        CPPUNIT_TEST(lastIndexOfChar_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class lastIndexOfChar

    class indexOfStr : public CppUnit::TestFixture
    {
        void indexOfStr_000()
            {
                OString aStr1("Line for an indexOfStr."_ostr);
                sal_Int32 nIndex = rtl_str_indexOfStr( aStr1.getStr(), "" );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("an empty substring is always not findable",
                                             sal_Int32(-1), nIndex);
            }

        void indexOfStr_001()
            {
                OString aStr1 = "Line for an indexOfStr."_ostr;

                sal_Int32 nIndex = rtl_str_indexOfStr( aStr1.getStr(), "Line" );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(0), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "for" );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(5), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "a" );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(9), nIndex);

                /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "an index" );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(9), nIndex);
            }

        void indexOfStr_002()
            {
                OString aStr1 = "Line for an indexOfStr."_ostr;
                sal_Int32 nIndex = rtl_str_indexOfStr( aStr1.getStr(), "not exist" );

                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(-1), nIndex);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(indexOfStr);
        CPPUNIT_TEST(indexOfStr_000);
        CPPUNIT_TEST(indexOfStr_001);
        CPPUNIT_TEST(indexOfStr_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class compare

    class lastIndexOfStr : public CppUnit::TestFixture
    {
        void lastIndexOfStr_000()
            {
                OString aStr1("Line for a lastIndexOfStr."_ostr);
                sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), "" );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("an empty substring is always not findable",
                                             sal_Int32(-1), nIndex);
            }

        void lastIndexOfStr_001()
            {
                OString aStr1 = "Line for a lastIndexOfStr."_ostr;
                OString aSearchStr = "Index"_ostr;

                sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(15), nIndex);

                /* OString */ aSearchStr = "Line"_ostr;
                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(0), nIndex);

                /* OString */ aSearchStr = ""_ostr;
                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(-1), nIndex);
            }

        void lastIndexOfStr_002()
            {
                OString aStr1 = "Line for a lastIndexOfStr."_ostr;
                OString aSearchStr = "foo"_ostr;
                sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(-1), nIndex);
            }

        void lastIndexOfStr_003()
            {
                OString aStr1 = "Line for a lastIndexOfStr."_ostr;
                OString aSearchStr = "O"_ostr;
                sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(20), nIndex);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(lastIndexOfStr);
        CPPUNIT_TEST(lastIndexOfStr_000);
        CPPUNIT_TEST(lastIndexOfStr_001);
        CPPUNIT_TEST(lastIndexOfStr_002);
        CPPUNIT_TEST(lastIndexOfStr_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class lastIndexOfStr

    class replaceChar : public CppUnit::TestFixture
    {
        void replaceChar_001()
            {
                OString aStr1 = "replace char."_ostr;
                OString aShouldStr1 = "ruplacu char."_ostr;

                char* pStr = static_cast<char*>(malloc(aStr1.getLength() + 1));
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != nullptr);
                strcpy(pStr, aStr1.getStr());

                rtl_str_replaceChar( pStr, 'e', 'u' );

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(OString(pStr)));
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(replaceChar);
        CPPUNIT_TEST(replaceChar_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar

    class replaceChar_WithLength : public CppUnit::TestFixture
    {
        void replaceChar_WithLength_000()
            {
                rtl_str_replaceChar_WithLength( nullptr, 0, 0, 0 );
            }

        void replaceChar_WithLength_001()
            {
                OString aStr1 = "replace char."_ostr;
                OString aShouldStr1 = "ruplace char."_ostr;

                char* pStr = static_cast<char*>(malloc(aStr1.getLength() + 1));
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != nullptr);
                strcpy(pStr, aStr1.getStr());

                rtl_str_replaceChar_WithLength( pStr, 6, 'e', 'u' );

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(OString(pStr)));
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(replaceChar_WithLength);
        CPPUNIT_TEST(replaceChar_WithLength_000);
        CPPUNIT_TEST(replaceChar_WithLength_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar

    class toAsciiLowerCase : public CppUnit::TestFixture
    {
        void toAsciiLowerCase_001()
            {
                OString aStr1 = "CHANGE THIS TO ASCII LOWER CASE."_ostr;
                OString aShouldStr1 = "change this to ascii lower case."_ostr;

                char* pStr = static_cast<char*>(malloc(aStr1.getLength() + 1));
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != nullptr);
                strcpy(pStr, aStr1.getStr());

                rtl_str_toAsciiLowerCase( pStr );

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(OString(pStr)));
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(toAsciiLowerCase);
        CPPUNIT_TEST(toAsciiLowerCase_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar

    class toAsciiLowerCase_WithLength : public CppUnit::TestFixture
    {
        void toAsciiLowerCase_WithLength_000()
            {
                rtl_str_toAsciiLowerCase_WithLength( nullptr, 0 );
            }

        void toAsciiLowerCase_WithLength_001()
            {
                OString aStr1 = "CHANGE THIS TO ASCII LOWER CASE."_ostr;
                OString aShouldStr1 = "change thiS TO ASCII LOWER CASE."_ostr;

                char* pStr = static_cast<char*>(malloc(aStr1.getLength() + 1));
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != nullptr);
                strcpy(pStr, aStr1.getStr());

                rtl_str_toAsciiLowerCase_WithLength( pStr, 10 );

                printf("Lowercase with length: '%s'\n", pStr);
                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(OString(pStr)));
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(toAsciiLowerCase_WithLength);
        CPPUNIT_TEST(toAsciiLowerCase_WithLength_000);
        CPPUNIT_TEST(toAsciiLowerCase_WithLength_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar

    class toAsciiUpperCase : public CppUnit::TestFixture
    {
        void toAsciiUpperCase_001()
            {
                OString aStr1 = "change this to ascii upper case."_ostr;
                OString aShouldStr1 = "CHANGE THIS TO ASCII UPPER CASE."_ostr;

                char* pStr = static_cast<char*>(malloc(aStr1.getLength() + 1));
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != nullptr);
                strcpy(pStr, aStr1.getStr());

                rtl_str_toAsciiUpperCase( pStr );

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(OString(pStr)));
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(toAsciiUpperCase);
        CPPUNIT_TEST(toAsciiUpperCase_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar

    class toAsciiUpperCase_WithLength : public CppUnit::TestFixture
    {
        void toAsciiUpperCase_WithLength_000()
            {
                rtl_str_toAsciiUpperCase_WithLength( nullptr, 0 );
            }

        void toAsciiUpperCase_WithLength_001()
            {
                OString aStr1 = "change this to ascii lower case."_ostr;
                OString aShouldStr1 = "CHANGE THIs to ascii lower case."_ostr;

                char* pStr = static_cast<char*>(malloc(aStr1.getLength() + 1));
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != nullptr);

                strcpy(pStr, aStr1.getStr());
                rtl_str_toAsciiUpperCase_WithLength( pStr, 10 );

                printf("Uppercase with length: '%s'\n", aStr1.getStr());
                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(OString(pStr)));
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(toAsciiUpperCase_WithLength);
        CPPUNIT_TEST(toAsciiUpperCase_WithLength_000);
        CPPUNIT_TEST(toAsciiUpperCase_WithLength_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar

    class trim_WithLength : public CppUnit::TestFixture
    {
        void trim_WithLength_000()
        {
            rtl_str_trim_WithLength(nullptr, 0);
            // should not GPF
        }

        void trim_WithLength_000_1()
        {
            char pStr[] = { "  trim this" };
            rtl_str_trim_WithLength( pStr, 0 );
        }

        void trim_WithLength_001()
        {
            char pStr[] = { "  trim this" };
            rtl_str_trim_WithLength( pStr, 2 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should be empty", size_t(0), strlen(pStr));
        }

        void trim_WithLength_002()
        {
            char pStr[] = { "trim this" };
            rtl_str_trim_WithLength( pStr, 5 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'trim'", size_t(4), strlen(pStr));
        }

        void trim_WithLength_003()
        {
            char pStr[] = {"     trim   this"};
            rtl_str_trim_WithLength( pStr, 11 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'trim'", size_t(4), strlen(pStr));
        }

        void trim_WithLength_004()
        {
            char pStr[] = { "\r\n\t \n\r    trim  \n this" };
            rtl_str_trim_WithLength( pStr, 17 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'trim'", size_t(4), strlen(pStr));
        }

        void trim_WithLength_005()
        {
            char pStr[] = { "\r\n\t \n\r    trim \t this \n\r\t\t     " };
            rtl_str_trim_WithLength( pStr, strlen(pStr) );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'trim \t this'", size_t(11), strlen(pStr));
        }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(trim_WithLength);
        CPPUNIT_TEST(trim_WithLength_000);
        CPPUNIT_TEST(trim_WithLength_000_1);
        CPPUNIT_TEST(trim_WithLength_001);
        CPPUNIT_TEST(trim_WithLength_002);
        CPPUNIT_TEST(trim_WithLength_003);
        CPPUNIT_TEST(trim_WithLength_004);
        CPPUNIT_TEST(trim_WithLength_005);
        CPPUNIT_TEST_SUITE_END();
    };

    class valueOfChar : public CppUnit::TestFixture
    {
        void valueOfChar_001()
            {
                char pStr[RTL_STR_MAX_VALUEOFCHAR];
                rtl_str_valueOfChar(pStr, 'A');

                CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'A'", 'A', pStr[0]);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(valueOfChar);
        CPPUNIT_TEST(valueOfChar_001);
        CPPUNIT_TEST_SUITE_END();
    };

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::compare);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::compareIgnoreAsciiCase);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::shortenedCompareIgnoreAsciiCase_WithLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::hashCode);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::indexOfChar);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::lastIndexOfChar);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::indexOfStr);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::lastIndexOfStr);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::replaceChar);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::replaceChar_WithLength);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::toAsciiLowerCase);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::toAsciiLowerCase_WithLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::toAsciiUpperCase);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::toAsciiUpperCase_WithLength);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::trim_WithLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_str::valueOfChar);

} // namespace rtl_str

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
