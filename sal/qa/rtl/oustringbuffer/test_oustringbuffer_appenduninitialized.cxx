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
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>

namespace {

class Test: public CppUnit::TestFixture {
private:
    void testEmpty();

    void testNonEmpty();

    void testZero();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testNonEmpty);
    CPPUNIT_TEST(testZero);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testEmpty() {
    OUStringBuffer s;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s.getLength());
    sal_Unicode * p = s.appendUninitialized(5);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<void const *>(s.getStr()), static_cast<void const *>(p));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), s.getLength());
}

void Test::testNonEmpty() {
    OUStringBuffer s("ab");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), s.getLength());
    sal_Unicode * p = s.appendUninitialized(5);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<void const *>(s.getStr() + 2),
        static_cast<void const *>(p));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), s.getLength());
}

void Test::testZero() {
    OUStringBuffer s;
    sal_Unicode * p = s.appendUninitialized(0);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<void const *>(s.getStr()), static_cast<void const *>(p));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), s.getLength());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
