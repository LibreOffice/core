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


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

/** print a UNI_CODE file name.
*/
/*
inline void printOUString( ::rtl::OUString const & _suStr )
{
    rtl::OString aString;

    printf( "OUString: " );
    aString = ::rtl::OUStringToOString( _suStr, RTL_TEXTENCODING_ASCII_US );
    printf( "%s\n", aString.getStr( ) );
}
*/


namespace rtl_ustr
{

    class compare : public CppUnit::TestFixture
    {
    public:


        void compare_000()
            {
                rtl_ustr_compare( NULL, NULL);
                // should not GPF
            }

        void compare_000_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl_ustr_compare( aStr1.getStr(), NULL);
                // should not GPF
            }
        void compare_001()
            {
                rtl::OUString aStr1;
                rtl::OUString aStr2;

                sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl::OUString aStr2("Line must be equal.");

                sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_003()
            {
                rtl::OUString aStr1("Line must differ.");
                rtl::OUString aStr2("Line foo bar, ok, differ.");

                sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
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
                rtl_ustr_compareIgnoreAsciiCase( NULL, NULL);
            }

        void compare_000_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), NULL);
            }
        void compare_001()
            {
                rtl::OUString aStr1;
                rtl::OUString aStr2;

                sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl::OUString aStr2("Line must be equal.");

                sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl::OUString aStr2("LINE MUST BE EQUAL.");

                sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal (if case insensitve).", nValue == 0);
            }

        void compare_003()
            {
                rtl::OUString aStr1("Line must differ.");
                rtl::OUString aStr2("Line foo bar, ok, differ.");

                sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
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
                rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( NULL, 0, NULL, 0, 0);
            }

        void compare_000_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), NULL, 0, 1);
            }
        void compare_001()
            {
                rtl::OUString aStr1;
                rtl::OUString aStr2;

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), aStr2.getStr(), aStr2.getLength(), aStr1.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl::OUString aStr2("Line must be equal.");

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void compare_002_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl::OUString aStr2("LINE MUST BE EQUAL.");

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal (if case insensitve).", nValue == 0);
            }

        void compare_003()
            {
                rtl::OUString aStr1("Line must differ.");
                rtl::OUString aStr2("Line foo bar, ok, differ.");

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       5);
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal first 5 characters.", nValue == 0);
            }

        void compare_004()
            {
                rtl::OUString aStr1("Line must differ.");
                rtl::OUString aStr2("Line foo bar, ok, differ.");

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
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


// // -----------------------------------------------------------------------------
//
//     class hashCode : public CppUnit::TestFixture
//     {
//     public:
//
//         void hashCode_000()
//             {
//                 sal_Int32 nHashCode = rtl_ustr_hashCode( NULL );
//                 volatile int dummy = 0;
//             }
//
//         void hashCode_001()
//             {
//                 rtl::OString aStr1 = "Line for a hashCode.";
//                 sal_Int32 nHashCode = rtl_ustr_hashCode( aStr1.getStr() );
//                 printf("hashcode: %d\n", nHashCode);
//                 // CPPUNIT_ASSERT_MESSAGE("failed.", nValue == 0);
//             }
//
//         void hashCode_002()
//             {
//                 rtl::OString aStr1 = "Line for a hashCode.";
//                 sal_Int32 nHashCode1 = rtl_ustr_hashCode( aStr1.getStr() );
//
//                 rtl::OString aStr2 = "Line for a hashCode.";
//                 sal_Int32 nHashCode2 = rtl_ustr_hashCode( aStr2.getStr() );
//
//                 CPPUNIT_ASSERT_MESSAGE("hashcodes must be equal.", nHashCode1 == nHashCode2 );
//             }
//
//         void hashCode_003()
//             {
//                 rtl::OString aStr1 = "Line for a hashCode.";
//                 sal_Int32 nHashCode1 = rtl_ustr_hashCode( aStr1.getStr() );
//
//                 rtl::OString aStr2 = "Line for an other hashcode.";
//                 sal_Int32 nHashCode2 = rtl_ustr_hashCode( aStr2.getStr() );
//
//                 CPPUNIT_ASSERT_MESSAGE("hashcodes must differ.", nHashCode1 != nHashCode2 );
//             }
//
//         // Change the following lines only, if you add, remove or rename
//         // member functions of the current class,
//         // because these macros are need by auto register mechanism.
//
//         CPPUNIT_TEST_SUITE(hashCode);
//         CPPUNIT_TEST(hashCode_000);
//         CPPUNIT_TEST(hashCode_001);
//         CPPUNIT_TEST(hashCode_002);
//         CPPUNIT_TEST(hashCode_003);
//         CPPUNIT_TEST_SUITE_END();
//     }; // class compare
//
//
// // -----------------------------------------------------------------------------
//
    class indexOfChar : public CppUnit::TestFixture
    {
    public:

        void indexOfChar_000()
            {
                rtl_ustr_indexOfChar( NULL, 0 );
            }

        void indexOfChar_001()
            {
                rtl::OUString aStr1("Line for a indexOfChar.");

                sal_Int32 nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'L' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 0);

                /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'i' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 1);

                /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'n' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 2);

                /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'e' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 3);
            }

        void indexOfChar_002()
            {
                rtl::OUString aStr1("Line for a indexOfChar.");
                sal_Int32 nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'y' );

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
    }; // class indexOfChar

