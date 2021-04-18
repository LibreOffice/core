/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/string.hxx>

namespace {

class Test: public CppUnit::TestFixture {
private:
    void testStartsWithIgnoreAsciiCase();
    void testCompareTo();
    void testUtf8StringLiterals();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStartsWithIgnoreAsciiCase);
    CPPUNIT_TEST(testCompareTo);
    CPPUNIT_TEST(testUtf8StringLiterals);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testStartsWithIgnoreAsciiCase() {
    {
        OString r;
        CPPUNIT_ASSERT(OString().startsWithIgnoreAsciiCase(std::string_view(), &r));
        CPPUNIT_ASSERT(r.isEmpty());
    }
    {
        OString r;
        CPPUNIT_ASSERT(OString("foo").startsWithIgnoreAsciiCase(std::string_view(), &r));
        CPPUNIT_ASSERT_EQUAL(OString("foo"), r);
    }
    {
        OString r;
        CPPUNIT_ASSERT(
            OString("foo").startsWithIgnoreAsciiCase("F", &r));
        CPPUNIT_ASSERT_EQUAL(OString("oo"), r);
    }
    {
        OString r("other");
        CPPUNIT_ASSERT(
            !OString("foo").startsWithIgnoreAsciiCase("bar", &r));
        CPPUNIT_ASSERT_EQUAL(OString("other"), r);
    }
    {
        OString r("other");
        CPPUNIT_ASSERT(
            !OString("foo").startsWithIgnoreAsciiCase("foobar", &r));
        CPPUNIT_ASSERT_EQUAL(OString("other"), r);
    }

    {
        OString r;
        CPPUNIT_ASSERT(OString().startsWithIgnoreAsciiCase("", &r));
        CPPUNIT_ASSERT(r.isEmpty());
    }
    {
        OString r;
        CPPUNIT_ASSERT(OString("foo").startsWithIgnoreAsciiCase("", &r));
        CPPUNIT_ASSERT_EQUAL(OString("foo"), r);
    }
    {
        OString r;
        CPPUNIT_ASSERT(
            OString("foo").startsWithIgnoreAsciiCase("F", &r));
        CPPUNIT_ASSERT_EQUAL(OString("oo"), r);
    }
    {
        OString r("other");
        CPPUNIT_ASSERT(
            !OString("foo").startsWithIgnoreAsciiCase("bar", &r));
        CPPUNIT_ASSERT_EQUAL(OString("other"), r);
    }
    {
        OString r("other");
        CPPUNIT_ASSERT(
            !OString("foo").startsWithIgnoreAsciiCase("foobar", &r));
        CPPUNIT_ASSERT_EQUAL(OString("other"), r);
    }
}

void Test::testCompareTo()
{
    // test that embedded NUL does not stop the compare
    char str1[2] = { '\0', 'x' };
    char str2[2] = { '\0', 'y' };

    OString s1(str1, 2);
    OString s2(str2, 2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), s1.compareTo(s1));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), s2.compareTo(s2));
    CPPUNIT_ASSERT(s1.compareTo(s2) < 0);
    CPPUNIT_ASSERT(s2.compareTo(s1) > 0);
    CPPUNIT_ASSERT(s1.compareTo(OString(s2 + "y")) < 0);
    CPPUNIT_ASSERT(s2.compareTo(OString(s1 + "x")) > 0);
    CPPUNIT_ASSERT(OString(s1 + "x").compareTo(s2) < 0);
    CPPUNIT_ASSERT(OString(s2 + "y").compareTo(s1) > 0);
}

void Test::testUtf8StringLiterals()
{
    const OString sIn(u8"ßa");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), sIn.getLength());
    CPPUNIT_ASSERT_EQUAL(195, int(static_cast<unsigned char>(sIn[0])));
    CPPUNIT_ASSERT_EQUAL(159, int(static_cast<unsigned char>(sIn[1])));
    CPPUNIT_ASSERT_EQUAL(97, int(static_cast<unsigned char>(sIn[2])));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
