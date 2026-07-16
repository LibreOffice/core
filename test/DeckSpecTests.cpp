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
#include <wsd/DeckSpec.hpp>

#include <test/lokassert.hpp>

#include <Poco/JSON/Object.h>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>

/// Unit tests for the declarative deck spec validator and compiler.
class DeckSpecTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(DeckSpecTests);
    CPPUNIT_TEST(testValidateAccepts);
    CPPUNIT_TEST(testValidateRejects);
    CPPUNIT_TEST(testCompileNoTemplate);
    CPPUNIT_TEST(testCompileWithTemplate);
    CPPUNIT_TEST(testCompileImageBrief);
    CPPUNIT_TEST(testCompiledOutputValidates);
    CPPUNIT_TEST_SUITE_END();

    void testValidateAccepts();
    void testValidateRejects();
    void testCompileNoTemplate();
    void testCompileWithTemplate();
    void testCompileImageBrief();
    void testCompiledOutputValidates();
};

namespace
{
Poco::JSON::Object::Ptr parse(const std::string& s)
{
    Poco::JSON::Object::Ptr obj;
    JsonUtil::parseJSON(s, obj);
    return obj;
}
}

void DeckSpecTests::testValidateAccepts()
{
    constexpr std::string_view testname = __func__;

    // A title slide with a subtitle text block, then a bullets slide.
    LOK_ASSERT(!DeckSpec::validateDeckSpec(parse(
                    R"({"slides":[
                        {"part":"opening","intent":"title","title":"Hello",
                         "blocks":[{"kind":"text","text":"A subtitle"}]},
                        {"part":"body","intent":"bullets","title":"Points",
                         "blocks":[{"kind":"bullets","items":["One","Two"]}]}]})"))
                    .has_value());

    // A comparison needs exactly two bullets blocks.
    LOK_ASSERT(!DeckSpec::validateDeckSpec(parse(
                    R"({"slides":[{"part":"body","intent":"comparison","title":"Us vs Them",
                        "blocks":[{"kind":"bullets","items":["a"]},
                                  {"kind":"bullets","items":["b"]}]}]})"))
                    .has_value());

    // A quote needs exactly one text block.
    LOK_ASSERT(!DeckSpec::validateDeckSpec(parse(
                    R"({"slides":[{"part":"divider","intent":"quote","title":"Wisdom",
                        "blocks":[{"kind":"text","text":"Stay curious."}]}]})"))
                    .has_value());

    // An image slide needs an image brief and no content blocks.
    LOK_ASSERT(!DeckSpec::validateDeckSpec(parse(
                    R"({"slides":[{"part":"body","intent":"image","title":"Chart",
                        "image":{"brief":"a bar chart","alt":"chart"}}]})"))
                    .has_value());

    // A section divider and a closing slide need only a title.
    LOK_ASSERT(!DeckSpec::validateDeckSpec(parse(
                    R"({"slides":[{"part":"divider","intent":"section","title":"Part Two"},
                        {"part":"closing","intent":"closing","title":"Thanks"}]})"))
                    .has_value());
}

void DeckSpecTests::testValidateRejects()
{
    constexpr std::string_view testname = __func__;

    auto errorFor = [](const std::string& s)
    { return DeckSpec::validateDeckSpec(parse(s)); };

    // An empty deck.
    LOK_ASSERT(errorFor(R"({"slides":[]})").has_value());

    // A part outside the enum names the field.
    auto partErr = errorFor(
        R"({"slides":[{"part":"intro","intent":"title","title":"X"}]})");
    LOK_ASSERT(partErr.has_value());
    LOK_ASSERT(partErr->find("part") != std::string::npos);

    // An intent outside the enum names the field.
    auto intentErr = errorFor(
        R"({"slides":[{"part":"body","intent":"chart","title":"X"}]})");
    LOK_ASSERT(intentErr.has_value());
    LOK_ASSERT(intentErr->find("intent") != std::string::npos);

    // A missing title names the field.
    auto titleErr = errorFor(
        R"({"slides":[{"part":"body","intent":"section","title":""}]})");
    LOK_ASSERT(titleErr.has_value());
    LOK_ASSERT(titleErr->find("title") != std::string::npos);

    // A comparison with one bullets block reports the count rule.
    auto cmpErr = errorFor(
        R"({"slides":[{"part":"body","intent":"comparison","title":"X",
            "blocks":[{"kind":"bullets","items":["a"]}]}]})");
    LOK_ASSERT(cmpErr.has_value());
    LOK_ASSERT(cmpErr->find("2 bullets") != std::string::npos);

    // A quote with no text block reports the count rule.
    LOK_ASSERT(errorFor(
                   R"({"slides":[{"part":"body","intent":"quote","title":"X"}]})")
                   .has_value());

    // An image slide with no image reports the image requirement.
    auto imgErr = errorFor(
        R"({"slides":[{"part":"body","intent":"image","title":"X"}]})");
    LOK_ASSERT(imgErr.has_value());
    LOK_ASSERT(imgErr->find("image") != std::string::npos);

    // A bullets block over the item budget.
    auto itemsErr = errorFor(
        R"({"slides":[{"part":"body","intent":"bullets","title":"X",
            "blocks":[{"kind":"bullets","items":["1","2","3","4","5","6","7"]}]}]})");
    LOK_ASSERT(itemsErr.has_value());
    LOK_ASSERT(itemsErr->find("6") != std::string::npos);

    // A bullet item over the length budget.
    const std::string longItem(DeckSpec::MaxItemLength + 1, 'x');
    auto lenErr = errorFor(
        R"({"slides":[{"part":"body","intent":"bullets","title":"X",
            "blocks":[{"kind":"bullets","items":[")" + longItem + R"("]}]}]})");
    LOK_ASSERT(lenErr.has_value());

    // More slides than the deck budget allows.
    std::string many = R"({"slides":[)";
    for (int i = 0; i < DeckSpec::MaxSlides + 1; ++i)
    {
        if (i)
            many += ',';
        many += R"({"part":"body","intent":"section","title":"S"})";
    }
    many += "]}";
    LOK_ASSERT(errorFor(many).has_value());
}

