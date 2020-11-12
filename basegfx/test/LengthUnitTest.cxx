/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basegfx/units/Length.hxx>
#include <basegfx/units/LengthTypes.hxx>
#include <basegfx/units/Range2DLWrap.hxx>
#include <basegfx/units/Size2DLWrap.hxx>
#include <basegfx/units/Tuple2DLWrap.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

static_assert(gfx::Length() == 0_mm);
static_assert(gfx::Length() == 0_emu);
static_assert(gfx::Length() == gfx::Length::emu(0));
static_assert(gfx::Length() == gfx::Length::from(gfx::LengthUnit::emu, 0));

static_assert(1_cm + gfx::Length::cm(2) + gfx::Length::from(gfx::LengthUnit::cm, 2) == 5_cm);
static_assert(5_cm - gfx::Length::cm(1) - gfx::Length::from(gfx::LengthUnit::cm, 2) == 2_cm);

constexpr gfx::Length checkOperators()
{
    gfx::Length value
        = gfx::Length::mm(20)
          - gfx::Length::from(gfx::LengthUnit::hmm, 2000) / 2; // 2_cm - 2_cm / 2 = 1_cm
    value -= 1_cm + 10_cm - 6_cm; // = -4_cm
    value += 1_cm + 1_cm + 3_cm; // = 1_cm
    value *= 1 + 1; // = 2_cm
    value -= 1_cm * 8; // = -6_cm
    value /= 1 + 1 + 1; // = -2_cm
    return -value;
}

static_assert(checkOperators() == 2_cm);

static_assert((2_cm).as_cm() == 2);
static_assert((2_cm).as_mm() == 20);
static_assert((2_cm).as_hmm() == 2000);

