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
    CPPUNIT_TEST(testNormalizeAIBaseUrl);
    CPPUNIT_TEST(testHostOfBaseUrl);
    CPPUNIT_TEST(testSlideCommandTable);
    CPPUNIT_TEST(testSlideCommandDocs);
    CPPUNIT_TEST_SUITE_END();

    void testValidateTransformStructure();
    void testParseLenientArgs();
    void testNormalizeAIBaseUrl();
    void testHostOfBaseUrl();
    void testSlideCommandTable();
    void testSlideCommandDocs();
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

void AIUtilTests::testNormalizeAIBaseUrl()
{
    constexpr std::string_view testname = __func__;

    // A bare origin is left as is.
    LOK_ASSERT_EQUAL(std::string("https://api.openai.com"),
                     AIUtil::normalizeAIBaseUrl("https://api.openai.com"));

    // One or several trailing slashes are removed.
    LOK_ASSERT_EQUAL(std::string("https://api.openai.com"),
                     AIUtil::normalizeAIBaseUrl("https://api.openai.com/"));
    LOK_ASSERT_EQUAL(std::string("http://localhost:11434"),
                     AIUtil::normalizeAIBaseUrl("http://localhost:11434///"));

    // A trailing "/v1", with or without a trailing slash, is removed.
    LOK_ASSERT_EQUAL(std::string("http://localhost:11434"),
                     AIUtil::normalizeAIBaseUrl("http://localhost:11434/v1"));
    LOK_ASSERT_EQUAL(std::string("http://localhost:11434"),
                     AIUtil::normalizeAIBaseUrl("http://localhost:11434/v1/"));

    // The match is case-insensitive.
    LOK_ASSERT_EQUAL(std::string("http://localhost:11434"),
                     AIUtil::normalizeAIBaseUrl("http://localhost:11434/V1"));

    // A path that ends in a different segment keeps that segment: "/openai" is
    // a real base (Groq), and "/v10" is not the "/v1" we strip.
    LOK_ASSERT_EQUAL(std::string("https://api.groq.com/openai"),
                     AIUtil::normalizeAIBaseUrl("https://api.groq.com/openai/"));
    LOK_ASSERT_EQUAL(std::string("https://example.com/v10"),
                     AIUtil::normalizeAIBaseUrl("https://example.com/v10"));

    // Only the single trailing "/v1" goes; an earlier "/v1" in the path stays.
    LOK_ASSERT_EQUAL(std::string("https://example.com/v1/api"),
                     AIUtil::normalizeAIBaseUrl("https://example.com/v1/api"));

    // The empty string maps to the empty string.
    LOK_ASSERT_EQUAL(std::string(), AIUtil::normalizeAIBaseUrl(""));
}

void AIUtilTests::testHostOfBaseUrl()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string("ai.example.com"),
                     AIUtil::hostOfBaseUrl("https://ai.example.com"));

    // A port is not part of the host.
    LOK_ASSERT_EQUAL(std::string("localhost"), AIUtil::hostOfBaseUrl("http://localhost:11434"));

    // A path on the base URL is left out too.
    LOK_ASSERT_EQUAL(std::string("ai.example.com"),
                     AIUtil::hostOfBaseUrl("https://ai.example.com/v1"));

    // An IPv6 literal comes back without its brackets.
    LOK_ASSERT_EQUAL(std::string("::1"), AIUtil::hostOfBaseUrl("http://[::1]:11434"));

    // The host comes back lowercase, so two spellings of one name give the same host.
    LOK_ASSERT_EQUAL(std::string("ai.example.com"),
                     AIUtil::hostOfBaseUrl("https://AI.Example.com"));

    // Nothing usable gives the empty string.
    LOK_ASSERT_EQUAL(std::string(), AIUtil::hostOfBaseUrl(""));
    LOK_ASSERT_EQUAL(std::string(), AIUtil::hostOfBaseUrl("not a url"));
}

