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
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/uno/Any.hxx>
#include <sal/types.h>

namespace {

class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testHas);
    CPPUNIT_TEST(testExtract);
    CPPUNIT_TEST(testInsert);
    CPPUNIT_TEST_SUITE_END();

    void testGet() {
        css::uno::Any a(false);
        CPPUNIT_ASSERT_EQUAL(a, a.get<css::uno::Any>());
        CPPUNIT_ASSERT_EQUAL(false, a.get<bool>());
    }

    void testHas() {
        css::uno::Any a(false);
        CPPUNIT_ASSERT_EQUAL(true, a.has<css::uno::Any>());
        CPPUNIT_ASSERT_EQUAL(true, a.has<bool>());
    }

    void testExtract() {
        css::uno::Any a1(false);
        css::uno::Any a2;
        CPPUNIT_ASSERT(a1 >>= a2);
        CPPUNIT_ASSERT_EQUAL(a1, a2);
    }

    void testInsert() {
        css::uno::Any a;
        a <<= css::uno::Any(false);
        CPPUNIT_ASSERT_EQUAL(css::uno::Any(false), a);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
