/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/uno/Any.h>
#include <rtl/ustring.hxx>

#include <iterator>
#include <vector>

#include "CommonFunctors.hxx"


class CommonFunctorsTest : public CppUnit::TestFixture
{
public:
     CPPUNIT_TEST_SUITE(CommonFunctorsTest);
     CPPUNIT_TEST(testAnyToString);
     CPPUNIT_TEST(testDoubleToString);
     CPPUNIT_TEST_SUITE_END();

     void testAnyToString();
     void testDoubleToString();

private:
};

void CommonFunctorsTest::testAnyToString()
{
    std::vector<css::uno::Any> aInput;
    aInput.push_back(css::uno::Any(2.0));
    aInput.push_back(css::uno::Any(10.0));
    aInput.push_back(css::uno::Any(12.0));
    aInput.push_back(css::uno::Any(15.0));
    aInput.push_back(css::uno::Any(25.234));
    aInput.push_back(css::uno::Any(123.456));
    aInput.push_back(css::uno::Any(0.123450));

    std::vector<OUString> aOutput;
    std::transform(aInput.begin(), aInput.end(),
            std::back_inserter(aOutput), chart::CommonFunctors::AnyToString());

    CPPUNIT_ASSERT_EQUAL(OUString("2"), aOutput[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("10"), aOutput[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("12"), aOutput[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("15"), aOutput[3]);
    CPPUNIT_ASSERT_EQUAL(OUString("25.234"), aOutput[4]);
    CPPUNIT_ASSERT_EQUAL(OUString("123.456"), aOutput[5]);
    CPPUNIT_ASSERT_EQUAL(OUString("0.12345"), aOutput[6]);
}

void CommonFunctorsTest::testDoubleToString()
{
    std::vector<double> aInput;
    aInput.push_back(2.0);
    aInput.push_back(10.0);
    aInput.push_back(12.0);
    aInput.push_back(15.0);
    aInput.push_back(25.234);
    aInput.push_back(123.456);
    aInput.push_back(0.123450);

    std::vector<OUString> aOutput;
    std::transform(aInput.begin(), aInput.end(),
            std::back_inserter(aOutput), chart::CommonFunctors::DoubleToOUString());

    CPPUNIT_ASSERT_EQUAL(OUString("2"), aOutput[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("10"), aOutput[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("12"), aOutput[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("15"), aOutput[3]);
    CPPUNIT_ASSERT_EQUAL(OUString("25.234"), aOutput[4]);
    CPPUNIT_ASSERT_EQUAL(OUString("123.456"), aOutput[5]);
    CPPUNIT_ASSERT_EQUAL(OUString("0.12345"), aOutput[6]);
}

CPPUNIT_TEST_SUITE_REGISTRATION(CommonFunctorsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