void AIUtilTests::testSlideCommandTable()
{
    constexpr std::string_view testname = __func__;

    // A server-only command is recognized with and without an object-index
    // suffix; commands the model may emit and unknown names are not flagged.
    LOK_ASSERT(AIUtil::isServerOnlySlideCommand("ApplyTemplate"));
    LOK_ASSERT(AIUtil::isServerOnlySlideCommand("ApplyTemplate.2"));
    LOK_ASSERT(!AIUtil::isServerOnlySlideCommand("SetText.0"));
    LOK_ASSERT(!AIUtil::isServerOnlySlideCommand("DeleteSlide"));
    LOK_ASSERT(!AIUtil::isServerOnlySlideCommand("BogusCommand"));

    // The model labels a slide's part itself, so SetSlidePart is not
    // server-only - it is kept when the model emits it.
    LOK_ASSERT(!AIUtil::isServerOnlySlideCommand("SetSlidePart"));

    // A server-only command still validates: the server splices it into
    // transforms, so the vocabulary must keep accepting it.
    auto parse = [](const std::string& s)
    {
        Poco::JSON::Object::Ptr obj;
        JsonUtil::parseJSON(s, obj);
        return obj;
    };
    LOK_ASSERT(!AIUtil::validateTransformStructure(
                    parse(R"({"Transforms":{"SlideCommands":[{"ApplyTemplate":"Mint"}]}})"))
                    .has_value());

    // A model-emitted SetSlidePart passes structural validation.
    LOK_ASSERT(
        !AIUtil::validateTransformStructure(
             parse(R"({"Transforms":{"SlideCommands":[{"SetSlidePart":"opening"}]}})"))
             .has_value());

    // SetNotes and SetSlideIntent are part of the vocabulary and not server-only,
    // so a compiled or model-emitted transform carrying them validates.
    LOK_ASSERT(!AIUtil::isServerOnlySlideCommand("SetNotes"));
    LOK_ASSERT(!AIUtil::isServerOnlySlideCommand("SetSlideIntent"));
    LOK_ASSERT(!AIUtil::validateTransformStructure(
                    parse(R"({"Transforms":{"SlideCommands":[{"SetNotes":"Say this aloud"},)"
                          R"({"SetSlideIntent":"quote"}]}})"))
                    .has_value());

    // The layout set knows its members.
    LOK_ASSERT(AIUtil::isKnownSlideLayout("AUTOLAYOUT_TITLE_CONTENT"));
    LOK_ASSERT(AIUtil::isKnownSlideLayout("AUTOLAYOUT_ONLY_TEXT"));
    LOK_ASSERT(!AIUtil::isKnownSlideLayout("AUTOLAYOUT_BOGUS"));
    LOK_ASSERT(!AIUtil::isKnownSlideLayout(""));
}

void AIUtilTests::testSlideCommandDocs()
{
    constexpr std::string_view testname = __func__;

    const std::string& docs = AIUtil::getSlideCommandDocs();

    // Every command the model may emit and that carries documentation
    // appears in the generated text; a server-only command does not.
    for (const auto& cmd : AIUtil::getSlideCommands())
    {
        const std::string token = "{\"" + std::string(cmd.name);
        if (cmd.allowedFromModel && !cmd.docLines.empty())
            LOK_ASSERT_MESSAGE("missing from docs: " + std::string(cmd.name),
                               docs.find(token) != std::string::npos);
        if (!cmd.allowedFromModel)
            LOK_ASSERT_MESSAGE("server-only command leaked into docs: " + std::string(cmd.name),
                               docs.find(std::string(cmd.name)) == std::string::npos);
    }

    // Every layout offered to the model is documented with its name and id.
    for (const auto& layout : AIUtil::getSlideLayouts())
    {
        const std::string line =
            "- " + std::string(layout.name) + " (id=" + std::to_string(layout.id) + ")";
        LOK_ASSERT_MESSAGE("missing layout: " + std::string(layout.name),
                           docs.find(line) != std::string::npos);
    }

    // The section grouping survives: each documented section title appears
    // exactly once, so two commands of one section share one heading.
    for (const auto& cmd : AIUtil::getSlideCommands())
    {
        if (cmd.docSection.empty())
            continue;
        const std::string title(cmd.docSection);
        const std::size_t first = docs.find(title);
        LOK_ASSERT(first != std::string::npos);
        LOK_ASSERT(docs.find(title, first + 1) == std::string::npos);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(AIUtilTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
