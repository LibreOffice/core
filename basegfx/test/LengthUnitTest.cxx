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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

static_assert(gfx::Length() == 0_mm);
static_assert(gfx::Length() == 0_emu);
static_assert(gfx::Length() == gfx::Length::emu(0));
static_assert(gfx::Length() == gfx::Length::from(gfx::LengthUnit::emu, 0));

static_assert(1_cm + gfx::Length::cm(2) + gfx::Length::from(gfx::LengthUnit::cm, 2) == 5_cm);
static_assert(5_cm - gfx::Length::cm(1) - gfx::Length::from(gfx::LengthUnit::cm, 2) == 2_cm);

static constexpr gfx::Length checkOperators()
{
    auto cm1 = gfx::Length::from(gfx::LengthUnit::hmm, 2000) / 2; // 2_cm / 2
    auto cm2 = gfx::Length::mm(20); // 2_cm

    gfx::Length value = cm2 - cm1; // 2_cm - 1_cm = 1_cm
    value -= 1_cm + 10_cm - 6_cm; // = -4_cm
    value += 1_cm + 1_cm + 3_cm; // = 1_cm
    value *= 1 + 1; // = 2_cm
    value -= 1_cm * 8; // = -6_cm
    value += 10 * 2_cm; // = 14_cm
    value /= 1 + 2 - 1; // = 7_cm
    return -value;
}

static_assert(checkOperators() == -7_cm);

static_assert(gfx::Length::cm(2).as_cm() == 2);
static_assert(gfx::Length::cm(2).as_mm() == 20);
static_assert(gfx::Length::cm(2).as_hmm() == 2000);

class LengthTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(LengthTest, testDefaultConstruction)
{
    gfx::Length nLength;
    CPPUNIT_ASSERT_EQUAL(0_emu, nLength);
    CPPUNIT_ASSERT_EQUAL(0_twip, nLength);
    CPPUNIT_ASSERT_EQUAL(0_hmm, nLength);
    CPPUNIT_ASSERT_EQUAL(0_cm, nLength);
}

CPPUNIT_TEST_FIXTURE(LengthTest, testCreation)
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

    auto forceInteger = gfx::Length::hmm<sal_Int64>(10);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3600), forceInteger.data());

    auto forceDouble = gfx::Length::hmm<double>(10.1);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3636), forceDouble.data());
}

CPPUNIT_TEST_FIXTURE(LengthTest, testUnitConversion)
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

CPPUNIT_TEST_FIXTURE(LengthTest, testLimits)
{
    gfx::Length maximum = gfx::Length::emu(SAL_MAX_INT64);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(256204778801.5, maximum.as_meter(), 1e-1);
    // 256204778 km
    CPPUNIT_ASSERT_EQUAL(SAL_MAX_INT64, maximum.data());

    gfx::Length minimum = gfx::Length::emu(SAL_MIN_INT64);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-256204778801.5, minimum.as_meter(), 1e-1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(SAL_MIN_INT64), minimum.as_emu(), 1e-1);
    CPPUNIT_ASSERT_EQUAL(SAL_MIN_INT64, minimum.data());
}

CPPUNIT_TEST_FIXTURE(LengthTest, testAdditionAndSubtraction)
{
    // Results in zero
    CPPUNIT_ASSERT_EQUAL(0_cm, 100_cm - 100_cm);
    CPPUNIT_ASSERT_EQUAL(0_cm, -100_cm + 100_cm);

    // Using + and - with zero
    CPPUNIT_ASSERT_EQUAL(10_cm, 10_cm + 0_cm);
    CPPUNIT_ASSERT_EQUAL(10_cm, 10_cm - 0_cm);
    CPPUNIT_ASSERT_EQUAL(10_cm, 10_cm - 0_cm + 0_emu - 0_twip);

    // Common
    CPPUNIT_ASSERT_EQUAL(6_cm, 1_cm + 5_cm);
    CPPUNIT_ASSERT_EQUAL(-4_cm, 1_cm - 5_cm);
    CPPUNIT_ASSERT_EQUAL(4_cm, -1_cm + 5_cm);
    CPPUNIT_ASSERT_EQUAL(-6_cm, -1_cm - 5_cm);

    // Long chain
    CPPUNIT_ASSERT_EQUAL(35129_hmm, 1_mm - 5_cm + 40_cm - 1_hmm + 30_hmm);

    // Floating point
    CPPUNIT_ASSERT_EQUAL(1_mm, 0.5_mm + 0.5_mm);

    // Mixed units
    CPPUNIT_ASSERT_EQUAL(180_emu + 635_emu, 0.5_hmm + 1_twip);
    CPPUNIT_ASSERT_EQUAL(554400_emu, 1_in - 1_cm);

    // Big values - 1km - 50m
    CPPUNIT_ASSERT_EQUAL(95000_cm, 1000_m - 5000_cm);

    // 27 emu + 33 emu + 360 emu = 420
    gfx::Length emus = 27_emu + 33_emu + 1_hmm;
    CPPUNIT_ASSERT_EQUAL(sal_Int64(420), emus.data());
}

