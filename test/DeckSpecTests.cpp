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

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

/// Unit tests for the declarative deck spec validator and compiler.
class DeckSpecTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(DeckSpecTests);
    CPPUNIT_TEST(testValidateSlideAccepts);
    CPPUNIT_TEST(testValidateSlideRejects);
    CPPUNIT_TEST(testValidateHostileShapes);
    CPPUNIT_TEST(testLimitsSentence);
    CPPUNIT_TEST(testCustomBudgets);
    CPPUNIT_TEST(testValidateOutline);
    CPPUNIT_TEST(testSanitizeOutline);
    CPPUNIT_TEST(testCompileTitleSlideNoTemplate);
    CPPUNIT_TEST(testCompileTitleSlideWithTemplate);
    CPPUNIT_TEST(testCompileImageBrief);
    CPPUNIT_TEST(testRejectImageAlt);
    CPPUNIT_TEST(testCompileNotes);
    CPPUNIT_TEST(testRejectNotes);
    CPPUNIT_TEST(testCompileEmphasis);
    CPPUNIT_TEST(testEmphasisVisibleLength);
    CPPUNIT_TEST(testCompileSlideSpecFirst);
    CPPUNIT_TEST(testCompileSlideSpecAppend);
    CPPUNIT_TEST(testCompileSlideSpecImage);
    CPPUNIT_TEST(testRewriteGenerateImageTargetsSlide);
    CPPUNIT_TEST(testCompileSlideSpecValidates);
    CPPUNIT_TEST(testBuildExpansionUserMessage);
    CPPUNIT_TEST_SUITE_END();

    void testValidateSlideAccepts();
    void testValidateSlideRejects();
    void testValidateHostileShapes();
    void testLimitsSentence();
    void testCustomBudgets();
    void testValidateOutline();
    void testSanitizeOutline();
    void testCompileTitleSlideNoTemplate();
    void testCompileTitleSlideWithTemplate();
    void testCompileImageBrief();
    void testRejectImageAlt();
    void testCompileNotes();
    void testRejectNotes();
    void testCompileEmphasis();
    void testEmphasisVisibleLength();
    void testCompileSlideSpecFirst();
    void testCompileSlideSpecAppend();
    void testCompileSlideSpecImage();
    void testRewriteGenerateImageTargetsSlide();
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

/// The built-in limits, used by the tests that do not exercise a custom budget.
const DeckSpec::Budgets kDefaultBudgets;

/// Compile options for the untemplated and the templated path, both without an
/// image style, used by the tests that do not exercise art direction.
const DeckSpec::CompileOptions kNoTemplate{ false, {} };
const DeckSpec::CompileOptions kWithTemplate{ true, {} };

/// Validate one slide object against the default budgets. Each per-slide test
/// passes the slide object directly, the shape the per-slide validator takes.
std::optional<std::string> slideError(const std::string& s)
{
    return DeckSpec::validateSlideSpec(parse(s), 0, kDefaultBudgets);
}

std::optional<std::string> outlineError(const std::string& s)
{
    return DeckSpec::validateOutline(parse(s), kDefaultBudgets);
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

void DeckSpecTests::testValidateSlideAccepts()
{
    constexpr std::string_view testname = __func__;

    // A title slide with a subtitle text block.
    LOK_ASSERT(!slideError(
                    R"({"part":"opening","intent":"title","title":"Hello",
                        "blocks":[{"kind":"text","text":"A subtitle"}]})")
                    .has_value());

    // A comparison needs exactly two bullets blocks.
    LOK_ASSERT(!slideError(
                    R"({"part":"body","intent":"comparison","title":"Us vs Them",
                        "blocks":[{"kind":"bullets","items":["a"]},
                                  {"kind":"bullets","items":["b"]}]})")
                    .has_value());

    // A quote needs exactly one text block.
    LOK_ASSERT(!slideError(
                    R"({"part":"divider","intent":"quote","title":"Wisdom",
                        "blocks":[{"kind":"text","text":"Stay curious."}]})")
                    .has_value());

    // An image slide needs an image brief and no content blocks.
    LOK_ASSERT(!slideError(
                    R"({"part":"body","intent":"image","title":"Chart",
                        "image":{"brief":"a bar chart","alt":"chart"}})")
                    .has_value());

    // A section divider and a closing slide need only a title.
    LOK_ASSERT(!slideError(R"({"part":"divider","intent":"section","title":"Part Two"})")
                   .has_value());
    LOK_ASSERT(!slideError(R"({"part":"closing","intent":"closing","title":"Thanks"})")
                   .has_value());
}

