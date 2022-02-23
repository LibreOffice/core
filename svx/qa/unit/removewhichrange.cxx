/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>
#include <svx/svdetc.hxx>

namespace
{
class TestRemoveWhichRange : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestRemoveWhichRange);
    CPPUNIT_TEST(testRemoveWhichRange);
    CPPUNIT_TEST_SUITE_END();

    void testRemoveWhichRange()
    {
        {
            WhichRangesContainer in;
            auto const out = RemoveWhichRange(in, 10, 20);
            CPPUNIT_ASSERT(out.empty());
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 0, 20);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(40), out[0].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 10, 20);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(40), out[0].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 15, 20);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(14), out[0].second);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), out[1].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(40), out[1].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 30, 40);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(20), out[0].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 30, 50);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(20), out[0].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 30, 35);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(20), out[0].second);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(36), out[1].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(40), out[1].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 15, 35);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(14), out[0].second);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(36), out[1].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(40), out[1].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 12, 15);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(11), out[0].second);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(16), out[1].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(20), out[1].second);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(30), out[2].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(40), out[2].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 30, 40>);
            auto const out = RemoveWhichRange(in, 0, 100);
            CPPUNIT_ASSERT(out.empty());
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 40, 50>);
            auto const out = RemoveWhichRange(in, 25, 35);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(20), out[0].second);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(40), out[1].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(50), out[1].second);
        }
        {
            WhichRangesContainer in(svl::Items<10, 20, 40, 50>);
            auto const out = RemoveWhichRange(in, 50, 100);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), out[0].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(20), out[0].second);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(40), out[1].first);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(49), out[1].second);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestRemoveWhichRange);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