CPPUNIT_TEST_FIXTURE(LengthTest, testMultiplication)
{
    // Length * Scalar case
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

    // Scalar * Length case
    CPPUNIT_ASSERT_EQUAL(sal_Int64(720), (2 * 1_hmm).data());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(720), (2.0 * 1_hmm).data());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(180), (0.5 * 1_hmm).data());

    // Floating point rounding
    // 360 * 0.3 = 108.0 (round, not truncate)
    CPPUNIT_ASSERT_EQUAL(sal_Int64(108), (1_hmm * 0.3).data());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(108), (0.3 * 1_hmm).data());
}

CPPUNIT_TEST_FIXTURE(LengthTest, testDivision)
{
    gfx::Length cm(1_cm);
    cm /= 2;
    CPPUNIT_ASSERT_EQUAL(sal_Int64(180000), cm.data());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, cm.as_cm(), 1e-4);

    gfx::Length cm4(1_cm);
    cm4 /= 2.0;
    CPPUNIT_ASSERT_EQUAL(sal_Int64(180000), cm4.data());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, cm4.as_cm(), 1e-4);

    // Floating point rounding
    // 360000 / 0.3 = 1200000 (round, not truncate)
    gfx::Length cm5(1_cm);
    cm5 /= 0.3;
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1200000), cm5.data());
}

CPPUNIT_TEST_FIXTURE(LengthTest, testLengthRatio)
{
    // Division of two lengths gives a unitless ratio
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, 10_cm / 5_cm, 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, 5_mm / 10_mm, 1e-9);

    double aRatio = gfx::Length::hmm(10) / gfx::Length::hmm(20);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, aRatio, 1e-9);

    // Works across units too
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, 1_cm / 1_mm, 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.4, 1_in / 1_mm, 0.001);
}

CPPUNIT_TEST_FIXTURE(LengthTest, testMinusOperator)
{
    gfx::Length a = 5_cm;
    gfx::Length b = -a;
    CPPUNIT_ASSERT_EQUAL(5_cm, a);
    CPPUNIT_ASSERT_EQUAL(-5_cm, b);
}

CPPUNIT_TEST_FIXTURE(LengthTest, testComparison)
{
    // Zero - should equal no matter what unit
    CPPUNIT_ASSERT_EQUAL(0_emu, 0_emu);
    CPPUNIT_ASSERT_EQUAL(0_emu, 0_in);
    CPPUNIT_ASSERT_EQUAL(0_twip, 0_hmm);
    CPPUNIT_ASSERT_EQUAL(0_emu, -0_emu);

    // Not equal - same units
    CPPUNIT_ASSERT(1_emu != 2_emu);
    CPPUNIT_ASSERT(1_emu != 0_emu);

    // Not equal - different units, same value
    CPPUNIT_ASSERT(1_hmm != 1_emu);
    CPPUNIT_ASSERT(1_twip != 1_hmm);

    // Less
    CPPUNIT_ASSERT_EQUAL(true, 1_emu < 2_emu);
    CPPUNIT_ASSERT_EQUAL(false, 1_emu < 1_emu);

    // Less or equal
    CPPUNIT_ASSERT_EQUAL(true, 1_emu <= 2_emu);
    CPPUNIT_ASSERT_EQUAL(true, 1_emu <= 1_emu);

    // More
    CPPUNIT_ASSERT_EQUAL(true, 2_emu > 1_emu);
    CPPUNIT_ASSERT_EQUAL(false, 1_emu < 1_emu);

    // More or equal
    CPPUNIT_ASSERT_EQUAL(true, 2_emu >= 1_emu);
    CPPUNIT_ASSERT_EQUAL(true, 2_emu >= 2_emu);
}

