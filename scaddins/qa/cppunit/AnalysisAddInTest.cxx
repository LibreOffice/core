/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <analysis/analysis.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace
{
class AnalysisAddInTest : public CppUnit::TestFixture
{
private:
    void testGetDec2Hex();

    CPPUNIT_TEST_SUITE(AnalysisAddInTest);
    CPPUNIT_TEST(testGetDec2Hex);
    CPPUNIT_TEST_SUITE_END();
};

void AnalysisAddInTest::testGetDec2Hex()
{
    // CPPUNIT_ASSERT_EQUAL("0x000004D2", someAnalysisAddIn.getDec2Hex(???, 1234, val));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(AnalysisAddInTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