void DeckSpecTests::testValidateSlideRejects()
{
    constexpr std::string_view testname = __func__;

    auto errorFor = [](const std::string& s) { return slideError(s); };

    // A part outside the enum names the field.
    auto partErr = errorFor(R"({"part":"intro","intent":"title","title":"X"})");
    LOK_ASSERT(partErr.has_value());
    LOK_ASSERT(partErr->find("part") != std::string::npos);

    // An intent outside the enum names the field.
    auto intentErr = errorFor(R"({"part":"body","intent":"chart","title":"X"})");
    LOK_ASSERT(intentErr.has_value());
    LOK_ASSERT(intentErr->find("intent") != std::string::npos);

    // A missing title names the field.
    auto titleErr = errorFor(R"({"part":"body","intent":"section","title":""})");
    LOK_ASSERT(titleErr.has_value());
    LOK_ASSERT(titleErr->find("title") != std::string::npos);

    // A comparison with one bullets block reports the count rule.
    auto cmpErr = errorFor(
        R"({"part":"body","intent":"comparison","title":"X",
            "blocks":[{"kind":"bullets","items":["a"]}]})");
    LOK_ASSERT(cmpErr.has_value());
    LOK_ASSERT(cmpErr->find("2 bullets") != std::string::npos);

    // A quote with no text block reports the count rule.
    LOK_ASSERT(errorFor(R"({"part":"body","intent":"quote","title":"X"})").has_value());

    // An image slide with no image reports the image requirement.
    auto imgErr = errorFor(R"({"part":"body","intent":"image","title":"X"})");
    LOK_ASSERT(imgErr.has_value());
    LOK_ASSERT(imgErr->find("image") != std::string::npos);

    // A bullets block over the item budget.
    auto itemsErr = errorFor(
        R"({"part":"body","intent":"bullets","title":"X",
            "blocks":[{"kind":"bullets","items":["1","2","3","4","5","6","7"]}]})");
    LOK_ASSERT(itemsErr.has_value());
    LOK_ASSERT(itemsErr->find("6") != std::string::npos);

    // A bullet item over the length budget.
    const std::string longItem(DeckSpec::MaxItemLength + 1, 'x');
    auto lenErr = errorFor(
        R"({"part":"body","intent":"bullets","title":"X",
            "blocks":[{"kind":"bullets","items":[")" + longItem + R"("]}]})");
    LOK_ASSERT(lenErr.has_value());
}

void DeckSpecTests::testValidateHostileShapes()
{
    constexpr std::string_view testname = __func__;

    // Structurally wrong shapes come back as validation errors, not thrown
    // exceptions, whatever JSON type sits where an object or array belongs.
    LOK_ASSERT(slideError(R"([1,2,3])").has_value());
    LOK_ASSERT(slideError(R"({"part":"body","intent":"bullets","title":"T",)"
                          R"("blocks":7})")
                   .has_value());
    LOK_ASSERT(slideError(R"({"part":"body","intent":"bullets","title":"T",)"
                          R"("blocks":[42]})")
                   .has_value());
    LOK_ASSERT(slideError(R"({"part":"body","intent":"bullets","title":"T",)"
                          R"("blocks":[{"kind":"bullets","items":[{"x":1}]}]})")
                   .has_value());
    LOK_ASSERT(
        slideError(R"({"part":"body","intent":"image","title":"T","image":"flat"})").has_value());
    LOK_ASSERT(slideError(R"({"part":3,"intent":true,"title":null})").has_value());

    // The outline validator holds to the same rule for its container and its
    // slide entries.
    LOK_ASSERT(outlineError(R"([1,2,3])").has_value());
    LOK_ASSERT(outlineError(R"({"slides":9})").has_value());
    LOK_ASSERT(outlineError(R"({"slides":[false]})").has_value());
}

