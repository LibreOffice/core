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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/string.hxx>
#include <cstring>

namespace rtl_str
{

    class compare : public CppUnit::TestFixture
    {
    public:

        void compare_000()
            {
                rtl_str_compare( NULL, NULL);
            }

        void compare_000_1()
            {
                rtl::OString aStr1 = "Line must be equal.";
                rtl_str_compare( aStr1.getStr(), NULL);
            }
        void compare_001()
            {
                rtl::OString aStr1 = "";
                rtl::OString aStr2 = "";

                sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002()
            {
                rtl::OString aStr1 = "Line must be equal.";
                rtl::OString aStr2 = "Line must be equal.";

                sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_003()
            {
                rtl::OString aStr1 = "Line must differ.";
                rtl::OString aStr2 = "Line foo bar, ok, differ.";

                sal_Int32 nValue = rtl_str_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(compare);
    CPPUNIT_TEST(compare_000);
    CPPUNIT_TEST(compare_000_1);
    CPPUNIT_TEST(compare_001);
    CPPUNIT_TEST(compare_002);
    CPPUNIT_TEST(compare_003);
    CPPUNIT_TEST_SUITE_END();
}; // class compare


    class compareIgnoreAsciiCase : public CppUnit::TestFixture
    {
    public:

        void compare_000()
            {
                rtl_str_compareIgnoreAsciiCase( NULL, NULL);
            }

        void compare_000_1()
            {
                rtl::OString aStr1 = "Line must be equal.";
                rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), NULL);
            }
        void compare_001()
            {
                rtl::OString aStr1 = "";
                rtl::OString aStr2 = "";

                sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002()
            {
                rtl::OString aStr1 = "Line must be equal.";
                rtl::OString aStr2 = "Line must be equal.";

                sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002_1()
            {
                rtl::OString aStr1 = "Line must be equal.";
                rtl::OString aStr2 = "LINE MUST BE EQUAL.";

                sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal (if case insensitve).", nValue == 0);
            }

        void compare_003()
            {
                rtl::OString aStr1 = "Line must differ.";
                rtl::OString aStr2 = "Line foo bar, ok, differ.";

                sal_Int32 nValue = rtl_str_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(compareIgnoreAsciiCase);
    CPPUNIT_TEST(compare_000);
    CPPUNIT_TEST(compare_000_1);
    CPPUNIT_TEST(compare_001);
    CPPUNIT_TEST(compare_002);
    CPPUNIT_TEST(compare_002_1);
    CPPUNIT_TEST(compare_003);
    CPPUNIT_TEST_SUITE_END();
    }; // class compareIgnoreAsciiCase

// -----------------------------------------------------------------------------

    class shortenedCompareIgnoreAsciiCase_WithLength : public CppUnit::TestFixture
    {
    public:

        void compare_000()
            {
                rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( NULL, 0, NULL, 0, 0);
            }

        void compare_000_1()
            {
                rtl::OString aStr1 = "Line must be equal.";
                rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), NULL, 0, 1);
            }
        void compare_001()
            {
                rtl::OString aStr1 = "";
                rtl::OString aStr2 = "";

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), aStr2.getStr(), aStr2.getLength(), aStr1.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002()
            {
                rtl::OString aStr1 = "Line must be equal.";
                rtl::OString aStr2 = "Line must be equal.";

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002_1()
            {
                rtl::OString aStr1 = "Line must be equal.";
                rtl::OString aStr2 = "LINE MUST BE EQUAL.";

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal (if case insensitve).", nValue == 0);
            }

        void compare_003()
            {
                rtl::OString aStr1 = "Line must differ.";
                rtl::OString aStr2 = "Line foo bar, ok, differ.";

                sal_Int32 nValue = rtl_str_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       5);
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal first 5 characters.", nValue == 0);
            }

        void compare_004()
            {
                rtl::OString aStr1 = "Line must differ.";
                rtl::OString aStr2 = "Line foo bar, ok, differ.";

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


// -----------------------------------------------------------------------------

    class hashCode : public CppUnit::TestFixture
    {
    public:

        void hashCode_000()
            {
                rtl_str_hashCode( NULL );
            }

        void hashCode_001()
            {
                rtl::OString aStr1 = "Line for a hashCode.";
                sal_Int32 nHashCode = rtl_str_hashCode( aStr1.getStr() );
                printf("hashcode: %d\n", nHashCode);
                // CPPUNIT_ASSERT_MESSAGE("failed.", nValue == 0);
            }

        void hashCode_002()
            {
                rtl::OString aStr1 = "Line for a hashCode.";
                sal_Int32 nHashCode1 = rtl_str_hashCode( aStr1.getStr() );

                rtl::OString aStr2 = "Line for a hashCode.";
                sal_Int32 nHashCode2 = rtl_str_hashCode( aStr2.getStr() );

                CPPUNIT_ASSERT_MESSAGE("hashcodes must be equal.", nHashCode1 == nHashCode2 );
            }

        void hashCode_003()
            {
                rtl::OString aStr1 = "Line for a hashCode.";
                sal_Int32 nHashCode1 = rtl_str_hashCode( aStr1.getStr() );

                rtl::OString aStr2 = "Line for an other hashcode.";
                sal_Int32 nHashCode2 = rtl_str_hashCode( aStr2.getStr() );

                CPPUNIT_ASSERT_MESSAGE("hashcodes must differ.", nHashCode1 != nHashCode2 );
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(hashCode);
        CPPUNIT_TEST(hashCode_000);
        CPPUNIT_TEST(hashCode_001);
        CPPUNIT_TEST(hashCode_002);
        CPPUNIT_TEST(hashCode_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class compare


// -----------------------------------------------------------------------------

    class indexOfChar : public CppUnit::TestFixture
    {
    public:

        void indexOfChar_000()
            {
                rtl_str_indexOfChar( NULL, 0 );
            }

        void indexOfChar_001()
            {
                rtl::OString aStr1 = "Line for a indexOfChar.";

                sal_Int32 nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'L' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 0);

                /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'i' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 1);

                /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'n' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 2);

                /* sal_Int32 */ nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'e' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 3);
            }

        void indexOfChar_002()
            {
                rtl::OString aStr1 = "Line for a indexOfChar.";
                sal_Int32 nIndex = rtl_str_indexOfChar( aStr1.getStr(), 'y' );

                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == -1 );
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

// -----------------------------------------------------------------------------
    class lastIndexOfChar : public CppUnit::TestFixture
    {
    public:

        void lastIndexOfChar_000()
            {
                rtl_str_lastIndexOfChar( NULL, 0 );
            }

        void lastIndexOfChar_001()
            {
                rtl::OString aStr1 = "Line for a lastIndexOfChar.";

                sal_Int32 nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'C' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 22);

                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'h' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 23);

                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'a' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 24);

                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'r' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 25);
            }

