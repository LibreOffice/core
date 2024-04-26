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

#include <sal/config.h>

#include <o3tl/cppunittraitshelper.hxx>
#include <rtl/ustring.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace rtl_ustr
{

    class compare : public CppUnit::TestFixture
    {
        void compare_001()
            {
                OUString aStr1;
                OUString aStr2;

                sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                OUString aStr2(u"Line must be equal."_ustr);

                sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_003()
            {
                OUString aStr1(u"Line must differ."_ustr);
                OUString aStr2(u"Line foo bar, ok, differ."_ustr);

                sal_Int32 nValue = rtl_ustr_compare( aStr1.getStr(), aStr2.getStr());
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
                OUString aStr1;
                OUString aStr2;

                sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                OUString aStr2(u"Line must be equal."_ustr);

                sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002_1()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                OUString aStr2(u"LINE MUST BE EQUAL."_ustr);

                sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal (if case insensitive).", sal_Int32(0), nValue);
            }

        void compare_003()
            {
                OUString aStr1(u"Line must differ."_ustr);
                OUString aStr2(u"Line foo bar, ok, differ."_ustr);

                sal_Int32 nValue = rtl_ustr_compareIgnoreAsciiCase( aStr1.getStr(), aStr2.getStr());
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
                rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( nullptr, 0, nullptr, 0, 0);
            }

        void compare_000_1()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), nullptr, 0, 1);
            }
        void compare_001()
            {
                OUString aStr1;
                OUString aStr2;

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), aStr2.getStr(), aStr2.getLength(), aStr1.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                OUString aStr2(u"Line must be equal."_ustr);

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void compare_002_1()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                OUString aStr2(u"LINE MUST BE EQUAL."_ustr);

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       aStr1.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal (if case insensitive).", sal_Int32(0), nValue);
            }

        void compare_003()
            {
                OUString aStr1(u"Line must differ."_ustr);
                OUString aStr2(u"Line foo bar, ok, differ."_ustr);

                sal_Int32 nValue = rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(),
                                                                                       aStr2.getStr(), aStr2.getLength(),
                                                                                       5);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal first 5 characters.", sal_Int32(0), nValue);
            }

        void compare_004()
            {
                OUString aStr1(u"Line must differ."_ustr);
                OUString aStr2(u"Line foo bar, ok, differ."_ustr);

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

//
//
//     class hashCode : public CppUnit::TestFixture
//     {
//         void hashCode_000()
//             {
//                 sal_Int32 nHashCode = rtl_ustr_hashCode( nullptr );
//                 volatile int dummy = 0;
//             }
//
//         void hashCode_001()
//             {
//                 OString aStr1 = "Line for a hashCode.";
//                 sal_Int32 nHashCode = rtl_ustr_hashCode( aStr1.getStr() );
//                 printf("hashcode: %d\n", nHashCode);
//                 // CPPUNIT_ASSERT_MESSAGE("failed.", nValue == 0);
//             }
//
//         void hashCode_002()
//             {
//                 OString aStr1 = "Line for a hashCode.";
//                 sal_Int32 nHashCode1 = rtl_ustr_hashCode( aStr1.getStr() );
//
//                 OString aStr2 = "Line for a hashCode.";
//                 sal_Int32 nHashCode2 = rtl_ustr_hashCode( aStr2.getStr() );
//
//                 CPPUNIT_ASSERT_MESSAGE("hashcodes must be equal.", nHashCode1 == nHashCode2 );
//             }
//
//         void hashCode_003()
//             {
//                 OString aStr1 = "Line for a hashCode.";
//                 sal_Int32 nHashCode1 = rtl_ustr_hashCode( aStr1.getStr() );
//
//                 OString aStr2 = "Line for another hashcode.";
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

    class indexOfChar : public CppUnit::TestFixture
    {
        void indexOfChar_000()
            {
                sal_Int32 nIndex = rtl_ustr_indexOfChar( u"", 0 );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Trailing zero character is not part of the string",
                                             sal_Int32(-1), nIndex);
            }

        void indexOfChar_001()
            {
                OUString aStr1(u"Line for an indexOfChar."_ustr);

                sal_Int32 nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'L' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(0), nIndex);

                /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'i' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(1), nIndex);

                /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'n' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(2), nIndex);

                /* sal_Int32 */ nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'e' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(3), nIndex);
            }

        void indexOfChar_002()
            {
                OUString aStr1(u"Line for an indexOfChar."_ustr);
                sal_Int32 nIndex = rtl_ustr_indexOfChar( aStr1.getStr(), 'y' );

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
    }; // class indexOfChar

    class lastIndexOfChar : public CppUnit::TestFixture
    {
        void lastIndexOfChar_000()
            {
                sal_Int32 nIndex = rtl_ustr_lastIndexOfChar( u"", 0 );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Trailing zero character is not part of the string",
                                             sal_Int32(-1), nIndex);
            }

        void lastIndexOfChar_001()
            {
                OUString aStr1(u"Line for a lastIndexOfChar."_ustr);

                sal_Int32 nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'C' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(22), nIndex);

                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'h' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(23), nIndex);

                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'a' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(24), nIndex);

                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'r' );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(25), nIndex);
            }

        void lastIndexOfChar_002()
            {
                OUString aStr1(u"Line for a lastIndexOfChar."_ustr);
                sal_Int32 nIndex = rtl_ustr_lastIndexOfChar( aStr1.getStr(), 'y' );

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
                OUString aStr1(u"Line for an indexOfStr."_ustr);
                sal_Int32 nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), u"" );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("an empty substring is always not findable",
                                             sal_Int32(-1), nIndex);
            }

        void indexOfStr_001()
            {
                OUString aStr1(u"Line for an indexOfStr."_ustr);

                OUString suSearch(u"Line"_ustr);
                sal_Int32 nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(0), nIndex);

                suSearch = "for";
                nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(5), nIndex);

                suSearch = "a";
                /* sal_Int32 */ nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(9), nIndex);

                suSearch = "an index";
                /* sal_Int32 */ nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(9), nIndex);
            }

        void indexOfStr_002()
            {
                OUString aStr1(u"Line for an indexOfStr."_ustr);
                OUString suSearch(u"not exist"_ustr);
                sal_Int32 nIndex = rtl_ustr_indexOfStr( aStr1.getStr(), suSearch.getStr() );

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
                OUString aStr1(u"Line for a lastIndexOfStr."_ustr);
                sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), u"" );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("an empty substring is always not findable",
                                             sal_Int32(-1), nIndex);
            }

        void lastIndexOfStr_001()
            {
                OUString aStr1(u"Line for a lastIndexOfStr."_ustr);
                OUString aSearchStr(u"Index"_ustr);

                sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(15), nIndex);

                /* OString */ aSearchStr = u"Line"_ustr;
                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(0), nIndex);

                /* OString */ aSearchStr = OUString();
                /* sal_Int32 */ nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );
                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(-1), nIndex);
            }

        void lastIndexOfStr_002()
            {
                OUString aStr1(u"Line for a lastIndexOfStr."_ustr);
                OUString aSearchStr(u"foo"_ustr);
                sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

                CPPUNIT_ASSERT_EQUAL_MESSAGE("index is wrong.", sal_Int32(-1), nIndex);
            }

        void lastIndexOfStr_003()
            {
                OUString aStr1(u"Line for a lastIndexOfStr."_ustr);
                OUString aSearchStr(u"O"_ustr);
                sal_Int32 nIndex = rtl_ustr_lastIndexOfStr( aStr1.getStr(), aSearchStr.getStr() );

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
                sal_Unicode pStr[] = u"replace char.";
                OUString aShouldStr1(u"ruplacu char."_ustr);

                rtl_ustr_replaceChar( pStr, 'e', 'u' );
                OUString suStr(pStr);

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(suStr));
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
                rtl_ustr_replaceChar_WithLength( nullptr, 0, 0, 0 );
            }

        void replaceChar_WithLength_001()
            {
                sal_Unicode pStr[] = u"replace char.";
                OUString aShouldStr1(u"ruplace char."_ustr);

                rtl_ustr_replaceChar_WithLength( pStr, 6, 'e', 'u' );
                OUString suStr(pStr);

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(suStr));
            }

        void replaceChar_WithLength_002()
            {
                sal_Unicode pStr[] = u"eeeeeeeeeeeee";
                OUString aShouldStr1(u"uuuuuueeeeeee"_ustr);

                rtl_ustr_replaceChar_WithLength( pStr, 6, 'e', 'u' );
                OUString suStr(pStr);

                CPPUNIT_ASSERT_MESSAGE("replace failed", aShouldStr1.equals(suStr));
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(replaceChar_WithLength);
        CPPUNIT_TEST(replaceChar_WithLength_000);
        CPPUNIT_TEST(replaceChar_WithLength_001);
        CPPUNIT_TEST(replaceChar_WithLength_002);
        CPPUNIT_TEST_SUITE_END();
    }; // class replaceChar

    class toAsciiLowerCase : public CppUnit::TestFixture
    {
        void toAsciiLowerCase_001()
            {
                sal_Unicode pStr[] = u"CHANGE THIS TO ASCII LOWER CASE.";
                OUString aShouldStr1(u"change this to ascii lower case."_ustr);

                rtl_ustr_toAsciiLowerCase( pStr );
                OUString suStr(pStr);

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr));
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
                rtl_ustr_toAsciiLowerCase_WithLength( nullptr, 0 );
            }

        void toAsciiLowerCase_WithLength_001()
            {
                sal_Unicode pStr[] = u"CHANGE THIS TO ASCII LOWER CASE.";
                OUString aShouldStr1(u"change thiS TO ASCII LOWER CASE."_ustr);

                rtl_ustr_toAsciiLowerCase_WithLength( pStr, 10 );
                OUString suStr(pStr);

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr));
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
                sal_Unicode pStr[] = u"change this to ascii upper case.";
                OUString aShouldStr1(u"CHANGE THIS TO ASCII UPPER CASE."_ustr);

                rtl_ustr_toAsciiUpperCase( pStr );
                OUString suStr(pStr);

                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr));
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
                rtl_ustr_toAsciiUpperCase_WithLength( nullptr, 0 );
            }

        void toAsciiUpperCase_WithLength_001()
            {
                sal_Unicode pStr[] = u"change this to ascii lower case.";
                OUString aShouldStr1(u"CHANGE THIs to ascii lower case."_ustr);

                rtl_ustr_toAsciiUpperCase_WithLength( pStr, 10 );
                OUString suStr(pStr);

                // printf("Uppercase with length: '%s'\n", aStr1.getStr());
                CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr));
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
            rtl_ustr_trim_WithLength(nullptr, 0);
            // should not GPF
        }

        void trim_WithLength_000_1()
        {
            sal_Unicode pStr[] = u"  trim this";

            rtl_ustr_trim_WithLength( pStr, 0 );
        }

        void trim_WithLength_001()
        {
            sal_Unicode pStr[] = u"  trim this";

            rtl_ustr_trim_WithLength( pStr, 2 );

            CPPUNIT_ASSERT_MESSAGE("string should be empty", OUString(pStr).isEmpty());
        }

        void trim_WithLength_002()
        {
            sal_Unicode pStr[] = u"trim this";

            rtl_ustr_trim_WithLength( pStr, 5 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'trim'", sal_Int32(4), OUString(pStr).getLength());
        }

        void trim_WithLength_003()
        {
            sal_Unicode pStr[] = u"     trim   this";

            rtl_ustr_trim_WithLength( pStr, 11 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'trim'", sal_Int32(4), OUString(pStr).getLength());
        }

        void trim_WithLength_004()
        {
            sal_Unicode pStr[] = u"\r\n\t \n\r    trim  \n this";

            rtl_ustr_trim_WithLength( pStr, 17 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'trim'", sal_Int32(4), OUString(pStr).getLength());
        }

        void trim_WithLength_005()
        {
            sal_Unicode pStr[] = u"\r\n\t \n\r    trim \t this \n\r\t\t     ";

            rtl_ustr_trim_WithLength( pStr, std::size(pStr) - 1 );

            CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'trim \\t this'", sal_Int32(11), OUString(pStr).getLength());
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
                sal_Unicode pStr[RTL_USTR_MAX_VALUEOFCHAR];
                rtl_ustr_valueOfChar(pStr, 'A');

                CPPUNIT_ASSERT_EQUAL_MESSAGE("string should contain 'A'", u'A', pStr[0]);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(valueOfChar);
        CPPUNIT_TEST(valueOfChar_001);
        CPPUNIT_TEST_SUITE_END();
    };

    class ascii_compare_WithLength : public CppUnit::TestFixture
    {
        void zero_length()
        {
            sal_Unicode pUnicode[] = {0xffff, 0xffff};
            char const pAscii[] = "reference";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(pUnicode, 0, pAscii);
            CPPUNIT_ASSERT_LESS(sal_Int32(0), value);
        }

        void equal_ascii_shorter()
        {
            OUString refStr(u"referenceString"_ustr);
            char const pAscii[] = "reference";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
            CPPUNIT_ASSERT_GREATER(sal_Int32(0), value);
        }

        void equal_ascii_shorter_asciiLength()
        {
            OUString refStr(u"referenceString"_ustr);
            char const pAscii[] = "reference";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, rtl_str_getLength(pAscii), pAscii);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), value);
        }

        void equal_ref_shorter()
        {
            OUString refStr(u"reference"_ustr);
            char const pAscii[] = "referenceString";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
            CPPUNIT_ASSERT_LESS(sal_Int32(0), value);
        }

        void equal()
        {
            OUString refStr(u"reference"_ustr);
            char const pAscii[] = "reference";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), value);
        }

        void unequal_reference_bigger()
       {
            OUString refStr(u"defghi"_ustr);
            char const pAscii[] = "abc";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);
            CPPUNIT_ASSERT_GREATER(sal_Int32(0), value);
        }

        void unequal_ascii_bigger()
        {
            OUString refStr(u"abc"_ustr);
            char const pAscii[] = "defghi";

            sal_Int32 value = rtl_ustr_ascii_compare_WithLength(refStr.pData->buffer, refStr.pData->length, pAscii);

            CPPUNIT_ASSERT_LESS(sal_Int32(0), value);
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
        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_000()
            {
                rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( nullptr, 0, "", 0);
                // should not GPF
            }

        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_000_1()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), aStr1.getLength(), "", 0);
                // should not GPF
            }
        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_000_2()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                OString sStr2 =                                 "Line is shorter."_ostr;
                rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( aStr1.getStr(), sStr2.getLength(), sStr2.getStr(), 0);
                // should not GPF
            }
        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_001()
            {
                OUString suStr1;
                OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( suStr1.getStr(), 0, sStr2.getStr(), 0);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_002()
            {
                OUString suStr1(u"Line must be equal."_ustr);
                OString sStr2 =                                  "Line must be equal."_ostr;

                sal_Int32 nValue = rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr(), sStr2.getLength());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void ascii_shortenedCompareIgnoreAsciiCase_WithLength_003()
            {
                OUString suStr1(u"Line must differ."_ustr);
                OString sStr2 =                                  "Line must be differ and longer."_ostr;

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

    class ascii_compareIgnoreAsciiCase_WithLength : public CppUnit::TestFixture
    {
        void ascii_compareIgnoreAsciiCase_WithLength_000()
            {
                rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( nullptr, 0, "");
                // should not GPF
            }

        void ascii_compareIgnoreAsciiCase_WithLength_000_1()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( aStr1.getStr(), 0, "");
                // should not GPF
            }
        void ascii_compareIgnoreAsciiCase_WithLength_000_2()
            {
                OUString aStr1(u"Line must be equal."_ustr);
                OString sStr2 =                                 "Line is shorter."_ostr;
                rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( aStr1.getStr(), sStr2.getLength(), sStr2.getStr());
                // should not GPF
            }
        void ascii_compareIgnoreAsciiCase_WithLength_001()
            {
                OUString suStr1;
                OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( suStr1.getStr(), 0, sStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compareIgnoreAsciiCase_WithLength failed, strings are equal.", sal_Int32(0), nValue);
            }

        void ascii_compareIgnoreAsciiCase_WithLength_002()
            {
                OUString suStr1(u"Line must be equal."_ustr);
                OString sStr2 =                                  "Line must be equal."_ostr;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( suStr1.getStr(), suStr1.getLength(), sStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void ascii_compareIgnoreAsciiCase_WithLength_003()
            {
                OUString suStr1(u"Line must differ."_ustr);
                OString sStr2 =                                  "Line must be differ and longer."_ostr;

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

    class ascii_compare : public CppUnit::TestFixture
    {
        void ascii_compare_001()
            {
                OUString suStr1;
                OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void ascii_compare_002()
            {
                OUString suStr1(u"Line must be equal."_ustr);
                OString sStr2 =                                  "Line must be equal."_ostr;

                sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void ascii_compare_003()
            {
                OUString suStr1(u"Line must differ."_ustr);
                OString sStr2 = "Line foo bar, ok, differ."_ostr;

                sal_Int32 nValue = rtl_ustr_ascii_compare( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ascii_compare);
        CPPUNIT_TEST(ascii_compare_001);
        CPPUNIT_TEST(ascii_compare_002);
        CPPUNIT_TEST(ascii_compare_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class ascii_compare

    class ascii_compareIgnoreAsciiCase : public CppUnit::TestFixture
    {
        void ascii_compareIgnoreAsciiCase_001()
            {
                OUString suStr1;
                OString sStr2;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void ascii_compareIgnoreAsciiCase_002()
            {
                OUString suStr1(u"Line must be equal."_ustr);
                OString sStr2 =                                  "Line must be equal."_ostr;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal.", sal_Int32(0), nValue);
            }

        void ascii_compareIgnoreAsciiCase_002_1()
            {
                OUString suStr1(u"Line must be equal, when ignore case."_ustr);
                OString sStr2 =                                 "LINE MUST BE EQUAL, WHEN IGNORE CASE."_ostr;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("compare failed, strings are equal (if case insensitive).", sal_Int32(0), nValue);
            }

        void ascii_compareIgnoreAsciiCase_003()
            {
                OUString suStr1(u"Line must differ."_ustr);
                OString sStr2 = "Line foo bar, ok, differ."_ostr;

                sal_Int32 nValue = rtl_ustr_ascii_compareIgnoreAsciiCase( suStr1.getStr(), sStr2.getStr());
                CPPUNIT_ASSERT_MESSAGE("compare failed, strings differ.", nValue != 0);
            }

        //! LLA: some more tests with some high level strings

        // void ascii_compareIgnoreAsciiCase_001()
        //     {
        //         OUString suStr1("change this to ascii upper case.");
        //         OUString aShouldStr1("CHANGE THIS TO ASCII UPPER CASE.");
        //
        //         sal_uInt32 nLength = suStr1.getLength() * sizeof(sal_Unicode);
        //         sal_Unicode* pStr = (sal_Unicode*) malloc(nLength + sizeof(sal_Unicode)); // length + null terminator
        //         CPPUNIT_ASSERT_MESSAGE("can't get memory for test", pStr != nullptr);
        //         memset(pStr, 0, nLength + sizeof(sal_Unicode));
        //         memcpy(pStr, suStr1.getStr(), nLength);
        //
        //         rtl_ustr_ascii_compareIgnoreAsciiCase( pStr );
        //         OUString suStr(pStr, suStr1.getLength());
        //
        //         CPPUNIT_ASSERT_MESSAGE("failed", aShouldStr1.equals(suStr));
        //         free(pStr);
        //     }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ascii_compareIgnoreAsciiCase);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_001);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_002);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_002_1);
        CPPUNIT_TEST(ascii_compareIgnoreAsciiCase_003);
        CPPUNIT_TEST_SUITE_END();
    }; // class ascii_compareIgnoreAsciiCase

    // sample out of inc/rtl/ustring.hxx
    // rtl_uString * pToken = nullptr;
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
        void getToken_000()
        {
            OUString s(u"a;b;c"_ustr);
            // Replace the string in place
            const sal_Int32 i = rtl_uString_getToken(&s.pData, s.pData, 1, ';', 0);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(4), i);
            CPPUNIT_ASSERT_EQUAL(u"b"_ustr, s);
        }

        CPPUNIT_TEST_SUITE(getToken);
        CPPUNIT_TEST(getToken_000);
        CPPUNIT_TEST_SUITE_END();
    }; // class ascii_compareIgnoreAsciiCase

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

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_ustr::getToken);

} // namespace rtl_ustr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
