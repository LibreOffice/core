/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <common/JsonUtil.hpp>
#include <wsd/AIUtil.hpp>

#include <test/lokassert.hpp>

#include <Poco/JSON/Object.h>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>

/// Unit tests for the AI helper utilities.
class AIUtilTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(AIUtilTests);
    CPPUNIT_TEST(testValidateTransformStructure);
    CPPUNIT_TEST(testParseLenientArgs);
    CPPUNIT_TEST_SUITE_END();

    void testValidateTransformStructure();
    void testParseLenientArgs();
};

void AIUtilTests::testValidateTransformStructure()
{
    constexpr std::string_view testname = __func__;

    auto parse = [](const std::string& s)
    {
        Poco::JSON::Object::Ptr obj;
        JsonUtil::parseJSON(s, obj);
        return obj;
    };

    // Valid Impress transform.
    LOK_ASSERT(!AIUtil::validateTransformStructure(
                    parse(R"({"Transforms":{"SlideCommands":[{"SetText.0":"Hi"}]}})"))
                    .has_value());

    // Valid Writer content-control transform (no SlideCommands).
    LOK_ASSERT(!AIUtil::validateTransformStructure(
                    parse(R"({"Transforms":{"ContentControls.ByIndex.0":{"content":"x"}}})"))
                    .has_value());

    // UnoCommand-only transform.
    LOK_ASSERT(!AIUtil::validateTransformStructure(
                    parse(R"({"UnoCommand":{"name":".uno:TrackChanges"}})"))
                    .has_value());

    // Neither Transforms nor UnoCommand.
    LOK_ASSERT(AIUtil::validateTransformStructure(parse(R"({"foo":1})")).has_value());

    // SlideCommands is not an array.
    LOK_ASSERT(AIUtil::validateTransformStructure(
                   parse(R"({"Transforms":{"SlideCommands":{"SetText.0":"Hi"}}})"))
                   .has_value());

    // Unknown command name is flagged and named in the message.
    auto err = AIUtil::validateTransformStructure(
        parse(R"({"Transforms":{"SlideCommands":[{"BogusCommand":1}]}})"));
    LOK_ASSERT(err.has_value());
    LOK_ASSERT(err->find("BogusCommand") != std::string::npos);
}

void AIUtilTests::testParseLenientArgs()
{
    constexpr std::string_view testname = __func__;

    // Plain object.
    {
        Poco::JSON::Object::Ptr args;
        LOK_ASSERT(AIUtil::parseLenientArgs(R"({"cell":"A1","formula":"=1"})", args));
        std::string cell;
        JsonUtil::findJSONValue(args, "cell", cell);
        LOK_ASSERT_EQUAL(std::string("A1"), cell);
    }

    // Array of objects gets merged into one.
    {
        Poco::JSON::Object::Ptr args;
        LOK_ASSERT(AIUtil::parseLenientArgs(
            R"json([{"cell":"A1"},{"formula":"=SUM(A1:A9)"}])json", args));
        std::string cell, formula;
        JsonUtil::findJSONValue(args, "cell", cell);
        JsonUtil::findJSONValue(args, "formula", formula);
        LOK_ASSERT_EQUAL(std::string("A1"), cell);
        LOK_ASSERT_EQUAL(std::string("=SUM(A1:A9)"), formula);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(AIUtilTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
