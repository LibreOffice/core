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

#include <vcl/gradient.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>

#include <drawables/GradientDrawableHelper.hxx>

namespace
{
class GradientDrawableHelperTest : public CppUnit::TestFixture
{
    void testExpandGradientOverBorder();
    void testGetStartColorIntensity();
    void testGetEndColorIntensity();
    void testGetStartColorIntensityValues();
    void testGetEndColorIntensityValues();
    void testGetColorIntensities();
    void testCalculateBorderWidth();
    void testGetLinearBorderValues();
    void testGetNonlinearBorderValues();
    void testGetGradientSteps();
    void testGetLinearGradientSteps();
    void testGetComplexGradientSteps();
    void testSetGrayscaleColors();

    Gradient MakeGradient(Color const& rStartColor, Color const& rEndColor,
                          GradientStyle eStyle = GradientStyle::Linear, sal_uInt16 nAngle = 900,
                          sal_uInt16 nBorder = 1);

    CPPUNIT_TEST_SUITE(GradientDrawableHelperTest);
    CPPUNIT_TEST(testExpandGradientOverBorder);
    CPPUNIT_TEST(testGetStartColorIntensity);
    CPPUNIT_TEST(testGetEndColorIntensity);
    CPPUNIT_TEST(testGetStartColorIntensityValues);
    CPPUNIT_TEST(testGetEndColorIntensityValues);
    CPPUNIT_TEST(testGetColorIntensities);
    CPPUNIT_TEST(testCalculateBorderWidth);
    CPPUNIT_TEST(testGetLinearBorderValues);
    CPPUNIT_TEST(testGetNonlinearBorderValues);
    CPPUNIT_TEST(testGetGradientSteps);
    CPPUNIT_TEST(testGetLinearGradientSteps);
    CPPUNIT_TEST(testGetComplexGradientSteps);
    CPPUNIT_TEST(testSetGrayscaleColors);
    CPPUNIT_TEST_SUITE_END();
};

Gradient GradientDrawableHelperTest::MakeGradient(Color const& rStartColor, Color const& rEndColor,
                                                  GradientStyle eStyle, sal_uInt16 nAngle,
                                                  sal_uInt16 nBorder)
{
    Gradient aGradient(eStyle, rStartColor, rEndColor);
    aGradient.SetAngle(nAngle);
    aGradient.SetBorder(nBorder);

    return aGradient;
}

void GradientDrawableHelperTest::testExpandGradientOverBorder()
{
    const long BORDERLEFT = 10L;
    const long BORDERTOP = 10L;
    const long BORDERRIGHT = 20L;
    const long BORDERBOTTOM = 20L;

    tools::Rectangle aRect(BORDERLEFT, BORDERTOP, BORDERRIGHT, BORDERBOTTOM);
    tools::Rectangle aAdjustedRect = vcl::GradientDrawableHelper::ExpandGradientOverBorder(aRect);

    const long ADJUSTEDBORDERLEFT = 9L;
    const long ADJUSTEDBORDERTOP = 9L;
    const long ADJUSTEDBORDERRIGHT = 21L;
    const long ADJUSTEDBORDERBOTTOM = 21L;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong left value", ADJUSTEDBORDERLEFT, aAdjustedRect.Left());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong top value", ADJUSTEDBORDERTOP, aAdjustedRect.Top());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong right value", ADJUSTEDBORDERRIGHT, aAdjustedRect.Right());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong bottom value", ADJUSTEDBORDERBOTTOM,
                                 aAdjustedRect.Bottom());
}

void GradientDrawableHelperTest::testGetStartColorIntensity()
{
    Gradient aGradient = MakeGradient(COL_WHITE, COL_GREEN);
    const long EXPECTEDINTENSITY = 0x80;

    CPPUNIT_ASSERT_EQUAL(EXPECTEDINTENSITY,
                         vcl::GradientDrawableHelper::GetStartColorIntensity(aGradient, 0x80));
}

void GradientDrawableHelperTest::testGetEndColorIntensity()
{
    Gradient aGradient = MakeGradient(COL_WHITE, COL_GREEN);
    const long EXPECTEDINTENSITY = 0x80;

    CPPUNIT_ASSERT_EQUAL(EXPECTEDINTENSITY,
                         vcl::GradientDrawableHelper::GetEndColorIntensity(aGradient, 0x80));
}

