/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "rtl/bootstrap.hxx"
#include "rtl/ustring.hxx"

namespace {

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp() override;

private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testDollar);
    CPPUNIT_TEST(testIndirectDollar);
    CPPUNIT_TEST_SUITE_END();

    void testDollar();

    void testIndirectDollar();
};

void Test::setUp() {
    rtl::Bootstrap::set("TEST", "<expanded TEST>");
    rtl::Bootstrap::set("WITH_DOLLAR", "foo\\$TEST");
    rtl::Bootstrap::set("INDIRECT", "$WITH_DOLLAR");
}

void Test::testDollar() {
    rtl::OUString s("$WITH_DOLLAR");
    rtl::Bootstrap::expandMacros(s);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("foo$TEST"), s);
}

void Test::testIndirectDollar() {
    rtl::OUString s("$INDIRECT");
    rtl::Bootstrap::expandMacros(s);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("foo$TEST"), s);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
