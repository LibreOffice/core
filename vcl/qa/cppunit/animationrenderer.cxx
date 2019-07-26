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
#include <vcl/window.hxx>

#include <AnimationRenderer.hxx>

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

class VclAnimationRendererTest : public test::BootstrapFixture
{
public:
    VclAnimationRendererTest()
        : BootstrapFixture(true, false)
    {
    }

    void testMatching();
    void testMatchingWindow();
    void testDrawToIndex();

    CPPUNIT_TEST_SUITE(VclAnimationRendererTest);
    CPPUNIT_TEST(testMatching);
    CPPUNIT_TEST(testDrawToIndex);
    CPPUNIT_TEST_SUITE_END();

private:
    Animation createAnimation();
};

void VclAnimationRendererTest::testMatching()
{
    Animation aTestAnim = createAnimation();
    OutputDevice* pTestRC = new TestRenderingContext();

    AnimationRenderer* pAnimRenderer
        = pTestRC->CreateAnimationRenderer(&aTestAnim, pTestRC, Point(0, 0), Size(10, 10), 5);
    CPPUNIT_ASSERT(pAnimRenderer->Matches(pTestRC, 5));
    CPPUNIT_ASSERT(!pAnimRenderer->Matches(pTestRC, 10));

    // caller ID of 0 only matches the OutputDevice
    CPPUNIT_ASSERT(pAnimRenderer->Matches(pTestRC, 0));
}

void VclAnimationRendererTest::testDrawToIndex()
{
    Animation aTestAnim = createAnimation();
    OutputDevice* pTestRC = new vcl::Window(nullptr, WB_APP | WB_STDWORK);

    // no assertions, just make sure it runs and doesn't fall over

    AnimationRenderer* pAnimRenderer
        = pTestRC->CreateAnimationRenderer(&aTestAnim, pTestRC, Point(0, 0), Size(10, 10), 5);
    pAnimRenderer->DrawToIndex(0);
    pAnimRenderer->DrawToIndex(1);
    pAnimRenderer->DrawToIndex(2);
    pAnimRenderer->DrawToIndex(10);
}

Animation VclAnimationRendererTest::createAnimation()
{
    Animation aAnimation;

    aAnimation.Insert(AnimationBitmap(BitmapEx(Size(3, 4), 24), Point(0, 0), Size(10, 10)));
    aAnimation.Insert(AnimationBitmap(BitmapEx(Size(3, 3), 24), Point(0, 0), Size(10, 10)));

    return aAnimation;
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclAnimationRendererTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
