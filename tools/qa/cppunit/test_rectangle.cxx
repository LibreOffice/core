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
class Test : public CppUnit::TestFixture
{
public:
    void test_rectangle();
    void test_normalize_alreadynormal();
    void test_normalize_zerorect();
    void test_normalize_reverse_topleft_bottomright();
    void test_normalize_topright_bottomleft();
    void test_normalize_bottomleft_topright();
    void test_normalize_zerowidth_top_bottom_reversed();
    void test_normalize_zeroheight_left_right_reversed();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_rectangle);
    CPPUNIT_TEST(test_normalize_zerorect);
    CPPUNIT_TEST(test_normalize_alreadynormal);
    CPPUNIT_TEST(test_normalize_reverse_topleft_bottomright);
    CPPUNIT_TEST(test_normalize_topright_bottomleft);
    CPPUNIT_TEST(test_normalize_bottomleft_topright);
    CPPUNIT_TEST(test_normalize_zerowidth_top_bottom_reversed);
    CPPUNIT_TEST(test_normalize_zeroheight_left_right_reversed);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test_rectangle()
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

    {
        constexpr tools::Rectangle aRect(Point(), Size(-1, -2));
        static_assert(!aRect.IsEmpty());
        static_assert(aRect.Right() == 0);
        static_assert(aRect.Bottom() == -1);

        tools::Rectangle aRect2;
        aRect2.SetSize(Size(-1, -2));
        CPPUNIT_ASSERT_EQUAL(aRect, aRect2);
    }

    {
        constexpr tools::Rectangle aRectTwip(100, 100, 100, 100);
        constexpr tools::Rectangle aRectMm100(
            o3tl::convert(aRectTwip, o3tl::Length::twip, o3tl::Length::mm100));
        static_assert(!aRectMm100.IsEmpty());
        // Make sure that we use coordinates for conversion, not width/height:
        // the latter is ambiguous, and e.g. GetWidth(aRectTwip) gives 1, which
        // would had been converted to 2, resulting in different LR coordinates
        static_assert(aRectMm100.Left() == aRectMm100.Right());
        static_assert(aRectMm100.Top() == aRectMm100.Bottom());
    }

    {
        constexpr tools::Rectangle aRectTwip(1, 1);
        constexpr tools::Rectangle aRectMm100(
            o3tl::convert(aRectTwip, o3tl::Length::twip, o3tl::Length::mm100));
        // Make sure that result keeps the empty flag
        static_assert(aRectMm100.IsEmpty());
        static_assert(aRectMm100.IsWidthEmpty());
        static_assert(aRectMm100.IsHeightEmpty());
        static_assert(aRectMm100.GetWidth() == 0);
        static_assert(aRectMm100.GetHeight() == 0);
    }

    {
        constexpr tools::Rectangle rect(Point(0, 0), Size(20, 20));
        constexpr tools::Rectangle inside(Point(10, 10), Size(10, 10));
        constexpr tools::Rectangle overlap(Point(10, 10), Size(20, 20));
        constexpr tools::Rectangle outside(Point(20, 20), Size(10, 10));
        CPPUNIT_ASSERT(rect.Contains(inside));
        CPPUNIT_ASSERT(rect.Contains(rect));
        CPPUNIT_ASSERT(!rect.Contains(overlap));
        CPPUNIT_ASSERT(!rect.Contains(outside));
        CPPUNIT_ASSERT(rect.Overlaps(inside));
        CPPUNIT_ASSERT(rect.Overlaps(rect));
        CPPUNIT_ASSERT(rect.Overlaps(overlap));
        CPPUNIT_ASSERT(!rect.Overlaps(outside));
    }
}

void Test::test_normalize_alreadynormal()
{
    Point aTopLeft(0, 0);
    Point aBottomRight(1, 1);

    tools::Rectangle aRect(aTopLeft, aBottomRight);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL(aRect.TopLeft(), aTopLeft);
    CPPUNIT_ASSERT_EQUAL(aRect.BottomRight(), aBottomRight);
}

void Test::test_normalize_zerorect()
{
    Point aTopLeft(53, 53);
    Point aBottomRight(53, 53);

    tools::Rectangle aRect(aTopLeft, aBottomRight);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL(aRect.TopLeft(), aTopLeft);
    CPPUNIT_ASSERT_EQUAL(aRect.BottomRight(), aBottomRight);
}

void Test::test_normalize_reverse_topleft_bottomright()
{
    Point aExpectedTopLeft(0, 0);
    Point aExpectedBottomRight(1, 1);

    Point aPoint1(1, 1);
    Point aPoint2(0, 0);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", aExpectedTopLeft, aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", aExpectedBottomRight,
                                 aRect.BottomRight());
}

void Test::test_normalize_topright_bottomleft()
{
    Point aExpectedTopLeft(0, 0);
    Point aExpectedBottomRight(1, 1);

    Point aPoint1(1, 0);
    Point aPoint2(0, 1);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", aExpectedTopLeft, aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", aExpectedBottomRight,
                                 aRect.BottomRight());
}

void Test::test_normalize_bottomleft_topright()
{
    Point aExpectedTopLeft(0, 0);
    Point aExpectedBottomRight(1, 1);

    Point aPoint1(0, 1);
    Point aPoint2(1, 0);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", aExpectedTopLeft, aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", aExpectedBottomRight,
                                 aRect.BottomRight());
}

void Test::test_normalize_zerowidth_top_bottom_reversed()
{
    Point aExpectedTopLeft(0, 0);
    Point aExpectedBottomRight(0, 1);

    Point aPoint1(0, 1);
    Point aPoint2(0, 0);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", aExpectedTopLeft, aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", aExpectedBottomRight,
                                 aRect.BottomRight());
}

void Test::test_normalize_zeroheight_left_right_reversed()
{
    Point aExpectedTopLeft(0, 0);
    Point aExpectedBottomRight(1, 0);

    Point aPoint1(1, 0);
    Point aPoint2(0, 0);

    tools::Rectangle aRect(aPoint1, aPoint2);
    aRect.Normalize();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("TopLeft() is wrong", aExpectedTopLeft, aRect.TopLeft());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("BottomRight() is wrong", aExpectedBottomRight,
                                 aRect.BottomRight());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
