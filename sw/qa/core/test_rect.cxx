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
    void testUnion();
    void testIntersection();

    CPPUNIT_TEST_SUITE(RectUnittest);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testUnion);
    CPPUNIT_TEST(testIntersection);
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

void RectUnittest::testUnion()
{
    SwRect rect1(Point(10, 10), Size(10, 10));
    SwRect rect2(Point(15, 15), Size(10, 10));
    SwRect rect3(Point(30, 30), Size(10, 10));
    SwRect tmp;

    tmp = rect1;
    tmp.Union(rect2);
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(10, 10), Size(15, 15)), tmp);
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(10, 10), Size(15, 15)), rect1.GetUnion(rect2));

    tmp = rect1;
    tmp.Union(rect3);
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(10, 10), Size(30, 30)), tmp);
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(10, 10), Size(30, 30)), rect1.GetUnion(rect3));

    tmp = rect1;
    tmp.Union(SwRect());
    CPPUNIT_ASSERT_EQUAL(rect1, tmp);
    CPPUNIT_ASSERT_EQUAL(rect1, rect1.GetUnion(SwRect()));
    tmp = SwRect();
    tmp.Union(rect1);
    CPPUNIT_ASSERT_EQUAL(rect1, tmp);
    CPPUNIT_ASSERT_EQUAL(rect1, SwRect().GetUnion(rect1));
}

void RectUnittest::testIntersection()
{
    SwRect rect1(Point(10, 10), Size(10, 10));
    SwRect rect2(Point(15, 15), Size(10, 10));
    SwRect rect3(Point(30, 30), Size(10, 10));
    SwRect tmp;

    tmp = rect1;
    tmp.Intersection(rect2);
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(15, 15), Size(5, 5)), tmp);
    CPPUNIT_ASSERT_EQUAL(SwRect(Point(15, 15), Size(5, 5)), rect1.GetIntersection(rect2));

    tmp = rect1;
    tmp.Intersection(rect3);
    CPPUNIT_ASSERT(tmp.IsEmpty());
    CPPUNIT_ASSERT(rect1.GetIntersection(rect3).IsEmpty());

    tmp = rect1;
    tmp.Intersection(SwRect());
    CPPUNIT_ASSERT(tmp.IsEmpty());
    CPPUNIT_ASSERT(rect1.GetIntersection(SwRect()).IsEmpty());
    tmp = SwRect();
    tmp.Intersection(rect1);
    CPPUNIT_ASSERT(tmp.IsEmpty());
    CPPUNIT_ASSERT(SwRect().GetIntersection(rect1).IsEmpty());
}

CPPUNIT_TEST_SUITE_REGISTRATION(RectUnittest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
