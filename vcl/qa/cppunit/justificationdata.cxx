/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <justificationdata.hxx>

class JustificationDataTest : public CppUnit::TestFixture
{
public:
    void testEmpty()
    {
        JustificationData stJust;
        CPPUNIT_ASSERT(stJust.empty());

        JustificationData stJust2{ /*nBaseIndex*/ 100, /*nSize*/ 5 };
        CPPUNIT_ASSERT(!stJust2.empty());
    }

    void testContainsAdvances()
    {
        JustificationData stJust{ /*nBaseIndex*/ 100, /*nSize*/ 5 };
        CPPUNIT_ASSERT(!stJust.ContainsAdvances());

        // Try to set advances out of bounds
        stJust.SetTotalAdvance(10, 1.0);
        CPPUNIT_ASSERT(!stJust.ContainsAdvances());

        stJust.SetTotalAdvance(200, 1.0);
        CPPUNIT_ASSERT(!stJust.ContainsAdvances());

        // Insert in bounds
        stJust.SetTotalAdvance(102, 1.0);
        CPPUNIT_ASSERT(stJust.ContainsAdvances());
    }

    void testAdvances()
    {
        JustificationData stJust{ /*nBaseIndex*/ 100, /*nSize*/ 3 };
        stJust.SetTotalAdvance(99, 1.0);
        stJust.SetTotalAdvance(100, 2.0);
        stJust.SetTotalAdvance(101, 3.0);
        stJust.SetTotalAdvance(102, 4.0);
        stJust.SetTotalAdvance(103, 5.0);

        // Total advance before the valid range must be 0
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, stJust.GetTotalAdvance(99), 0.05);

        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, stJust.GetTotalAdvance(100), 0.05);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, stJust.GetTotalAdvance(101), 0.05);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, stJust.GetTotalAdvance(102), 0.05);

        // Total advance after the valid range must be the last advance
        CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, stJust.GetTotalAdvance(103), 0.05);
    }

    void testContainsKashidaPositions()
    {
        JustificationData stJust{ /*nBaseIndex*/ 100, /*nSize*/ 5 };
        CPPUNIT_ASSERT(!stJust.ContainsKashidaPositions());

        // Try to set kashida positions out of bounds
        stJust.SetKashidaPosition(10, true);
        CPPUNIT_ASSERT(!stJust.ContainsKashidaPositions());

        stJust.SetKashidaPosition(200, true);
        CPPUNIT_ASSERT(!stJust.ContainsKashidaPositions());

        // Insert in bounds
        stJust.SetKashidaPosition(102, true);
        CPPUNIT_ASSERT(stJust.ContainsKashidaPositions());
    }

    void testKashidaPositions()
    {
        JustificationData stJust{ /*nBaseIndex*/ 100, /*nSize*/ 1 };

        CPPUNIT_ASSERT(!stJust.GetPositionHasKashida(99).has_value());
        CPPUNIT_ASSERT(!stJust.GetPositionHasKashida(100).value_or(true));
        CPPUNIT_ASSERT(!stJust.GetPositionHasKashida(101).has_value());

        stJust.SetKashidaPosition(99, true);
        stJust.SetKashidaPosition(100, true);
        stJust.SetKashidaPosition(101, true);

        CPPUNIT_ASSERT(!stJust.GetPositionHasKashida(99).has_value());
        CPPUNIT_ASSERT(stJust.GetPositionHasKashida(100).value_or(false));
        CPPUNIT_ASSERT(!stJust.GetPositionHasKashida(101).has_value());

        stJust.SetKashidaPosition(100, false);

        CPPUNIT_ASSERT(!stJust.GetPositionHasKashida(100).value_or(true));
    }

    CPPUNIT_TEST_SUITE(JustificationDataTest);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testContainsAdvances);
    CPPUNIT_TEST(testAdvances);
    CPPUNIT_TEST(testContainsKashidaPositions);
    CPPUNIT_TEST(testKashidaPositions);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(JustificationDataTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
