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
#include <wsd/DeckSpec.hpp>
#include <wsd/McpDeck.hpp>

#include <test/lokassert.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <Poco/JSON/Array.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

/// Whitebox tests for the design manifest a presentation template declares. The manifest is read
/// straight from the template file, so these tests hand the reader manifest text and check what a
/// caller is told about the design.
class McpDeckTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(McpDeckTests);
    CPPUNIT_TEST(testDesignWithoutManifestKeepsOnlyItsName);
    CPPUNIT_TEST(testUnparsableManifestLeavesTheDesignBare);
    CPPUNIT_TEST(testManifestBelowSchemaVersionOneIsIgnored);
    CPPUNIT_TEST(testPartsFollowTheDeckOrderNotTheManifestOrder);
    CPPUNIT_TEST(testUnknownPartNamesAreLeftOut);
    CPPUNIT_TEST(testArtDirectionIsCleanedOfControlCharacters);
    CPPUNIT_TEST(testManifestLowersALimitButCannotRaiseOne);
    CPPUNIT_TEST(testADeckNeedsAtLeastOneSlide);
    CPPUNIT_TEST(testADeckOverTheSlideLimitIsRefused);
    CPPUNIT_TEST(testAFaultyLastSlideRefusesTheWholeDeck);
    CPPUNIT_TEST(testASlideAskingForAPictureIsRefused);
    CPPUNIT_TEST(testTheFirstSlideFillsTheStartingSlideAndTheRestAreAdded);
    CPPUNIT_TEST(testANamedDesignLeadsTheCommands);
    CPPUNIT_TEST(testAnUnstyledDeckCarriesNoDesignCommand);
    CPPUNIT_TEST_SUITE_END();

    void testDesignWithoutManifestKeepsOnlyItsName();
    void testUnparsableManifestLeavesTheDesignBare();
    void testManifestBelowSchemaVersionOneIsIgnored();
    void testPartsFollowTheDeckOrderNotTheManifestOrder();
    void testUnknownPartNamesAreLeftOut();
    void testArtDirectionIsCleanedOfControlCharacters();
    void testManifestLowersALimitButCannotRaiseOne();
    void testADeckNeedsAtLeastOneSlide();
    void testADeckOverTheSlideLimitIsRefused();
    void testAFaultyLastSlideRefusesTheWholeDeck();
    void testASlideAskingForAPictureIsRefused();
    void testTheFirstSlideFillsTheStartingSlideAndTheRestAreAdded();
    void testANamedDesignLeadsTheCommands();
    void testAnUnstyledDeckCarriesNoDesignCommand();

    /// True when the design offers a master design for the named slide part.
    static bool covers(const McpDeck::DesignTemplate& design, const std::string& part)
    {
        return std::find(design.parts.begin(), design.parts.end(), part) != design.parts.end();
    }
};

void McpDeckTests::testDesignWithoutManifestKeepsOnlyItsName()
{
    constexpr std::string_view testname = __func__;

    // A template that declares no manifest is still offered, so a deck can be built on it; it
    // simply says nothing about its parts or its style.
    const McpDeck::DesignTemplate design =
        McpDeck::designTemplateFromManifest("Plain", std::string());

    LOK_ASSERT_EQUAL(std::string("Plain"), design.name);
    LOK_ASSERT(design.parts.empty());
    LOK_ASSERT(design.artDirection.empty());
    LOK_ASSERT_EQUAL(DeckSpec::budgetsFromConfig().maxSlides, design.budgets.maxSlides);
}

void McpDeckTests::testUnparsableManifestLeavesTheDesignBare()
{
    constexpr std::string_view testname = __func__;

    const McpDeck::DesignTemplate design =
        McpDeck::designTemplateFromManifest("Broken", "{not valid json");

    LOK_ASSERT_EQUAL(std::string("Broken"), design.name);
    LOK_ASSERT(design.parts.empty());
}

void McpDeckTests::testManifestBelowSchemaVersionOneIsIgnored()
{
    constexpr std::string_view testname = __func__;

    // A reader of a later schema reads what it knows and ignores the rest, so version 1 is the
    // lowest that means anything. Anything below it reads as no manifest at all rather than as a
    // partly understood one.
    const McpDeck::DesignTemplate design = McpDeck::designTemplateFromManifest(
        "Old", R"({"schemaVersion":0,"masters":{"OldBody":"body"},"artDirection":"soft pastels"})");

    LOK_ASSERT(design.parts.empty());
    LOK_ASSERT(design.artDirection.empty());
}