void DeckSpecTests::testLimitsSentence()
{
    constexpr std::string_view testname = __func__;

    // With default budgets the limits sentence is exactly the wording the tool
    // description used to carry inline, so the composed description is unchanged
    // when the flag is on and the budgets are left at their defaults.
    LOK_ASSERT_EQUAL(
        std::string("\n\nLimits: at most 30 slides, at most 6 items per bullets block, and keep"
                    " each item short. Do not prefix items with \"- \"; bullet markers are added"
                    " for you, so put only the items themselves in each block."),
        DeckSpec::limitsSentence(DeckSpec::Budgets{}));

    // Lowered budgets are reflected in the numbers the model is told.
    DeckSpec::Budgets tight;
    tight.maxSlides = 8;
    tight.maxItemsPerBullets = 3;
    const std::string sentence = DeckSpec::limitsSentence(tight);
    LOK_ASSERT(sentence.find("at most 8 slides") != std::string::npos);
    LOK_ASSERT(sentence.find("at most 3 items") != std::string::npos);
}

void DeckSpecTests::testCustomBudgets()
{
    constexpr std::string_view testname = __func__;

    // An outline that passes the default slide budget but breaks a tightened
    // one: the tightened budget is what the validator enforces.
    const std::string outline =
        R"({"slides":[
            {"part":"opening","intent":"title","title":"One"},
            {"part":"body","intent":"section","title":"Two"},
            {"part":"closing","intent":"closing","title":"Three"}]})";

    LOK_ASSERT(!DeckSpec::validateOutline(parse(outline), kDefaultBudgets).has_value());

    DeckSpec::Budgets fewSlides;
    fewSlides.maxSlides = 2;
    auto slidesErr = DeckSpec::validateOutline(parse(outline), fewSlides);
    LOK_ASSERT(slidesErr.has_value());
    LOK_ASSERT(slidesErr->find("at most 2 slides") != std::string::npos);

    // A bullets slide within the default item budget but over a tightened one.
    const std::string bulletsSlide =
        R"({"part":"body","intent":"bullets","title":"X",
            "blocks":[{"kind":"bullets","items":["a","b","c","d"]}]})";
    DeckSpec::Budgets fewItems;
    fewItems.maxItemsPerBullets = 3;
    auto itemsErr = DeckSpec::validateSlideSpec(parse(bulletsSlide), 0, fewItems);
    LOK_ASSERT(itemsErr.has_value());
    LOK_ASSERT(itemsErr->find("at most 3 items") != std::string::npos);

    // A short title accepted by default is rejected by a tightened title budget.
    const std::string titleSlide =
        R"({"part":"body","intent":"section","title":"A slightly long title"})";
    DeckSpec::Budgets shortTitle;
    shortTitle.maxTitleLength = 5;
    LOK_ASSERT(DeckSpec::validateSlideSpec(parse(titleSlide), 0, shortTitle).has_value());
}

void DeckSpecTests::testCompileTitleSlideNoTemplate()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"opening","intent":"title","title":"Hello",
            "blocks":[{"kind":"text","text":"A subtitle"}]})";

    // Without a template the compiler adds the house-style bold-title command
    // itself.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE"},)"
        R"({"SetText.0":"Hello"},)"
        R"({"SetText.1":"A subtitle"},)"
        R"({"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},)"
        R"({"SetSlidePart":"opening"},)"
        R"({"SetSlideIntent":"title"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 0, kNoTemplate));
}

void DeckSpecTests::testCompileTitleSlideWithTemplate()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"opening","intent":"title","title":"Hello",
            "blocks":[{"kind":"text","text":"A subtitle"}]})";

    // With a template the master slides own the title look, so no bold-title
    // command is emitted.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE"},)"
        R"({"SetText.0":"Hello"},)"
        R"({"SetText.1":"A subtitle"},)"
        R"({"SetSlidePart":"opening"},)"
        R"({"SetSlideIntent":"title"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 0, kWithTemplate));
}

void DeckSpecTests::testCompileImageBrief()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"body","intent":"image","title":"Chart",
            "image":{"brief":"a bar chart","alt":"chart"}})";

    // An image slide compiles to a GenerateImage object carrying the prompt and
    // the alt text. Without an art direction the prompt is the brief alone.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Chart"},)"
        R"({"GenerateImage.1":{"alt":"chart","prompt":"a bar chart"}},)"
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"image"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 0, kWithTemplate));
}