class LengthTest : public CppUnit::TestFixture
{
public:
    void testCreation()
    {
        //  Creation from integer number
        int number = 10;
        auto asCm = gfx::Length::cm(number);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, asCm.as_cm(), 1e-4);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(3600000), asCm.data());

        auto asMm = gfx::Length::mm(number);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, asMm.as_mm(), 1e-4);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(360000), asMm.data());

        auto asInch = gfx::Length::in(number);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, asInch.as_in(), 1e-4);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(9144000), asInch.data());

        auto forceInteger = gfx::Length::hmm<sal_Int64>(10.1);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(3600), forceInteger.data());

        auto forceDouble = gfx::Length::hmm<double>(10.1);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(3636), forceDouble.data());
    }

    void testUnitConversion()
    {
        // Big values
        CPPUNIT_ASSERT_EQUAL(sal_Int64(34200000000), (950_m).data());
        CPPUNIT_ASSERT_EQUAL(95000_cm, 950_m);
        CPPUNIT_ASSERT_EQUAL(950000_mm, 950_m);
        CPPUNIT_ASSERT_EQUAL(95000000_hmm, 950_m);
        CPPUNIT_ASSERT_EQUAL(34200000000_emu, 950_m);

        CPPUNIT_ASSERT_EQUAL(sal_Int64(-34200000000), (-950_m).data());
        CPPUNIT_ASSERT_EQUAL(-95000_cm, -950_m);
        CPPUNIT_ASSERT_EQUAL(-950000_mm, -950_m);
        CPPUNIT_ASSERT_EQUAL(-95000000_hmm, -950_m);
        CPPUNIT_ASSERT_EQUAL(-34200000000_emu, -950_m);

        // To double value in chosen unit
        CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, (4_cm).as_cm(), 1e-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.04, (4_cm).as_meter(), 1e-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(40.0, (4_cm).as_mm(), 1e-4);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(1440000), (4_cm).data());

        // (635 * 20) + 3 * (635 * 15) = 41275EMU
        gfx::Length pt = 1_pt + 3_px;
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3.25, pt.as_pt(), 1e-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(65.0, pt.as_twip(), 1e-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0451, pt.as_in(), 1e-4);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(41275), pt.data());

        gfx::Length inch = 1_in; // 1440 * 635
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1440.0, inch.as_twip(), 1e-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(96.0, inch.as_px(), 1e-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, inch.as_in(), 1e-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(914400.0, inch.as_emu(), 1e-4);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(914400), inch.data());
    }

    void testLimits()
    {
        gfx::Length maximum = gfx::Length::emu(SAL_MAX_INT64);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(256204778801.5, maximum.as_meter(), 1e-1);
        // 256204778 km
        CPPUNIT_ASSERT_EQUAL(sal_Int64(SAL_MAX_INT64), maximum.data());

        gfx::Length minimum = gfx::Length::emu(SAL_MIN_INT64);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-256204778801.5, minimum.as_meter(), 1e-1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(SAL_MIN_INT64), minimum.as_emu(), 1e-1);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(SAL_MIN_INT64), minimum.data());
    }

    void testAdditionAndSubstitution()
    {
        // results in zero
        CPPUNIT_ASSERT_EQUAL(0_cm, 100_cm - 100_cm);
        CPPUNIT_ASSERT_EQUAL(0_cm, -100_cm + 100_cm);

        // + and - with zero
        CPPUNIT_ASSERT_EQUAL(10_cm, 10_cm + 0_cm);
        CPPUNIT_ASSERT_EQUAL(10_cm, 10_cm - 0_cm);
        CPPUNIT_ASSERT_EQUAL(10_cm, 10_cm - 0_cm + 0_emu - 0_twip);

        // common
        CPPUNIT_ASSERT_EQUAL(6_cm, 1_cm + 5_cm);
        CPPUNIT_ASSERT_EQUAL(-4_cm, 1_cm - 5_cm);
        CPPUNIT_ASSERT_EQUAL(4_cm, -1_cm + 5_cm);
        CPPUNIT_ASSERT_EQUAL(-6_cm, -1_cm - 5_cm);

        // long chain
        CPPUNIT_ASSERT_EQUAL(35129_hmm, 1_mm - 5_cm + 40_cm - 1_hmm + 30_hmm);

        // floating point
        CPPUNIT_ASSERT_EQUAL(1_mm, 0.5_mm + 0.5_mm);

        // mixed units
        CPPUNIT_ASSERT_EQUAL(180_emu + 635_emu, 0.5_hmm + 1_twip);
        CPPUNIT_ASSERT_EQUAL(554400_emu, 1_in - 1_cm);

        // Big values - 1km - 50m
        CPPUNIT_ASSERT_EQUAL(95000_cm, 1000_m - 5000_cm);

        // 27 emu + 33 emu + 360 emu = 420
        gfx::Length emus = 27_emu + 33_emu + 1_hmm;
        CPPUNIT_ASSERT_EQUAL(sal_Int64(420), emus.data());
    }

    void testMultiplication()
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int64(0), (1_hmm * 0).data());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(720), (1_hmm * 2).data());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-720), (1_hmm * -2).data());

        CPPUNIT_ASSERT_EQUAL(sal_Int64(0), (1_hmm * 0.0).data());

        CPPUNIT_ASSERT_EQUAL(sal_Int64(720), (1_hmm * 2.0).data());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-720), (1_hmm * -2.0).data());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-720), (-1_hmm * 2.0).data());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(720), (-1_hmm * -2.0).data());

        CPPUNIT_ASSERT_EQUAL(sal_Int64(180), (1_hmm * 0.5).data());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-180), (-1_hmm * 0.5).data());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-180), (1_hmm * -0.5).data());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(180), (-1_hmm * -0.5).data());

        CPPUNIT_ASSERT_EQUAL(sal_Int64(90), (0.5_hmm * 0.5).data());
    }

    void testDivision()
    {
        gfx::Length cm(1_cm);
        cm /= 2;
        CPPUNIT_ASSERT_EQUAL(sal_Int64(180000), cm.data());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, cm.as_cm(), 1e-4);

        gfx::Length cm4(1_cm);
        cm4 /= 2.0;
        CPPUNIT_ASSERT_EQUAL(sal_Int64(180000), cm4.data());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, cm4.as_cm(), 1e-4);

        // with division of 2 length units you get a ratio
        double aRatio = gfx::Length::hmm(10) / gfx::Length::hmm(20);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, aRatio, 1e-9);
    }

    void testComparison()
    {
        // Zero - should equal no matter what unit
        CPPUNIT_ASSERT(0_emu == 0_emu);
        CPPUNIT_ASSERT(0_emu == 0_in);
        CPPUNIT_ASSERT(0_twip == 0_hmm);
        CPPUNIT_ASSERT(-0_emu == 0_emu);

        // Not equal - same units
        CPPUNIT_ASSERT(1_emu != 2_emu);
        CPPUNIT_ASSERT(1_emu != 0_emu);

        // Not equal - different units, same value
        CPPUNIT_ASSERT(1_hmm != 1_emu);
        CPPUNIT_ASSERT(1_twip != 1_hmm);

        // Less
        CPPUNIT_ASSERT_EQUAL(true, 1_emu < 2_emu);
        CPPUNIT_ASSERT_EQUAL(false, 1_emu < 1_emu);
        // Less or Equal
        CPPUNIT_ASSERT_EQUAL(true, 1_emu <= 2_emu);
        CPPUNIT_ASSERT_EQUAL(true, 1_emu <= 1_emu);
        // More
        CPPUNIT_ASSERT_EQUAL(true, 2_emu > 1_emu);
        CPPUNIT_ASSERT_EQUAL(false, 1_emu < 1_emu);
        //More or equal
        CPPUNIT_ASSERT_EQUAL(true, 2_emu >= 1_emu);
        CPPUNIT_ASSERT_EQUAL(true, 2_emu >= 2_emu);
    }

    void testGenericFrom()
    {
        auto hmm1 = gfx::Length::from(gfx::LengthUnit::hmm, 1);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(360), hmm1.data());
        CPPUNIT_ASSERT_EQUAL(gfx::Length::hmm(1), hmm1);

        auto twip2 = gfx::Length::from(gfx::LengthUnit::twip, 2);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(1270), twip2.data());
        CPPUNIT_ASSERT_EQUAL(gfx::Length::twip(2), twip2);
    }

    void testGenericAs()
    {
        CPPUNIT_ASSERT_EQUAL(1.0, (100_hmm).as(gfx::LengthUnit::mm));
        CPPUNIT_ASSERT_EQUAL(100.0, (1_m).as(gfx::LengthUnit::cm));
    }

    void testInTuple()
    {
        gfx::Tuple2DL aTuple(0.5_pt, 1_pt);
        CPPUNIT_ASSERT_EQUAL(6350_emu, aTuple.getX());
        CPPUNIT_ASSERT_EQUAL(12700_emu, aTuple.getY());

        gfx::Tuple2DL aTuple2(0_pt, 0_pt);
        aTuple2.setX(0.5_pt);
        aTuple2.setY(1_pt);

        CPPUNIT_ASSERT_EQUAL(6350_emu, aTuple2.getX());
        CPPUNIT_ASSERT_EQUAL(12700_emu, aTuple2.getY());

        CPPUNIT_ASSERT_EQUAL(true, aTuple == aTuple2);
        CPPUNIT_ASSERT_EQUAL(true, aTuple != gfx::Tuple2DL(0_emu, 0_emu));

        CPPUNIT_ASSERT_EQUAL(true, aTuple == aTuple - gfx::Tuple2DL(0_emu, 0_emu));
        CPPUNIT_ASSERT_EQUAL(true, aTuple == aTuple + gfx::Tuple2DL(0_emu, 0_emu));
    }

    void testInSize()
    {
        gfx::Size2DL aSize(0.5_pt, 1_pt);
        CPPUNIT_ASSERT_EQUAL(6350_emu, aSize.getWidth());
        CPPUNIT_ASSERT_EQUAL(12700_emu, aSize.getHeight());

        gfx::Size2DL aSize2(0_pt, 0_pt);
        aSize2.setWidth(0.5_pt);
        aSize2.setHeight(1_pt);

        CPPUNIT_ASSERT_EQUAL(6350_emu, aSize2.getWidth());
        CPPUNIT_ASSERT_EQUAL(12700_emu, aSize2.getHeight());

        CPPUNIT_ASSERT_EQUAL(true, aSize == aSize2);
        CPPUNIT_ASSERT_EQUAL(true, aSize != gfx::Size2DL(0_emu, 0_emu));

        CPPUNIT_ASSERT_EQUAL(true, aSize == aSize - gfx::Size2DL(0_emu, 0_emu));
        CPPUNIT_ASSERT_EQUAL(true, aSize == aSize + gfx::Size2DL(0_emu, 0_emu));
    }

    void testSizeLWrap()
    {
        {
            gfx::Size2DLWrap aSize(0.5_pt, 1_pt);
            CPPUNIT_ASSERT_EQUAL(6350_emu, aSize.getWidth());
            CPPUNIT_ASSERT_EQUAL(12700_emu, aSize.getHeight());

            gfx::Size2DLWrap aSize2(0_pt, 0_pt);
            aSize2.setWidth(0.5_pt);
            aSize2.setHeight(1_pt);

            CPPUNIT_ASSERT_EQUAL(6350_emu, aSize2.getWidth());
            CPPUNIT_ASSERT_EQUAL(12700_emu, aSize2.getHeight());

            CPPUNIT_ASSERT_EQUAL(true, aSize == aSize2);
            CPPUNIT_ASSERT_EQUAL(true, aSize != gfx::Size2DLWrap(0_emu, 0_emu));

            CPPUNIT_ASSERT_EQUAL(true, aSize == aSize - gfx::Size2DLWrap(0_emu, 0_emu));
            CPPUNIT_ASSERT_EQUAL(true, aSize == aSize + gfx::Size2DLWrap(0_emu, 0_emu));
        }
        {
            gfx::Size2DLWrap aSize_Hmm(5_hmm, 8_hmm);
            Size aBaseSize = aSize_Hmm.toToolsSize();
            CPPUNIT_ASSERT_EQUAL(tools::Long(5), aBaseSize.Width());
            CPPUNIT_ASSERT_EQUAL(tools::Long(8), aBaseSize.Height());
        }
        {
            gfx::Size2DLWrap aSize_Twip(5_twip, 8_twip, gfx::LengthUnit::twip);
            Size aBaseSize = aSize_Twip.toToolsSize();
            CPPUNIT_ASSERT_EQUAL(tools::Long(5), aBaseSize.Width());
            CPPUNIT_ASSERT_EQUAL(tools::Long(8), aBaseSize.Height());
        }
        {
            auto aSize = gfx::Size2DLWrap::create(Size(5, 8));
            CPPUNIT_ASSERT_EQUAL(5_hmm, aSize.getWidth());
            CPPUNIT_ASSERT_EQUAL(8_hmm, aSize.getHeight());
        }
        {
            auto aSize = gfx::Size2DLWrap::create(Size(5, 8), gfx::LengthUnit::twip);
            CPPUNIT_ASSERT_EQUAL(5_twip, aSize.getWidth());
            CPPUNIT_ASSERT_EQUAL(8_twip, aSize.getHeight());
        }
    }

    void testConversionToRectangle()
    {
        {
            tools::Rectangle aEmpty;
            gfx::Range2DLWrap aEmptyRange = gfx::Range2DLWrap::create(aEmpty);
            CPPUNIT_ASSERT_EQUAL(true, aEmptyRange.isEmpty());

            tools::Rectangle aRectangle(10, 20, 110, 120);
            gfx::Range2DLWrap aRange = gfx::Range2DLWrap::create(aRectangle);
            CPPUNIT_ASSERT_EQUAL(10_hmm, aRange.getMinX());
            CPPUNIT_ASSERT_EQUAL(20_hmm, aRange.getMinY());
            CPPUNIT_ASSERT_EQUAL(110_hmm, aRange.getMaxX());
            CPPUNIT_ASSERT_EQUAL(120_hmm, aRange.getMaxY());

            tools::Rectangle aRectangleConverted = aRange.toToolsRect();
            CPPUNIT_ASSERT_EQUAL(aRectangle, aRectangleConverted);
        }
        {
            tools::Rectangle aRectangle1(10, 20, 110, 120);
            tools::Rectangle aRectangle2(Point(10, 20), Size(101, 101));
            CPPUNIT_ASSERT_EQUAL(aRectangle1, aRectangle2);
        }
        {
            tools::Rectangle aRectangle(10, 20, 110, 120);
            gfx::Range2DLWrap aRange = gfx::Range2DLWrap::create(aRectangle);

            aRectangle.Move(1000, 1000);
            aRange.shift(1000_hmm, 1000_hmm);
            CPPUNIT_ASSERT_EQUAL(aRectangle, aRange.toToolsRect());
        }
        {
            tools::Rectangle aRectangle(10, 20, 110, 120);
            gfx::Range2DLWrap aRange = gfx::Range2DLWrap::create(aRectangle);

            aRectangle.SetSize(Size(201, 201));
            aRange.setSize(200_hmm, 200_hmm);
            CPPUNIT_ASSERT_EQUAL(aRectangle, aRange.toToolsRect());
        }
        {
            tools::Rectangle aRectangle(10, 20, 110, 120);
            gfx::Range2DLWrap aRange = gfx::Range2DLWrap::create(aRectangle);

            aRectangle.SetPos(Point(500, 500));
            aRange.setPosition(500_hmm, 500_hmm);
            CPPUNIT_ASSERT_EQUAL(aRectangle, aRange.toToolsRect());
        }
        {
            tools::Rectangle aRectangle(Point(0, 0), Size(0, 31));
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRectangle.Left());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRectangle.Top());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRectangle.GetWidth());
            CPPUNIT_ASSERT_EQUAL(tools::Long(31), aRectangle.GetHeight());

            gfx::Range2DL aRange = gfx::Range2DLWrap::create(aRectangle);
            CPPUNIT_ASSERT_EQUAL(0_hmm, aRange.getMinX());
            CPPUNIT_ASSERT_EQUAL(0_hmm, aRange.getMinY());
            CPPUNIT_ASSERT_EQUAL(0_hmm, aRange.getMaxX());
            CPPUNIT_ASSERT_EQUAL(30_hmm, aRange.getMaxY());
        }
        {
            tools::Rectangle aRectangle;
            gfx::Range2DLWrap aRange = gfx::Range2DLWrap::create(aRectangle);
            aRectangle.Move(100, 100);
            aRange.shift(100_hmm, 100_hmm);
            CPPUNIT_ASSERT_EQUAL(tools::Long(100), aRectangle.Left());
            CPPUNIT_ASSERT_EQUAL(tools::Long(100), aRectangle.Top());
            CPPUNIT_ASSERT_EQUAL(tools::Long(100), aRectangle.Right());
            CPPUNIT_ASSERT_EQUAL(tools::Long(100), aRectangle.Bottom());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRectangle.GetWidth());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRectangle.GetHeight());

            CPPUNIT_ASSERT_EQUAL(100_hmm, aRange.getMinX());
            CPPUNIT_ASSERT_EQUAL(100_hmm, aRange.getMinY());
            CPPUNIT_ASSERT_EQUAL(100_hmm, aRange.getMaxX());
            CPPUNIT_ASSERT_EQUAL(100_hmm, aRange.getMaxY());
            CPPUNIT_ASSERT_EQUAL(0_hmm, aRange.getWidth());
            CPPUNIT_ASSERT_EQUAL(0_hmm, aRange.getHeight());

            auto aRectFromRange = aRange.toToolsRect();
            CPPUNIT_ASSERT_EQUAL(tools::Long(100), aRectFromRange.Left());
            CPPUNIT_ASSERT_EQUAL(tools::Long(100), aRectFromRange.Top());
            CPPUNIT_ASSERT_EQUAL(tools::Long(100), aRectFromRange.Right());
            CPPUNIT_ASSERT_EQUAL(tools::Long(100), aRectFromRange.Bottom());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRectFromRange.GetWidth());
            CPPUNIT_ASSERT_EQUAL(tools::Long(0), aRectFromRange.GetHeight());
        }
        {
            basegfx::B2DRange aB2DRange(0.5, 0.5, 1.5, 1.5);
            gfx::Range2DL aRange = gfx::Range2DLWrap::create(aB2DRange, gfx::LengthUnit::hmm);
            CPPUNIT_ASSERT_EQUAL(180_emu, aRange.getMinX());
            CPPUNIT_ASSERT_EQUAL(180_emu, aRange.getMinY());
            CPPUNIT_ASSERT_EQUAL(540_emu, aRange.getMaxX());
            CPPUNIT_ASSERT_EQUAL(540_emu, aRange.getMaxY());
        }
    }

    void testTuple2DLWrap()
    {
        {
            auto aTuple = gfx::Tuple2DLWrap::create(Point(1, 2), gfx::LengthUnit::hmm);
            CPPUNIT_ASSERT_EQUAL(1_hmm, aTuple.getX());
            CPPUNIT_ASSERT_EQUAL(2_hmm, aTuple.getY());
        }
        {
            gfx::Tuple2DLWrap aTuple{ 1_cm, 2_cm };
            CPPUNIT_ASSERT_EQUAL(1_cm, aTuple.getX());
            CPPUNIT_ASSERT_EQUAL(2_cm, aTuple.getY());
        }
        {
            gfx::Tuple2DLWrap aTuple(gfx::LengthUnit::twip);
            aTuple.setUnitX(1);
            aTuple.setUnitY(2);
            CPPUNIT_ASSERT_EQUAL(1_twip, aTuple.getX());
            CPPUNIT_ASSERT_EQUAL(2_twip, aTuple.getY());
            CPPUNIT_ASSERT_EQUAL(1.0, aTuple.getUnitX());
            CPPUNIT_ASSERT_EQUAL(2.0, aTuple.getUnitY());
            aTuple += gfx::Tuple2DLWrap{ 8_twip, 16_twip };
            CPPUNIT_ASSERT_EQUAL(9_twip, aTuple.getX());
            CPPUNIT_ASSERT_EQUAL(18_twip, aTuple.getY());

            Point aPoint = aTuple.toPoint();
            CPPUNIT_ASSERT_EQUAL(tools::Long(9), aPoint.X());
            CPPUNIT_ASSERT_EQUAL(tools::Long(18), aPoint.Y());
        }
    }

    CPPUNIT_TEST_SUITE(LengthTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testUnitConversion);
    CPPUNIT_TEST(testLimits);
    CPPUNIT_TEST(testAdditionAndSubstitution);
    CPPUNIT_TEST(testMultiplication);
    CPPUNIT_TEST(testDivision);
    CPPUNIT_TEST(testComparison);
    CPPUNIT_TEST(testGenericFrom);
    CPPUNIT_TEST(testGenericAs);
    //
    CPPUNIT_TEST(testSizeLWrap);
    CPPUNIT_TEST(testInTuple);
    CPPUNIT_TEST(testConversionToRectangle);
    CPPUNIT_TEST_SUITE_END();
};
CPPUNIT_TEST_SUITE_REGISTRATION(LengthTest);

