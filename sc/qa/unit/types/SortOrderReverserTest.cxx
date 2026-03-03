/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <SheetView.hxx>

class SortOrderReverserTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testUnsort)
{
    // Unchanged order - output rows should be the same as input
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 1, 2, 3, 4 }, {} });

        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(5, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.unsort(6, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.unsort(7, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(8, 0));
    }

    // Reversed order - output rows should be in reverse order than input
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 4, 3, 2, 1 }, {} });

        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(5, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.unsort(6, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.unsort(7, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(8, 0));

        // Column outside sort area - no change
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(5, 1));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(8, 1));

        // Row outside sort area - no change
        CPPUNIT_ASSERT_EQUAL(SCROW(4), aReverser.unsort(4, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(9), aReverser.unsort(9, 0));
    }

    // Some random order - output rows should follow the order
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });

        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.unsort(5, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(6, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(7, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.unsort(8, 0));

        // Column outside sort area - no change
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(5, 1));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(8, 1));

        // Row outside sort area - no change
        CPPUNIT_ASSERT_EQUAL(SCROW(4), aReverser.unsort(4, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(9), aReverser.unsort(9, 0));
    }
}

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testCombiningSortOrder)
{
    sc::SortOrderReverser aReverser;

    // first sorting order
    aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });

    // second sorting order - only reverses the order
    aReverser.addOrderIndices({ 0, 0, 5, 8, { 4, 3, 2, 1 }, {} });

    // check
    CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.unsort(5, 0));
    CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(6, 0));
    CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(7, 0));
    CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.unsort(8, 0));

    // directly compare the order
    std::vector<SCCOLROW> aExpectedOrder{ 2, 4, 1, 3 };
    CPPUNIT_ASSERT(std::equal(aExpectedOrder.begin(), aExpectedOrder.end(),
                              aReverser.maSortInfo.maOrder.begin()));
}

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testCombiningDifferentKeyStatesReplaces)
{
    ScSortKeyState aKeyAscending;
    aKeyAscending.nField = 0;
    aKeyAscending.bDoSort = true;
    aKeyAscending.bAscending = true;
    aKeyAscending.aColorSortMode = ScColorSortMode::None;

    ScSortKeyState aKeyDescending;
    aKeyDescending.nField = 0;
    aKeyDescending.bDoSort = true;
    aKeyDescending.bAscending = false;
    aKeyDescending.aColorSortMode = ScColorSortMode::None;

    sc::SortOrderReverser aReverser;

    // First sort: ascending on column 0
    aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, { aKeyAscending } });

    // Second sort: descending on column 0 - different key state, should replace
    aReverser.addOrderIndices({ 0, 0, 5, 8, { 4, 3, 2, 1 }, { aKeyDescending } });

    // The order should be replaced, not merged - so it's the second sort's order
    std::vector<SCCOLROW> aExpectedOrder{ 4, 3, 2, 1 };
    CPPUNIT_ASSERT_EQUAL(aExpectedOrder.size(), aReverser.maSortInfo.maOrder.size());
    CPPUNIT_ASSERT(std::equal(aExpectedOrder.begin(), aExpectedOrder.end(),
                              aReverser.maSortInfo.maOrder.begin()));

    // And key state should be the second sort's
    CPPUNIT_ASSERT_EQUAL(size_t(1), aReverser.maSortInfo.maKeyStates.size());
    CPPUNIT_ASSERT_EQUAL(false, aReverser.maSortInfo.maKeyStates[0].bAscending);
}

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testCombiningSameKeyStatesMerges)
{
    ScSortKeyState aKeyAscending;
    aKeyAscending.nField = 0;
    aKeyAscending.bDoSort = true;
    aKeyAscending.bAscending = true;
    aKeyAscending.aColorSortMode = ScColorSortMode::None;

    sc::SortOrderReverser aReverser;

    // First sort: ascending on column 0
    aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, { aKeyAscending } });

    // Second sort: same key state, should merge
    aReverser.addOrderIndices({ 0, 0, 5, 8, { 4, 3, 2, 1 }, { aKeyAscending } });

    // The order should be merged (same as testCombiningSortOrder)
    std::vector<SCCOLROW> aExpectedOrder{ 2, 4, 1, 3 };
    CPPUNIT_ASSERT_EQUAL(aExpectedOrder.size(), aReverser.maSortInfo.maOrder.size());
    CPPUNIT_ASSERT(std::equal(aExpectedOrder.begin(), aExpectedOrder.end(),
                              aReverser.maSortInfo.maOrder.begin()));

    // Key state should be preserved
    CPPUNIT_ASSERT_EQUAL(size_t(1), aReverser.maSortInfo.maKeyStates.size());
    CPPUNIT_ASSERT_EQUAL(true, aReverser.maSortInfo.maKeyStates[0].bAscending);
}

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testCombiningDifferentRowRangeReplaces)
{
    sc::SortOrderReverser aReverser;

    // First sort on rows 5-8
    aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });

    // Second sort on rows 10-13 - different range, should replace
    aReverser.addOrderIndices({ 0, 0, 10, 13, { 4, 3, 2, 1 }, {} });

    // The order should be replaced with the second sort's values
    std::vector<SCCOLROW> aExpectedOrder{ 4, 3, 2, 1 };
    CPPUNIT_ASSERT_EQUAL(aExpectedOrder.size(), aReverser.maSortInfo.maOrder.size());
    CPPUNIT_ASSERT(std::equal(aExpectedOrder.begin(), aExpectedOrder.end(),
                              aReverser.maSortInfo.maOrder.begin()));
    CPPUNIT_ASSERT_EQUAL(SCROW(10), aReverser.maSortInfo.mnFirstRow);
    CPPUNIT_ASSERT_EQUAL(SCROW(13), aReverser.maSortInfo.mnLastRow);
}

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testResort)
{
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 4, 3, 2, 1 }, {} });

        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.resort(5, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.resort(6, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.resort(7, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.resort(8, 0));
    }

    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });

        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.resort(5, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.resort(6, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.resort(7, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.resort(8, 0));

        // Column outside sort area - no change
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.resort(5, 1));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.resort(8, 1));

        // Row outside sort area - no change
        CPPUNIT_ASSERT_EQUAL(SCROW(4), aReverser.resort(4, 0));
        CPPUNIT_ASSERT_EQUAL(SCROW(9), aReverser.resort(9, 0));
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