        void lastIndexOfChar_002()
            {
                rtl::OString aStr1 = "Line for a lastIndexOfChar.";
                sal_Int32 nIndex = rtl_str_lastIndexOfChar( aStr1.getStr(), 'y' );

                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == -1 );
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


// -----------------------------------------------------------------------------

    class indexOfStr : public CppUnit::TestFixture
    {
    public:

        void indexOfStr_000()
            {
                rtl_str_indexOfStr( NULL, 0 );
            }

        void indexOfStr_000_1()
            {
                rtl::OString aStr1 = "Line for a indexOfStr.";
                rtl_str_indexOfStr( aStr1.getStr(), 0 );
            }

        void indexOfStr_001()
            {
                rtl::OString aStr1 = "Line for a indexOfStr.";

                sal_Int32 nIndex = rtl_str_indexOfStr( aStr1.getStr(), "Line" );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 0);

                /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "for" );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 5);

                /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "a" );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 9);

                /* sal_Int32 */ nIndex = rtl_str_indexOfStr( aStr1.getStr(), "a index" );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex ==9);
            }

        void indexOfStr_002()
            {
                rtl::OString aStr1 = "Line for a indexOfStr.";
                sal_Int32 nIndex = rtl_str_indexOfStr( aStr1.getStr(), "not exist" );

                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == -1 );
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
// -----------------------------------------------------------------------------


    class lastIndexOfStr : public CppUnit::TestFixture
    {
    public:

        void lastIndexOfStr_000()
            {
                rtl_str_lastIndexOfStr( NULL, NULL );
            }

        void lastIndexOfStr_000_1()
            {
                rtl::OString aStr1 = "Line for a lastIndexOfStr.";
                rtl_str_lastIndexOfStr( aStr1.getStr(), NULL );
            }

        void lastIndexOfStr_001()
            {
                rtl::OString aStr1 = "Line for a lastIndexOfStr.";
                rtl::OString aSearchStr = "Index";

                sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 15);

                /* rtl::OString */ aSearchStr = "Line";
                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 0);

                /* rtl::OString */ aSearchStr = "";
                /* sal_Int32 */ nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == -1);
            }

        void lastIndexOfStr_002()
            {
                rtl::OString aStr1 = "Line for a lastIndexOfStr.";
                rtl::OString aSearchStr = "foo";
                sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == -1 );
            }

        void lastIndexOfStr_003()
            {
                rtl::OString aStr1 = "Line for a lastIndexOfStr.";
                rtl::OString aSearchStr = "O";
                sal_Int32 nIndex = rtl_str_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 20 );
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

