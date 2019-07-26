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

class VclAnimationRenderersTest : public test::BootstrapFixture
{
public:
    VclAnimationRenderersTest()
        : BootstrapFixture(true, false)
    {
    }

    void testCreateDefault();

    CPPUNIT_TEST_SUITE(VclAnimationRenderersTest);
    CPPUNIT_TEST(testCreateDefault);
    CPPUNIT_TEST_SUITE_END();

private:
    Animation createAnimation();
};

void VclAnimationRenderersTest::testCreateDefault()
{
    Animation aTestAnim = createAnimation();
    OutputDevice* pTestRC = new TestRenderingContext();

    AnimationRenderers aAnimRenderers;
    CPPUNIT_ASSERT_EQUAL(size_t(0), aAnimRenderers.GetSize());

    aAnimRenderers.CreateDefaultRenderer(&aTestAnim, pTestRC, Point(0, 0), Size(10, 10), 5);

    CPPUNIT_ASSERT_EQUAL(size_t(1), aAnimRenderers.GetSize());
}

Animation VclAnimationRenderersTest::createAnimation()
{
    Animation aAnimation;

    aAnimation.Insert(AnimationFrame(BitmapEx(Size(3, 4), 24), Point(0, 0), Size(10, 10)));
    aAnimation.Insert(AnimationFrame(BitmapEx(Size(3, 3), 24), Point(0, 0), Size(10, 10)));

    return aAnimation;
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclAnimationRenderersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
