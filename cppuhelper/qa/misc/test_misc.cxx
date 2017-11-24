/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <cppuhelper/exc_hlp.hxx>

namespace
{
class Test : public ::CppUnit::TestFixture
{
public:
    void testCatchThrow();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCatchThrow);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testCatchThrow()
{
    css::uno::Any aSavedException;
    try
    {
        throw css::uno::RuntimeException("RuntimeException");
    }
    catch (const css::uno::RuntimeException&)
    {
        aSavedException = cppu::getCaughtException();
    }
    CPPUNIT_ASSERT(aSavedException.hasValue());
    try
    {
        cppu::throwException(aSavedException);
    }
    catch (const css::uno::RuntimeException&)
    {
        // the expected case
    }
    catch (...)
    {
        CPPUNIT_ASSERT(false);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