void DeckSpecTests::testRejectImageAlt()
{
    constexpr std::string_view testname = __func__;

    // An image without alt text is rejected; the error names both required
    // fields so the model fixes the right one.
    auto missingAlt = slideError(
        R"({"part":"body","intent":"image","title":"X","image":{"brief":"a chart"}})");
    LOK_ASSERT(missingAlt.has_value());
    LOK_ASSERT(missingAlt->find("brief") != std::string::npos);
    LOK_ASSERT(missingAlt->find("alt") != std::string::npos);

    // Alt text over the item-length budget is rejected.
    const std::string longAlt(DeckSpec::MaxItemLength + 1, 'x');
    auto overLength = slideError(
        R"({"part":"body","intent":"image","title":"X",
            "image":{"brief":"a chart","alt":")" + longAlt + R"("}})");
    LOK_ASSERT(overLength.has_value());
    LOK_ASSERT(overLength->find("alt") != std::string::npos);
}

void DeckSpecTests::testCompileNotes()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"body","intent":"bullets","title":"Points",
            "blocks":[{"kind":"bullets","items":["One","Two"]}],
            "notes":"Explain each point with an example."})";

    // Notes are emitted after the content blocks and before the house-style
    // formatting.
    const std::string expectedNoTemplate =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Points"},)"
        R"({"SetText.1":"One\nTwo"},)"
        R"({"SetNotes":"Explain each point with an example."},)"
        R"({"EditTextObject.0":[{"SelectText":[]},{"UnoCommand":".uno:Bold"}]},)"
        R"({"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},)"
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"bullets"}]}})";

    LOK_ASSERT_EQUAL(expectedNoTemplate, DeckSpec::compileSlideSpec(parse(slide), 0, kNoTemplate));

    // With a template the notes still ride along and the bullet markers are
    // turned on; only the bold-title house style is gone.
    const std::string expectedTemplate =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Points"},)"
        R"({"SetText.1":"One\nTwo"},)"
        R"({"SetNotes":"Explain each point with an example."},)"
        R"({"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},)"
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"bullets"}]}})";

    LOK_ASSERT_EQUAL(expectedTemplate, DeckSpec::compileSlideSpec(parse(slide), 0, kWithTemplate));
}

void DeckSpecTests::testRejectNotes()
{
    constexpr std::string_view testname = __func__;

    // Notes within the default budget are accepted; over the budget rejected.
    const std::string overLength(DeckSpec::MaxNotesLength + 1, 'x');
    auto lenErr = slideError(
        R"({"part":"body","intent":"section","title":"X","notes":")" + overLength + R"("})");
    LOK_ASSERT(lenErr.has_value());
    LOK_ASSERT(lenErr->find("notes") != std::string::npos);

    // A tightened notes budget rejects notes the default budget would accept.
    const std::string slide =
        R"({"part":"body","intent":"section","title":"X",
            "notes":"A sentence of speaker notes."})";
    LOK_ASSERT(!DeckSpec::validateSlideSpec(parse(slide), 0, kDefaultBudgets).has_value());

    DeckSpec::Budgets tightNotes;
    tightNotes.maxNotesLength = 10;
    auto tightErr = DeckSpec::validateSlideSpec(parse(slide), 0, tightNotes);
    LOK_ASSERT(tightErr.has_value());
    LOK_ASSERT(tightErr->find("10 characters") != std::string::npos);
}

