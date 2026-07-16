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

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

/// Unit tests for the declarative deck spec validator and compiler.
class DeckSpecTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(DeckSpecTests);
    CPPUNIT_TEST(testValidateAccepts);
    CPPUNIT_TEST(testValidateRejects);
    CPPUNIT_TEST(testValidateOutline);
    CPPUNIT_TEST(testCompileNoTemplate);
    CPPUNIT_TEST(testCompileWithTemplate);
    CPPUNIT_TEST(testCompileImageBrief);
    CPPUNIT_TEST(testCompiledOutputValidates);
    CPPUNIT_TEST(testCompileSlideSpecFirst);
    CPPUNIT_TEST(testCompileSlideSpecAppend);
    CPPUNIT_TEST(testCompileSlideSpecImage);
    CPPUNIT_TEST(testCompileSlideSpecValidates);
    CPPUNIT_TEST(testBuildExpansionUserMessage);
    CPPUNIT_TEST_SUITE_END();

    void testValidateAccepts();
    void testValidateRejects();
    void testValidateOutline();
    void testCompileNoTemplate();
    void testCompileWithTemplate();
    void testCompileImageBrief();
    void testCompiledOutputValidates();
    void testCompileSlideSpecFirst();
    void testCompileSlideSpecAppend();
    void testCompileSlideSpecImage();
    void testCompileSlideSpecValidates();
    void testBuildExpansionUserMessage();
};

namespace
{
Poco::JSON::Object::Ptr parse(const std::string& s)
{
    Poco::JSON::Object::Ptr obj;
    JsonUtil::parseJSON(s, obj);
    return obj;
}

/// The number of placeholders the named layout puts on a slide, or -1 when the
/// name is not one the model is offered.
int placeholdersOfLayout(const std::string& name)
{
    for (const auto& layout : AIUtil::getSlideLayouts())
        if (name == layout.name)
            return layout.placeholderCount;
    return -1;
}

/// The highest placeholder index a compiled transform addresses, or -1 when it
/// addresses none. A command key that carries an index ends in ".N", and the
/// commands that take one are the ones that fill a placeholder.
int highestSlotAddressed(const Poco::JSON::Object::Ptr& transformObj)
{
    int highest = -1;
    Poco::JSON::Array::Ptr cmds =
        transformObj->getObject("Transforms")->getArray("SlideCommands");
    for (unsigned i = 0; cmds && i < cmds->size(); ++i)
    {
        Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
        for (const auto& key : cmd->getNames())
        {
            const std::size_t dot = key.find('.');
            if (dot == std::string::npos)
                continue;
            const std::string base = key.substr(0, dot);
            if (base != "SetText" && base != "EditTextObject" && base != "GenerateImage" &&
                base != "InsertImage")
                continue;
            highest = std::max(highest, std::stoi(key.substr(dot + 1)));
        }
    }
    return highest;
}

/// The layout a compiled transform selects, empty when it selects none.
std::string layoutOfTransform(const Poco::JSON::Object::Ptr& transformObj)
{
    Poco::JSON::Array::Ptr cmds =
        transformObj->getObject("Transforms")->getArray("SlideCommands");
    for (unsigned i = 0; cmds && i < cmds->size(); ++i)
    {
        Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
        if (cmd->has("ChangeLayoutByName"))
            return cmd->getValue<std::string>("ChangeLayoutByName");
    }
    return std::string();
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

void DeckSpecTests::testValidateOutline()
{
    constexpr std::string_view testname = __func__;

    // A well-formed outline with a deck title and per-slide gists.
    LOK_ASSERT(!DeckSpec::validateOutline(parse(
                    R"({"title":"My Deck","slides":[
                        {"part":"opening","intent":"title","title":"Welcome",
                         "gist":"Set the scene and introduce the topic."},
                        {"part":"body","intent":"bullets","title":"Key points",
                         "gist":"Three reasons this matters."},
                        {"part":"closing","intent":"closing","title":"Thanks"}]})"))
                    .has_value());

    // A gist is optional and a deck title is optional.
    LOK_ASSERT(!DeckSpec::validateOutline(parse(
                    R"({"slides":[{"part":"body","intent":"section","title":"Only a title"}]})"))
                    .has_value());

    auto errorFor = [](const std::string& s)
    { return DeckSpec::validateOutline(parse(s)); };

    // An outline with no slides.
    LOK_ASSERT(errorFor(R"({"slides":[]})").has_value());

    // A part outside the enum names the field.
    auto partErr =
        errorFor(R"({"slides":[{"part":"intro","intent":"title","title":"X"}]})");
    LOK_ASSERT(partErr.has_value());
    LOK_ASSERT(partErr->find("part") != std::string::npos);

    // An intent outside the enum names the field.
    auto intentErr =
        errorFor(R"({"slides":[{"part":"body","intent":"chart","title":"X"}]})");
    LOK_ASSERT(intentErr.has_value());
    LOK_ASSERT(intentErr->find("intent") != std::string::npos);

    // A missing title names the field.
    auto titleErr =
        errorFor(R"({"slides":[{"part":"body","intent":"section","title":""}]})");
    LOK_ASSERT(titleErr.has_value());
    LOK_ASSERT(titleErr->find("title") != std::string::npos);

    // A gist over the length budget.
    const std::string longGist(DeckSpec::MaxGistLength + 1, 'x');
    auto gistErr = errorFor(
        R"({"slides":[{"part":"body","intent":"section","title":"X","gist":")" + longGist +
        R"("}]})");
    LOK_ASSERT(gistErr.has_value());
    LOK_ASSERT(gistErr->find("gist") != std::string::npos);

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