void McpDeckTests::testPartsFollowTheDeckOrderNotTheManifestOrder()
{
    constexpr std::string_view testname = __func__;

    // The masters are listed closing-first here, and two of them play the same part, so the answer
    // shows both that the order is the deck's own and that a part is named once however many
    // masters cover it.
    const McpDeck::DesignTemplate design = McpDeck::designTemplateFromManifest(
        "Cobalt", R"({"schemaVersion":1,"masters":{"C3":"closing","C1":"opening","C":"body",)"
                  R"("C4":"body","C2":"divider"}})");

    const std::vector<std::string> expected = { "opening", "divider", "body", "closing" };
    LOK_ASSERT_EQUAL(expected.size(), design.parts.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
        LOK_ASSERT_EQUAL(expected[i], design.parts[i]);
}

void McpDeckTests::testUnknownPartNamesAreLeftOut()
{
    constexpr std::string_view testname = __func__;

    // "other" is a utility master that plays no part in a deck, and "sidebar" is not a part name at
    // all. Neither is offered to a caller.
    const McpDeck::DesignTemplate design = McpDeck::designTemplateFromManifest(
        "Mixed", R"({"schemaVersion":1,"masters":{"M1":"body","M2":"other","M3":"sidebar"}})");

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), design.parts.size());
    LOK_ASSERT(covers(design, "body"));
}

void McpDeckTests::testArtDirectionIsCleanedOfControlCharacters()
{
    constexpr std::string_view testname = __func__;

    // The style sentence comes from a template file an integrator may edit and goes on to compose
    // an image prompt, so line breaks and runs of spaces in it collapse to single spaces.
    const McpDeck::DesignTemplate design = McpDeck::designTemplateFromManifest(
        "Ivory", "{\"schemaVersion\":1,\"artDirection\":\"  warm\\n\\nivory   tones \"}");

    LOK_ASSERT_EQUAL(std::string("warm ivory tones"), design.artDirection);
}

void McpDeckTests::testManifestLowersALimitButCannotRaiseOne()
{
    constexpr std::string_view testname = __func__;

    const DeckSpec::Budgets configured = DeckSpec::budgetsFromConfig();
    const McpDeck::DesignTemplate design = McpDeck::designTemplateFromManifest(
        "Slate", R"({"schemaVersion":1,"budgets":{"maxSlides":3,"maxItemsPerBullets":99}})");

    LOK_ASSERT_EQUAL(3, design.budgets.maxSlides);
    LOK_ASSERT_EQUAL(configured.maxItemsPerBullets, design.budgets.maxItemsPerBullets);
}

namespace
{
/// A deck spec built from slide objects given as JSON text.
Poco::JSON::Object::Ptr specOf(const std::vector<std::string>& slides)
{
    std::string json = R"({"slides":[)";
    for (std::size_t i = 0; i < slides.size(); ++i)
        json += (i ? "," : "") + slides[i];
    json += "]}";

    Poco::JSON::Object::Ptr spec = new Poco::JSON::Object();
    JsonUtil::parseJSON(json, spec);
    return spec;
}

/// A bullets slide that satisfies every default budget.
std::string bulletsSlide(const std::string& title)
{
    return R"({"part":"body","intent":"bullets","title":")" + title +
           R"(","blocks":[{"kind":"bullets","items":["one","two"]}]})";
}

/// The SlideCommands array of a compiled deck.
Poco::JSON::Array::Ptr commandsOf(const McpDeck::CompiledDeck& deck)
{
    Poco::JSON::Object::Ptr transform = new Poco::JSON::Object();
    JsonUtil::parseJSON(deck.transformJSON, transform);
    Poco::JSON::Object::Ptr transforms = transform->getObject("Transforms");
    return transforms ? transforms->getArray("SlideCommands") : nullptr;
}

/// How many commands of the deck carry the named key.
int countCommand(const McpDeck::CompiledDeck& deck, const std::string& key)
{
    Poco::JSON::Array::Ptr commands = commandsOf(deck);
    int found = 0;
    for (std::size_t i = 0; commands && i < commands->size(); ++i)
    {
        Poco::JSON::Object::Ptr command = commands->getObject(i);
        if (command && command->has(key))
            ++found;
    }
    return found;
}
}

