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

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_rectangle);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test_rectangle()
{
    {
        tools::Rectangle aRect(1, 1, 1, 1);

        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());

        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getHeight());
    }

    {
        tools::Rectangle aRect(Point(), Size(1, 1));

        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Left());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Top());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Right());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.Bottom());

        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());

        // getWidth and getHeight return the size that excludes one of the bounds,
        // unlike the ctor and GetWidth / GetHeight that operate on inclusive size
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getWidth());
        CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRect.getHeight());

        aRect.SetPosX(12);
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetHeight());
        aRect.SetPosY(12);
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aRect.GetWidth());
    }

    {
        constexpr tools::Rectangle aRectTwip(1, 1, 1, 1);
        constexpr tools::Rectangle aRectMm100(
            o3tl::convert(aRectTwip, o3tl::Length::twip, o3tl::Length::mm100));
        static_assert(!aRectMm100.IsEmpty());
        // Make sure that we use coordinates for conversion, not width/height:
        // the latter is ambiguous, and e.g. GetWidth(aRectTwip) gives 1, which
        // would had been converted to 2, resulting in different LR coordinates
        // TODO: static_assert(aRectMm100.Left() == aRectMm100.Right());
        // TODO: static_assert(aRectMm100.Top() == aRectMm100.Bottom());
    }

    {
        constexpr tools::Rectangle aRectTwip(1, 1);
        constexpr tools::Rectangle aRectMm100(
            o3tl::convert(aRectTwip, o3tl::Length::twip, o3tl::Length::mm100));
        // Make sure that result keeps the empty flag
        static_assert(aRectMm100.IsEmpty());
        static_assert(aRectMm100.GetWidth() == 0);
        static_assert(aRectMm100.GetHeight() == 0);
    }

    {
        constexpr tools::Rectangle aRect(Point(), Size(-1, -2));
        static_assert(!aRect.IsEmpty());
        // TODO: static_assert(aRect.Right() == 0);
        // TODO: static_assert(aRect.Bottom() == -1);

        // TODO: tools::Rectangle aRect2;
        // TODO: aRect2.SetSize(Size(-1, -2));
        // TODO: CPPUNIT_ASSERT_EQUAL(aRect, aRect2);
    }

    {
        tools::Rectangle aRect; // empty
        aRect.SetRight(0); // height is still empty
        const Point ptMove{ 1, 1 };
        tools::Rectangle aRect2 = aRect + ptMove;
        aRect += ptMove;
        // operator + and operator += must be consistent
        CPPUNIT_ASSERT_EQUAL(aRect, aRect2);
        aRect2 = aRect - ptMove;
        aRect -= ptMove;
        // operator - and operator -= must be consistent
        CPPUNIT_ASSERT_EQUAL(aRect, aRect2);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
