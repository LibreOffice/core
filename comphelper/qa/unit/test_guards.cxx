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

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, test_comphelperGuards)
{
    bool bFlag = true;
    {
        // Test that comphelper::ScopeGuard executes its parameter on destruction
        comphelper::ScopeGuard aGuard([&bFlag] { bFlag = false; });
        CPPUNIT_ASSERT(bFlag);
    }
    CPPUNIT_ASSERT(!bFlag);

    {
        // Test that comphelper::FlagGuard properly sets and resets the flag
        comphelper::FlagGuard aGuard(bFlag);
        CPPUNIT_ASSERT(bFlag);
    }
    CPPUNIT_ASSERT(!bFlag);

    bFlag = true;
    {
        // Test that comphelper::FlagGuard properly sets and resets the flag
        comphelper::FlagGuard aGuard(bFlag);
        CPPUNIT_ASSERT(bFlag);
    }
    // comphelper::FlagGuard must reset flag to false on destruction unconditionally
    CPPUNIT_ASSERT(!bFlag);

    {
        // Test that comphelper::FlagRestorationGuard properly sets and resets the flag
        comphelper::FlagRestorationGuard aGuard(bFlag, true);
        CPPUNIT_ASSERT(bFlag);
    }
    CPPUNIT_ASSERT(!bFlag);

    bFlag = true;
    {
        // Test that comphelper::FlagRestorationGuard properly sets and resets the flag
        comphelper::FlagRestorationGuard aGuard(bFlag, false);
        CPPUNIT_ASSERT(!bFlag);
    }
    // comphelper::FlagRestorationGuard must reset flag to initial state on destruction
    CPPUNIT_ASSERT(bFlag);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
