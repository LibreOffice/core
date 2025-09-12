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
        aReverser.addOrderIndices({ 1, 2, 3, 4 }, 5, 8);

        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(5));
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.unsort(6));
        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.unsort(7));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(8));
    }

    // Reversed order - output rows should be in reverse order than input
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 4, 3, 2, 1 }, 5, 8);

        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(5));
        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.unsort(6));
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.unsort(7));
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(8));
    }

    // Some random order - output rows should follow the order
    {
        sc::SortOrderReverser aReverser;
        aReverser.addOrderIndices({ 3, 1, 4, 2 }, 5, 8);

        CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.unsort(5));
        CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(6));
        CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(7));
        CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.unsort(8));
    }
}

CPPUNIT_TEST_FIXTURE(SortOrderReverserTest, testCombiningSortOrder)
{
    sc::SortOrderReverser aReverser;

    // first sorting order
    aReverser.addOrderIndices({ 3, 1, 4, 2 }, 5, 8);

    // second sorting order - only reverses the order
    aReverser.addOrderIndices({ 4, 3, 2, 1 }, 5, 8);

    // check
    CPPUNIT_ASSERT_EQUAL(SCROW(6), aReverser.unsort(5));
    CPPUNIT_ASSERT_EQUAL(SCROW(8), aReverser.unsort(6));
    CPPUNIT_ASSERT_EQUAL(SCROW(5), aReverser.unsort(7));
    CPPUNIT_ASSERT_EQUAL(SCROW(7), aReverser.unsort(8));

    // directly compare the order
    std::vector<SCCOLROW> aExpectedOrder{ 2, 4, 1, 3 };
    CPPUNIT_ASSERT(
        std::equal(aExpectedOrder.begin(), aExpectedOrder.end(), aReverser.maOrder.begin()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
