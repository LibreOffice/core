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
#include <tools/gen.hxx>

namespace
{
class RectangleTest : public CppUnit::TestFixture
{
public:
    void testConstruction();
    void testOpenClosedSize();
    void testUnitConvesion();
    void testSetOperators();
    void test_rectnormalize_alreadynormal();
    void test_rectnormalize_zerorect();
    void test_rectnormalize_reverse_topleft_bottomright();
    void test_rectnormalize_topright_bottomleft();
    void test_rectnormalize_bottomleft_topright();
    void test_rectnormalize_zerowidth_top_bottom_reversed();
    void test_rectnormalize_zeroheight_left_right_reversed();

    CPPUNIT_TEST_SUITE(RectangleTest);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testOpenClosedSize);
    CPPUNIT_TEST(testUnitConvesion);
    CPPUNIT_TEST(testSetOperators);
    CPPUNIT_TEST(test_rectnormalize_zerorect);
    CPPUNIT_TEST(test_rectnormalize_alreadynormal);
    CPPUNIT_TEST(test_rectnormalize_reverse_topleft_bottomright);
    CPPUNIT_TEST(test_rectnormalize_topright_bottomleft);
    CPPUNIT_TEST(test_rectnormalize_bottomleft_topright);
    CPPUNIT_TEST(test_rectnormalize_zerowidth_top_bottom_reversed);
    CPPUNIT_TEST(test_rectnormalize_zeroheight_left_right_reversed);
    CPPUNIT_TEST_SUITE_END();
};

void RectangleTest::testConstruction()
{
    {
        tools::Rectangle aRect1(Point(), Size(0, 20));
        CPPUNIT_ASSERT_EQUAL(true, aRect1.IsEmpty());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect1.getOpenWidth());

        tools::Rectangle aRect2{ Point(), Point(0, 20) };
        CPPUNIT_ASSERT_EQUAL(false, aRect2.IsEmpty());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect2.getOpenWidth());

        tools::Rectangle aRect3(0, 0, 0, 20);
        CPPUNIT_ASSERT_EQUAL(false, aRect3.IsEmpty());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect3.getOpenWidth());
    }
    {
        static constexpr tools::Rectangle aRect(Point(), Size(-1, -2));
        static_assert(!aRect.IsEmpty());
        static_assert(aRect.Right() == 0);
        static_assert(aRect.Bottom() == -1);

        tools::Rectangle aRect2;
        aRect2.SetSize(Size(-1, -2));
        CPPUNIT_ASSERT_EQUAL(aRect, aRect2);

        static constexpr tools::Rectangle aRect3(Point(), Size(0, 0));
        static_assert(aRect3.IsEmpty());
        static_assert(aRect3.Right() == 0);
        static_assert(aRect3.Bottom() == 0);

        static constexpr tools::Rectangle aRect4(Point(), Size(1, 1));
        static_assert(!aRect4.IsEmpty());
        static_assert(aRect4.Right() == 0);
        static_assert(aRect4.Bottom() == 0);

        static constexpr tools::Rectangle aRect5(Point(), Size(-1, -1));
        static_assert(!aRect5.IsEmpty());
        static_assert(aRect5.Right() == 0);
        static_assert(aRect5.Bottom() == 0);
    }
}

void RectangleTest::testOpenClosedSize()
{
    {
        tools::Rectangle aRect(1, 1, 1, 1);

        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());

        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getOpenWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getOpenHeight());
    }

    {
        tools::Rectangle aRect(Point(), Size(1, 1));

        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Left());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Top());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Right());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Bottom());

        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());

        // getOpenWidth and getOpenHeight return the size that excludes one of the bounds,
        // unlike the ctor and GetWidth / GetHeight that operate on inclusive size
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getOpenWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getOpenHeight());

        aRect.SetPosX(12);
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());
        aRect.SetPosY(12);
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
    }
}

