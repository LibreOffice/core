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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/fract.hxx>
#include <tools/mapunit.hxx>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>

namespace
{
class TestVirtualDevice : public VirtualDevice
{
public:
    // Expose protected methods using 'using'
    using OutputDevice::LogicHeightToDevicePixel;
    using OutputDevice::LogicWidthToDevicePixel;
    using OutputDevice::SetOutOffXPixel;
    using OutputDevice::SetOutOffYPixel;

    // Explicit wrapper if 'using' doesn't satisfy specific compiler strictness
    // (Optional, but 'using' is usually sufficient for access)
};

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testIdentity)
{
    // Test that converting to the same MapMode changes nothing
    MapMode aMode(MapUnit::MapMM);
    Point aPt(1234, 5678);

    Point aResult = OutputDevice::LogicToLogic(aPt, aMode, aMode);

    CPPUNIT_ASSERT_EQUAL(tools::Long(1234), aResult.X());
    CPPUNIT_ASSERT_EQUAL(tools::Long(5678), aResult.Y());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testSimpleUnitConversion)
{
    // Test MM to CM (10mm = 1cm)
    MapMode aSource(MapUnit::MapMM);
    MapMode aDest(MapUnit::MapCM);
    Point aPt(100, 200); // 100mm, 200mm

    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aResult.X()); // 10cm
    CPPUNIT_ASSERT_EQUAL(tools::Long(20), aResult.Y()); // 20cm
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testFractionScaling)
{
    // Converting 100 Source -> 50 Dest

    MapMode aSource(MapUnit::MapMM);
    aSource.SetScaleX(0.5);
    aSource.SetScaleY(0.5);

    MapMode aDest(MapUnit::MapMM); // Default 1:1

    Point aPt(100, 200);
    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    CPPUNIT_ASSERT_EQUAL(tools::Long(50), aResult.X());
    CPPUNIT_ASSERT_EQUAL(tools::Long(100), aResult.Y());
}

// Fails on Windows 32-bit. See include/tools/long.hxx
#if defined _WIN64 || !defined _WIN32
CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testIntermediateOverflow)
{
    constexpr tools::Long nLarge = static_cast<tools::Long>(1LL << 30);
    constexpr tools::Long nExpected = static_cast<tools::Long>(25400000000);

    MapMode aSource(MapUnit::MapInch);
    aSource.SetScaleX(nLarge);

    MapMode aDest(MapUnit::Map100thMM);
    aDest.SetScaleX(nLarge);

    // n1 (Coord) = 10,000,000
    // n2 (Scale) = 2^30 (~10^9)
    // n3 (Unit)  = 2540
    // Product = 10^7 * 10^9 * 2540 ~= 2.5 * 10^19
    // Max Int64 ~= 9 * 10^18
    // THIS WILL OVERFLOW 64-bit integer logic if BigInt is missing.
    Point aPt(10000000, 0);

    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    // Expected: 10,000,000 inches = 25,400,000,000 100thMM
    CPPUNIT_ASSERT_EQUAL(nExpected, aResult.X());
}
#endif

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testRoundingBehavior)
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

// Fails on Windows 32-bit. See include/tools/long.hxx
#if defined _WIN64 || !defined _WIN32
CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testOverflowProtectionPositive)
{
    // Scenario: Convert Inch to MM (IsSimple() == true)
    // Factor: 127 / 5 (1 inch = 25.4 mm = 254/10 = 127/5)
    //
    // We need an input N such that:
    // 1. N * 127 > INT64_MAX (9.22e18)  --> Triggers overflow flag in o3tl::convert
    // 2. (N * 127) / 5 < INT64_MAX      --> Final result fits in tools::Long

    constexpr tools::Long nInput = static_cast<tools::Long>(100000000000000000LL); // 1e17
    constexpr tools::Long nExpected = static_cast<tools::Long>(2540000000000000000LL); // 2.54e18

    MapMode aSource(MapUnit::MapInch);
    MapMode aDest(MapUnit::MapMM);

    // Sanity check: Ensure our MapModes are indeed "Simple" so we hit fn3
    CPPUNIT_ASSERT(aSource.IsSimple());
    CPPUNIT_ASSERT(aDest.IsSimple());

    Point aPt(nInput, 0);
    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    CPPUNIT_ASSERT_EQUAL(nExpected, aResult.X());
}
#endif

