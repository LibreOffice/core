/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <tools/urlobj.hxx>

#include <dbmgr.hxx>

namespace
{
/// Test suite for unit tests covering uibase code.
class Test : public CppUnit::TestFixture
{
public:
    void testTdf98168();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testTdf98168);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testTdf98168()
{
    INetURLObject aURL("file:///tmp/test.xlsx");
    // This was sw::DBConnURIType::UNKNOWN, xlsx was not recognized.
    CPPUNIT_ASSERT_EQUAL(sw::DBConnURIType::CALC, sw::GetDBunoType(aURL));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