void DeckSpecTests::testCompileEmphasis()
{
    constexpr std::string_view testname = __func__;

    // Bold and italic within one bullet item. The emphasis EditTextObject comes
    // first, then the bullet-marker EditTextObject that every slide emits.
    const std::string boldItalic =
        R"({"part":"body","intent":"bullets","title":"T",
            "blocks":[{"kind":"bullets","items":["**Bold** and *italic*"]}]})";
    const std::string expectedBoldItalic =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"T"},)"
        R"({"SetText.1":"Bold and italic"},)"
        R"({"EditTextObject.1":[)"
        R"({"SelectText":[0,0,0,4]},{"UnoCommand":".uno:Bold"},)"
        R"({"SelectText":[0,9,0,15]},{"UnoCommand":".uno:Italic"}]},)"
        R"({"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},)"
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"bullets"}]}})";
    LOK_ASSERT_EQUAL(expectedBoldItalic,
                     DeckSpec::compileSlideSpec(parse(boldItalic), 0, kWithTemplate));

    // Three markers toggle bold and italic together on one run.
    const std::string both =
        R"({"part":"body","intent":"bullets","title":"T",
            "blocks":[{"kind":"bullets","items":["***both***"]}]})";
    const std::string expectedBoth =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"T"},)"
        R"({"SetText.1":"both"},)"
        R"({"EditTextObject.1":[)"
        R"({"SelectText":[0,0,0,4]},{"UnoCommand":".uno:Bold"},{"UnoCommand":".uno:Italic"}]},)"
        R"({"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},)"
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"bullets"}]}})";
    LOK_ASSERT_EQUAL(expectedBoth, DeckSpec::compileSlideSpec(parse(both), 0, kWithTemplate));

    // An unbalanced marker stays literal: the asterisk is kept and no emphasis
    // is emitted.
    const std::string unbalanced =
        R"({"part":"body","intent":"bullets","title":"T",
            "blocks":[{"kind":"bullets","items":["*oops"]}]})";
    const std::string expectedUnbalanced =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"T"},)"
        R"({"SetText.1":"*oops"},)"
        R"({"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},)"
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"bullets"}]}})";
    LOK_ASSERT_EQUAL(expectedUnbalanced,
                     DeckSpec::compileSlideSpec(parse(unbalanced), 0, kWithTemplate));

    // A title's markers are stripped but no emphasis EditTextObject is emitted.
    const std::string titleMarkup =
        R"({"part":"opening","intent":"title","title":"**Big**"})";
    const std::string expectedTitle =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE"},)"
        R"({"SetText.0":"Big"},)"
        R"({"SetSlidePart":"opening"},)"
        R"({"SetSlideIntent":"title"}]}})";
    LOK_ASSERT_EQUAL(expectedTitle,
                     DeckSpec::compileSlideSpec(parse(titleMarkup), 0, kWithTemplate));

    // A non-BMP code point before a marker counts as two UTF-16 units, so the
    // bold run starts at offset 2, not 1.
    const std::string emoji =
        R"({"part":"body","intent":"bullets","title":"T",
            "blocks":[{"kind":"bullets","items":["😀**x**"]}]})";
    const std::string expectedEmoji =
        "{\"Transforms\":{\"SlideCommands\":["
        "{\"ChangeLayoutByName\":\"AUTOLAYOUT_TITLE_CONTENT\"},"
        "{\"SetText.0\":\"T\"},"
        "{\"SetText.1\":\"\xF0\x9F\x98\x80x\"},"
        "{\"EditTextObject.1\":[{\"SelectText\":[0,2,0,3]},{\"UnoCommand\":\".uno:Bold\"}]},"
        "{\"EditTextObject.1\":[{\"SelectText\":[]},{\"UnoCommand\":\".uno:DefaultBullet\"}]},"
        "{\"SetSlidePart\":\"body\"},"
        "{\"SetSlideIntent\":\"bullets\"}]}}";
    LOK_ASSERT_EQUAL(expectedEmoji, DeckSpec::compileSlideSpec(parse(emoji), 0, kWithTemplate));
}

void DeckSpecTests::testEmphasisVisibleLength()
{
    constexpr std::string_view testname = __func__;

    // An item whose visible text is exactly at the limit is accepted even when
    // the emphasis markers push the raw string over it.
    const std::string atLimit(DeckSpec::MaxItemLength, 'a');
    LOK_ASSERT(!slideError(
                    R"({"part":"body","intent":"bullets","title":"X",
                        "blocks":[{"kind":"bullets","items":["**)" + atLimit + R"(**"]}]})")
                    .has_value());

    // One more visible character than the limit is rejected.
    const std::string overLimit(DeckSpec::MaxItemLength + 1, 'a');
    LOK_ASSERT(slideError(
                   R"({"part":"body","intent":"bullets","title":"X",
                       "blocks":[{"kind":"bullets","items":["**)" + overLimit + R"(**"]}]})")
                   .has_value());
}

void DeckSpecTests::testValidateOutline()
{
    constexpr std::string_view testname = __func__;

    // A well-formed outline with a deck title and per-slide gists.
    LOK_ASSERT(!outlineError(
                    R"({"title":"My Deck","slides":[
                        {"part":"opening","intent":"title","title":"Welcome",
                         "gist":"Set the scene and introduce the topic."},
                        {"part":"body","intent":"bullets","title":"Key points",
                         "gist":"Three reasons this matters."},
                        {"part":"closing","intent":"closing","title":"Thanks"}]})")
                    .has_value());

    // A gist is optional and a deck title is optional.
    LOK_ASSERT(!outlineError(
                    R"({"slides":[{"part":"body","intent":"section","title":"Only a title"}]})")
                    .has_value());

    auto errorFor = [](const std::string& s) { return outlineError(s); };

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