void GradientDrawableHelperTest::testGetStartColorIntensityValues()
{
    Gradient aGradient = MakeGradient(COL_RED, COL_GREEN);
    const long EXPECTEDREDINTENSITY = 0x80;
    const long EXPECTEDGREENINTENSITY = 0x00;
    const long EXPECTEDBLUEINTENSITY = 0x00;

    long nRed, nGreen, nBlue;
    std::tie(nRed, nGreen, nBlue)
        = vcl::GradientDrawableHelper::GetStartColorIntensityValues(aGradient);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start red value", EXPECTEDREDINTENSITY, nRed);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start green value", EXPECTEDGREENINTENSITY, nGreen);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start blue value", EXPECTEDBLUEINTENSITY, nBlue);
}

void GradientDrawableHelperTest::testGetEndColorIntensityValues()
{
    Gradient aGradient = MakeGradient(COL_RED, COL_GREEN);
    const long EXPECTEDREDINTENSITY = 0x00;
    const long EXPECTEDGREENINTENSITY = 0x80;
    const long EXPECTEDBLUEINTENSITY = 0x00;

    long nRed, nGreen, nBlue;
    std::tie(nRed, nGreen, nBlue)
        = vcl::GradientDrawableHelper::GetEndColorIntensityValues(aGradient);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end red value", EXPECTEDREDINTENSITY, nRed);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end green value", EXPECTEDGREENINTENSITY, nGreen);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end blue value", EXPECTEDBLUEINTENSITY, nBlue);
}

void GradientDrawableHelperTest::testGetColorIntensities()
{
    Gradient aGradient = MakeGradient(COL_RED, COL_GREEN);
    const long EXPECTED_START_REDINTENSITY = 0x80;
    const long EXPECTED_START_GREENINTENSITY = 0x00;
    const long EXPECTED_START_BLUEINTENSITY = 0x00;
    const long EXPECTED_END_REDINTENSITY = 0x00;
    const long EXPECTED_END_GREENINTENSITY = 0x80;
    const long EXPECTED_END_BLUEINTENSITY = 0x00;

    long nStartRed, nStartGreen, nStartBlue;
    long nEndRed, nEndGreen, nEndBlue;
    std::tie(nStartRed, nStartGreen, nStartBlue, nEndRed, nEndGreen, nEndBlue)
        = vcl::GradientDrawableHelper::GetColorIntensities(aGradient);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start red value", EXPECTED_START_REDINTENSITY, nStartRed);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start green value", EXPECTED_START_GREENINTENSITY,
                                 nStartGreen);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start blue value", EXPECTED_START_BLUEINTENSITY,
                                 nStartBlue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end red value", EXPECTED_END_REDINTENSITY, nEndRed);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end green value", EXPECTED_END_GREENINTENSITY, nEndGreen);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end blue value", EXPECTED_END_BLUEINTENSITY, nEndBlue);
}

void GradientDrawableHelperTest::testCalculateBorderWidth()
{
    tools::Rectangle aRect(0, 0, 10, 10);
    const sal_uInt16 ANGLE = 900;
    const sal_uInt16 BORDER = 5;

    const double EXPECTEDLINEARBORDER = 0.55;
    const double EXPECTEDNONLINEARBORDER = 0.275;

    Gradient aGradient = MakeGradient(COL_RED, COL_GREEN, GradientStyle::Linear, ANGLE, BORDER);
    CPPUNIT_ASSERT_EQUAL(EXPECTEDLINEARBORDER,
                         vcl::GradientDrawableHelper::CalculateBorderWidth(aGradient, aRect));

    aGradient = MakeGradient(COL_RED, COL_GREEN, GradientStyle::Axial, ANGLE, BORDER);
    CPPUNIT_ASSERT_EQUAL(EXPECTEDNONLINEARBORDER,
                         vcl::GradientDrawableHelper::CalculateBorderWidth(aGradient, aRect));
}

