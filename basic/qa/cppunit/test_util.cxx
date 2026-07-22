/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <basic/sbutil.hxx>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/TestFixture.h>

namespace
{
class TestUtil : public CppUnit::TestFixture
{
private:
    void testSearchEOL();

    CPPUNIT_TEST_SUITE(TestUtil);
    CPPUNIT_TEST(testSearchEOL);
    CPPUNIT_TEST_SUITE_END();
};

void TestUtil::testSearchEOL()
{
    // Find the ending when \n is the first one
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), sb::searchEOL(u"one\ntwo\r\three\r\n"));
    // Find the ending when \r\n is the first one
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), sb::searchEOL(u"one\r\ntwo\r\three\n"));
    // Find the ending when \r is the first one
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), sb::searchEOL(u"one\rtwo\n\three\r\n"));

    // The three types of ending at the end of the string
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), sb::searchEOL(u"one\n"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), sb::searchEOL(u"one\r"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), sb::searchEOL(u"one\r\n"));

    // No ending
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), sb::searchEOL(u""));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), sb::searchEOL(u"unending string"));

    // Skip an ending
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), sb::searchEOL(u"one\ntwo\n", 4));

    // Start at an ending
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), sb::searchEOL(u"one\ntwo\n", 3));
}
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestUtil);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
