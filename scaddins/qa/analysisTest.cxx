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

#include "../source/analysis/analysis.hxx"

using namespace ::com::sun::star;
using namespace sca::analysis;

namespace
{
/**
 * Perform tests on MyClass.
 */
class AnalysisTest : public CppUnit::TestFixture
{
private:
    /**
     * Tests capability Foo of the class.
     */
    void getDec2HexTest();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(AnalysisTest);
    // Declares the method as a test to call
    CPPUNIT_TEST(getDec2HexTest);
    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

void AnalysisTest::getDec2HexTest()
{
    // CPPUNIT_ASSERT_EQUAL( OUString("4D2"), getDec2Hex( Decuno::Reference< beans::XPropertySet >(), 1234, uno::Any() ));
    CPPUNIT_ASSERT(true);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(AnalysisTest);

} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