void GradientDrawableHelperTest::testGetLinearBorderValues()
{
    tools::Rectangle aRect(0, 0, 10, 10);
    Gradient aGradient = MakeGradient(COL_RED, COL_GREEN, GradientStyle::Linear);

    tools::Rectangle aBorderRect, aMirroredBorderRect;
    Point aCenter;
    double fBorderWidth;

    std::tie(aBorderRect, aMirroredBorderRect, aCenter, fBorderWidth)
        = vcl::GradientDrawableHelper::GetBorderValues(aGradient, aRect);

    const long BORDERLEFT = 0;
    const long BORDERTOP = 0;
    const long BORDERRIGHT = 10;
    const long BORDERBOTTOM = 10;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left border rectangle wrong", BORDERLEFT, aBorderRect.Left());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Top border rectangle wrong", BORDERTOP, aBorderRect.Top());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right border rectangle wrong", BORDERRIGHT, aBorderRect.Right());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom border rectangle wrong", BORDERBOTTOM,
                                 aBorderRect.Right());

    const long MIRRORBORDERLEFT = 0;
    const long MIRRORBORDERTOP = 5;
    const long MIRRORBORDERRIGHT = 10;
    const long MIRRORBORDERBOTTOM = 10;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mirrored left border rectangle wrong", MIRRORBORDERLEFT,
                                 aMirroredBorderRect.Left());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mirrored top border rectangle wrong", MIRRORBORDERTOP,
                                 aMirroredBorderRect.Top());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mirrored right border rectangle wrong", MIRRORBORDERRIGHT,
                                 aMirroredBorderRect.Right());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mirrored bottom border rectangle wrong", MIRRORBORDERBOTTOM,
                                 aMirroredBorderRect.Right());

    const Point EXPECTEDCENTER(5, 5);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Center wrong", EXPECTEDCENTER, aCenter);

    const double EXPECTEDBORDER = 0.11;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Border wrong", EXPECTEDBORDER, fBorderWidth);
}

void GradientDrawableHelperTest::testGetNonlinearBorderValues()
{
    tools::Rectangle aRect(10, 10, 20, 20);
    Gradient aGradient = MakeGradient(COL_RED, COL_GREEN, GradientStyle::Axial);

    tools::Rectangle aBorderRect, aMirroredBorderRect;
    Point aCenter;
    double fBorderWidth;

    std::tie(aBorderRect, aMirroredBorderRect, aCenter, fBorderWidth)
        = vcl::GradientDrawableHelper::GetBorderValues(aGradient, aRect);

    const long BORDERLEFT = 10;
    const long BORDERTOP = 10;
    const long BORDERRIGHT = 20;
    const long BORDERBOTTOM = 20;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left border rectangle wrong", BORDERLEFT, aBorderRect.Left());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Top border rectangle wrong", BORDERTOP, aBorderRect.Top());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right border rectangle wrong", BORDERRIGHT, aBorderRect.Right());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom border rectangle wrong", BORDERBOTTOM,
                                 aBorderRect.Right());

    const long MIRRORBORDERLEFT = 10;
    const long MIRRORBORDERTOP = 15;
    const long MIRRORBORDERRIGHT = 20;
    const long MIRRORBORDERBOTTOM = 20;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mirrored left border rectangle wrong", MIRRORBORDERLEFT,
                                 aMirroredBorderRect.Left());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mirrored top border rectangle wrong", MIRRORBORDERTOP,
                                 aMirroredBorderRect.Top());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mirrored right border rectangle wrong", MIRRORBORDERRIGHT,
                                 aMirroredBorderRect.Right());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Mirrored bottom border rectangle wrong", MIRRORBORDERBOTTOM,
                                 aMirroredBorderRect.Right());

    const Point CENTER(15, 15);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Center wrong", CENTER, aCenter);

    const double EXPECTEDBORDER = 0.055;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Border wrong", EXPECTEDBORDER, fBorderWidth);
}