// -----------------------------------------------------------------------------

    class replaceChar : public CppUnit::TestFixture
    {
    public:

        void replaceChar_000()
            {
                rtl_str_replaceChar( NULL, 0, 0 );
            }

        void replaceChar_001()
            {
                rtl::OString aStr1 = "replace char.";
                rtl::OString aShouldStr1 = "ruplacu char.";

                sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                strcpy(pStr, aStr1.getStr());

                rtl_str_replaceChar( pStr, 'e', 'u' );

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(rtl::OString(pStr)) == sal_True);
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(replaceChar);
        CPPUNIT_TEST(replaceChar_000);
        CPPUNIT_TEST(replaceChar_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar

// -----------------------------------------------------------------------------

    class replaceChar_WithLength : public CppUnit::TestFixture
    {
    public:

        void replaceChar_WithLength_000()
            {
                rtl_str_replaceChar_WithLength( NULL, 0, 0, 0 );
            }

        void replaceChar_WithLength_000_1()
            {
                rtl_str_replaceChar_WithLength( NULL, 1, 0, 0 );
            }
        void replaceChar_WithLength_001()
            {
                rtl::OString aStr1 = "replace char.";
                rtl::OString aShouldStr1 = "ruplace char.";

                sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                strcpy(pStr, aStr1.getStr());

                rtl_str_replaceChar_WithLength( pStr, 6, 'e', 'u' );

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(rtl::OString(pStr)) == sal_True);
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(replaceChar_WithLength);
        CPPUNIT_TEST(replaceChar_WithLength_000);
        CPPUNIT_TEST(replaceChar_WithLength_000_1);
        CPPUNIT_TEST(replaceChar_WithLength_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar


// -----------------------------------------------------------------------------

    class toAsciiLowerCase : public CppUnit::TestFixture
    {
    public:

        void toAsciiLowerCase_000()
            {
                rtl_str_toAsciiLowerCase( NULL );
            }

        void toAsciiLowerCase_001()
            {
                rtl::OString aStr1 = "CHANGE THIS TO ASCII LOWER CASE.";
                rtl::OString aShouldStr1 = "change this to ascii lower case.";

                sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                strcpy(pStr, aStr1.getStr());

                rtl_str_toAsciiLowerCase( pStr );

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(rtl::OString(pStr)) == sal_True);
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(toAsciiLowerCase);
        CPPUNIT_TEST(toAsciiLowerCase_000);
        CPPUNIT_TEST(toAsciiLowerCase_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar


    class toAsciiLowerCase_WithLength : public CppUnit::TestFixture
    {
    public:

        void toAsciiLowerCase_WithLength_000()
            {
                rtl_str_toAsciiLowerCase_WithLength( NULL, 0 );
            }

        void toAsciiLowerCase_WithLength_001()
            {
                rtl::OString aStr1 = "CHANGE THIS TO ASCII LOWER CASE.";
                rtl::OString aShouldStr1 = "change thiS TO ASCII LOWER CASE.";

                sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                strcpy(pStr, aStr1.getStr());

                rtl_str_toAsciiLowerCase_WithLength( pStr, 10 );

                printf("Lowercase with length: '%s'\n", pStr);
                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(rtl::OString(pStr)) == sal_True);
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

// -----------------------------------------------------------------------------

    class toAsciiUpperCase : public CppUnit::TestFixture
    {
    public:

        void toAsciiUpperCase_000()
            {
                rtl_str_toAsciiUpperCase( NULL );
            }

        void toAsciiUpperCase_001()
            {
                rtl::OString aStr1 = "change this to ascii upper case.";
                rtl::OString aShouldStr1 = "CHANGE THIS TO ASCII UPPER CASE.";

                sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                strcpy(pStr, aStr1.getStr());

                rtl_str_toAsciiUpperCase( pStr );

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(rtl::OString(pStr)) == sal_True);
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(toAsciiUpperCase);
        CPPUNIT_TEST(toAsciiUpperCase_000);
        CPPUNIT_TEST(toAsciiUpperCase_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar


    class toAsciiUpperCase_WithLength : public CppUnit::TestFixture
    {
    public:

        void toAsciiUpperCase_WithLength_000()
            {
                rtl_str_toAsciiUpperCase_WithLength( NULL, 0 );
            }

        void toAsciiUpperCase_WithLength_001()
            {
                rtl::OString aStr1 = "change this to ascii lower case.";
                rtl::OString aShouldStr1 = "CHANGE THIs to ascii lower case.";

                sal_Char* pStr = (sal_Char*) malloc(aStr1.getLength() + 1);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);

                strcpy(pStr, aStr1.getStr());
                rtl_str_toAsciiUpperCase_WithLength( pStr, 10 );

                printf("Uppercase with length: '%s'\n", aStr1.getStr());
                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(rtl::OString(pStr)) == sal_True);
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


    // -----------------------------------------------------------------------------

    class trim_WithLength : public CppUnit::TestFixture
    {
      public:
        void trim_WithLength_000()
        {
            rtl_str_trim_WithLength(NULL, 0);
            // should not GPF
        }

        void trim_WithLength_000_1()
        {
            char pStr[] = { "  trim this" };
            rtl_str_trim_WithLength( pStr, 0 );
        }

        void trim_WithLength_001()
        {
            char const *pStr = "  trim this";
            sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
            if (pStr2)
            {
                strcpy(pStr2, pStr);
                rtl_str_trim_WithLength( pStr2, 2 );

                CPPUNIT_ASSERT_MESSAGE("string should be empty", strlen(pStr2) == 0);
                free(pStr2);
            }
        }

        void trim_WithLength_002()
        {
            char const *pStr = "trim this";
            sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
            if (pStr2)
            {
                strcpy(pStr2, pStr);
                rtl_str_trim_WithLength( pStr2, 5 );

                CPPUNIT_ASSERT_MESSAGE("string should contain 'trim'", strlen(pStr2) == 4);
                free(pStr2);
            }
        }

        void trim_WithLength_003()
        {
            char const *pStr = "     trim   this";
            sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
            if (pStr2)
            {
                strcpy(pStr2, pStr);
                rtl_str_trim_WithLength( pStr2, 11 );

                CPPUNIT_ASSERT_MESSAGE("string should contain 'trim'", strlen(pStr2) == 4);
                free(pStr2);
            }
        }

        void trim_WithLength_004()
        {
            char const *pStr = "\r\n\t \n\r    trim  \n this";
            sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
            if (pStr2)
            {
                strcpy(pStr2, pStr);
                rtl_str_trim_WithLength( pStr2, 17 );

                CPPUNIT_ASSERT_MESSAGE("string should contain 'trim'", strlen(pStr2) == 4);
                free(pStr2);
            }
        }

        void trim_WithLength_005()
        {
            char const *pStr = "\r\n\t \n\r    trim \t this \n\r\t\t     ";
            sal_Char *pStr2 = (sal_Char*)malloc(strlen(pStr) + 1);
            if (pStr2)
            {
                strcpy(pStr2, pStr);
                rtl_str_trim_WithLength( pStr2, strlen(pStr2) );

                CPPUNIT_ASSERT_MESSAGE("string should contain 'trim'", strlen(pStr2) == 11);
                free(pStr2);
            }
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

    // -----------------------------------------------------------------------------

    class valueOfChar : public CppUnit::TestFixture
    {
      public:
        void valueOfChar_000()
            {
                rtl_str_valueOfChar(NULL, 0);
                // should not GPF
            }
        void valueOfChar_001()
            {
                sal_Char *pStr = (sal_Char*)malloc(RTL_STR_MAX_VALUEOFCHAR);
                if (pStr)
                {
                    rtl_str_valueOfChar(pStr, 'A');

                    CPPUNIT_ASSERT_MESSAGE("string should contain 'A'", pStr[0] == 'A');
                    free(pStr);
                }
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(valueOfChar);
        CPPUNIT_TEST(valueOfChar_000);
        CPPUNIT_TEST(valueOfChar_001);
        CPPUNIT_TEST_SUITE_END();
    };

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
