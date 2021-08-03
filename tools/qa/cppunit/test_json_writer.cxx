/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstdlib>

#include <cppunit/extensions/HelperMacros.h>
#include <test/bootstrapfixture.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <tools/json_writer.hxx>

namespace
{
class JsonWriterTest : public test::BootstrapFixture
{
public:
    JsonWriterTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual void setUp() override {}

    void test1();
    void test2();

    CPPUNIT_TEST_SUITE(JsonWriterTest);
    CPPUNIT_TEST(test1);
    CPPUNIT_TEST(test2);
    CPPUNIT_TEST_SUITE_END();
};

struct Free
{
    void operator()(void* p) const { std::free(p); }
};

void JsonWriterTest::test1()
{
    tools::JsonWriter aJson;

    {
        auto testNode = aJson.startNode("node");
        aJson.put("oustring", OUString("val1"));
        aJson.put("ostring", OString("val2"));
        aJson.put("charptr", "val3");
        aJson.put("int", 12);
    }

    std::unique_ptr<char, Free> result(aJson.extractData());

    CPPUNIT_ASSERT_EQUAL(std::string("{ \"node\": { \"oustring\": \"val1\", \"ostring\": \"val2\", "
                                     "\"charptr\": \"val3\", \"int\": \"12\"}}"),
                         std::string(result.get()));
}

void JsonWriterTest::test2()
{
    tools::JsonWriter aJson;

    {
        auto testNode = aJson.startNode("node");
        aJson.put("field1", OUString("val1"));
        aJson.put("field2", OUString("val2"));
        {
            auto testNode2 = aJson.startNode("node");
            aJson.put("field3", OUString("val3"));
            {
                auto testNode3 = aJson.startNode("node");
                aJson.put("field4", OUString("val4"));
                aJson.put("field5", OUString("val5"));
            }
        }
    }

    std::unique_ptr<char, Free> result(aJson.extractData());

    CPPUNIT_ASSERT_EQUAL(std::string("{ \"node\": { \"field1\": \"val1\", \"field2\": \"val2\", "
                                     "\"node\": { \"field3\": \"val3\", \"node\": { \"field4\": "
                                     "\"val4\", \"field5\": \"val5\"}}}}"),
                         std::string(result.get()));
}

CPPUNIT_TEST_SUITE_REGISTRATION(JsonWriterTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