void DeckSpecTests::testSanitizeOutline()
{
    constexpr std::string_view testname = __func__;

    // The stripped outline carries the validated fields only. Extra keys, at the
    // top level and on a slide, are left behind rather than travelling into the
    // prompt of every per-slide request.
    Poco::JSON::Object::Ptr clean = DeckSpec::sanitizeOutline(parse(
        R"({"title":"Deck","junk":"xxx",
            "slides":[{"part":"body","intent":"bullets","title":"T","gist":"G",
                       "padding":"yyy"}]})"));

    std::ostringstream oss;
    clean->stringify(oss);
    LOK_ASSERT_EQUAL(std::string(R"({"slides":[{"gist":"G","intent":"bullets","part":"body",)"
                                 R"("title":"T"}],"title":"Deck"})"),
                     oss.str());

    // A title and a gist are written into a one-line sentence, so a line break
    // in either becomes a space.
    Poco::JSON::Object::Ptr flattened = DeckSpec::sanitizeOutline(parse(
        "{\"title\":\"A\\nB\","
        "\"slides\":[{\"part\":\"body\",\"intent\":\"bullets\",\"title\":\"C\\nD\","
        "\"gist\":\"E\\r\\nF\"}]}"));

    LOK_ASSERT_EQUAL(std::string("A B"), flattened->getValue<std::string>("title"));
    Poco::JSON::Object::Ptr slide = flattened->getArray("slides")->getObject(0);
    LOK_ASSERT_EQUAL(std::string("C D"), slide->getValue<std::string>("title"));
    LOK_ASSERT_EQUAL(std::string("E  F"), slide->getValue<std::string>("gist"));

    // A null outline gives an outline with no slides rather than a null pointer.
    Poco::JSON::Object::Ptr empty = DeckSpec::sanitizeOutline(nullptr);
    LOK_ASSERT(empty);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), empty->getArray("slides")->size());
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
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"bullets"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 0, kNoTemplate));
}

void DeckSpecTests::testCompileSlideSpecAppend()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"body","intent":"bullets","title":"Points",
            "blocks":[{"kind":"bullets","items":["One","Two"]}]})";

    // A later slide moves to the end of the deck and inserts a new slide first.
    // With a template the masters own the title look, so no bold-title command;
    // the bullet markers are still turned on.
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"JumpToSlide":"last"},)"
        R"({"InsertMasterSlide":0},)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Points"},)"
        R"({"SetText.1":"One\nTwo"},)"
        R"({"EditTextObject.1":[{"SelectText":[]},{"UnoCommand":".uno:DefaultBullet"}]},)"
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"bullets"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 1, kWithTemplate));
}

void DeckSpecTests::testCompileSlideSpecImage()
{
    constexpr std::string_view testname = __func__;

    const std::string slide =
        R"({"part":"body","intent":"image","title":"Chart",
            "image":{"brief":"a bar chart","alt":"chart"}})";

    // An image slide compiles to a GenerateImage object on the content
    // placeholder, carrying the composed prompt and the alt text.
    const DeckSpec::CompileOptions artDirected{ true, "muted editorial style" };
    const std::string expected =
        R"({"Transforms":{"SlideCommands":[)"
        R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
        R"({"SetText.0":"Chart"},)"
        R"({"GenerateImage.1":{"alt":"chart","prompt":"a bar chart. Style: muted editorial style"}},)"
        R"({"SetSlidePart":"body"},)"
        R"({"SetSlideIntent":"image"}]}})";

    LOK_ASSERT_EQUAL(expected, DeckSpec::compileSlideSpec(parse(slide), 0, artDirected));
}

