/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_OUTDEVTESTS_HXX
#define INCLUDED_VCL_OUTDEVTESTS_HXX

#include <vcl/virdev.hxx>

namespace vcl::test {

/** Rendering test result.
 *
 * Test either "Passed", "Failed" or "PassedWithQuirks" which means
 * the test passed but at least one rendering quirk was detected.
 */
enum class TestResult
{
    Failed,
    PassedWithQuirks,
    Passed
};

/** Common subclass for output device rendering tests.
 */
class VCL_DLLPUBLIC OutputDeviceTestCommon
{
protected:

    ScopedVclPtr<VirtualDevice> mpVirtualDevice;
    tools::Rectangle maVDRectangle;

    static const Color constBackgroundColor;
    static const Color constLineColor;
    static const Color constFillColor;

public:
    OutputDeviceTestCommon();

    OUString getRenderBackendName() const;

    void initialSetup(long nWidth, long nHeight, Color aColor, bool bEnableAA = false, bool bAlphaVirtualDevice = false);

    static TestResult checkRectangle(Bitmap& rBitmap);
    static TestResult checkRectangleAA(Bitmap& rBitmap);
    static TestResult checkFilledRectangle(Bitmap& rBitmap, bool useLineColor);
    static TestResult checkLines(Bitmap& rBitmap);
    static TestResult checkAALines(Bitmap& rBitmap);
    static TestResult checkDiamond(Bitmap& rBitmap);

    static TestResult checkInvertRectangle(Bitmap& rBitmap);
    static TestResult checkInvertN50Rectangle(Bitmap& aBitmap);
    static TestResult checkInvertTrackFrameRectangle(Bitmap& aBitmap);

    static TestResult checkRectangles(Bitmap& rBitmap, std::vector<Color>& aExpectedColors);
    static TestResult checkRectangle(Bitmap& rBitmap, int aLayerNumber, Color aExpectedColor);

    static TestResult checkFilled(Bitmap& rBitmap, tools::Rectangle aRectangle, Color aExpectedColor);
    static TestResult checkChecker(Bitmap& rBitmap, sal_Int32 nStartX, sal_Int32 nEndX,
                                   sal_Int32 nStartY, sal_Int32 nEndY, std::vector<Color> const & rExpected);

    static TestResult checkLinearGradient(Bitmap& bitmap);
    static TestResult checkLinearGradientAngled(Bitmap& bitmap);
    static TestResult checkLinearGradientBorder(Bitmap& bitmap);
    static TestResult checkLinearGradientIntensity(Bitmap& bitmap);
    static TestResult checkLinearGradientSteps(Bitmap& bitmap);
    static TestResult checkAxialGradient(Bitmap& bitmap);
    static TestResult checkRadialGradient(Bitmap& bitmap);
    static TestResult checkRadialGradientOfs(Bitmap& bitmap);

    static void createDiamondPoints(tools::Rectangle rRect, int nOffset,
                                    Point& rPoint1, Point& rPoint2,
                                    Point& rPoint3, Point& rPoint4);

    static void createHorizontalVerticalDiagonalLinePoints(tools::Rectangle rRect,
                                Point& rHorizontalLinePoint1, Point& rHorizontalLinePoint2,
                                Point& rVerticalLinePoint1, Point& rVerticalLinePoint2,
                                Point& rDiagonalLinePoint1, Point& rDiagonalLinePoint2);
    // tools
    static tools::Rectangle alignToCenter(tools::Rectangle aRect1, tools::Rectangle aRect2);

    static TestResult checkBezier(Bitmap& rBitmap);
};

class VCL_DLLPUBLIC OutputDeviceTestBitmap : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestBitmap() = default;

    Bitmap setupDrawTransformedBitmap();
    Bitmap setupDrawBitmap();
    Bitmap setupDrawBitmapExWithAlpha();
    Bitmap setupDrawMask();
    BitmapEx setupDrawBlend();

    static TestResult checkTransformedBitmap(Bitmap& rBitmap);
    static TestResult checkBitmapExWithAlpha(Bitmap& rBitmap);
    static TestResult checkMask(Bitmap& rBitmap);
    static TestResult checkBlend(const BitmapEx& rBitmap);
};

class VCL_DLLPUBLIC OutputDeviceTestAnotherOutDev : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestAnotherOutDev() = default;

    Bitmap setupDrawOutDev();
    Bitmap setupXOR();

    static TestResult checkDrawOutDev(Bitmap& rBitmap);
    static TestResult checkXOR(Bitmap& rBitmap);
};

class VCL_DLLPUBLIC OutputDeviceTestPixel : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestPixel() = default;

    Bitmap setupRectangle(bool bEnableAA);
};

class VCL_DLLPUBLIC OutputDeviceTestLine : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestLine() = default;

    Bitmap setupRectangle(bool bEnableAA);
    Bitmap setupDiamond();
    Bitmap setupLines();
    Bitmap setupAALines();

    Bitmap setupDashedLine();
    static TestResult checkDashedLine(Bitmap& rBitmap);
};

class VCL_DLLPUBLIC OutputDeviceTestPolyLine : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestPolyLine() = default;

    Bitmap setupRectangle(bool bEnableAA);
    Bitmap setupDiamond();
    Bitmap setupLines();
    Bitmap setupAALines();
};

class VCL_DLLPUBLIC OutputDeviceTestPolyLineB2D : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestPolyLineB2D() = default;

    Bitmap setupRectangle(bool bEnableAA);
    Bitmap setupDiamond();
    Bitmap setupBezier();
    Bitmap setupAABezier();
};

class VCL_DLLPUBLIC OutputDeviceTestRect : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestRect() = default;

    Bitmap setupRectangle(bool bEnableAA);
    Bitmap setupFilledRectangle(bool useLineColor);
    Bitmap setupInvert_NONE();
    Bitmap setupInvert_N50();
    Bitmap setupInvert_TrackFrame();
};

class VCL_DLLPUBLIC OutputDeviceTestPolygon : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestPolygon() = default;

    Bitmap setupRectangle(bool bEnableAA);
    Bitmap setupFilledRectangle(bool useLineColor);
    Bitmap setupDiamond();
    Bitmap setupLines();
    Bitmap setupAALines();
};

class VCL_DLLPUBLIC OutputDeviceTestPolyPolygon : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestPolyPolygon() = default;

    Bitmap setupRectangle(bool bEnableAA);
    Bitmap setupFilledRectangle(bool useLineColor);
};

class VCL_DLLPUBLIC OutputDeviceTestPolyPolygonB2D : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestPolyPolygonB2D() = default;

    Bitmap setupRectangle(bool bEnableAA);
    Bitmap setupFilledRectangle(bool useLineColor);
};

class VCL_DLLPUBLIC OutputDeviceTestGradient : public OutputDeviceTestCommon
{
public:
    OutputDeviceTestGradient() = default;

    Bitmap setupLinearGradient();
    Bitmap setupLinearGradientAngled();
    Bitmap setupLinearGradientBorder();
    Bitmap setupLinearGradientIntensity();
    Bitmap setupLinearGradientSteps();
    Bitmap setupAxialGradient();
    Bitmap setupRadialGradient();
    Bitmap setupRadialGradientOfs();
};

class VCL_DLLPUBLIC OutputDeviceTestClip : public OutputDeviceTestCommon
{
public:
    Bitmap setupClipRectangle();
    Bitmap setupClipPolygon();
    Bitmap setupClipPolyPolygon();
    Bitmap setupClipB2DPolyPolygon();

    static TestResult checkClip(Bitmap& rBitmap);
};


} // end namespace vcl::test

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
