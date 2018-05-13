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
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>
#include <tools/Rectangle.hxx>

namespace
{

class TestRect: public CppUnit::TestFixture
{
public:
    void test_rectangle()
    {
        {
            tools::Rectangle aRect(1,1,1,1);

            CPPUNIT_ASSERT_EQUAL(long(1), aRect.GetWidth());
            CPPUNIT_ASSERT_EQUAL(long(1), aRect.GetHeight());

            CPPUNIT_ASSERT_EQUAL(long(0), aRect.getWidth());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.getHeight());
        }

        {
            tools::Rectangle aRect(Point(), Size(1,1));

            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Bottom());

            CPPUNIT_ASSERT_EQUAL(long(1), aRect.GetWidth());
            CPPUNIT_ASSERT_EQUAL(long(1), aRect.GetHeight());

            aRect.setX(12);
            CPPUNIT_ASSERT_EQUAL(long(1), aRect.GetHeight());
            aRect.setY(12);
            CPPUNIT_ASSERT_EQUAL(long(1), aRect.GetWidth());
        }
    }

    void test_empty()
    {
        {
            tools::Rectangle aRect;

            CPPUNIT_ASSERT(aRect.IsEmpty());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(-32767), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(-32767), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(0, 0, 10, 10);
            aRect += Point(10, 10);

            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }
        {
            tools::Rectangle aRect(10, 10, 20, 20);
            aRect.SetEmpty();
            CPPUNIT_ASSERT(aRect.IsWidthEmpty());
            CPPUNIT_ASSERT(aRect.IsHeightEmpty());
            CPPUNIT_ASSERT(aRect.IsEmpty());
        }

        {
            tools::Rectangle aRect(10, 10, 20, 20);
            aRect.SetWidthEmpty();
            CPPUNIT_ASSERT(aRect.IsWidthEmpty());
            CPPUNIT_ASSERT(!aRect.IsHeightEmpty());
            CPPUNIT_ASSERT(aRect.IsEmpty());
        }

        {
            tools::Rectangle aRect(10, 10, 20, 20);
            aRect.SetHeightEmpty();
            CPPUNIT_ASSERT(!aRect.IsWidthEmpty());
            CPPUNIT_ASSERT(aRect.IsHeightEmpty());
            CPPUNIT_ASSERT(aRect.IsEmpty());
        }
    }

    void test_setcorners()
    {
        tools::Rectangle aRect;
        aRect.SetLeft(10);
        aRect.SetRight(20);
        aRect.SetTop(30);
        aRect.SetBottom(40);

        CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
        CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
        CPPUNIT_ASSERT_EQUAL(long(30), aRect.Top());
        CPPUNIT_ASSERT_EQUAL(long(40), aRect.Bottom());
    }

    void test_getcorners()
    {
        {
            tools::Rectangle aRect(10, 30, 20, 40);

            CPPUNIT_ASSERT_EQUAL(Point(10, 30), aRect.TopLeft());
            CPPUNIT_ASSERT_EQUAL(Point(20, 30), aRect.TopRight());
            CPPUNIT_ASSERT_EQUAL(Point(15, 30), aRect.TopCenter());
            CPPUNIT_ASSERT_EQUAL(Point(10, 40), aRect.BottomLeft());
            CPPUNIT_ASSERT_EQUAL(Point(20, 40), aRect.BottomRight());
            CPPUNIT_ASSERT_EQUAL(Point(15, 40), aRect.BottomCenter());
            CPPUNIT_ASSERT_EQUAL(Point(10, 35), aRect.LeftCenter());
            CPPUNIT_ASSERT_EQUAL(Point(20, 35), aRect.RightCenter());
            CPPUNIT_ASSERT_EQUAL(Point(15, 35), aRect.Center());
        }

        {
            tools::Rectangle aRect(9, 29, 19, 39);

            CPPUNIT_ASSERT_EQUAL(Point(9, 29), aRect.TopLeft());
            CPPUNIT_ASSERT_EQUAL(Point(19, 29), aRect.TopRight());
            CPPUNIT_ASSERT_EQUAL(Point(14, 29), aRect.TopCenter());
            CPPUNIT_ASSERT_EQUAL(Point(9, 39), aRect.BottomLeft());
            CPPUNIT_ASSERT_EQUAL(Point(19, 39), aRect.BottomRight());
            CPPUNIT_ASSERT_EQUAL(Point(14, 39), aRect.BottomCenter());
            CPPUNIT_ASSERT_EQUAL(Point(9, 34), aRect.LeftCenter());
            CPPUNIT_ASSERT_EQUAL(Point(19, 34), aRect.RightCenter());
            CPPUNIT_ASSERT_EQUAL(Point(14, 34), aRect.Center());
        }
    }

    void test_move()
    {
        {
            tools::Rectangle aRect(10, 10, 20, 20);
            aRect.Move(10, 10);

            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(30), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(30), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(10, 10, 20, 20);
            aRect.Move(Size(10, 10));

            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(30), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(30), aRect.Bottom());
        }
    }

    void test_adjust()
    {
        {
            tools::Rectangle aRect(10, 10, 20, 20);
            CPPUNIT_ASSERT_EQUAL(long(15), aRect.AdjustLeft(5));
            CPPUNIT_ASSERT_EQUAL(long(15), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(10, 10, 20, 20);
            CPPUNIT_ASSERT_EQUAL(long(25), aRect.AdjustRight(5));
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(25), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(10, 10, 20, 20);
            CPPUNIT_ASSERT_EQUAL(long(15), aRect.AdjustTop(5));
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(15), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(10, 10, 20, 20);
            CPPUNIT_ASSERT_EQUAL(long(25), aRect.AdjustBottom(5));
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(25), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(0, 0, 10, 10);
            aRect += Point(10, 10);

            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(20, 20, 30, 30);
            aRect -= Point(10, 10);

            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }
    }

    void test_setpos()
    {
        tools::Rectangle aRect(10, 10, 20, 20);
        aRect.SetPos(Point(0, 0));

        CPPUNIT_ASSERT_EQUAL(long(0), aRect.Left());
        CPPUNIT_ASSERT_EQUAL(long(10), aRect.Right());
        CPPUNIT_ASSERT_EQUAL(long(0), aRect.Top());
        CPPUNIT_ASSERT_EQUAL(long(10), aRect.Bottom());
    }

    void test_setsize()
    {
        {
            tools::Rectangle aRect(0, 0, 10, 10);
            aRect.SetSize(Size(10, 10));

            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(9), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(9), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(0, 0, 10, 10);
            aRect.SetSize(Size(-10, -10));

            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(-9), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(-9), aRect.Bottom());
        }
    }

    void test_getsize()
    {
        tools::Rectangle aRect(0, 0, 10, 10);
        Size aSize(aRect.GetSize());

        CPPUNIT_ASSERT_EQUAL(long(11), aSize.getHeight());
        CPPUNIT_ASSERT_EQUAL(long(11), aSize.getWidth());
    }

    void test_union()
    {
        {
            tools::Rectangle aRect;
            tools::Rectangle aRectUnion(aRect.Union(tools::Rectangle(10, 10, 20, 20)));

            CPPUNIT_ASSERT_EQUAL(aRect, aRectUnion);
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(10, 10, 20, 20);
            tools::Rectangle aRectUnion;

            aRect.Union(aRectUnion);
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }

        {
            tools::Rectangle aRect(0, 0, 10, 10);
            aRect.Union(tools::Rectangle(10, 10, 20, 20));

            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Right());
            CPPUNIT_ASSERT_EQUAL(long(0), aRect.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aRect.Bottom());
        }
    }

    void test_justify()
    {
        tools::Rectangle aRect(10, 10, 0, 0);

        CPPUNIT_ASSERT_EQUAL(long(10), aRect.Left());
        CPPUNIT_ASSERT_EQUAL(long(0), aRect.Right());
        CPPUNIT_ASSERT_EQUAL(long(10), aRect.Top());
        CPPUNIT_ASSERT_EQUAL(long(0), aRect.Bottom());

        aRect.Justify();

        CPPUNIT_ASSERT_EQUAL(long(0), aRect.Left());
        CPPUNIT_ASSERT_EQUAL(long(10), aRect.Right());
        CPPUNIT_ASSERT_EQUAL(long(0), aRect.Top());
        CPPUNIT_ASSERT_EQUAL(long(10), aRect.Bottom());
    }

    void test_intersection()
    {
        {
            tools::Rectangle aRect(0, 0, 100, 100);
            tools::Rectangle aIntersection(aRect.Intersection(tools::Rectangle(10, 10, 20, 20)));

            CPPUNIT_ASSERT_EQUAL(long(10), aIntersection.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aIntersection.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aIntersection.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aIntersection.Bottom());
        }

        {
            tools::Rectangle aRect(100, 100, 0, 0);
            tools::Rectangle aIntersection(aRect.Intersection(tools::Rectangle(10, 10, 20, 20)));

            CPPUNIT_ASSERT_EQUAL(long(10), aIntersection.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aIntersection.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aIntersection.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aIntersection.Bottom());
        }

        {
            tools::Rectangle aRect(0, 0, 100, 100);
            tools::Rectangle aIntersection(aRect.Intersection(tools::Rectangle(20, 20, 10, 10)));

            CPPUNIT_ASSERT_EQUAL(long(10), aIntersection.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aIntersection.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aIntersection.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aIntersection.Bottom());
        }

        {
            tools::Rectangle aRect(0, 0, 100, 100);
            tools::Rectangle aIntersection(aRect.Intersection(tools::Rectangle(10, 10, 20, 20)));

            CPPUNIT_ASSERT_EQUAL(long(10), aIntersection.Left());
            CPPUNIT_ASSERT_EQUAL(long(20), aIntersection.Right());
            CPPUNIT_ASSERT_EQUAL(long(10), aIntersection.Top());
            CPPUNIT_ASSERT_EQUAL(long(20), aIntersection.Bottom());
        }

        {
            tools::Rectangle aRect(0, 0, 10, 10);
            tools::Rectangle aIntersection(aRect.Intersection(tools::Rectangle(20, 20, 30, 30)));

            CPPUNIT_ASSERT(aIntersection.IsEmpty());
        }
    }

    void test_isinside()
    {
        tools::Rectangle aRect(0, 0, 100, 100);
        CPPUNIT_ASSERT(aRect.IsInside(tools::Rectangle(10, 10, 20, 20)));
        CPPUNIT_ASSERT(aRect.IsInside(Point(10, 10)));

        CPPUNIT_ASSERT(!aRect.IsInside(tools::Rectangle(110, 110, 120, 120)));
        CPPUNIT_ASSERT(!aRect.IsInside(tools::Rectangle(10, 10, 120, 120)));
        CPPUNIT_ASSERT(!aRect.IsInside(Point(110, 110)));
    }

    void test_isover()
    {
        tools::Rectangle aRect(10, 10, 20, 20);
        CPPUNIT_ASSERT(aRect.IsOver(tools::Rectangle(0, 0, 100, 100)));
        CPPUNIT_ASSERT(aRect.IsOver(tools::Rectangle(15, 15, 25, 25)));
        CPPUNIT_ASSERT(!aRect.IsOver(tools::Rectangle(0, 0, 5, 5)));
    }

    void test_equality()
    {
        tools::Rectangle aRect1(10, 10, 20, 20);
        tools::Rectangle aRect2(10, 10, 20, 20);
        tools::Rectangle aRect3(15, 15, 25, 25);

        CPPUNIT_ASSERT_EQUAL(aRect1, aRect2);
        CPPUNIT_ASSERT(aRect1 != aRect3);
    }

    CPPUNIT_TEST_SUITE(TestRect);
    CPPUNIT_TEST(test_rectangle);
    CPPUNIT_TEST(test_empty);
    CPPUNIT_TEST(test_setcorners);
    CPPUNIT_TEST(test_getcorners);
    CPPUNIT_TEST(test_move);
    CPPUNIT_TEST(test_adjust);
    CPPUNIT_TEST(test_setpos);
    CPPUNIT_TEST(test_setsize);
    CPPUNIT_TEST(test_getsize);
    CPPUNIT_TEST(test_union);
    CPPUNIT_TEST(test_justify);
    CPPUNIT_TEST(test_intersection);
    CPPUNIT_TEST(test_isinside);
    CPPUNIT_TEST(test_isover);
    CPPUNIT_TEST(test_equality);
    CPPUNIT_TEST_SUITE_END();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TestRect);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
