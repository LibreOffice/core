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
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <impanmvw.hxx>

namespace
{
class TestRenderingContext : public OutputDevice
{
public:
    TestRenderingContext()
        : OutputDevice(OutDevType::OUTDEV_VIRDEV)
    {
    }

    void SaveBackground(VirtualDevice&, const Point&, const Size&, const Size&) const override {}
    bool AcquireGraphics() const override { return true; }
    void ReleaseGraphics(bool) override {}
    bool UsePolyPolygonForComplexGradient() override { return false; }
};
}

class VclImplAnimViewTest : public test::BootstrapFixture
{
public:
    VclImplAnimViewTest()
        : BootstrapFixture(true, false)
    {
    }

    void testMatching();
    void testDrawToPos();
    void testGetPosSizeWindow();

    CPPUNIT_TEST_SUITE(VclImplAnimViewTest);
    CPPUNIT_TEST(testMatching);
    CPPUNIT_TEST(testDrawToPos);
    CPPUNIT_TEST(testGetPosSizeWindow);
    CPPUNIT_TEST_SUITE_END();

private:
    Animation createAnimation();
};

void VclImplAnimViewTest::testMatching()
{
    Animation aTestAnim = createAnimation();
    ScopedVclPtrInstance<TestRenderingContext> pTestRC;

    ImplAnimView* pImplAnimView
        = new ImplAnimView(&aTestAnim, pTestRC, Point(0, 0), Size(10, 10), 5);
    CPPUNIT_ASSERT(pImplAnimView->matches(pTestRC, 5));
    CPPUNIT_ASSERT(!pImplAnimView->matches(pTestRC, 10));

    // caller ID of 0 only matches the OutputDevice
    CPPUNIT_ASSERT(pImplAnimView->matches(pTestRC, 0));
}

void VclImplAnimViewTest::testDrawToPos()
{
    Animation aTestAnim = createAnimation();
    ScopedVclPtrInstance<VirtualDevice> pTestRC;

    ImplAnimView* pImplAnimView
        = new ImplAnimView(&aTestAnim, pTestRC.get(), Point(0, 0), Size(10, 10), 5);
    pImplAnimView->drawToPos(0);
    pImplAnimView->drawToPos(1);
    pImplAnimView->drawToPos(2);
    pImplAnimView->drawToPos(10);

    CPPUNIT_ASSERT_EQUAL(Size(1, 1), pTestRC->GetOutputSizePixel());
}

void VclImplAnimViewTest::testGetPosSizeWindow()
{
    Animation aTestAnim = createAnimation();
    ScopedVclPtrInstance<TestRenderingContext> pTestRC;

    ImplAnimView* pImplAnimView
        = new ImplAnimView(&aTestAnim, pTestRC, Point(0, 0), Size(10, 10), 5);
    AnimationBitmap aAnimBmp(BitmapEx(Size(3, 4), vcl::PixelFormat::N24_BPP), Point(0, 0),
                             Size(10, 10));
    Point aPos;
    Size aSize;

    pImplAnimView->getPosSize(aAnimBmp, aPos, aSize);

    CPPUNIT_ASSERT_EQUAL(Point(0, 0), aPos);
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), aSize);
}

Animation VclImplAnimViewTest::createAnimation()
{
    Animation aAnimation;

    aAnimation.Insert(AnimationBitmap(BitmapEx(Size(3, 4), vcl::PixelFormat::N24_BPP), Point(0, 0),
                                      Size(10, 10)));
    aAnimation.Insert(AnimationBitmap(BitmapEx(Size(3, 3), vcl::PixelFormat::N24_BPP), Point(0, 0),
                                      Size(10, 10)));

    return aAnimation;
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclImplAnimViewTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
