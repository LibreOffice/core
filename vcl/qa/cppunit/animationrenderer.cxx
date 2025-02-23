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

#include <animate/AnimationRenderer.hxx>

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
    bool CanAnimate() const override { return false; }
};

Animation createAnimation()
{
    Animation aAnimation;

    aAnimation.Insert(
        AnimationFrame(BitmapEx(Size(3, 4), vcl::PixelFormat::N24_BPP), Point(0, 0), Size(10, 10)));
    aAnimation.Insert(
        AnimationFrame(BitmapEx(Size(3, 3), vcl::PixelFormat::N24_BPP), Point(0, 0), Size(10, 10)));

    return aAnimation;
}
}

class VclAnimationRendererTest : public test::BootstrapFixture
{
public:
    VclAnimationRendererTest()
        : BootstrapFixture(true, false)
    {
    }
};

CPPUNIT_TEST_FIXTURE(VclAnimationRendererTest, testMatching)
{
    Animation aTestAnim = createAnimation();
    ScopedVclPtrInstance<TestRenderingContext> pTestRC;

    AnimationRenderer* pAnimationRenderer
        = new AnimationRenderer(&aTestAnim, pTestRC, Point(0, 0), Size(10, 10), 5);
    CPPUNIT_ASSERT(pAnimationRenderer->matches(pTestRC, 5));
    CPPUNIT_ASSERT(!pAnimationRenderer->matches(pTestRC, 10));

    // caller ID of 0 only matches the OutputDevice
    CPPUNIT_ASSERT(pAnimationRenderer->matches(pTestRC, 0));
}

CPPUNIT_TEST_FIXTURE(VclAnimationRendererTest, testDrawToPos)
{
    Animation aTestAnim = createAnimation();
    ScopedVclPtrInstance<VirtualDevice> pTestRC;

    AnimationRenderer* pAnimationRenderer
        = new AnimationRenderer(&aTestAnim, pTestRC.get(), Point(0, 0), Size(10, 10), 5);
    pAnimationRenderer->drawToIndex(0);
    pAnimationRenderer->drawToIndex(1);
    pAnimationRenderer->drawToIndex(2);
    pAnimationRenderer->drawToIndex(10);

    CPPUNIT_ASSERT_EQUAL(Size(1, 1), pTestRC->GetOutputSizePixel());
}

CPPUNIT_TEST_FIXTURE(VclAnimationRendererTest, testGetPosSizeWindow)
{
    Animation aTestAnim = createAnimation();
    ScopedVclPtrInstance<TestRenderingContext> pTestRC;

    AnimationRenderer* pAnimationRenderer
        = new AnimationRenderer(&aTestAnim, pTestRC, Point(0, 0), Size(10, 10), 5);
    AnimationFrame aAnimBmp(BitmapEx(Size(3, 4), vcl::PixelFormat::N24_BPP), Point(0, 0),
                            Size(10, 10));
    Point aPos;
    Size aSize;

    pAnimationRenderer->getPosSize(aAnimBmp, aPos, aSize);

    CPPUNIT_ASSERT_EQUAL(Point(0, 0), aPos);
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), aSize);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