CPPUNIT_TEST_FIXTURE(LengthTest, testGenericFrom)
{
    auto hmm3 = gfx::Length::from(gfx::LengthUnit::hmm, 3);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1080), hmm3.data());
    CPPUNIT_ASSERT_EQUAL(gfx::Length::hmm(3), hmm3);
    CPPUNIT_ASSERT_EQUAL(3_hmm, hmm3);

    auto twip2 = gfx::Length::from(gfx::LengthUnit::twip, 2);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1270), twip2.data());
    CPPUNIT_ASSERT_EQUAL(gfx::Length::twip(2), twip2);
}

CPPUNIT_TEST_FIXTURE(LengthTest, testGenericAs)
{
    CPPUNIT_ASSERT_EQUAL(1.0, (100_hmm).as(gfx::LengthUnit::mm));
    CPPUNIT_ASSERT_EQUAL(100.0, (1_m).as(gfx::LengthUnit::cm));
}

CPPUNIT_TEST_FIXTURE(LengthTest, testRoundTrip)
{
    // Integer twip and hmm round-trip must be exact
    for (sal_Int32 number : { 0, 1, -1, 100, -100, 1440, 32767 })
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(number), gfx::Length::twip(number).as_twip(), 0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(double(number), gfx::Length::hmm(number).as_hmm(), 0);
    }

    // twip -> hmm is generally not an integer
    // 100 twips = 63500 EMU, 63500 / 360 = 176.388...
    CPPUNIT_ASSERT(gfx::Length::twip(100).as_hmm() != 176.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(176.3889, gfx::Length::twip(100).as_hmm(), 0.001);
}

CPPUNIT_TEST_FIXTURE(LengthTest, testSmallestUnit)
{
    // 1 EMU is the smallest representable non-zero difference
    CPPUNIT_ASSERT(1_emu != 0_emu);
    CPPUNIT_ASSERT_EQUAL(2_emu, 1_emu + 1_emu);
    CPPUNIT_ASSERT_EQUAL(0_emu, 1_emu - 1_emu);
}

CPPUNIT_TEST_FIXTURE(LengthTest, testMixedUnitArithmetic)
{
    // 1 twip = 635 EMU, 1 hmm = 360 EMU
    CPPUNIT_ASSERT_EQUAL(sal_Int64(995), (1_twip + 1_hmm).data());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(275), (1_twip - 1_hmm).data());
}

CPPUNIT_TEST_FIXTURE(LengthTest, testCopyAndAssignment)
{
    gfx::Length a = 5_cm;
    gfx::Length b(a);
    CPPUNIT_ASSERT_EQUAL(a, b);

    gfx::Length c;
    c = a;
    CPPUNIT_ASSERT_EQUAL(a, c);

    // Modifying copy doesn't affect original
    b += 1_cm;
    CPPUNIT_ASSERT_EQUAL(5_cm, a);
    CPPUNIT_ASSERT_EQUAL(6_cm, b);
}

CPPUNIT_TEST_FIXTURE(LengthTest, testNegativeValues)
{
    auto negative = gfx::Length::twip(-500);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-500.0, negative.as_twip(), 0);
    CPPUNIT_ASSERT(negative < 0_emu);

    auto negativeHmm = gfx::Length::hmm(-1000);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1000.0, negativeHmm.as_hmm(), 0);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(-360000), negativeHmm.data());
}

class Tuple2DLTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(Tuple2DLTest, testTuple)
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

class Size2DLTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(Size2DLTest, testSize)
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

class Range2DLTest : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(Range2DLTest, testRange)
{
    gfx::Range2DL aRange(1_cm, 2_cm, 2_cm, 30_mm);
    CPPUNIT_ASSERT_EQUAL(1_cm, aRange.getMinX());
    CPPUNIT_ASSERT_EQUAL(2_cm, aRange.getMaxX());
    CPPUNIT_ASSERT_EQUAL(2_cm, aRange.getMinY());
    CPPUNIT_ASSERT_EQUAL(3_cm, aRange.getMaxY());

    CPPUNIT_ASSERT_EQUAL(1_cm, aRange.getWidth());
    CPPUNIT_ASSERT_EQUAL(10_mm, aRange.getHeight());
}