void GradientDrawableHelperTest::testGetGradientSteps()
{
    tools::Rectangle aRect(0, 0, 100, 100);
    const bool bUseMetafile = false;

    {
        Gradient aGradient = MakeGradient(COL_RED, COL_GREEN, GradientStyle::Axial);

        VclPtrInstance<VirtualDevice> pRenderContext;
        const long EXPECTEDSTEPS = 25;

        CPPUNIT_ASSERT_EQUAL(EXPECTEDSTEPS,
                             vcl::GradientDrawableHelper::GetGradientSteps(
                                 pRenderContext, aGradient, aRect, bUseMetafile, true));

        aGradient.SetStyle(GradientStyle::Square);
        CPPUNIT_ASSERT_EQUAL(EXPECTEDSTEPS,
                             vcl::GradientDrawableHelper::GetGradientSteps(
                                 pRenderContext, aGradient, aRect, bUseMetafile, true));
    }

    {
        Gradient aGradient = MakeGradient(COL_RED, COL_GREEN, GradientStyle::Axial);

        VclPtrInstance<Printer> pRenderContext;
        const long EXPECTEDSTEPS = 10;

        CPPUNIT_ASSERT_EQUAL(EXPECTEDSTEPS,
                             vcl::GradientDrawableHelper::GetGradientSteps(
                                 pRenderContext, aGradient, aRect, bUseMetafile, true));

        aGradient.SetStyle(GradientStyle::Square);
        CPPUNIT_ASSERT_EQUAL(EXPECTEDSTEPS,
                             vcl::GradientDrawableHelper::GetGradientSteps(
                                 pRenderContext, aGradient, aRect, bUseMetafile, true));
    }
}

void GradientDrawableHelperTest::testGetLinearGradientSteps()
{
    const long STARTRED = 0x80;
    const long STARTGREEN = 0x80;
    const long STARTBLUE = 0x80;
    const long ENDRED = 0xFF;
    const long ENDGREEN = 0xFF;
    const long ENDBLUE = 0xFF;

    const long TESTMAXSTEPCOUNT = 100L;
    const long TESTMIDSTEPCOUNT = 130L;
    const long TESTMINSTEPCOUNT = 1L;

    const long EXPECTEDMAXSTEPS = 100L;
    const long EXPECTEDMIDSTEPS = 127L;
    const long EXPECTEDMINSTEPS = 3L;

    CPPUNIT_ASSERT_EQUAL(EXPECTEDMAXSTEPS, vcl::GradientDrawableHelper::GetLinearGradientSteps(
                                               TESTMAXSTEPCOUNT, STARTRED, STARTGREEN, STARTBLUE,
                                               ENDRED, ENDGREEN, ENDBLUE));
    CPPUNIT_ASSERT_EQUAL(EXPECTEDMIDSTEPS, vcl::GradientDrawableHelper::GetLinearGradientSteps(
                                               TESTMIDSTEPCOUNT, STARTRED, STARTGREEN, STARTBLUE,
                                               ENDRED, ENDGREEN, ENDBLUE));
    CPPUNIT_ASSERT_EQUAL(EXPECTEDMINSTEPS, vcl::GradientDrawableHelper::GetLinearGradientSteps(
                                               TESTMINSTEPCOUNT, STARTRED, STARTGREEN, STARTBLUE,
                                               ENDRED, ENDGREEN, ENDBLUE));
}

void GradientDrawableHelperTest::testGetComplexGradientSteps()
{
    tools::Rectangle aRect(0, 0, 100, 100);
    Gradient aGradient = MakeGradient(COL_RED, COL_GREEN, GradientStyle::Square);
    VclPtrInstance<VirtualDevice> pRenderContext;

    const long STARTRED = 0x80;
    const long STARTGREEN = 0x80;
    const long STARTBLUE = 0x80;

    const long EXPECTEDSTEPS = 101;

    CPPUNIT_ASSERT_EQUAL(EXPECTEDSTEPS,
                         vcl::GradientDrawableHelper::GetComplexGradientSteps(
                             pRenderContext, aGradient, aRect, STARTRED, STARTGREEN, STARTBLUE));
}

void GradientDrawableHelperTest::testSetGrayscaleColors()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetDrawMode(DrawModeFlags::GrayGradient);

    Gradient aGradient = MakeGradient(COL_RED, COL_GREEN, GradientStyle::Square);

    vcl::GradientDrawableHelper::SetGrayscaleColors(pRenderContext, aGradient);

    const Color EXPECTEDSTARTGRAYSCALE(0x26, 0x26, 0x26);
    const Color EXPECTEDENDGRAYSCALE(0x4B, 0x4B, 0x4B);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong start grayscale", EXPECTEDSTARTGRAYSCALE,
                                 aGradient.GetStartColor());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong end grayscale", EXPECTEDENDGRAYSCALE,
                                 aGradient.GetEndColor());
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(GradientDrawableHelperTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