void McpDeckTests::testADeckNeedsAtLeastOneSlide()
{
    constexpr std::string_view testname = __func__;

    const McpDeck::CompiledDeck deck =
        McpDeck::compileDeck(specOf({}), McpDeck::designTemplateFromManifest("D", std::string()));

    LOK_ASSERT(!deck.ok);
    LOK_ASSERT(!deck.errorText.empty());
}

void McpDeckTests::testADeckOverTheSlideLimitIsRefused()
{
    constexpr std::string_view testname = __func__;

    // The design's own limit is what a deck is measured against, so a manifest that asks for two
    // slides refuses a third.
    const McpDeck::DesignTemplate design = McpDeck::designTemplateFromManifest(
        "Small", R"({"schemaVersion":1,"budgets":{"maxSlides":2}})");
    const McpDeck::CompiledDeck deck = McpDeck::compileDeck(
        specOf({ bulletsSlide("One"), bulletsSlide("Two"), bulletsSlide("Three") }), design);

    LOK_ASSERT(!deck.ok);
    LOK_ASSERT(deck.errorText.find("at most 2 slides") != std::string::npos);
}

void McpDeckTests::testAFaultyLastSlideRefusesTheWholeDeck()
{
    constexpr std::string_view testname = __func__;

    // Nothing is built until every slide has passed, so a fault on the last slide leaves no
    // half-built deck behind. The message names the slide the caller has to fix, counted from one.
    const McpDeck::CompiledDeck deck = McpDeck::compileDeck(
        specOf({ bulletsSlide("Good"),
                 R"({"part":"body","intent":"nonsense","title":"Bad"})" }),
        McpDeck::designTemplateFromManifest("D", std::string()));

    LOK_ASSERT(!deck.ok);
    LOK_ASSERT(deck.errorText.find("Slide 2") != std::string::npos);
    LOK_ASSERT(deck.transformJSON.empty());
}

void McpDeckTests::testASlideAskingForAPictureIsRefused()
{
    constexpr std::string_view testname = __func__;

    // There is no picture generator behind this path, so an image slide is turned down with a
    // message that says what to do instead.
    const McpDeck::CompiledDeck deck = McpDeck::compileDeck(
        specOf({ R"({"part":"body","intent":"image","title":"A view",)"
                 R"("image":{"brief":"a wide valley","alt":"a valley"}})" }),
        McpDeck::designTemplateFromManifest("D", std::string()));

    LOK_ASSERT(!deck.ok);
    LOK_ASSERT(deck.errorText.find("generated picture") != std::string::npos);
}

void McpDeckTests::testTheFirstSlideFillsTheStartingSlideAndTheRestAreAdded()
{
    constexpr std::string_view testname = __func__;

    // A new presentation already has one slide, so the first slide of the deck fills it and only
    // the other two are inserted.
    const McpDeck::CompiledDeck deck = McpDeck::compileDeck(
        specOf({ bulletsSlide("One"), bulletsSlide("Two"), bulletsSlide("Three") }),
        McpDeck::designTemplateFromManifest("D", std::string()));

    LOK_ASSERT(deck.ok);
    LOK_ASSERT_EQUAL(2, countCommand(deck, "JumpToSlide"));
}

void McpDeckTests::testANamedDesignLeadsTheCommands()
{
    constexpr std::string_view testname = __func__;

    // The design is applied once, before any slide is built, so every slide the deck produces lands
    // on the template's masters.
    McpDeck::DesignTemplate design = McpDeck::designTemplateFromManifest("Cobalt", std::string());
    const McpDeck::CompiledDeck deck =
        McpDeck::compileDeck(specOf({ bulletsSlide("One"), bulletsSlide("Two") }), design);

    LOK_ASSERT(deck.ok);
    LOK_ASSERT_EQUAL(1, countCommand(deck, "ApplyTemplate"));

    Poco::JSON::Array::Ptr commands = commandsOf(deck);
    LOK_ASSERT(commands && commands->size() > 0);
    LOK_ASSERT_EQUAL(std::string("Cobalt"),
                     commands->getObject(0)->getValue<std::string>("ApplyTemplate"));
}

void McpDeckTests::testAnUnstyledDeckCarriesNoDesignCommand()
{
    constexpr std::string_view testname = __func__;

    McpDeck::DesignTemplate none;
    none.budgets = DeckSpec::budgetsFromConfig();
    const McpDeck::CompiledDeck deck = McpDeck::compileDeck(specOf({ bulletsSlide("One") }), none);

    LOK_ASSERT(deck.ok);
    LOK_ASSERT_EQUAL(0, countCommand(deck, "ApplyTemplate"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(McpDeckTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
