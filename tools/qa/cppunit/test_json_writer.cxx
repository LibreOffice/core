/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <o3tl/deleter.hxx>
#include <rtl/ustring.hxx>
#include <tools/json_writer.hxx>

namespace
{
class JsonWriterTest : public CppUnit::TestFixture
{
public:
    void test1();
    void test2();
    void testArray();

    CPPUNIT_TEST_SUITE(JsonWriterTest);
    CPPUNIT_TEST(test1);
    CPPUNIT_TEST(test2);
    CPPUNIT_TEST(testArray);
    CPPUNIT_TEST_SUITE_END();
};

void JsonWriterTest::test1()
{
    tools::JsonWriter aJson;

    {
        auto testNode = aJson.startNode("node");
        aJson.put("oustring", u"val1"_ustr);
        aJson.put("charptr", "val3");
        aJson.put("int", static_cast<sal_Int32>(12));
    }

    OString result(aJson.finishAndGetAsOString());

    CPPUNIT_ASSERT_EQUAL("{ \"node\": { \"oustring\": \"val1\", "
                         "\"charptr\": \"val3\", \"int\": 12}}"_ostr,
                         result);
}

void JsonWriterTest::test2()
{
    tools::JsonWriter aJson;

    {
        auto testNode = aJson.startNode("node");
        aJson.put("field1", u"val1"_ustr);
        aJson.put("field2", u"val2"_ustr);
        {
            auto testNode2 = aJson.startNode("node");
            aJson.put("field3", u"val3"_ustr);
            {
                auto testNode3 = aJson.startNode("node");
                aJson.put("field4", u"val4"_ustr);
                aJson.put("field5", u"val5"_ustr);
            }
        }
    }

    OString result(aJson.finishAndGetAsOString());

    CPPUNIT_ASSERT_EQUAL("{ \"node\": { \"field1\": \"val1\", \"field2\": \"val2\", "
                         "\"node\": { \"field3\": \"val3\", \"node\": { \"field4\": "
                         "\"val4\", \"field5\": \"val5\"}}}}"_ostr,
                         result);
}

void JsonWriterTest::testArray()
{
    tools::JsonWriter aJson;
    {
        auto aArray = aJson.startArray("items");
        aJson.putSimpleValue(u"foo");
        aJson.putSimpleValue(u"bar");
    }

    OString aResult(aJson.finishAndGetAsOString());

    CPPUNIT_ASSERT_EQUAL("{ \"items\": [ \"foo\", \"bar\"]}"_ostr, aResult);
}

CPPUNIT_TEST_SUITE_REGISTRATION(JsonWriterTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
