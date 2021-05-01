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

#include <SvgNumber.hxx>

namespace
{
class TestNumber : public CppUnit::TestFixture
{
    void test();

public:
    CPPUNIT_TEST_SUITE(TestNumber);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void TestNumber::test()
{
    {
        svgio::svgreader::SvgNumber aNumber;
        CPPUNIT_ASSERT_EQUAL(svgio::svgreader::SvgUnit::px, aNumber.getUnit());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aNumber.getNumber(), 1e-8);
        CPPUNIT_ASSERT_EQUAL(false, aNumber.isSet());
    }
    {
        svgio::svgreader::SvgNumber aNumber(0.01);
        CPPUNIT_ASSERT_EQUAL(svgio::svgreader::SvgUnit::px, aNumber.getUnit());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.01, aNumber.getNumber(), 1e-8);
        CPPUNIT_ASSERT_EQUAL(true, aNumber.isSet());
    }
    {
        svgio::svgreader::SvgNumber aNumber(1.01, svgio::svgreader::SvgUnit::cm);
        CPPUNIT_ASSERT_EQUAL(svgio::svgreader::SvgUnit::cm, aNumber.getUnit());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.01, aNumber.getNumber(), 1e-8);
        CPPUNIT_ASSERT_EQUAL(true, aNumber.isSet());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestNumber);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
