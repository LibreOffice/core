/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>

#include <vcl/animate/Animation.hxx>

class VclAnimationTest : public test::BootstrapFixture
{
public:
    VclAnimationTest()
        : BootstrapFixture(true, false)
    {
    }

    void testFrameCount();
    void testDisplaySize();

    CPPUNIT_TEST_SUITE(VclAnimationTest);
    CPPUNIT_TEST(testFrameCount);
    CPPUNIT_TEST(testDisplaySize);
    CPPUNIT_TEST_SUITE_END();
};

void VclAnimationTest::testFrameCount()
{
    Animation aAnimation;

    CPPUNIT_ASSERT_EQUAL(size_t(0), aAnimation.Count());

    aAnimation.Insert(
        AnimationFrame(BitmapEx(Size(3, 4), vcl::PixelFormat::N24_BPP), Point(0, 0), Size(3, 4)));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aAnimation.Count());

    aAnimation.Insert(
        AnimationFrame(BitmapEx(Size(3, 3), vcl::PixelFormat::N24_BPP), Point(0, 0), Size(10, 10)));
    CPPUNIT_ASSERT_EQUAL(size_t(2), aAnimation.Count());

    aAnimation.Clear();
    CPPUNIT_ASSERT_EQUAL(size_t(0), aAnimation.Count());
}

void VclAnimationTest::testDisplaySize()
{
    Animation aAnimation;
    CPPUNIT_ASSERT_EQUAL(Size(0, 0), aAnimation.GetDisplaySizePixel());

    aAnimation.Insert(
        AnimationFrame(BitmapEx(Size(3, 4), vcl::PixelFormat::N24_BPP), Point(0, 0), Size(3, 4)));
    CPPUNIT_ASSERT_EQUAL(Size(3, 4), aAnimation.GetDisplaySizePixel());

    aAnimation.Insert(AnimationFrame(BitmapEx(Size(10, 10), vcl::PixelFormat::N24_BPP), Point(0, 0),
                                     Size(10, 10)));
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), aAnimation.GetDisplaySizePixel());

    aAnimation.Clear();
    CPPUNIT_ASSERT_EQUAL(Size(0, 0), aAnimation.GetDisplaySizePixel());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclAnimationTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