// Fails on Windows 32-bit. See include/tools/long.hxx
#if defined _WIN64 || !defined _WIN32
CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testOverflowProtectionNegative)
{
    // Same as above, but testing negative handling in BigInt path
    constexpr tools::Long nInput = static_cast<tools::Long>(-100000000000000000LL); // -1e17
    constexpr tools::Long nExpected = static_cast<tools::Long>(-2540000000000000000LL);

    MapMode aSource(MapUnit::MapInch);
    MapMode aDest(MapUnit::MapMM);

    Point aPt(nInput, 0);
    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    CPPUNIT_ASSERT_EQUAL(nExpected, aResult.X());
}
#endif

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testRounding)
{
    // Conversion: MapMM to MapInch
    // Factor: 5 / 127

    MapMode aSource(MapUnit::MapMM);
    MapMode aDest(MapUnit::MapInch);

    // 13 * 5 / 127 = 65 / 127 ~= 0.5118 -> Rounds to 1
    Point aPt(13, 12);
    // 12 * 5 / 127 = 60 / 127 ~= 0.47 -> Rounds to 0

    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    CPPUNIT_ASSERT_EQUAL(tools::Long(1), aResult.X());
    CPPUNIT_ASSERT_EQUAL(tools::Long(0), aResult.Y());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testTwipsTo100thMM)
{
    // 1 Twip = 1/1440 inch
    // 1 inch = 2540 100thMM
    // Factor = 2540 / 1440 = 127 / 72 ~= 1.7638

    MapMode aSource(MapUnit::MapTwip);
    MapMode aDest(MapUnit::Map100thMM);

    Point aPt(7200, 1440); // 5 inches, 1 inch

    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    // 5 inches = 12700 100thMM
    CPPUNIT_ASSERT_EQUAL(tools::Long(12700), aResult.X());
    // 1 inch = 2540 100thMM
    CPPUNIT_ASSERT_EQUAL(tools::Long(2540), aResult.Y());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testPointsToInch)
{
    // 1 Point = 1/72 inch
    MapMode aSource(MapUnit::MapPoint);
    MapMode aDest(MapUnit::MapInch);

    Point aPt(72, 144); // 1 inch, 2 inches

    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    CPPUNIT_ASSERT_EQUAL(tools::Long(1), aResult.X());
    CPPUNIT_ASSERT_EQUAL(tools::Long(2), aResult.Y());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testZoomScaling)
{
    // Simulating a View Zoom
    // Source: 100thMM at 400% zoom (Scale 4:1)
    // Dest:   100thMM at 100% zoom (Scale 1:1)

    MapMode aSource(MapUnit::Map100thMM);
    aSource.SetScaleX(4.0);
    aSource.SetScaleY(4.0);

    MapMode aDest(MapUnit::Map100thMM);

    Point aPt(100, 100);
    // If I have 100 units at 400% zoom, that is physically 400 units.
    // Converting to 100% zoom, it should remain 400 units.

    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    CPPUNIT_ASSERT_EQUAL(tools::Long(400), aResult.X());
    CPPUNIT_ASSERT_EQUAL(tools::Long(400), aResult.Y());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testComplexFractions)
{
    // Use prime number fractions to ensure rational arithmetic
    // is maintained without premature truncation.

    // Source Scale: 1/3
    // Dest Scale:   1/7
    // Conversion:   Val * (1/3) / (1/7) = Val * 7/3 ~= Val * 2.333

    MapMode aSource(MapUnit::MapMM);
    aSource.SetScaleX(1.0 / 3);

    MapMode aDest(MapUnit::MapMM);
    aDest.SetScaleX(1.0 / 7);

    Point aPt(300, 0);
    // 300 * 7 / 3 = 100 * 7 = 700

    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    CPPUNIT_ASSERT_EQUAL(tools::Long(700), aResult.X());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testNullptrMapMode)
{
    // Ensure that passing nullptr to the LogicToLogic pointer overload correctly
    // falls back to the OutputDevice's MapMode without dereferencing a null pointer.

    ScopedVclPtrInstance<VirtualDevice> pDev;
    pDev->SetMapMode(MapMode(MapUnit::MapMM));

    Point aPt(10, 20);
    MapMode aDestMode(MapUnit::Map100thMM);

    // Source is nullptr (falls back to device MapMM) -> Dest is Map100thMM
    // 10mm -> 1000 100thMM, 20mm -> 2000 100thMM
    Point aResult1 = pDev->LogicToLogic(aPt, nullptr, &aDestMode);
    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), aResult1.X());
    CPPUNIT_ASSERT_EQUAL(tools::Long(2000), aResult1.Y());

    // Source is Map100thMM -> Dest is nullptr (falls back to device MapMM)
    // 1500 100thMM -> 15mm, 2500 100thMM -> 25mm
    Point aPt2(1500, 2500);
    Point aResult2 = pDev->LogicToLogic(aPt2, &aDestMode, nullptr);
    CPPUNIT_ASSERT_EQUAL(tools::Long(15), aResult2.X());
    CPPUNIT_ASSERT_EQUAL(tools::Long(25), aResult2.Y());

    // Both are nullptr (falls back to device MapMM for both)
    // Should return the exact same coordinates
    Point aResult3 = pDev->LogicToLogic(aPt, nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(tools::Long(10), aResult3.X());
    CPPUNIT_ASSERT_EQUAL(tools::Long(20), aResult3.Y());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testCombinedScaleAndOrigin)
{
    // Scenario: Source is MM with 50% scale. Dest is 100thMM.
    MapMode aSource(MapUnit::MapMM);
    aSource.SetScaleX(0.5);
    aSource.SetScaleY(0.5);
    aSource.SetOrigin(Point(10, 0));

    MapMode aDest(MapUnit::Map100thMM);

    Point aPt(30, 0);

    // Standard GDI logic would imply: (Point - Origin) * Scale
    // i.e., (30 - 10) * 0.5 = 10mm -> 1000 100thMM.
    //
    // However, VCL historically treats SetOrigin as a Translation Offset (Additive).
    // Actual Calculation: (Point + Origin) * Scale
    // i.e., (30 + 10) * 0.5 = 20mm -> 2000 100thMM.

    Point aResult = OutputDevice::LogicToLogic(aPt, aSource, aDest);

    // We assert 2000 to enforce consistency with LogicToPixel and historical rendering.
    CPPUNIT_ASSERT_EQUAL(tools::Long(2000), aResult.X());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testBasicLogicToPixel)
{
    // Use VirtualDevice as it is lightweight and headless-compatible
    ScopedVclPtr<TestVirtualDevice> pDev(VclPtr<TestVirtualDevice>::Create());

    // 1. Default MapMode (MapPixel)
    // In MapPixel, Logic == Pixel
    Point aLogicPt(100, 100);
    Point aPixelPt = pDev->LogicToPixel(aLogicPt);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default MapMode should map 1:1", aLogicPt, aPixelPt);

    // 2. LogicToPixel with specific MapMode override
    // 100th MM to Pixel.
    // Assuming 96 DPI for the test VirtualDevice (standard for headless)
    // 1 inch = 2540 100th MM.
    // 96 pixels = 2540 logic units.
    // 1000 logic units = (1000 * 96) / 2540 ~= 37 pixels.
    MapMode aMap100thMM(MapUnit::Map100thMM);
    Point aConverted = pDev->LogicToPixel(Point(1000, 1000), aMap100thMM);

    // We allow a small margin for integer arithmetic rounding, though 37 is exact-ish
    CPPUNIT_ASSERT(aConverted.X() >= 37);
    CPPUNIT_ASSERT(aConverted.X() <= 38);
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testMapModePropagation)
{
    ScopedVclPtr<TestVirtualDevice> pDev(VclPtr<TestVirtualDevice>::Create());

    // Set MapMode on OutDev and ensure logic functions use it
    MapMode aMapMode(MapUnit::Map100thMM);
    pDev->SetMapMode(aMapMode);

    // Verify state getter
    CPPUNIT_ASSERT_EQUAL(MapUnit::Map100thMM, pDev->GetMapMode().GetMapUnit());

    // Test calculation through the residual wrapper
    // 1000 100thmm @ 96 DPI ~= 37.7 pixels
    long nHeightVal = 1000;
    long nPixelHeight = pDev->LogicHeightToDevicePixel(nHeightVal);

    CPPUNIT_ASSERT(nPixelHeight >= 37);
    CPPUNIT_ASSERT(nPixelHeight <= 38);

    // Change MapMode again to verify Reset/Update logic in map.cxx
    MapMode aTwips(MapUnit::MapTwip);
    pDev->SetMapMode(aTwips);

    // 1440 Twips = 1 Inch = 96 Pixels (usually)
    long nPixelTwips = pDev->LogicHeightToDevicePixel(1440);

    // Check if the change propagated to the internal mapper
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1440 Twips should be approx 96 pixels", long(96), nPixelTwips);
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testSetRelativeMapMode)
{
    ScopedVclPtr<TestVirtualDevice> pDev(VclPtr<TestVirtualDevice>::Create());

    // Initial MapMode: 100th MM
    pDev->SetMapMode(MapMode(MapUnit::Map100thMM));

    // Target MapMode: Scale by 2 (Relative)
    MapMode aScaleMode(MapUnit::Map100thMM);
    aScaleMode.SetScaleX(2);
    aScaleMode.SetScaleY(2);

    // This calls the residual SetRelativeMapMode in map.cxx
    pDev->SetRelativeMapMode(aScaleMode);

    // 1000 logic units * 2 (scale) -> converted to pixels
    // 1000 units normally ~37 pixels. Scaled by 2 should be ~75 pixels.
    Point aPt(1000, 1000);
    Point aPix = pDev->LogicToPixel(aPt);

    CPPUNIT_ASSERT(aPix.X() > 70);
    CPPUNIT_ASSERT(aPix.X() < 80);

    // Ensure the internal mapper updated its scale factor
    double nResScale = pDev->GetMapMode().GetScaleX();
    CPPUNIT_ASSERT_EQUAL(double(2), nResScale);
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testViewTransformation)
{
    ScopedVclPtr<TestVirtualDevice> pDev(VclPtr<TestVirtualDevice>::Create());

    pDev->SetMapMode(MapMode(MapUnit::Map100thMM));
    pDev->SetOutOffXPixel(10);

    // Get transformation matrix via residual wrapper
    basegfx::B2DHomMatrix aMat = pDev->GetViewTransformation();

    // Transform a point manually using the matrix
    basegfx::B2DPoint aPt(1000.0, 1000.0); // 1000 100th mm
    aPt *= aMat;

    // Transform using the convenience function
    Point aLogicPt(1000, 1000);
    Point aPix = pDev->LogicToPixel(aLogicPt);

    // Compare Matrix result vs Helper result
    // Matrix result includes floating point precision, Helper rounds.
    // LogicToPixel = (Logic * Scale) + Offset
    // Offset is 10. 1000 100thMM is approx 37.8 pixels. Total ~47.8.

    CPPUNIT_ASSERT_DOUBLES_EQUAL(double(aPix.X()), aPt.getX(), 1.0);
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testMapRelativeScaling)
{
    // Setup Mapper with a known state (e.g., 100thMM)
    // We use a dummy OutputDevice/VirtualDevice to get a mapper instance
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));

    // Create a Relative MapMode that specifies a 200% zoom (2/1 scale)
    // and a translation offset of 100 units.
    MapMode aRelativeMode(MapUnit::MapRelative);
    aRelativeMode.SetScaleX(2.0);
    aRelativeMode.SetScaleY(2.0);
    aRelativeMode.SetOrigin(Point(100, 100));

    // Test conversion from a Point in 'Relative' space to the 'Base' space.
    // LogicToLogic(Point, pSourceMode, pDestMode)
    // Passing nullptr for a mode pointer defaults to the Device's current MapMode.
    Point aPt(50, 50);

    // The ResolveMapRes logic inside the mapper is triggered here:
    Point aResult = pVDev->LogicToLogic(aPt, &aRelativeMode, nullptr);

    // EXPECTED CALCULATION:
    // VCL MapMode math applies the Origin translation BEFORE the Scale factor!
    // Result.X = (Point.X + RelativeOrigin.X) * RelativeScale.X
    // Result.X = (50 + 100) * 2 = 150 * 2 = 300
    //
    // IF THE BUG IS PRESENT (Early Return):
    // The mapper returns the default resolution immediately, ignoring the scale/origin.
    // Result.X = 50

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Relative scaling was ignored!", tools::Long(300), aResult.X());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Relative origin was ignored!", tools::Long(300), aResult.Y());
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testMapModeInvalidation)
{
    ScopedVclPtrInstance<VirtualDevice> pVDev;
    MapMode aMapMode(MapUnit::Map100thMM); // 1 unit = 0.01mm
    pVDev->SetMapMode(aMapMode);
    pVDev->EnableMapMode(true);

    // Capture the initial logic-to-pixel result
    Point aLogicPt(1000, 1000);
    Point aPixelPt1 = pVDev->LogicToPixel(aLogicPt);

    // DISABLE MapMode
    // This is where your bug lived!
    pVDev->EnableMapMode(false);
    Point aPixelPt2 = pVDev->LogicToPixel(aLogicPt);

    // In 'false' mode, LogicToPixel should be an identity (1:1)
    CPPUNIT_ASSERT_EQUAL(aLogicPt.X(), aPixelPt2.X());
    CPPUNIT_ASSERT_EQUAL(aLogicPt.Y(), aPixelPt2.Y());

    // RE-ENABLE MapMode
    // This verifies the 'true' restoration correctly invalidates the cache
    pVDev->EnableMapMode(true);
    Point aPixelPt3 = pVDev->LogicToPixel(aLogicPt);

    // This should match the very first calculation
    CPPUNIT_ASSERT_EQUAL(aPixelPt1.X(), aPixelPt3.X());
    CPPUNIT_ASSERT_EQUAL(aPixelPt1.Y(), aPixelPt3.Y());
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
