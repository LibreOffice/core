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

#include <wsd/DeckSpec.hpp>
#include <wsd/McpDeck.hpp>

#include <test/lokassert.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <algorithm>
#include <string>

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
    CPPUNIT_TEST_SUITE_END();

    void testDesignWithoutManifestKeepsOnlyItsName();
    void testUnparsableManifestLeavesTheDesignBare();
    void testManifestBelowSchemaVersionOneIsIgnored();
    void testPartsFollowTheDeckOrderNotTheManifestOrder();
    void testUnknownPartNamesAreLeftOut();
    void testArtDirectionIsCleanedOfControlCharacters();
    void testManifestLowersALimitButCannotRaiseOne();

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

CPPUNIT_TEST_SUITE_REGISTRATION(McpDeckTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
