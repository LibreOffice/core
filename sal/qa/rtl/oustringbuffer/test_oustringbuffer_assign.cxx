/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

namespace {

class Test: public CppUnit::TestFixture {
private:
    void test() {
        OUStringBuffer b1;
        OUString s1("123456789012345");
        b1 = s1;
        CPPUNIT_ASSERT_EQUAL(s1, b1.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b1.getCapacity());
        OUString s2("abc");
        b1 = s2;
        CPPUNIT_ASSERT_EQUAL(s2, b1.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b1.getCapacity());
        OUString s3("1234567890123456");
        b1 = s3;
        CPPUNIT_ASSERT_EQUAL(s3, b1.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(32), b1.getCapacity());
        OUStringBuffer b2;
        b2 = "123456789012345";
        CPPUNIT_ASSERT_EQUAL(s1, b2.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b2.getCapacity());
        b2 = "abc";
        CPPUNIT_ASSERT_EQUAL(s2, b2.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b2.getCapacity());
        b2 = "1234567890123456";
        CPPUNIT_ASSERT_EQUAL(s3, b2.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(32), b2.getCapacity());
#if HAVE_CXX11_UTF16_STRING_LITERAL && !defined SAL_W32
    // sal_Unicode is still wchar_t not char16_t even for MSVC 2015
        OUStringBuffer b3;
        b3 = u"123456789012345";
        CPPUNIT_ASSERT_EQUAL(s1, b3.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b3.getCapacity());
        b3 = u"abc";
        CPPUNIT_ASSERT_EQUAL(s2, b3.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b3.getCapacity());
        b3 = u"1234567890123456";
        CPPUNIT_ASSERT_EQUAL(s3, b3.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(32), b3.getCapacity());
#endif
        OUStringBuffer b4;
        b4 = OUStringLiteral("1") + "23456789012345";
        CPPUNIT_ASSERT_EQUAL(s1, b4.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b4.getCapacity());
        b4 = OUStringLiteral("a") + "bc";
        CPPUNIT_ASSERT_EQUAL(s2, b4.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), b4.getCapacity());
        b4 = OUStringLiteral("1") + "234567890123456";
        CPPUNIT_ASSERT_EQUAL(s3, b4.toString());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(32), b4.getCapacity());
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
