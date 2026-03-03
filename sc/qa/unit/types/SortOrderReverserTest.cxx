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

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testInsertedRows)
{
    // Insert after sort range - no change
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        aReverser.insertedRows(10, 2);
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT_EQUAL(size_t(4), aReverser.maSortInfo.maOrder.size());
        // Order values unchanged
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[0]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[1]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(4), aReverser.maSortInfo.maOrder[2]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[3]);
    }

    // Insert before sort range - shift the whole range
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        aReverser.insertedRows(2, 3);
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(11), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT_EQUAL(size_t(4), aReverser.maSortInfo.maOrder.size());
        // Order values unchanged
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[0]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[1]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(4), aReverser.maSortInfo.maOrder[2]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[3]);
    }

    // Insert at start of sort range - expand the range
    {
        sc::SortOrderReverser aReverser;
        // Order {3, 1, 4, 2}
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        // Insert 1 row at row 5
        aReverser.insertedRows(5, 1);
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.maSortInfo.mnFirstRow); // unchanged
        CPPUNIT_ASSERT_EQUAL(SCROW(9), aReverser.maSortInfo.mnLastRow); // expanded by 1
        CPPUNIT_ASSERT_EQUAL(size_t(5), aReverser.maSortInfo.maOrder.size()); // 4 + 1 new
        // Shift all >=1 by 1, append 1
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(4), aReverser.maSortInfo.maOrder[0]); // 3 -> 4
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[1]); // 1 -> 2
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(5), aReverser.maSortInfo.maOrder[2]); // 4 -> 5
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[3]); // 2 -> 3
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[4]); // new, 1
    }

    // Insert at end of sort range - expand the range
    {
        sc::SortOrderReverser aReverser;
        // Order {3, 1, 4, 2}
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        // Insert 1 row at row 8
        aReverser.insertedRows(8, 1);
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(9), aReverser.maSortInfo.mnLastRow); // expanded by 1
        CPPUNIT_ASSERT_EQUAL(size_t(5), aReverser.maSortInfo.maOrder.size()); // 4 + 1 new
        // Shift all >=4 by 1, append 4
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[0]); // no change
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[1]); // no change
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(5), aReverser.maSortInfo.maOrder[2]); // 4 -> 5
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[3]); // no change
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(4), aReverser.maSortInfo.maOrder[4]); // new, 4
    }

    // Insert within sort range - expand and update order
    {
        sc::SortOrderReverser aReverser;
        // Order {3, 1, 4, 2}
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        // Insert 2 rows at row 7 (offset 3 in 1-based = within range)
        aReverser.insertedRows(7, 2);

        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(10), aReverser.maSortInfo.mnLastRow); // expanded by 2
        CPPUNIT_ASSERT_EQUAL(size_t(6), aReverser.maSortInfo.maOrder.size()); // 4 + 2 new

        // Shift all >=3 by 2, append 3 and 4
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(5), aReverser.maSortInfo.maOrder[0]); // 3 -> 5
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[1]); // no change
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(6), aReverser.maSortInfo.maOrder[2]); // 4 -> 6
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[3]); // no change
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[4]); // new, 3
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(4), aReverser.maSortInfo.maOrder[5]); // new, 4
    }
}

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testDeletedRows)
{
    // Delete after sort range - no change
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        aReverser.deletedRows(10, 2);
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT_EQUAL(size_t(4), aReverser.maSortInfo.maOrder.size());
        // Order values unchanged
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[0]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[1]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(4), aReverser.maSortInfo.maOrder[2]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[3]);
    }

    // Delete before sort range - shift the whole range
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        aReverser.deletedRows(2, 2);
        CPPUNIT_ASSERT_EQUAL(SCROW(3), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT_EQUAL(size_t(4), aReverser.maSortInfo.maOrder.size());
        // Order values unchanged
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[0]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[1]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(4), aReverser.maSortInfo.maOrder[2]);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[3]);
    }

    // Delete overlaps start - truncate from top
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        // Delete rows 4-5: row 4 is before range, row 5 is first row in range
        aReverser.deletedRows(4, 2);
        CPPUNIT_ASSERT_EQUAL(SCROW(4), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT_EQUAL(size_t(3), aReverser.maSortInfo.maOrder.size());
        // Remove value 1, shift remaining by -1
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[0]); // 3 -> 2
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[1]); // 4 -> 3
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[2]); // 2 -> 1
    }

    // Delete entire sort range - clear everything
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        aReverser.deletedRows(3, 8);
        CPPUNIT_ASSERT_EQUAL(SCROW(0), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(0), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT(aReverser.maSortInfo.maOrder.empty());
    }

    // Delete within sort range - shrink and update order
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        // Delete row 6 (offset 2 within range 5-8)
        aReverser.deletedRows(6, 1);
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT_EQUAL(size_t(3), aReverser.maSortInfo.maOrder.size());
        // Remove value 2 (offset 2), shift values >= 3 by -1
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[0]); // 3 -> 2
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[1]); // unchanged
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(3), aReverser.maSortInfo.maOrder[2]); // 4 -> 3
    }

    // Delete overlaps end - truncate from bottom
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        // Delete rows 7-10: rows 7-8 are within range, 9-10 are after
        aReverser.deletedRows(7, 4);
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT_EQUAL(size_t(2), aReverser.maSortInfo.maOrder.size());
        // Remove values 3 and 4 (offset 3, count 2)
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[0]); // unchanged
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[1]); // unchanged
    }

    // Delete multiple rows within sort range - shrink and update order
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 0, 0, 5, 8, { 3, 1, 4, 2 }, {} });
        // Delete rows 6-7 (offsets 2-3 within range 5-8)
        aReverser.deletedRows(6, 2);
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.maSortInfo.mnFirstRow);
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.maSortInfo.mnLastRow);
        CPPUNIT_ASSERT_EQUAL(size_t(2), aReverser.maSortInfo.maOrder.size());
        // Remove values 2 and 3 (offset 2, count 2)
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(1), aReverser.maSortInfo.maOrder[0]); // unchanged
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aReverser.maSortInfo.maOrder[1]); // 4 -> 2
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