class Range2DLTest : public CppUnit::TestFixture
{
    void testInRange()
    {
        gfx::Range2DL aRange(1_cm, 2_cm, 2_cm, 30_mm);
        CPPUNIT_ASSERT_EQUAL(1_cm, aRange.getMinX());
        CPPUNIT_ASSERT_EQUAL(2_cm, aRange.getMaxX());
        CPPUNIT_ASSERT_EQUAL(2_cm, aRange.getMinY());
        CPPUNIT_ASSERT_EQUAL(3_cm, aRange.getMaxY());

        CPPUNIT_ASSERT_EQUAL(1_cm, aRange.getWidth());
        CPPUNIT_ASSERT_EQUAL(10_mm, aRange.getHeight());

        aRange.shift(1_cm, 1_cm);
        CPPUNIT_ASSERT_EQUAL(2_cm, aRange.getMinX());
        CPPUNIT_ASSERT_EQUAL(3_cm, aRange.getMinY());
        CPPUNIT_ASSERT_EQUAL(3_cm, aRange.getMaxX());
        CPPUNIT_ASSERT_EQUAL(40_mm, aRange.getMaxY());

        aRange.setSize(5_cm, 2_cm);
        CPPUNIT_ASSERT_EQUAL(2_cm, aRange.getMinX());
        CPPUNIT_ASSERT_EQUAL(3_cm, aRange.getMinY());
        CPPUNIT_ASSERT_EQUAL(7_cm, aRange.getMaxX());
        CPPUNIT_ASSERT_EQUAL(5_cm, aRange.getMaxY());

        aRange.setPosition(0_cm, 0_cm);
        CPPUNIT_ASSERT_EQUAL(0_cm, aRange.getMinX());
        CPPUNIT_ASSERT_EQUAL(0_cm, aRange.getMinY());
        CPPUNIT_ASSERT_EQUAL(5_cm, aRange.getMaxX());
        CPPUNIT_ASSERT_EQUAL(2_cm, aRange.getMaxY());
    }

    CPPUNIT_TEST_SUITE(Range2DLTest);
    CPPUNIT_TEST(testInRange);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Range2DLTest);
