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
    void testSetting();
    void testSolve();

public:
    CPPUNIT_TEST_SUITE(TestNumber);
    CPPUNIT_TEST(testSetting);
    CPPUNIT_TEST(testSolve);
    CPPUNIT_TEST_SUITE_END();
};

class TestInfoProvider : public svgio::svgreader::InfoProvider
{
public:
    basegfx::B2DRange getCurrentViewPort() const override
    {
        return basegfx::B2DRange(0.0, 0.0, 0.0, 0.0);
    }

    double getCurrentFontSize() const override { return 12.0; }

    double getCurrentXHeight() const override { return 5.0; }
};

void TestNumber::testSetting()
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

void TestNumber::testSolve()
{
    {
        svgio::svgreader::SvgNumber aNumber(1.01);
        TestInfoProvider aInfoProvider;
        double aSolvedNumber = aNumber.solve(aInfoProvider);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.01, aSolvedNumber, 1e-8);
    }
    {
        svgio::svgreader::SvgNumber aNumber(1.0, svgio::svgreader::SvgUnit::pt);
        TestInfoProvider aInfoProvider;
        double aSolvedNumber = aNumber.solve(aInfoProvider);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.333, aSolvedNumber, 1e-3);
    }
    {
        svgio::svgreader::SvgNumber aNumber(2.54, svgio::svgreader::SvgUnit::cm);
        TestInfoProvider aInfoProvider;
        double aSolvedNumber = aNumber.solve(aInfoProvider);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(96.0, aSolvedNumber, 1e-3);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestNumber);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
