/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/flagguard.hxx>
#include <unotest/bootstrapfixturebase.hxx>

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testScopeGuard)
{
    // Test that comphelper::ScopeGuard executes its parameter on destruction

    // initial value "true", out-of-scope ScopeGuard function executes and changes the value to "false"
    bool bFlag = true;
    {
        comphelper::ScopeGuard aGuard([&bFlag] { bFlag = false; });
        CPPUNIT_ASSERT(bFlag);
    }
    CPPUNIT_ASSERT(!bFlag);
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testFlagGuard)
{
    // Test that comphelper::FlagGuard properly sets and resets the flag

    // initial value "false", change to "true", out-of-scope change to "false"
    bool bFlag = false;
    {
        comphelper::FlagGuard aGuard(bFlag);
        CPPUNIT_ASSERT(bFlag);
    }
    // comphelper::FlagGuard must reset flag to false on destruction unconditionally
    CPPUNIT_ASSERT(!bFlag);

    // initial value "true", retain the value at "true", out-of-scope change to "false"
    bFlag = true;
    {
        comphelper::FlagGuard aGuard(bFlag);
        CPPUNIT_ASSERT(bFlag);
    }
    // comphelper::FlagGuard must reset flag to false on destruction unconditionally
    CPPUNIT_ASSERT(!bFlag);
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testFlagRestorationGuard)
{
    // Test that comphelper::FlagRestorationGuard properly sets and resets the flag

    // initial value "true", change to "false", out-of-scope change to "true"

    bool bFlag = true;
    {
        comphelper::FlagRestorationGuard aGuard(bFlag, false);
        CPPUNIT_ASSERT(!bFlag);
    }
    // comphelper::FlagRestorationGuard must reset flag to initial state on destruction
    CPPUNIT_ASSERT(bFlag);
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testValueRestorationGuard)
{
    // Test that comphelper::ValueRestorationGuard properly sets and resets the (int) value

    int value = 199;

    // set value and restore after scope ends
    {
        CPPUNIT_ASSERT_EQUAL(199, value);
        comphelper::ValueRestorationGuard aGuard(value, 100);
        CPPUNIT_ASSERT_EQUAL(100, value);
    }
    CPPUNIT_ASSERT_EQUAL(199, value);

    // set value, manually setto another value and restore after scope ends
    {
        CPPUNIT_ASSERT_EQUAL(199, value);
        comphelper::ValueRestorationGuard aGuard(value, 100);
        CPPUNIT_ASSERT_EQUAL(100, value);
        value = 200;
    }
    CPPUNIT_ASSERT_EQUAL(199, value);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
