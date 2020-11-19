/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tools/poly.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
public:
    void testTdf137068();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testTdf137068);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testTdf137068()
{
    // Make sure PolyPolygon::Clip() does not break bezier curves.
    const Point points[] = { { 1337, 411 }, { 1337, 471 }, { 1313, 530 }, { 1268, 582 } };
    const PolyFlags flags[]
        = { PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control, PolyFlags::Normal };
    tools::Polygon polygon(4, points, flags);
    tools::PolyPolygon polyPolygon(polygon);
    polyPolygon.Clip(tools::Rectangle(Point(0, 0), Size(1920, 1080)));
    // operator== is stupid and just compares pointers, compare data manually
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), polyPolygon.Count());
    tools::Polygon result = polyPolygon.GetObject(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4), result.GetSize());
    CPPUNIT_ASSERT_EQUAL(points[0], result.GetPoint(0));
    CPPUNIT_ASSERT_EQUAL(points[1], result.GetPoint(1));
    CPPUNIT_ASSERT_EQUAL(points[2], result.GetPoint(2));
    CPPUNIT_ASSERT_EQUAL(points[3], result.GetPoint(3));
    CPPUNIT_ASSERT_EQUAL(flags[0], result.GetFlags(0));
    CPPUNIT_ASSERT_EQUAL(flags[1], result.GetFlags(1));
    CPPUNIT_ASSERT_EQUAL(flags[2], result.GetFlags(2));
    CPPUNIT_ASSERT_EQUAL(flags[3], result.GetFlags(3));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