void RectangleTest::testUnitConvesion()
{
    {
        static constexpr tools::Rectangle aRectTwip(100, 100, 100, 100);
        static constexpr tools::Rectangle aRectMm100(
            o3tl::convert(aRectTwip, o3tl::Length::twip, o3tl::Length::mm100));
        static_assert(!aRectMm100.IsEmpty());
        // Make sure that we use coordinates for conversion, not width/height:
        // the latter is ambiguous, and e.g. GetWidth(aRectTwip) gives 1, which
        // would had been converted to 2, resulting in different LR coordinates
        static_assert(aRectMm100.Left() == aRectMm100.Right());
        static_assert(aRectMm100.Top() == aRectMm100.Bottom());
    }

    {
        static constexpr tools::Rectangle aRectTwip(1, 1);
        static constexpr tools::Rectangle aRectMm100(
            o3tl::convert(aRectTwip, o3tl::Length::twip, o3tl::Length::mm100));
        // Make sure that result keeps the empty flag
        static_assert(aRectMm100.IsEmpty());
        static_assert(aRectMm100.IsWidthEmpty());
        static_assert(aRectMm100.IsHeightEmpty());
        static_assert(aRectMm100.GetWidth() == 0);
        static_assert(aRectMm100.GetHeight() == 0);
    }
}

void RectangleTest::testSetOperators()
{
    static constexpr tools::Rectangle rect(Point(0, 0), Size(20, 20));
    static constexpr tools::Rectangle inside(Point(10, 10), Size(10, 10));
    static constexpr tools::Rectangle overlap(Point(10, 10), Size(20, 20));
    static constexpr tools::Rectangle outside(Point(20, 20), Size(10, 10));
    CPPUNIT_ASSERT(rect.Contains(inside));
    CPPUNIT_ASSERT(rect.Contains(rect));
    CPPUNIT_ASSERT(!rect.Contains(overlap));
    CPPUNIT_ASSERT(!rect.Contains(outside));
    CPPUNIT_ASSERT(rect.Overlaps(inside));
    CPPUNIT_ASSERT(rect.Overlaps(rect));
    CPPUNIT_ASSERT(rect.Overlaps(overlap));
    CPPUNIT_ASSERT(!rect.Overlaps(outside));
}

void RectangleTest::test_rectnormalize_alreadynormal()
{
    Point aTopLeft(0, 0);
    Point aBottomRight(1, 1);

    tools::Rectangle aRect(aTopLeft, aBottomRight);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL(aRect.TopLeft(), aTopLeft);
    CPPUNIT_ASSERT_EQUAL(aRect.BottomRight(), aBottomRight);
}

void RectangleTest::test_rectnormalize_zerorect()
{
    Point aTopLeft(53, 53);
    Point aBottomRight(53, 53);

    tools::Rectangle aRect(aTopLeft, aBottomRight);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL(aRect.TopLeft(), aTopLeft);
    CPPUNIT_ASSERT_EQUAL(aRect.BottomRight(), aBottomRight);
}

void RectangleTest::test_rectnormalize_reverse_topleft_bottomright()
{
    Point aPoint1(1, 1);
    Point aPoint2(0, 0);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", Point(0, 0), aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", Point(1, 1), aRect.BottomRight());
}

void RectangleTest::test_rectnormalize_topright_bottomleft()
{
    Point aPoint1(1, 0);
    Point aPoint2(0, 1);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", Point(0, 0), aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", Point(1, 1), aRect.BottomRight());
}

void RectangleTest::test_rectnormalize_bottomleft_topright()
{
    Point aPoint1(0, 1);
    Point aPoint2(1, 0);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", Point(0, 0), aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", Point(1, 1), aRect.BottomRight());
}

void RectangleTest::test_rectnormalize_zerowidth_top_bottom_reversed()
{
    Point aPoint1(0, 1);
    Point aPoint2(0, 0);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", Point(0, 0), aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", Point(0, 1), aRect.BottomRight());
}

void RectangleTest::test_rectnormalize_zeroheight_left_right_reversed()
{
    Point aPoint1(1, 0);
    Point aPoint2(0, 0);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", Point(0, 0), aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", Point(1, 0), aRect.BottomRight());
}

CPPUNIT_TEST_SUITE_REGISTRATION(RectangleTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