// // -----------------------------------------------------------------------------
    class lastIndexOfChar : public CppUnit::TestFixture
    {
    public:

        void lastIndexOfChar_000()
            {
                rtl_ustr_lastIndexOfChar( NULL, 0 );
            }

        void lastIndexOfChar_001()
            {
                rtl::OUString aStr1("Line for a lastIndexOfChar.");

                sal_Int32 nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'C' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 22);

                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'h' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 23);

                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'a' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 24);

                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'r' );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 25);
            }

        void lastIndexOfChar_002()
            {
                rtl::OUString aStr1("Line for a lastIndexOfChar.");
                sal_Int32 nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'y' );

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
                rtl_ustr_indexOfStr( NULL, 0 );
            }

        void indexOfStr_000_1()
            {
                rtl::OUString aStr1("Line for a indexOfStr.");
                rtl_ustr_indexOfStr( aStr1.getStr(), 0 );
            }

        void indexOfStr_001()
            {
                rtl::OUString aStr1("Line for a indexOfStr.");

                rtl::OUString suSearch("Line");
                sal_Int32 nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 0);

                /* rtl::OUString */ suSearch("for");
                /* sal_Int32 */ nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 5);

                /* rtl::OUString */ suSearch("a");
                /* sal_Int32 */ nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 9);

                /* rtl::OUString */ suSearch("a index");
                /* sal_Int32 */ nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex ==9);
            }

        void indexOfStr_002()
            {
                rtl::OUString aStr1("Line for a indexOfStr.");
                rtl::OUString suSearch("not exist");
                sal_Int32 nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch );

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
                rtl_ustr_lastIndexOfStr( NULL, NULL );
            }

        void lastIndexOfStr_000_1()
            {
                rtl::OUString aStr1("Line for a lastIndexOfStr.");
                rtl_ustr_lastIndexOfStr( aStr1.getStr(), NULL );
            }

        void lastIndexOfStr_001()
            {
                rtl::OUString aStr1("Line for a lastIndexOfStr.");
                rtl::OUString aSearchStr("Index");

                sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 15);

                /* rtl::OString */ aSearchStr = rtl::OUString("Line");
                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == 0);

                /* rtl::OString */ aSearchStr = rtl::OUString("");
                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == -1);
            }

        void lastIndexOfStr_002()
            {
                rtl::OUString aStr1("Line for a lastIndexOfStr.");
                rtl::OUString aSearchStr("foo");
                sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

                CPPUNIT_ASSERT_MESSAGE("index is wrong.", nIndex == -1 );
            }

        void lastIndexOfStr_003()
            {
                rtl::OUString aStr1("Line for a lastIndexOfStr.");
                rtl::OUString aSearchStr("O");
                sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

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
                rtl_ustr_replaceChar( NULL, 0, 0 );
            }

        void replaceChar_001()
            {
                rtl::OUString aStr1("replace char.");
                rtl::OUString aShouldStr1("ruplacu char.");

                sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
                sal_Unicode* pStr = (sal_Unicode*) malloc( nLength + sizeof(sal_Unicode)); // length + 1 (null terminator)
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                memset(pStr, 0, nLength + sizeof(sal_Unicode));
                memcpy(pStr, aStr1.getStr(), nLength);

                rtl_ustr_replaceChar( pStr, 'e', 'u' );
                rtl::OUString suStr(pStr, aStr1.getLength());

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(suStr) == sal_True);
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
                rtl_ustr_replaceChar_WithLength( NULL, 0, 0, 0 );
            }

        void replaceChar_WithLength_000_1()
            {
                rtl_ustr_replaceChar_WithLength( NULL, 1, 0, 0 );
            }
        void replaceChar_WithLength_001()
            {
                rtl::OUString aStr1("replace char.");
                rtl::OUString aShouldStr1("ruplace char.");

                sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
                sal_Unicode* pStr = (sal_Unicode*) malloc(nLength);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                memcpy(pStr, aStr1.getStr(), nLength);

                rtl_ustr_replaceChar_WithLength( pStr, 6, 'e', 'u' );
                rtl::OUString suStr(pStr, aStr1.getLength());

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(suStr) == sal_True);
                free(pStr);
            }

        void replaceChar_WithLength_002()
            {
                rtl::OUString aStr1      ("eeeeeeeeeeeee");
                rtl::OUString aShouldStr1("uuuuuueeeeeee");

                sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
                sal_Unicode* pStr = (sal_Unicode*) malloc(nLength);                 // no null terminator is need
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                memcpy(pStr, aStr1.getStr(), nLength);

                rtl_ustr_replaceChar_WithLength( pStr, 6, 'e', 'u' );
                rtl::OUString suStr(pStr, aStr1.getLength());

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(suStr) == sal_True);
                free(pStr);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(replaceChar_WithLength);
        CPPUNIT_TEST(replaceChar_WithLength_000);
        CPPUNIT_TEST(replaceChar_WithLength_000_1);
        CPPUNIT_TEST(replaceChar_WithLength_001);
        CPPUNIT_TEST(replaceChar_WithLength_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar


// -----------------------------------------------------------------------------

    class toAsciiLowerCase : public CppUnit::TestFixture
    {
    public:

        void toAsciiLowerCase_000()
            {
                rtl_ustr_toAsciiLowerCase( NULL );
            }

        void toAsciiLowerCase_001()
            {
                rtl::OUString aStr1("CHANGE THIS TO ASCII LOWER CASE.");
                rtl::OUString aShouldStr1("change this to ascii lower case.");

                sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
                sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode) );  // we need to add '\0' so one more
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                memset(pStr, 0, nLength + sizeof(sal_Unicode));                             // empty the sal_Unicode array
                memcpy(pStr, aStr1.getStr(), nLength);

                rtl_ustr_toAsciiLowerCase( pStr );
                rtl::OUString suStr(pStr, aStr1.getLength());

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr) == sal_True);
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
                rtl_ustr_toAsciiLowerCase_WithLength( NULL, 0 );
            }

        void toAsciiLowerCase_WithLength_001()
            {
                rtl::OUString aStr1("CHANGE THIS TO ASCII LOWER CASE.");
                rtl::OUString aShouldStr1("change thiS TO ASCII LOWER CASE.");

                sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
                sal_Unicode* pStr = (sal_Unicode*) malloc(nLength);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                memcpy(pStr, aStr1.getStr(), nLength);

                rtl_ustr_toAsciiLowerCase_WithLength( pStr, 10 );

                rtl::OUString suStr(pStr, aStr1.getLength());
                sal_Bool bResult = aShouldStr1.equals(suStr);

                printOUString(suStr);
                printf("Result length: %d\n", suStr.getLength() );
                printf("Result: %d\n", bResult);

                CPPUNIT_ASSERT_MESSAGE("failed", bResult == sal_True);
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
                rtl_ustr_toAsciiUpperCase( NULL );
            }

        void toAsciiUpperCase_001()
            {
                rtl::OUString aStr1("change this to ascii upper case.");
                rtl::OUString aShouldStr1("CHANGE THIS TO ASCII UPPER CASE.");

                sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
                sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode)); // length + null terminator
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
                memset(pStr, 0, nLength + sizeof(sal_Unicode));
                memcpy(pStr, aStr1.getStr(), nLength);

                rtl_ustr_toAsciiUpperCase( pStr );
                rtl::OUString suStr(pStr, aStr1.getLength());

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr) == sal_True);
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
                rtl_ustr_toAsciiUpperCase_WithLength( NULL, 0 );
            }

        void toAsciiUpperCase_WithLength_001()
            {
                rtl::OUString aStr1("change this to ascii lower case.");
                rtl::OUString aShouldStr1("CHANGE THIs to ascii lower case.");

                sal_uInt32 nLength = aStr1.getLength() * sizeof(sal_Unicode);
                sal_Unicode* pStr = (sal_Unicode*) malloc(nLength);
                CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);

                memcpy(pStr, aStr1.getStr(), nLength);
                rtl_ustr_toAsciiUpperCase_WithLength( pStr, 10 );
                rtl::OUString suStr(pStr, aStr1.getLength());

                // printf("Uppercase with length: '%s'\n", aStr1.getStr());
                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr) == sal_True);
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
            rtl_ustr_trim_WithLength(NULL, 0);
            // should not GPF
        }

        void trim_WithLength_000_1()
        {
            rtl::OUString suStr("  trim this");

            sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
            sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
            memcpy(pStr, suStr.getStr(), nLength);

            rtl_ustr_trim_WithLength( pStr, 0 );
            free(pStr);
        }

        void trim_WithLength_001()
        {
            rtl::OUString suStr("  trim this");
            sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
            sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
            memcpy(pStr, suStr.getStr(), nLength);

            rtl_ustr_trim_WithLength( pStr, 2 );

            CPPUNIT_ASSERT_MESSAGE("string should be empty", rtl::OUString(pStr).getLength() == 0);
            free(pStr);
        }


        void trim_WithLength_002()
        {
            rtl::OUString suStr("trim this");

            sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
            sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
            memcpy(pStr, suStr.getStr(), nLength);

            rtl_ustr_trim_WithLength( pStr, 5 );

            CPPUNIT_ASSERT_MESSAGE("string should contain 'trim'", rtl::OUString(pStr).getLength() == 4);
            free(pStr);
        }


        void trim_WithLength_003()
        {
            rtl::OUString suStr("     trim   this");

            sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
            sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
            memcpy(pStr, suStr.getStr(), nLength);

            rtl_ustr_trim_WithLength( pStr, 11 );

            CPPUNIT_ASSERT_MESSAGE("string should contain 'trim'", rtl::OUString(pStr).getLength() == 4);
            free(pStr);
        }

        void trim_WithLength_004()
        {
            rtl::OUString suStr("\r\n\t \n\r    trim  \n this");

            sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
            sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
            memcpy(pStr, suStr.getStr(), nLength);

            rtl_ustr_trim_WithLength( pStr, 17 );

            CPPUNIT_ASSERT_MESSAGE("string should contain 'trim'", rtl::OUString(pStr).getLength() == 4);
            free(pStr);
        }

        void trim_WithLength_005()
        {
            rtl::OUString suStr("\r\n\t \n\r    trim \t this \n\r\t\t     ");

            sal_uInt32 nLength = suStr.getLength() * sizeof(sal_Unicode);
            sal_Unicode *pStr = (sal_Unicode*)malloc(nLength);
            memcpy(pStr, suStr.getStr(), nLength);

            rtl_ustr_trim_WithLength( pStr, suStr.getLength() );

            CPPUNIT_ASSERT_MESSAGE("string should contain 'trim \\t this'", rtl::OUString(pStr).getLength() == 11);
            free(pStr);
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
                rtl_ustr_valueOfChar(NULL, 0);
                // should not GPF
            }
        void valueOfChar_001()
            {
                sal_Unicode *pStr = (sal_Unicode*)malloc(RTL_USTR_MAX_VALUEOFCHAR);
                if (pStr)
                {
                    rtl_ustr_valueOfChar(pStr, 'A');

                    CPPUNIT_ASSERT_MESSAGE("string should contain 'A'", pStr[0] == L'A');
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




    class ascii_compare_WithLength : public CppUnit::TestFixture
    {
    public:
        void zero_length()
        {
            sal_Unicode pUnicode[] = {0xffff, 0xffff};
            char const * pAscii = "reference";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(pUnicode, 0, pAscii);
            CPPUNIT_ASSERT_MESSAGE("ref string is empty, compare failed, needs to be <0.", value < 0);
        }

        void equal_ascii_shorter()
        {
            rtl::OUString refStr("referenceString");
            char const * pAscii = "reference";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
            CPPUNIT_ASSERT_MESSAGE("ref string is bigger, compare failed, needs to be >0.", value > 0);
        }

        void equal_ascii_shorter_asciiLength()
        {
            rtl::OUString refStr("referenceString");
            char const * pAscii = "reference";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, rtl_str_getLength(pAscii), pAscii);
            CPPUNIT_ASSERT_MESSAGE("ref string is bigger despite ascii length, compare failed, needs to be == 0.", value == 0);
        }

        void equal_ref_shorter()
        {
            rtl::OUString refStr("reference");
            char const * pAscii = "referenceString";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
            CPPUNIT_ASSERT_MESSAGE("ascii string is bigger, but only compared to ref length, needs to be 0.", value < 0);
        }

        void equal()
        {
            rtl::OUString refStr("reference");
            char const * pAscii = "reference";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
            CPPUNIT_ASSERT_MESSAGE("strings are equal, compare failed, needs to be 0.", value == 0);
        }

        void unequal_reference_bigger()
       {
            rtl::OUString refStr("defghi");
            char const * pAscii = "abc";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
            CPPUNIT_ASSERT_MESSAGE("strings are unequal and ref is bigger, needs to be >0.", value > 0);
        }

        void unequal_ascii_bigger()
        {
            rtl::OUString refStr("abc");
            char const * pAscii = "defghi";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);

            CPPUNIT_ASSERT_MESSAGE("strings are unequal and ascii is bigger, needs to be <0.", value < 0);
        }

        CPPUNIT_TEST_SUITE(ascii_compare_WithLength);
        CPPUNIT_TEST(zero_length);
        CPPUNIT_TEST(equal_ascii_shorter);
        CPPUNIT_TEST(equal_ascii_shorter_asciiLength);
        CPPUNIT_TEST(equal_ref_shorter);
        CPPUNIT_TEST(equal);
        CPPUNIT_TEST(unequal_reference_bigger);
        CPPUNIT_TEST(unequal_ascii_bigger);
        CPPUNIT_TEST_SUITE_END();
    };




    class ascii_shortenedCompareIgnoreAsciiCase_WithLength : public CppUnit::TestFixture
    {
    public:

        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_000()
            {
                rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( NULL, 0, NULL, 0);
                // should not GPF
            }

        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_000_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), NULL, 0);
                // should not GPF
            }
        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_000_2()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl::OString sStr2 =                                 "Line is shorter.";
                rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), sStr2.getLength(), sStr2.getStr(), 0);
                // should not GPF
            }
        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_001()
            {
                rtl::OUString suStr1;
                rtl::OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( suStr1, 0, sStr2.getStr(), 0);
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_002()
            {
                rtl::OUString suStr1("Line must be equal.");
                rtl::OString sStr2 =                                  "Line must be equal.";

                sal_Int32 nValue = rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr(), sStr2.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_003()
            {
                rtl::OUString suStr1("Line must differ.");
                rtl::OString sStr2 =                                  "Line must be differ and longer.";

                sal_Int32 nValue = rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr(), sStr2.getLength());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ascii_shortenedCompareIgnoreAsciiCase_WithLength);
        CPPUNIT_TEST(ascii_shortenedCompareIgnoreAsciiCase_WithLength_000);
        CPPUNIT_TEST(ascii_shortenedCompareIgnoreAsciiCase_WithLength_000_1);
        CPPUNIT_TEST(ascii_shortenedCompareIgnoreAsciiCase_WithLength_000_2);
        CPPUNIT_TEST(ascii_shortenedCompareIgnoreAsciiCase_WithLength_001);
        CPPUNIT_TEST(ascii_shortenedCompareIgnoreAsciiCase_WithLength_002);
        CPPUNIT_TEST(ascii_shortenedCompareIgnoreAsciiCase_WithLength_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class ascii_shortenedCompareIgnoreAsciiCase_WithLength

// -----------------------------------------------------------------------------

    class ascii_compareIgnoreAsciiCase_WithLength : public CppUnit::TestFixture
    {
    public:

        void ascii_compareIgnoreAsciiCase_WithLength_000()
            {
                rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( NULL, 0, NULL);
                // should not GPF
            }

        void ascii_compareIgnoreAsciiCase_WithLength_000_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( aStr1.getStr(), 0, NULL);
                // should not GPF
            }
        void ascii_compareIgnoreAsciiCase_WithLength_000_2()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl::OString sStr2 =                                 "Line is shorter.";
                rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( aStr1.getStr(), sStr2.getLength(), sStr2.getStr());
                // should not GPF
            }
        void ascii_compareIgnoreAsciiCase_WithLength_001()
            {
                rtl::OUString suStr1;
                rtl::OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( suStr1, 0, sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compareIgnoreAsciiCase_WithLength failed, strings are equal.", nValue == 0);
            }

        void ascii_compareIgnoreAsciiCase_WithLength_002()
            {
                rtl::OUString suStr1("Line must be equal.");
                rtl::OString sStr2 =                                  "Line must be equal.";

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_compareIgnoreAsciiCase_WithLength_003()
            {
                rtl::OUString suStr1("Line must differ.");
                rtl::OString sStr2 =                                  "Line must be differ and longer.";

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ascii_compareIgnoreAsciiCase_WithLength);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_WithLength_000);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_WithLength_000_1);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_WithLength_000_2);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_WithLength_001);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_WithLength_002);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_WithLength_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class ascii_compareIgnoreAsciiCase_WithLength