void DeckSpecTests::testRewriteGenerateImageTargetsSlide()
{
    constexpr std::string_view testname = __func__;

    const std::string placeholder = "file:///placeholder.png";

    // A later slide's transform appends a new slide. With two slides already in
    // the deck the image belongs on the third slide, index 2 - the case that
    // used to record index 1 and drop the image on the second slide.
    {
        Poco::JSON::Object::Ptr transform = parse(
            R"({"Transforms":{"SlideCommands":[)"
            R"({"JumpToSlide":"last"},)"
            R"({"InsertMasterSlide":0},)"
            R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
            R"({"SetText.0":"Chart"},)"
            R"({"GenerateImage.1":{"prompt":"a bar chart","alt":"chart"}},)"
            R"({"SetSlidePart":"body"}]}})");

        const std::vector<DeckSpec::ImageInsertion> images =
            DeckSpec::rewriteGenerateImageCommands(transform, /*nExistingSlides=*/2, placeholder);

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), images.size());
        LOK_ASSERT_EQUAL(2, images[0].slideIndex);
        LOK_ASSERT_EQUAL(1, images[0].objId);
        LOK_ASSERT_EQUAL(std::string("a bar chart"), images[0].prompt);
        LOK_ASSERT_EQUAL(std::string("chart"), images[0].alt);

        // GenerateImage.1 is gone, replaced by an InsertImage.1 that points at
        // the loading placeholder.
        Poco::JSON::Array::Ptr cmds =
            transform->getObject("Transforms")->getArray("SlideCommands");
        bool sawPlaceholder = false;
        for (unsigned i = 0; i < cmds->size(); ++i)
        {
            Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
            LOK_ASSERT(!cmd->has("GenerateImage.1"));
            if (cmd->has("InsertImage.1"))
            {
                LOK_ASSERT_EQUAL(placeholder, cmd->getValue<std::string>("InsertImage.1"));
                sawPlaceholder = true;
            }
        }
        LOK_ASSERT(sawPlaceholder);
    }

    // The first built slide reuses the deck's one starting slide, so its
    // transform has no navigation and the image belongs on slide 0.
    {
        Poco::JSON::Object::Ptr transform = parse(
            R"({"Transforms":{"SlideCommands":[)"
            R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
            R"({"GenerateImage.1":{"prompt":"a logo","alt":"logo"}}]}})");

        const std::vector<DeckSpec::ImageInsertion> images =
            DeckSpec::rewriteGenerateImageCommands(transform, /*nExistingSlides=*/1, placeholder);

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), images.size());
        LOK_ASSERT_EQUAL(0, images[0].slideIndex);
    }

    // A transform that appends several slides in one array walks the index
    // forward across consecutive InsertMasterSlide commands: the two images land
    // on slides 1 and 2. This also covers the bare prompt-string form.
    {
        Poco::JSON::Object::Ptr transform = parse(
            R"({"Transforms":{"SlideCommands":[)"
            R"({"ChangeLayoutByName":"AUTOLAYOUT_TITLE_CONTENT"},)"
            R"({"InsertMasterSlide":0},)"
            R"({"GenerateImage.1":"first image"},)"
            R"({"InsertMasterSlide":0},)"
            R"({"GenerateImage.1":"second image"}]}})");

        const std::vector<DeckSpec::ImageInsertion> images =
            DeckSpec::rewriteGenerateImageCommands(transform, /*nExistingSlides=*/1, placeholder);

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), images.size());
        LOK_ASSERT_EQUAL(1, images[0].slideIndex);
        LOK_ASSERT_EQUAL(std::string("first image"), images[0].prompt);
        LOK_ASSERT_EQUAL(2, images[1].slideIndex);
        LOK_ASSERT_EQUAL(std::string("second image"), images[1].prompt);
    }
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
        R"({"part":"body","intent":"image","title":"G","image":{"brief":"pic","alt":"pic"}})",
        R"({"part":"divider","intent":"section","title":"H"})",
        R"({"part":"closing","intent":"closing","title":"I"})",
    };

    // Both the reuse-current-slide and the append case, in both template modes.
    for (const std::string& slide : slides)
    {
        LOK_ASSERT(!DeckSpec::validateSlideSpec(parse(slide), 0, kDefaultBudgets).has_value());
        for (int docSlideIndex : { 0, 1 })
        {
            for (const DeckSpec::CompileOptions& options : { kNoTemplate, kWithTemplate })
            {
                const std::string transform =
                    DeckSpec::compileSlideSpec(parse(slide), docSlideIndex, options);
                Poco::JSON::Object::Ptr transformObj;
                LOK_ASSERT(JsonUtil::parseJSON(transform, transformObj));
                LOK_ASSERT(!AIUtil::validateTransformStructure(transformObj).has_value());

                // Every slot the slide fills has a placeholder on the layout the
                // slide selected. The engine addresses a placeholder by its
                // index, so a slot past the last one has nothing to write to.
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