void DeckSpecTests::testCompileSlideSpecFirst()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"body","intent":"bullets","title":"Points",
            "blocks":[{"kind":"bullets","items":["One","Two"]}]})";

    // The first built slide reuses the deck's starting slide, so no navigation
    // or insert command. Without a template the house-style formatting rides
    // along.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Points"},)"
        R"({"SetText.1":"One\nTwo"},)"
        R"({"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},)"
        R"({"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},)"
        R"({"SetSlidePart":"body"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 0, false));
}

void DeckSpecTests::testCompileSlideSpecAppend()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"body","intent":"bullets","title":"Points",
            "blocks":[{"kind":"bullets","items":["One","Two"]}]})";

    // A later slide moves to the end of the deck and inserts a new slide first.
    // With a template no formatting is emitted; the masters own the look.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"JumpToSlide":"last"},)"
        R"({"InsertMasterSlide":0},)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Points"},)"
        R"({"SetText.1":"One\nTwo"},)"
        R"({"SetSlidePart":"body"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 1, true));
}

void DeckSpecTests::testCompileSlideSpecImage()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"body","intent":"image","title":"Chart",
            "image":{"brief":"a bar chart","alt":"chart"}})";

    // An image slide compiles to a GenerateImage on the content placeholder.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Chart"},)"
        R"({"GenerateImage.1":"a bar chart"},)"
        R"({"SetSlidePart":"body"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 0, true));
}

void DeckSpecTests::testCompileSlideSpecValidates()
{
    constexpr std::string_view testname = __func__;

    // One slide per intent, so the check covers each layout and block shape the
    // per-slide compiler can emit.
    const std::array<std::string, 10> slides = {
        R"({"part":"opening","intent":"title","title":"T","blocks":[{"kind":"text","text":"s"}]})",
        R"({"part":"body","intent":"agenda","title":"A","blocks":[{"kind":"bullets","items":["x"]}]})",
        R"({"part":"body","intent":"bullets","title":"B","blocks":[{"kind":"bullets","items":["x"]}]})",
        R"({"part":"body","intent":"two-column","title":"C",
            "blocks":[{"kind":"bullets","items":["x"]},{"kind":"bullets","items":["y"]}]})",
        R"({"part":"body","intent":"comparison","title":"D",
            "blocks":[{"kind":"bullets","items":["x"]},{"kind":"bullets","items":["y"]}]})",
        R"({"part":"divider","intent":"quote","title":"E","blocks":[{"kind":"text","text":"q"}]})",
        R"({"part":"body","intent":"big-number","title":"F","blocks":[{"kind":"text","text":"42"}]})",
        R"({"part":"body","intent":"image","title":"G","image":{"brief":"pic"}})",
        R"({"part":"divider","intent":"section","title":"H"})",
        R"({"part":"closing","intent":"closing","title":"I"})",
    };

    // Both the reuse-current-slide and the append case, in both template modes.
    for (const std::string& slide : slides)
    {
        LOK_ASSERT(!DeckSpec::validateSlideSpec(parse(slide), 0).has_value());
        for (int docSlideIndex : { 0, 1 })
        {
            for (bool haveTemplate : { false, true })
            {
                const std::string transform =
                    DeckSpec::compileSlideSpec(parse(slide), docSlideIndex, haveTemplate);
                Poco::JSON::Object::Ptr transformObj;
                LOK_ASSERT(JsonUtil::parseJSON(transform, transformObj));
                LOK_ASSERT(!AIUtil::validateTransformStructure(transformObj).has_value());

                // Every slot the slide fills has a placeholder on the layout the
                // slide selected. A slot past the last placeholder is dropped by
                // the engine, so the text would never reach the slide.
                const std::string layout = layoutOfTransform(transformObj);
                const int placeholders = placeholdersOfLayout(layout);
                LOK_ASSERT_MESSAGE("unknown layout " + layout, placeholders >= 0);
                LOK_ASSERT_MESSAGE(layout + " offers " + std::to_string(placeholders) +
                                       " placeholders, slide addresses slot " +
                                       std::to_string(highestSlotAddressed(transformObj)),
                                   highestSlotAddressed(transformObj) < placeholders);
            }
        }
    }
}

void DeckSpecTests::testBuildExpansionUserMessage()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"body","intent":"bullets","title":"Key points",
            "gist":"Three reasons this matters."})";

    // A first attempt states the slide's place and outline fields.
    LOK_ASSERT_EQUAL(
        std::string("Write slide 4 of 9. part: body; intent: bullets; title: Key points;"
                    " gist: Three reasons this matters. . Provide the blocks this intent"
                    " expects."),
        DeckSpec::buildExpansionUserMessage(parse(slide), 4, 9, std::string()));

    // A retry appends the previous error and asks to fix exactly that.
    LOK_ASSERT_EQUAL(
        std::string("Write slide 4 of 9. part: body; intent: bullets; title: Key points;"
                    " gist: Three reasons this matters. . Provide the blocks this intent"
                    " expects. Your previous attempt was rejected: needs one bullets block."
                    " Fix exactly that."),
        DeckSpec::buildExpansionUserMessage(parse(slide), 4, 9, "needs one bullets block"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(DeckSpecTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