// -----------------------------------------------------------------------------

    class ascii_compare : public CppUnit::TestFixture
    {
    public:

        void ascii_compare_000()
            {
                rtl_ustr_ascii_compare( NULL, NULL);
                // should not GPF
            }

        void ascii_compare_000_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl_ustr_ascii_compare( aStr1.getStr(), NULL);
                // should not GPF
            }
        void ascii_compare_001()
            {
                rtl::OUString suStr1;
                rtl::OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1, sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_compare_002()
            {
                rtl::OUString suStr1("Line must be equal.");
                rtl::OString sStr2 =                                  "Line must be equal.";

                sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_compare_003()
            {
                rtl::OUString suStr1("Line must differ.");
                rtl::OString sStr2 = "Line foo bar, ok, differ.";

                sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ascii_compare);
        CPPUNIT_TEST(ascii_compare_000);
        CPPUNIT_TEST(ascii_compare_000_1);
        CPPUNIT_TEST(ascii_compare_001);
        CPPUNIT_TEST(ascii_compare_002);
        CPPUNIT_TEST(ascii_compare_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class ascii_compare

// -----------------------------------------------------------------------------

    class ascii_compareIgnoreAsciiCase : public CppUnit::TestFixture
    {
    public:

        void ascii_compareIgnoreAsciiCase_000()
            {
                rtl_ustr_ascii_compareIgnoreAsciiCase( NULL, NULL);
                // should not GPF
            }

        void ascii_compareIgnoreAsciiCase_000_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl_ustr_ascii_compareIgnoreAsciiCase( aStr1.getStr(), NULL);
                // should not GPF
            }
        void ascii_compareIgnoreAsciiCase_001()
            {
                rtl::OUString suStr1;
                rtl::OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1, sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_compareIgnoreAsciiCase_002()
            {
                rtl::OUString suStr1("Line must be equal.");
                rtl::OString sStr2 =                                  "Line must be equal.";

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_compareIgnoreAsciiCase_002_1()
            {
                rtl::OUString suStr1("Line must be equal, when ignore case.");
                rtl::OString sStr2 =                                 "LINE MUST BE EQUAL, WHEN IGNORE CASE.";

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal (if case insensitve).", nValue == 0);
            }

        void ascii_compareIgnoreAsciiCase_003()
            {
                rtl::OUString suStr1("Line must differ.");
                rtl::OString sStr2 = "Line foo bar, ok, differ.";

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

        //! LLA: some more tests with some high level strings

        // void ascii_compareIgnoreAsciiCase_001()
        //     {
        //         rtl::OUString suStr1("change this to ascii upper case.");
        //         rtl::OUString aShouldStr1("CHANGE THIS TO ASCII UPPER CASE.");
        //
        //         sal_uInt32 nLength = suStr1.getLength() * sizeof(sal_Unicode);
        //         sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode)); // length + null terminator
        //         CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
        //         memset(pStr, 0, nLength + sizeof(sal_Unicode));
        //         memcpy(pStr, suStr1.getStr(), nLength);
        //
        //         rtl_ustr_ascii_compareIgnoreAsciiCase( pStr );
        //         rtl::OUString suStr(pStr, suStr1.getLength());
        //
        //         CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr) == sal_True);
        //         free(pStr);
        //     }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ascii_compareIgnoreAsciiCase);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_000);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_000_1);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_001);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_002);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_002_1);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class ascii_compareIgnoreAsciiCase


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

    class getToken : public CppUnit::TestFixture
    {
    public:

        void getToken_000()
            {
                rtl_ustr_ascii_compareIgnoreAsciiCase( NULL, NULL);
                // should not GPF
            }

        void ascii_compareIgnoreAsciiCase_000_1()
            {
                rtl::OUString aStr1("Line must be equal.");
                rtl_ustr_ascii_compareIgnoreAsciiCase( aStr1.getStr(), NULL);
                // should not GPF
            }
        void ascii_compareIgnoreAsciiCase_001()
            {
                rtl::OUString suStr1;
                rtl::OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1, sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_compareIgnoreAsciiCase_002()
            {
                rtl::OUString suStr1("Line must be equal.");
                rtl::OString sStr2 =                                  "Line must be equal.";

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal.", nValue == 0);
            }

        void ascii_compareIgnoreAsciiCase_002_1()
            {
                rtl::OUString suStr1("Line must be equal, when ignore case.");
                rtl::OString sStr2 =                                 "LINE MUST BE EQUAL, WHEN IGNORE CASE.";

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings are equal (if case insensitve).", nValue == 0);
            }

        void ascii_compareIgnoreAsciiCase_003()
            {
                rtl::OUString suStr1("Line must differ.");
                rtl::OString sStr2 = "Line foo bar, ok, differ.";

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

        //! LLA: some more tests with some high level strings

        // void ascii_compareIgnoreAsciiCase_001()
        //     {
        //         rtl::OUString suStr1("change this to ascii upper case.");
        //         rtl::OUString aShouldStr1("CHANGE THIS TO ASCII UPPER CASE.");
        //
        //         sal_uInt32 nLength = suStr1.getLength() * sizeof(sal_Unicode);
        //         sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode)); // length + null terminator
        //         CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != NULL);
        //         memset(pStr, 0, nLength + sizeof(sal_Unicode));
        //         memcpy(pStr, suStr1.getStr(), nLength);
        //
        //         rtl_ustr_ascii_compareIgnoreAsciiCase( pStr );
        //         rtl::OUString suStr(pStr, suStr1.getLength());
        //
        //         CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr) == sal_True);
        //         free(pStr);
        //     }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ascii_compareIgnoreAsciiCase);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_000);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_000_1);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_001);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_002);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_002_1);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class ascii_compareIgnoreAsciiCase

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::compare);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::compareIgnoreAsciiCase);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::ascii_compare_WithLength);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::shortenedCompareIgnoreAsciiCase_WithLength);
// CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::hashCode);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::indexOfChar);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::lastIndexOfChar);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::indexOfStr);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::lastIndexOfStr);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::replaceChar);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::replaceChar_WithLength);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::toAsciiLowerCase);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::toAsciiLowerCase_WithLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::toAsciiUpperCase);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::toAsciiUpperCase_WithLength);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::trim_WithLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::valueOfChar);

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::ascii_compare);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::ascii_compareIgnoreAsciiCase);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::ascii_compareIgnoreAsciiCase_WithLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::ascii_shortenedCompareIgnoreAsciiCase_WithLength);

} // namespace rtl_ustr

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions("")
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