void DeckSpecTests::testCompileNoTemplate()
{
    constexpr std::string_view testname = __func__;

    const std::string deck =
        R"({"slides":[
            {"part":"opening","intent":"title","title":"Hello",
             "blocks":[{"kind":"text","text":"A subtitle"}]},
            {"part":"body","intent":"bullets","title":"Points",
             "blocks":[{"kind":"bullets","items":["One","Two"]}]}]})";

    // Without a template the compiler adds the house-style bold-title and
    // bulleted-content commands itself.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE"},)"
        R"({"SetText.0":"Hello"},)"
        R"({"SetText.1":"A subtitle"},)"
        R"({"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},)"
        R"({"SetSlidePart":"opening"},)"
        R"({"InsertMasterSlide":0},)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Points"},)"
        R"({"SetText.1":"One\nTwo"},)"
        R"({"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},)"
        R"({"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},)"
        R"({"SetSlidePart":"body"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileDeckSpec(parse(deck), false));
}

void DeckSpecTests::testCompileWithTemplate()
{
    constexpr std::string_view testname = __func__;

    const std::string deck =
        R"({"slides":[
            {"part":"opening","intent":"title","title":"Hello",
             "blocks":[{"kind":"text","text":"A subtitle"}]},
            {"part":"body","intent":"bullets","title":"Points",
             "blocks":[{"kind":"bullets","items":["One","Two"]}]}]})";

    // With a template the compiler emits no formatting; the master slides own
    // the look.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE"},)"
        R"({"SetText.0":"Hello"},)"
        R"({"SetText.1":"A subtitle"},)"
        R"({"SetSlidePart":"opening"},)"
        R"({"InsertMasterSlide":0},)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Points"},)"
        R"({"SetText.1":"One\nTwo"},)"
        R"({"SetSlidePart":"body"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileDeckSpec(parse(deck), true));
}

void DeckSpecTests::testCompileImageBrief()
{
    constexpr std::string_view testname = __func__;

    const std::string deck =
        R"({"slides":[
            {"part":"opening","intent":"title","title":"Deck"},
            {"part":"body","intent":"image","title":"Chart",
             "image":{"brief":"a bar chart","alt":"chart"}}]})";

    // An image slide compiles to a GenerateImage on the content placeholder.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE"},)"
        R"({"SetText.0":"Deck"},)"
        R"({"SetSlidePart":"opening"},)"
        R"({"InsertMasterSlide":0},)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Chart"},)"
        R"({"GenerateImage.1":"a bar chart"},)"
        R"({"SetSlidePart":"body"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileDeckSpec(parse(deck), true));
}

void DeckSpecTests::testCompiledOutputValidates()
{
    constexpr std::string_view testname = __func__;

    // Every intent, so the check covers each layout and block shape the
    // compiler can emit.
    const std::string deck =
        R"({"slides":[
            {"part":"opening","intent":"title","title":"T","blocks":[{"kind":"text","text":"s"}]},
            {"part":"body","intent":"agenda","title":"A","blocks":[{"kind":"bullets","items":["x"]}]},
            {"part":"body","intent":"bullets","title":"B","blocks":[{"kind":"bullets","items":["x"]}]},
            {"part":"body","intent":"two-column","title":"C",
             "blocks":[{"kind":"bullets","items":["x"]},{"kind":"bullets","items":["y"]}]},
            {"part":"body","intent":"comparison","title":"D",
             "blocks":[{"kind":"bullets","items":["x"]},{"kind":"bullets","items":["y"]}]},
            {"part":"divider","intent":"quote","title":"E","blocks":[{"kind":"text","text":"q"}]},
            {"part":"body","intent":"big-number","title":"F","blocks":[{"kind":"text","text":"42"}]},
            {"part":"body","intent":"image","title":"G","image":{"brief":"pic"}},
            {"part":"divider","intent":"section","title":"H"},
            {"part":"closing","intent":"closing","title":"I"}]})";

    LOK_ASSERT(!DeckSpec::validateDeckSpec(parse(deck)).has_value());

    for (bool haveTemplate : { false, true })
    {
        const std::string transform = DeckSpec::compileDeckSpec(parse(deck), haveTemplate);
        Poco::JSON::Object::Ptr transformObj;
        LOK_ASSERT(JsonUtil::parseJSON(transform, transformObj));
        LOK_ASSERT(!AIUtil::validateTransformStructure(transformObj).has_value());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(DeckSpecTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
