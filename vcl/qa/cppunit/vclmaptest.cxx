/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <tools/fract.hxx>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>

namespace
{
class VclMapTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(VclMapTest);
    CPPUNIT_TEST(testIdentity);
    CPPUNIT_TEST(testSimpleUnitConversion);
    CPPUNIT_TEST(testFractionScaling);
    CPPUNIT_TEST(testIntermediateOverflow);
    CPPUNIT_TEST(testRoundingBehavior);
    CPPUNIT_TEST_SUITE_END();

public:
    void testIdentity()
    {
        // Test that converting to the same MapMode changes nothing
        MapMode aMode(MapUnit::MapMM);
        Point aPt(1234, 5678);

        Point aResult = OutputDevice::LogicToLogic(aPt, aMode, aMode);

        CPPUNIT_ASSERT_EQUAL(tools::Long(1234), aResult.X());
        CPPUNIT_ASSERT_EQUAL(tools::Long(5678), aResult.Y());
    }

    void testSimpleUnitConversion()
    {
        // Test MM to CM (10mm = 1cm)
        MapMode aSource(MapUnit::MapMM);
        MapMode aDest(MapUnit::MapCM);
        Point aPt(100, 200); // 100mm, 200mm

        Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

        CPPUNIT_ASSERT_EQUAL(tools::Long(10), aResult.X()); // 10cm
        CPPUNIT_ASSERT_EQUAL(tools::Long(20), aResult.Y()); // 20cm
    }

    void testFractionScaling()
    {
        // Converting 100 Source -> 50 Dest

        MapMode aSource(MapUnit::MapMM);
        aSource.SetScaleX(Fraction(1, 2));
        aSource.SetScaleY(Fraction(1, 2));

        MapMode aDest(MapUnit::MapMM); // Default 1:1

        Point aPt(100, 200);
        Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

        CPPUNIT_ASSERT_EQUAL(tools::Long(50), aResult.X());
        CPPUNIT_ASSERT_EQUAL(tools::Long(100), aResult.Y());
    }

    void testIntermediateOverflow()
    {
        tools::Long nLarge = 1LL << 30;

        MapMode aSource(MapUnit::MapInch);
        aSource.SetScaleX(Fraction(nLarge, 1));

        MapMode aDest(MapUnit::Map100thMM);
        aDest.SetScaleX(Fraction(nLarge, 1));

        // n1 (Coord) = 10,000,000
        // n2 (Scale) = 2^30 (~10^9)
        // n3 (Unit)  = 2540
        // Product = 10^7 * 10^9 * 2540 ~= 2.5 * 10^19
        // Max Int64 ~= 9 * 10^18
        // THIS WILL OVERFLOW 64-bit integer logic if BigInt is missing.
        Point aPt(10000000, 0);

        Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

        // Expected: 10,000,000 inches = 25,400,000,000 100thMM
        CPPUNIT_ASSERT_EQUAL(tools::Long(25400000000), aResult.X());
    }

    void testRoundingBehavior()
    {
        // Test that 0.5 rounds correctly (usually to nearest)
        // Source: 100thMM
        // Dest:   MM
        // 150 100thMM = 1.5 MM -> Should round to 2
        // 140 100thMM = 1.4 MM -> Should round to 1

        MapMode aSource(MapUnit::Map100thMM);
        MapMode aDest(MapUnit::MapMM);

        Point aPt1(150, 140);
        Point aResult1 = OutputDevice::LogicToLogic(aPt1, aSource, aDest);

        CPPUNIT_ASSERT_EQUAL(tools::Long(2), aResult1.X()); // 1.5 -> 2
        CPPUNIT_ASSERT_EQUAL(tools::Long(1), aResult1.Y()); // 1.4 -> 1

        // Test Negative Rounding
        Point aPt2(-150, -140);
        Point aResult2 = OutputDevice::LogicToLogic(aPt2, aSource, aDest);

        CPPUNIT_ASSERT_EQUAL(tools::Long(-2), aResult2.X()); // -1.5 -> -2
        CPPUNIT_ASSERT_EQUAL(tools::Long(-1), aResult2.Y()); // -1.4 -> -1
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(VclMapTest);

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
