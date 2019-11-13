/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <vcl/drawables/GradientDrawable.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace
{
class GradientDrawableTest : public CppUnit::TestFixture
{
    void testLinearGradientDrawable();
    void testAxialGradientDrawable();
    void testRadialGradientDrawable();
    void testEllipticalGradientDrawable();
    void testSquareGradientDrawable();
    void testRectGradientDrawable();

    Gradient MakeGradient(GradientStyle eStyle, sal_uInt16 nAngle);

    CPPUNIT_TEST_SUITE(GradientDrawableTest);
    CPPUNIT_TEST(testLinearGradientDrawable);
    CPPUNIT_TEST(testAxialGradientDrawable);
    CPPUNIT_TEST(testRadialGradientDrawable);
    CPPUNIT_TEST(testEllipticalGradientDrawable);
    CPPUNIT_TEST(testSquareGradientDrawable);
    CPPUNIT_TEST(testRectGradientDrawable);
    CPPUNIT_TEST_SUITE_END();
};

Gradient GradientDrawableTest::MakeGradient(GradientStyle eStyle, sal_uInt16 nAngle)
{
    Gradient aGradient(eStyle, Color(0xFF, 0xFF, 0xFF), Color(0x00, 0x00, 0x00));
    aGradient.SetAngle(nAngle);

    return aGradient;
}

void GradientDrawableTest::testLinearGradientDrawable()

{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetOutputSizePixel(Size(15, 15));

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::GradientDrawable(
        tools::Rectangle(0, 0, 10, 10), MakeGradient(GradientStyle::Linear, 900))));
}

void GradientDrawableTest::testAxialGradientDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetOutputSizePixel(Size(15, 15));

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::GradientDrawable(
        tools::Rectangle(0, 0, 10, 10), MakeGradient(GradientStyle::Axial, 900))));
}

void GradientDrawableTest::testRadialGradientDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetOutputSizePixel(Size(15, 15));

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::GradientDrawable(
        tools::Rectangle(0, 0, 10, 10), MakeGradient(GradientStyle::Radial, 900))));
}

void GradientDrawableTest::testEllipticalGradientDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetOutputSizePixel(Size(15, 15));

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::GradientDrawable(
        tools::Rectangle(0, 0, 10, 10), MakeGradient(GradientStyle::Elliptical, 900))));
}

void GradientDrawableTest::testSquareGradientDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetOutputSizePixel(Size(15, 15));

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::GradientDrawable(
        tools::Rectangle(0, 0, 10, 10), MakeGradient(GradientStyle::Square, 900))));
}

void GradientDrawableTest::testRectGradientDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetOutputSizePixel(Size(15, 15));

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::GradientDrawable(
        tools::Rectangle(0, 0, 10, 10), MakeGradient(GradientStyle::Rect, 900))));
}
} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GradientDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
