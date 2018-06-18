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
#include <tools/colordata.hxx>

#include <lib/init.hxx>

#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>

using namespace css;

/// Unit tests for desktop/source/lib/init.cxx internals.
class LOKInitTest : public ::CppUnit::TestFixture
{
public:
    LOKInitTest()
    {
    }

    void testJsonToPropertyValues();
    void testJsonToPropertyValuesBorder();

    CPPUNIT_TEST_SUITE(LOKInitTest);
    CPPUNIT_TEST(testJsonToPropertyValues);
    CPPUNIT_TEST(testJsonToPropertyValuesBorder);
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

void LOKInitTest::testJsonToPropertyValuesBorder()
{
    const char arguments[] = "{"
        "\"OuterBorder\": {"
            "\"type\" : \"[]any\","
            "\"value\" : ["
                "{ \"type\" : \"com.sun.star.table.BorderLine2\", \"value\" : { \"Color\" : { \"type\" : \"com.sun.star.util.Color\", \"value\" : 0 }, \"InnerLineWidth\" : { \"type\" : \"short\", \"value\" : 0 }, \"OuterLineWidth\" : { \"type\" : \"short\", \"value\" : 1 }, \"LineDistance\" : { \"type\" : \"short\", \"value\" : 0 },  \"LineStyle\" : { \"type\" : \"short\", \"value\" : 0 }, \"LineWidth\" : { \"type\" : \"unsigned long\", \"value\" : 1 } } },"
                "{ \"type\" : \"com.sun.star.table.BorderLine2\", \"value\" : { \"Color\" : { \"type\" : \"com.sun.star.util.Color\", \"value\" : 0 }, \"InnerLineWidth\" : { \"type\" : \"short\", \"value\" : 0 }, \"OuterLineWidth\" : { \"type\" : \"short\", \"value\" : 1 }, \"LineDistance\" : { \"type\" : \"short\", \"value\" : 0 },  \"LineStyle\" : { \"type\" : \"short\", \"value\" : 0 }, \"LineWidth\" : { \"type\" : \"unsigned long\", \"value\" : 1 } } },"
                "{ \"type\" : \"com.sun.star.table.BorderLine2\", \"value\" : { \"Color\" : { \"type\" : \"com.sun.star.util.Color\", \"value\" : 0 }, \"InnerLineWidth\" : { \"type\" : \"short\", \"value\" : 0 }, \"OuterLineWidth\" : { \"type\" : \"short\", \"value\" : 1 }, \"LineDistance\" : { \"type\" : \"short\", \"value\" : 0 },  \"LineStyle\" : { \"type\" : \"short\", \"value\" : 0 }, \"LineWidth\" : { \"type\" : \"unsigned long\", \"value\" : 1 } } },"
                "{ \"type\" : \"com.sun.star.table.BorderLine2\", \"value\" : { \"Color\" : { \"type\" : \"com.sun.star.util.Color\", \"value\" : 0 }, \"InnerLineWidth\" : { \"type\" : \"short\", \"value\" : 0 }, \"OuterLineWidth\" : { \"type\" : \"short\", \"value\" : 1 }, \"LineDistance\" : { \"type\" : \"short\", \"value\" : 0 },  \"LineStyle\" : { \"type\" : \"short\", \"value\" : 0 }, \"LineWidth\" : { \"type\" : \"unsigned long\", \"value\" : 1 } } },"
                "{ \"type\" : \"long\", \"value\" : 0 },"
                "{ \"type\" : \"long\", \"value\" : 0 },"
                "{ \"type\" : \"long\", \"value\" : 0 },"
                "{ \"type\" : \"long\", \"value\" : 0 },"
                "{ \"type\" : \"long\", \"value\" : 0 }"
            "]"
        "},"
        "\"InnerBorder\":{"
            "\"type\" : \"[]any\","
            "\"value\" : ["
                "{ \"type\" : \"com.sun.star.table.BorderLine2\", \"value\" : { \"Color\" : { \"type\" : \"com.sun.star.util.Color\", \"value\" : 0 }, \"InnerLineWidth\" : { \"type\" : \"short\", \"value\" : 0 }, \"OuterLineWidth\" : { \"type\" : \"short\", \"value\" : 1 }, \"LineDistance\" : { \"type\" : \"short\", \"value\" : 0 },  \"LineStyle\" : { \"type\" : \"short\", \"value\" : 0 }, \"LineWidth\" : { \"type\" : \"unsigned long\", \"value\" : 1 } } },"
                "{ \"type\" : \"com.sun.star.table.BorderLine2\", \"value\" : { \"Color\" : { \"type\" : \"com.sun.star.util.Color\", \"value\" : 0 }, \"InnerLineWidth\" : { \"type\" : \"short\", \"value\" : 0 }, \"OuterLineWidth\" : { \"type\" : \"short\", \"value\" : 1 }, \"LineDistance\" : { \"type\" : \"short\", \"value\" : 0 },  \"LineStyle\" : { \"type\" : \"short\", \"value\" : 0 }, \"LineWidth\" : { \"type\" : \"unsigned long\", \"value\" : 1 } } },"
                "{ \"type\" : \"short\", \"value\" : 0 },"
                "{ \"type\" : \"short\", \"value\" : 127 },"
                "{ \"type\" : \"long\", \"value\" : 0 }"
            "]"
        "}}";

    // see SvxBoxItem::QueryValue for details
    uno::Sequence<uno::Any> aOuterSeq(9);
    table::BorderLine2 aLine(sal_Int32(COL_BLACK), 0, 1, 0, table::BorderLineStyle::SOLID, 1);
    aOuterSeq[0] = uno::makeAny(aLine); // left
    aOuterSeq[1] = uno::makeAny(aLine); // right
    aOuterSeq[2] = uno::makeAny(aLine); // bottom
    aOuterSeq[3] = uno::makeAny(aLine); // top
    aOuterSeq[4] = uno::makeAny((sal_Int32)(0));
    aOuterSeq[5] = uno::makeAny((sal_Int32)(0));
    aOuterSeq[6] = uno::makeAny((sal_Int32)(0));
    aOuterSeq[7] = uno::makeAny((sal_Int32)(0));
    aOuterSeq[8] = uno::makeAny((sal_Int32)(0));

    // see SvxBoxInfoItem::QueryValue() for details
    uno::Sequence<uno::Any> aInnerSeq(5);
    aInnerSeq[0] = uno::makeAny(aLine); // horizontal
    aInnerSeq[1] = uno::makeAny(aLine); // vertical
    aInnerSeq[2] = uno::makeAny((sal_Int16)(0));
    aInnerSeq[3] = uno::makeAny((sal_Int16)(0x7F));
    aInnerSeq[4] = uno::makeAny((sal_Int32)(0));

    uno::Sequence<beans::PropertyValue> aArgs(2);
    aArgs[0].Name = "OuterBorder";
    aArgs[0].Value <<= aOuterSeq;
    aArgs[1].Name = "InnerBorder";
    aArgs[1].Value <<= aInnerSeq;

    assertSequencesEqual(aArgs, comphelper::containerToSequence(desktop::jsonToPropertyValuesVector(arguments)));
}

CPPUNIT_TEST_SUITE_REGISTRATION(LOKInitTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
