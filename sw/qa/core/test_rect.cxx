/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <swrect.hxx>

class RectUnittest : public CppUnit::TestFixture
{
public:
    void testBasic();

    CPPUNIT_TEST_SUITE(RectUnittest);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST_SUITE_END();
};

void RectUnittest::testBasic()
{
    SwRect rect(Point(10, 15), Size(20, 25));

    CPPUNIT_ASSERT_EQUAL(rect, SwRect(10, 15, 20, 25));
    CPPUNIT_ASSERT_EQUAL(rect, SwRect(Point(10, 15), Point(10 + 20 - 1, 15 + 25 - 1)));

    CPPUNIT_ASSERT_EQUAL(tools::Long(20), rect.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(25), rect.Height());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), rect.Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(15), rect.Top());
    CPPUNIT_ASSERT_EQUAL(tools::Long(10 + 20 - 1), rect.Right());
    CPPUNIT_ASSERT_EQUAL(tools::Long(15 + 25 - 1), rect.Bottom());

    CPPUNIT_ASSERT_EQUAL(Point(rect.Left(), rect.Top()), rect.TopLeft());
    CPPUNIT_ASSERT_EQUAL(Point(rect.Right(), rect.Top()), rect.TopRight());
    CPPUNIT_ASSERT_EQUAL(Point(rect.Left(), rect.Bottom()), rect.BottomLeft());
    CPPUNIT_ASSERT_EQUAL(Point(rect.Right(), rect.Bottom()), rect.BottomRight());
}

CPPUNIT_TEST_SUITE_REGISTRATION(RectUnittest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
