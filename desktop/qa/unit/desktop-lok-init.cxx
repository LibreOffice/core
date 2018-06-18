/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <boost/property_tree/json_parser.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include <comphelper/anytostring.hxx>
#include <comphelper/sequence.hxx>
#include <cstdlib>
#include <string>
#include <stdio.h>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <vcl/scheduler.hxx>

#include <lib/init.hxx>

using namespace css;

/// Unit tests for desktop/source/lib/init.cxx internals.
class LOKInitTest : public ::CppUnit::TestFixture
{
public:
    LOKInitTest()
    {
    }

    void testJsonToPropertyValues();

    CPPUNIT_TEST_SUITE(LOKInitTest);
    CPPUNIT_TEST(testJsonToPropertyValues);
    CPPUNIT_TEST_SUITE_END();
};

namespace {

void assertSequencesEqual(const uno::Sequence<beans::PropertyValue>& expected, const uno::Sequence<beans::PropertyValue>& actual)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The sequences should have the same length", expected.getLength(), actual.getLength());
    for (int i = 0; i < expected.getLength(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(expected[i].Name, actual[i].Name);
        CPPUNIT_ASSERT_EQUAL(comphelper::anyToString(expected[i].Value), comphelper::anyToString(actual[i].Value));
    }
}

}

void LOKInitTest::testJsonToPropertyValues()
{
    const char arguments[] = "{"
        "\"FileName\":{"
            "\"type\":\"string\","
            "\"value\":\"something.odt\""
        "}}";

    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FileName";
    aArgs[0].Value <<= OUString("something.odt");

    assertSequencesEqual(aArgs, comphelper::containerToSequence(desktop::jsonToPropertyValuesVector(arguments)));
}

CPPUNIT_TEST_SUITE_REGISTRATION(LOKInitTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
