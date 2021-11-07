/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/outputdevice.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <salgdi.hxx>

namespace vcl::test {

namespace
{


int deltaColor(BitmapColor aColor1, BitmapColor aColor2)
{
    int deltaR = std::abs(aColor1.GetRed()   - aColor2.GetRed());
    int deltaG = std::abs(aColor1.GetGreen() - aColor2.GetGreen());
    int deltaB = std::abs(aColor1.GetBlue()  - aColor2.GetBlue());

    return std::max(std::max(deltaR, deltaG), deltaB);
}

void checkValue(BitmapScopedWriteAccess& pAccess, int x, int y, Color aExpected,
                      int& nNumberOfQuirks, int& nNumberOfErrors, bool bQuirkMode, int nColorDeltaThresh = 0)
{
    const bool bColorize = false;
    Color aColor = pAccess->GetPixel(y, x);
    int nColorDelta = deltaColor(aColor, aExpected);

    if (nColorDelta <= nColorDeltaThresh)
    {
        if (bColorize)
            pAccess->SetPixel(y, x, COL_LIGHTGREEN);
    }
    else if (bQuirkMode)
    {
        nNumberOfQuirks++;
        if (bColorize)
            pAccess->SetPixel(y, x, COL_YELLOW);
    }
    else
    {
        nNumberOfErrors++;
        if (bColorize)
            pAccess->SetPixel(y, x, COL_LIGHTRED);
    }
}

void checkValue(BitmapScopedWriteAccess& pAccess, const Point& point, Color aExpected,
                      int& nNumberOfQuirks, int& nNumberOfErrors, bool bQuirkMode, int nColorDeltaThresh = 0)
{
    checkValue(pAccess, point.getX(), point.getY(), aExpected, nNumberOfQuirks, nNumberOfErrors, bQuirkMode, nColorDeltaThresh);
}

void checkValue(BitmapScopedWriteAccess& pAccess, int x, int y, Color aExpected,
                      int& nNumberOfQuirks, int& nNumberOfErrors, int nColorDeltaThresh, int nColorDeltaThreshQuirk = 0)
{
    const bool bColorize = false;
    Color aColor = pAccess->GetPixel(y, x);
    int nColorDelta = deltaColor(aColor, aExpected);
    nColorDeltaThreshQuirk = std::max( nColorDeltaThresh, nColorDeltaThreshQuirk);

    if (nColorDelta <= nColorDeltaThresh)
    {
        if (bColorize)
            pAccess->SetPixel(y, x, COL_LIGHTGREEN);
    }
    else if (nColorDelta <= nColorDeltaThreshQuirk)
    {
        nNumberOfQuirks++;
        if (bColorize)
            pAccess->SetPixel(y, x, COL_YELLOW);
    }
    else
    {
        nNumberOfErrors++;
        if (bColorize)
            pAccess->SetPixel(y, x, COL_LIGHTRED);
    }
}

char returnDominantColor(Color aColor)
{
    int aRed = aColor.GetRed();
    int aGreen = aColor.GetGreen();
    int aBlue = aColor.GetBlue();
    if (aRed > aGreen && aRed > aBlue)
        return 'R';

    if (aGreen > aRed && aGreen > aBlue)
        return 'G';

    if(aBlue > aRed && aBlue > aGreen)
        return 'B';

    return 'X'; //No Dominant Color.
}

void checkValueAA(BitmapScopedWriteAccess& pAccess, int x, int y, Color aExpected,
                  int& nNumberOfQuirks, int& nNumberOfErrors, int nColorDeltaThresh = 64)
{
    const bool bColorize = false;
    Color aColor = pAccess->GetPixel(y, x);
    bool aColorResult = returnDominantColor(aExpected) == returnDominantColor(aColor);
    int nColorDelta = deltaColor(aColor, aExpected);

    if (nColorDelta <= nColorDeltaThresh && aColorResult)
    {
        if (bColorize)
            pAccess->SetPixel(y, x, COL_LIGHTGREEN);
    }
    else if (aColorResult)
    {
        nNumberOfQuirks++;
        if (bColorize)
            pAccess->SetPixel(y, x, COL_YELLOW);
    }
    else
    {
        nNumberOfErrors++;
        if (bColorize)
            pAccess->SetPixel(y, x, COL_LIGHTRED);
    }
}

// Return all colors in the rectangle and their count.
std::map<Color, int> collectColors(Bitmap& bitmap, const tools::Rectangle& rectangle)
{
    std::map<Color, int> colors;
    BitmapScopedWriteAccess pAccess(bitmap);
    for (tools::Long y = rectangle.Top(); y < rectangle.Bottom(); ++y)
        for (tools::Long x = rectangle.Left(); x < rectangle.Right(); ++x)
          ++colors[pAccess->GetPixel(y, x)]; // operator[] initializes to 0 (default ctor) if creating
    return colors;
}

bool checkConvexHullProperty(Bitmap& bitmap, Color constLineColor, int nWidthOffset,
                             int nHeightOffset)
{
    BitmapScopedWriteAccess pAccess(bitmap);
    tools::Long thresholdWidth = pAccess->Width() - nWidthOffset;
    tools::Long thresholdHeight = pAccess->Height() - nHeightOffset;
    for (tools::Long y = 0; y < pAccess->Height(); ++y)
    {
        for (tools::Long x = 0; x < pAccess->Width(); ++x)
        {
            /*
                If the shape exceeds the threshold limit of height or width or both,
                this would indicate that the bezier curve is not within its convex polygon and
                hence is faulty.
            */
            if (pAccess->GetPixel(y, x) == constLineColor
                && (thresholdHeight < y || thresholdWidth < x))
            {
                return false;
            }
        }
    }
    return true;
}

TestResult checkRect(Bitmap& rBitmap, int aLayerNumber, Color aExpectedColor)
{
    BitmapScopedWriteAccess pAccess(rBitmap);
    tools::Long nHeight = pAccess->Height();
    tools::Long nWidth = pAccess->Width();

    tools::Long firstX = 0 + aLayerNumber;
    tools::Long firstY = 0 + aLayerNumber;

    tools::Long lastX = nWidth  - aLayerNumber - 1;
    tools::Long lastY = nHeight - aLayerNumber - 1;

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // check corner quirks
    checkValue(pAccess, firstX, firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, firstX, lastY,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  lastY,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);

    for (tools::Long y = firstY + 1; y <= lastY - 1; y++)
    {
        checkValue(pAccess, firstX, y,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        checkValue(pAccess, lastX,  y,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
    }
    for (tools::Long x = firstX + 1; x <= lastX - 1; x++)
    {
        checkValue(pAccess, x, firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        checkValue(pAccess, x, lastY,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
    }
    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

TestResult checkHorizontalVerticalDiagonalLines(Bitmap& rBitmap, Color aExpectedColor, int nColorThresh)
{
    BitmapScopedWriteAccess pAccess(rBitmap);
    tools::Long nWidth  = pAccess->Width();
    tools::Long nHeight = pAccess->Height();

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // check horizontal line
    {
        tools::Long startX = 4;
        tools::Long endX   = nWidth - 2;

        tools::Long y = 1;

        checkValue(pAccess, startX, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);
        checkValue(pAccess, endX,   y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);

        for (tools::Long x = startX + 1; x <= endX - 1; x++)
        {
            checkValue(pAccess, x, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false, nColorThresh);
        }
    }

    // check vertical line
    {
        tools::Long startY = 4;
        tools::Long endY   = nHeight - 2;

        tools::Long x = 1;

        checkValue(pAccess, x, startY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);
        checkValue(pAccess, x, endY,   aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);

        for (tools::Long y = startY + 1; y <= endY - 1; y++)
        {
            checkValue(pAccess, x, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false, nColorThresh);
        }
    }

    // check diagonal line
    {
        tools::Long startX = 1;
        tools::Long endX   = nWidth - 2;

        tools::Long startY = 1;
        tools::Long endY   = nHeight - 2;

        tools::Long x = startX;
        tools::Long y = startY;

        checkValue(pAccess, startX, startY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);
        checkValue(pAccess, endX,   endY,   aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);

        x++; y++;

        while(y <= endY - 1  && x <= endX - 1)
        {
            checkValue(pAccess, x, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false, nColorThresh);
            x++; y++;
        }
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

TestResult checkDiamondLine(Bitmap& rBitmap, int aLayerNumber, Color aExpectedColor)
{
    BitmapScopedWriteAccess pAccess(rBitmap);
    tools::Long nHeight = pAccess->Height();
    tools::Long nWidth = pAccess->Width();

    tools::Long midX = nWidth  / 2;
    tools::Long midY = nHeight / 2;

    tools::Long firstX = aLayerNumber;
    tools::Long lastX  = nWidth - aLayerNumber - 1;

    tools::Long firstY = aLayerNumber;
    tools::Long lastY  = nHeight - aLayerNumber - 1;

    tools::Long offsetFromMid = 0;

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    checkValue(pAccess, firstX, midY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  midY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, midX, firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, midX,  lastY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);

    offsetFromMid = 1;
    for (tools::Long x = firstX + 1; x <= midX - 1; x++)
    {
        checkValue(pAccess, x, midY - offsetFromMid, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        checkValue(pAccess, x, midY + offsetFromMid, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);

        offsetFromMid++;
    }

    offsetFromMid = midY - aLayerNumber - 1;

    for (tools::Long x = midX + 1; x <= lastX - 1; x++)
    {
        checkValue(pAccess, x, midY - offsetFromMid, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        checkValue(pAccess, x, midY + offsetFromMid, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);

        offsetFromMid--;
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

} // end anonymous namespace

const Color OutputDeviceTestCommon::constBackgroundColor(COL_LIGHTGRAY);
const Color OutputDeviceTestCommon::constLineColor(COL_LIGHTBLUE);
const Color OutputDeviceTestCommon::constFillColor(COL_BLUE);

OutputDeviceTestCommon::OutputDeviceTestCommon()
{}

OUString OutputDeviceTestCommon::getRenderBackendName() const
{
    if (mpVirtualDevice && mpVirtualDevice->GetGraphics())
    {
        SalGraphics const * pGraphics = mpVirtualDevice->GetGraphics();
        return pGraphics->getRenderBackendName();
    }
    return OUString();
}

void OutputDeviceTestCommon::initialSetup(tools::Long nWidth, tools::Long nHeight, Color aColor, bool bEnableAA, bool bAlphaVirtualDevice)
{
    if (bAlphaVirtualDevice)
        mpVirtualDevice = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    else
        mpVirtualDevice = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);

    maVDRectangle = tools::Rectangle(Point(), Size (nWidth, nHeight));
    mpVirtualDevice->SetOutputSizePixel(maVDRectangle.GetSize());
    if (bEnableAA)
        mpVirtualDevice->SetAntialiasing(AntialiasingFlags::Enable | AntialiasingFlags::PixelSnapHairline);
    else
        mpVirtualDevice->SetAntialiasing(AntialiasingFlags::NONE);
    mpVirtualDevice->SetBackground(Wallpaper(aColor));
    mpVirtualDevice->Erase();
}

TestResult OutputDeviceTestCommon::checkLines(Bitmap& rBitmap)
{
    return checkHorizontalVerticalDiagonalLines(rBitmap, constLineColor, 0);
}

TestResult OutputDeviceTestCommon::checkAALines(Bitmap& rBitmap)
{
    return checkHorizontalVerticalDiagonalLines(rBitmap, constLineColor, 30); // 30 color values threshold delta
}

static void checkResult(TestResult eResult, TestResult & eTotal)
{
    if (eTotal == TestResult::Failed)
        return;

    if (eResult == TestResult::Failed)
        eTotal = TestResult::Failed;

    if (eResult == TestResult::PassedWithQuirks)
        eTotal = TestResult::PassedWithQuirks;
}

TestResult OutputDeviceTestCommon::checkInvertRectangle(Bitmap& aBitmap)
{
    TestResult aReturnValue = TestResult::Passed;
    TestResult eResult;

    std::vector<Color> aExpected{ COL_WHITE, COL_WHITE };
    eResult = checkRectangles(aBitmap, aExpected);
    checkResult(eResult, aReturnValue);

    eResult = checkFilled(aBitmap, tools::Rectangle(Point(2, 2), Size(8, 8)), COL_LIGHTCYAN);
    checkResult(eResult, aReturnValue);

    eResult = checkFilled(aBitmap, tools::Rectangle(Point(10, 2), Size(8, 8)), COL_LIGHTMAGENTA);
    checkResult(eResult, aReturnValue);

    eResult = checkFilled(aBitmap, tools::Rectangle(Point(2, 10), Size(8, 8)), COL_YELLOW);
    checkResult(eResult, aReturnValue);

    eResult = checkFilled(aBitmap, tools::Rectangle(Point(10, 10), Size(8, 8)), COL_BLACK);
    checkResult(eResult, aReturnValue);

    return aReturnValue;
}

TestResult OutputDeviceTestCommon::checkChecker(Bitmap& rBitmap, sal_Int32 nStartX, sal_Int32 nEndX, sal_Int32 nStartY, sal_Int32 nEndY, std::vector<Color> const & rExpected)
{
    TestResult aReturnValue = TestResult::Passed;

    int choice = 0;
    for (sal_Int32 y = nStartY; y <= nEndY; ++y)
    {
        for (sal_Int32 x = nStartX; x <= nEndX; ++x)
        {
            TestResult eResult = checkFilled(rBitmap, tools::Rectangle(Point(x, y), Size(1, 1)), rExpected[choice % 2]);
            checkResult(eResult, aReturnValue);
            choice++;
        }
        choice++;
    }
    return aReturnValue;
}

TestResult OutputDeviceTestCommon::checkInvertN50Rectangle(Bitmap& aBitmap)
{
    TestResult aReturnValue = TestResult::Passed;
    TestResult eResult;

    std::vector<Color> aExpected{ COL_WHITE, COL_WHITE };
    eResult = checkRectangles(aBitmap, aExpected);
    checkResult(eResult, aReturnValue);

    eResult = checkChecker(aBitmap, 2, 9, 2, 9, { COL_LIGHTCYAN, COL_LIGHTRED });
    checkResult(eResult, aReturnValue);
    eResult = checkChecker(aBitmap, 2, 9, 10, 17, { COL_YELLOW, COL_LIGHTBLUE });
    checkResult(eResult, aReturnValue);
    eResult = checkChecker(aBitmap, 10, 17, 2, 9, { COL_LIGHTMAGENTA, COL_LIGHTGREEN });
    checkResult(eResult, aReturnValue);
    eResult = checkChecker(aBitmap, 10, 17, 10, 17, { COL_BLACK, COL_WHITE });
    checkResult(eResult, aReturnValue);

    return aReturnValue;
}

TestResult OutputDeviceTestCommon::checkInvertTrackFrameRectangle(Bitmap& aBitmap)
{
    std::vector<Color> aExpected
    {
        COL_WHITE, COL_WHITE
    };
    return checkRectangles(aBitmap, aExpected);
}

TestResult OutputDeviceTestCommon::checkRectangle(Bitmap& aBitmap)
{
    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor, constLineColor,
        constBackgroundColor, constBackgroundColor, constLineColor, constBackgroundColor
    };
    return checkRectangles(aBitmap, aExpected);
}

TestResult OutputDeviceTestCommon::checkRectangles(Bitmap& rBitmap, bool aEnableAA)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    std::vector<Color> aExpected = { constBackgroundColor, constLineColor, constLineColor };

    for (size_t aLayerNumber = 0; aLayerNumber < aExpected.size(); aLayerNumber++)
    {
        tools::Long startX = aLayerNumber, endX = pAccess->Width() / 2 - aLayerNumber + 1;
        tools::Long startY = aLayerNumber, endY = pAccess->Height() - aLayerNumber - 1;

        for (tools::Long ptX = startX; ptX <= endX; ++ptX)
        {
            if (aEnableAA)
            {
                checkValueAA(pAccess, ptX, startY + (aLayerNumber == 2 ? 2 : 0),
                             aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors);
                checkValueAA(pAccess, ptX, endY - (aLayerNumber == 2 ? 2 : 0),
                             aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors);
            }
            else
            {
                checkValue(pAccess, ptX, startY + (aLayerNumber == 2 ? 2 : 0),
                           aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors, true);
                checkValue(pAccess, ptX, endY - (aLayerNumber == 2 ? 2 : 0),
                           aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors, true);
            }
        }
        for (tools::Long ptY = startY + (aLayerNumber == 2 ? 2 : 0);
             ptY <= endY - (aLayerNumber == 2 ? 2 : 0); ++ptY)
        {
            if (aEnableAA)
            {
                checkValueAA(pAccess, startX, ptY, aExpected[aLayerNumber], nNumberOfQuirks,
                             nNumberOfErrors);
                checkValueAA(pAccess, endX, ptY,  aExpected[aLayerNumber], nNumberOfQuirks,
                             nNumberOfErrors);
            }
            else
            {
                checkValue(pAccess, startX, ptY, aExpected[aLayerNumber], nNumberOfQuirks,
                           nNumberOfErrors, true);
                checkValue(pAccess, endX, ptY, aExpected[aLayerNumber], nNumberOfQuirks,
                           nNumberOfErrors, true);
            }
        }
    }
    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

TestResult OutputDeviceTestCommon::checkRectangleAA(Bitmap& aBitmap)
{
    return checkRectangles(aBitmap, true);
}

TestResult OutputDeviceTestCommon::checkFilledRectangle(Bitmap& aBitmap, bool useLineColor)
{
    std::vector<Color> aExpected{ constBackgroundColor,
                                  useLineColor ? constLineColor : constFillColor, constFillColor,
                                  constFillColor, constFillColor };

    BitmapScopedWriteAccess pAccess(aBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    for (size_t aLayerNumber = 0; aLayerNumber < aExpected.size(); aLayerNumber++)
    {
        tools::Long startX = aLayerNumber, endX = pAccess->Width() / 2 - aLayerNumber + 1;
        tools::Long startY = aLayerNumber, endY = pAccess->Height() - aLayerNumber - 1;

        for (tools::Long ptX = startX; ptX <= endX; ++ptX)
        {
            checkValue(pAccess, ptX, startY, aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors,
                       true);
            checkValue(pAccess, ptX, endY, aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors, true);
        }
        for (tools::Long ptY = startY; ptY <= endY; ++ptY)
        {
            checkValue(pAccess, startX, ptY, aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors,
                       true);
            checkValue(pAccess, endX, ptY, aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors, true);
        }
    }
    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

TestResult OutputDeviceTestCommon::checkFilled(Bitmap& rBitmap, tools::Rectangle aRectangle, Color aExpectedColor)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    for (tools::Long y = aRectangle.Top(); y < aRectangle.Top() + aRectangle.GetHeight(); y++)
    {
        for (tools::Long x = aRectangle.Left(); x < aRectangle.Left() + aRectangle.GetWidth(); x++)
        {
            checkValue(pAccess, x, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        }
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;

    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;

    return aResult;
}

TestResult OutputDeviceTestCommon::checkRectangles(Bitmap& aBitmap, std::vector<Color>& aExpectedColors)
{
    TestResult aReturnValue = TestResult::Passed;
    for (size_t i = 0; i < aExpectedColors.size(); i++)
    {
        TestResult eResult = checkRect(aBitmap, i, aExpectedColors[i]);

        if (eResult == TestResult::Failed)
            aReturnValue = TestResult::Failed;
        if (eResult == TestResult::PassedWithQuirks && aReturnValue != TestResult::Failed)
            aReturnValue = TestResult::PassedWithQuirks;
    }
    return aReturnValue;
}

TestResult OutputDeviceTestCommon::checkRectangle(Bitmap& rBitmap, int aLayerNumber, Color aExpectedColor)
{
    return checkRect(rBitmap, aLayerNumber, aExpectedColor);
}

tools::Rectangle OutputDeviceTestCommon::alignToCenter(tools::Rectangle aRect1, tools::Rectangle aRect2)
{
    Point aPoint((aRect1.GetWidth()  / 2.0) - (aRect2.GetWidth()  / 2.0),
                 (aRect1.GetHeight() / 2.0) - (aRect2.GetHeight() / 2.0));

    return tools::Rectangle(aPoint, aRect2.GetSize());
}

TestResult OutputDeviceTestCommon::checkDiamond(Bitmap& rBitmap)
{
    return checkDiamondLine(rBitmap, 1, constLineColor);
}

void OutputDeviceTestCommon::createDiamondPoints(tools::Rectangle rRect, int nOffset,
                                                 Point& rPoint1, Point& rPoint2,
                                                 Point& rPoint3, Point& rPoint4)
{
    tools::Long midPointX = rRect.Left() + (rRect.Right()  - rRect.Left()) / 2.0;
    tools::Long midPointY = rRect.Top()  + (rRect.Bottom() - rRect.Top())  / 2.0;

    rPoint1 = Point(midPointX         , midPointY - nOffset);
    rPoint2 = Point(midPointX + nOffset, midPointY         );
    rPoint3 = Point(midPointX         , midPointY + nOffset);
    rPoint4 = Point(midPointX - nOffset, midPointY         );
}

tools::Polygon OutputDeviceTestCommon::createDropShapePolygon()
{
    tools::Polygon aPolygon(15);

    aPolygon.SetPoint(Point(10, 2), 0);
    aPolygon.SetFlags(0, PolyFlags::Normal);
    aPolygon.SetPoint(Point(14, 2), 1);
    aPolygon.SetFlags(1, PolyFlags::Control);
    aPolygon.SetPoint(Point(18, 6), 2);
    aPolygon.SetFlags(2, PolyFlags::Control);
    aPolygon.SetPoint(Point(18, 10), 3);

    aPolygon.SetFlags(3, PolyFlags::Normal);
    aPolygon.SetPoint(Point(18, 10), 4);
    aPolygon.SetFlags(4, PolyFlags::Normal);
    aPolygon.SetPoint(Point(18, 14), 5);
    aPolygon.SetFlags(5, PolyFlags::Control);
    aPolygon.SetPoint(Point(14, 18), 6);
    aPolygon.SetFlags(6, PolyFlags::Control);
    aPolygon.SetPoint(Point(10, 18), 7);
    aPolygon.SetFlags(7, PolyFlags::Normal);

    aPolygon.SetPoint(Point(10, 18), 8);
    aPolygon.SetFlags(8, PolyFlags::Normal);
    aPolygon.SetPoint(Point(6, 18), 9);
    aPolygon.SetFlags(9, PolyFlags::Control);
    aPolygon.SetPoint(Point(2, 14), 10);
    aPolygon.SetFlags(10, PolyFlags::Control);
    aPolygon.SetPoint(Point(2, 10), 11);
    aPolygon.SetFlags(11, PolyFlags::Normal);

    aPolygon.SetPoint(Point(2, 10), 12);
    aPolygon.SetFlags(12, PolyFlags::Normal);
    aPolygon.SetPoint(Point(2, 2), 13);
    aPolygon.SetFlags(13, PolyFlags::Normal);
    aPolygon.SetPoint(Point(10, 2), 14);
    aPolygon.SetFlags(14, PolyFlags::Normal);

    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    return aPolygon;
}

basegfx::B2DPolygon OutputDeviceTestCommon::createHalfEllipsePolygon()
{
    basegfx::B2DPolygon aPolygon;

    aPolygon.append({ 9.0, 1.0 });
    aPolygon.append({ 17.0, 10.0 });
    aPolygon.append({ 1.0, 10.0 });
    aPolygon.setClosed(true);

    aPolygon.setControlPoints(0, { 1.5, 1.5 }, { 16.5, 1.5 });

    return aPolygon;
}

tools::Polygon OutputDeviceTestCommon::createClosedBezierLoop(const tools::Rectangle& rRect)
{
    tools::Long minX = rRect.Left();
    tools::Long maxX = rRect.Right() - 2;
    tools::Long minY = rRect.Top();
    tools::Long maxY = rRect.Bottom() - 2;

    tools::Polygon aPolygon(4);

    aPolygon.SetPoint(Point((maxX / 2.0), maxY), 0);
    aPolygon.SetFlags(0, PolyFlags::Normal);
    aPolygon.SetPoint(Point(maxX, minY), 1);
    aPolygon.SetFlags(1, PolyFlags::Control);
    aPolygon.SetPoint(Point(minX, minY), 2);
    aPolygon.SetFlags(2, PolyFlags::Control);
    aPolygon.SetPoint(Point((maxX / 2.0), maxY), 3);
    aPolygon.SetFlags(3, PolyFlags::Normal);

    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    return aPolygon;
}

basegfx::B2DPolygon OutputDeviceTestCommon::createOpenPolygon(const tools::Rectangle& rRect, int nOffset)
{
    int nMidOffset = rRect.GetWidth() / 2;
    basegfx::B2DPolygon aPolygon{
        basegfx::B2DPoint(rRect.Left() + nOffset - (nOffset + 1) / 2, rRect.Top() + nOffset - 1),
        basegfx::B2DPoint(rRect.Left() + nOffset - (nOffset + 1) / 2, rRect.Bottom() - nOffset + 1),
        basegfx::B2DPoint(rRect.Right() - nMidOffset - nOffset / 3, rRect.Bottom() - nOffset + 1),
        basegfx::B2DPoint(rRect.Right() - nMidOffset - nOffset / 3, rRect.Top() + nOffset - 1),
    };
    aPolygon.setClosed(false);
    return aPolygon;
}

basegfx::B2DPolygon OutputDeviceTestCommon::createOpenBezier()
{
    basegfx::B2DPolygon aPolygon;

    aPolygon.append({ 5.0, 2.0 });
    aPolygon.append({ 3.0, 14.0 });
    aPolygon.setClosed(false);

    aPolygon.setControlPoints(0, { 15.0, 2.0 }, { 15.0, 15.0 });

    return aPolygon;
}

TestResult OutputDeviceTestCommon::checkDropShape(Bitmap& rBitmap, bool aEnableAA)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    std::map<std::pair<int, int>, bool> SetPixels
        = { { { 2, 2 }, true },  { { 3, 2 }, true },   { { 4, 2 }, true },   { { 5, 2 }, true },
            { { 6, 2 }, true },  { { 7, 2 }, true },   { { 8, 2 }, true },   { { 9, 2 }, true },
            { { 10, 2 }, true }, { { 11, 2 }, true },  { { 12, 2 }, true },  { { 2, 3 }, true },
            { { 13, 3 }, true }, { { 14, 3 }, true },  { { 2, 4 }, true },   { { 15, 4 }, true },
            { { 2, 5 }, true },  { { 16, 5 }, true },  { { 2, 6 }, true },   { { 17, 6 }, true },
            { { 2, 7 }, true },  { { 17, 7 }, true },  { { 2, 8 }, true },   { { 18, 8 }, true },
            { { 2, 9 }, true },  { { 18, 9 }, true },  { { 2, 10 }, true },  { { 18, 10 }, true },
            { { 2, 11 }, true }, { { 18, 11 }, true }, { { 2, 12 }, true },  { { 18, 12 }, true },
            { { 3, 13 }, true }, { { 17, 13 }, true }, { { 3, 14 }, true },  { { 17, 14 }, true },
            { { 4, 15 }, true }, { { 16, 15 }, true }, { { 5, 16 }, true },  { { 15, 16 }, true },
            { { 6, 17 }, true }, { { 7, 17 }, true },  { { 13, 17 }, true }, { { 14, 17 }, true },
            { { 8, 18 }, true }, { { 9, 18 }, true },  { { 10, 18 }, true }, { { 11, 18 }, true },
            { { 12, 18 }, true } };

    for (tools::Long x = 0; x < pAccess->Width(); x++)
    {
        for (tools::Long y = 0; y < pAccess->Height(); y++)
        {
            if (SetPixels[{ x, y }])
            {
                if (aEnableAA)
                {
                    // coverity[swapped_arguments : FALSE] - this is in the correct order
                    checkValueAA(pAccess, y, x, constLineColor, nNumberOfQuirks, nNumberOfErrors);
                }
                else
                    checkValue(pAccess, y, x, constLineColor, nNumberOfQuirks, nNumberOfErrors,
                               true);
            }
            else
            {
                if (!aEnableAA)
                    checkValue(pAccess, y, x, constBackgroundColor, nNumberOfQuirks, nNumberOfErrors,
                                true);
            }
        }
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

void OutputDeviceTestCommon::createHorizontalVerticalDiagonalLinePoints(tools::Rectangle rRect,
                                Point& rHorizontalLinePoint1, Point& rHorizontalLinePoint2,
                                Point& rVerticalLinePoint1, Point& rVerticalLinePoint2,
                                Point& rDiagonalLinePoint1, Point& rDiagonalLinePoint2)
{
    rHorizontalLinePoint1 = Point(4, 1);
    rHorizontalLinePoint2 = Point(rRect.Right() - 1, 1);

    rVerticalLinePoint1 = Point(1, 4);
    rVerticalLinePoint2 = Point(1,rRect.Bottom() - 1);

    rDiagonalLinePoint1 = Point(1, 1);
    rDiagonalLinePoint2 = Point(rRect.Right() - 1, rRect.Bottom() - 1);
}

TestResult OutputDeviceTestCommon::checkBezier(Bitmap& rBitmap)
{
    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor
    };
    // Check the bezier doesn't go over to the margins first
    // TODO extend the check with more exact assert
    return checkRectangles(rBitmap, aExpected);
}

TestResult OutputDeviceTestCommon::checkHalfEllipse(Bitmap& rBitmap, bool aEnableAA)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    std::map<std::pair<tools::Long, tools::Long>, bool> SetPixels = {
        { { 8, 1 }, true },   { { 9, 1 }, true },   { { 10, 1 }, true },  { { 6, 2 }, true },
        { { 7, 2 }, true },   { { 10, 2 }, true },  { { 4, 3 }, true },   { { 5, 3 }, true },
        { { 10, 3 }, true },  { { 3, 4 }, true },   { { 10, 4 }, true },  { { 2, 5 }, true },
        { { 10, 5 }, true },  { { 2, 6 }, true },   { { 10, 6 }, true },  { { 1, 7 }, true },
        { { 10, 7 }, true },  { { 1, 8 }, true },   { { 10, 8 }, true },  { { 1, 9 }, true },
        { { 10, 9 }, true },  { { 1, 10 }, true },  { { 10, 10 }, true }, { { 1, 11 }, true },
        { { 10, 11 }, true }, { { 2, 12 }, true },  { { 10, 12 }, true }, { { 2, 13 }, true },
        { { 10, 13 }, true }, { { 3, 14 }, true },  { { 10, 14 }, true }, { { 4, 15 }, true },
        { { 5, 15 }, true },  { { 10, 15 }, true }, { { 6, 16 }, true },  { { 7, 16 }, true },
        { { 10, 16 }, true }, { { 8, 17 }, true },  { { 9, 17 }, true },  { { 10, 17 }, true }
    };

    for (tools::Long x = 0; x < pAccess->Width(); x++)
    {
        for (tools::Long y = 0; y < pAccess->Height(); ++y)
        {
            // coverity[swapped_arguments : FALSE] - this is in the correct order
            if (SetPixels[{ y, x }])
            {
                if (aEnableAA)
                    checkValueAA(pAccess, x, y, constLineColor, nNumberOfQuirks, nNumberOfErrors);
                else
                    checkValue(pAccess, x, y, constLineColor, nNumberOfQuirks, nNumberOfErrors,
                               true);
            }
            else
            {
                if (!aEnableAA)
                    checkValue(pAccess, x, y, constBackgroundColor, nNumberOfQuirks,
                               nNumberOfErrors, true);
            }
        }
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

TestResult OutputDeviceTestCommon::checkClosedBezier(Bitmap& rBitmap)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    std::map<std::pair<tools::Long, tools::Long>, bool> SetPixels
        = { { { 3, 8 }, true },  { { 3, 9 }, true },   { { 3, 10 }, true },  { { 4, 7 }, true },
            { { 4, 8 }, true },  { { 4, 9 }, true },   { { 4, 10 }, true },  { { 4, 11 }, true },
            { { 5, 7 }, true },  { { 5, 11 }, true },  { { 6, 6 }, true },   { { 6, 12 }, true },
            { { 7, 6 }, true },  { { 7, 12 }, true },  { { 8, 7 }, true },   { { 8, 11 }, true },
            { { 9, 7 }, true },  { { 9, 11 }, true },  { { 10, 7 }, true },  { { 10, 11 }, true },
            { { 11, 8 }, true }, { { 11, 9 }, true },  { { 11, 10 }, true }, { { 12, 8 }, true },
            { { 12, 9 }, true }, { { 12, 10 }, true }, { { 13, 9 }, true } };

    for (tools::Long x = 0; x < pAccess->Width(); x++)
    {
        for (tools::Long y = 0; y < pAccess->Height(); ++y)
        {
            // coverity[swapped_arguments : FALSE] - this is in the correct order
            if (SetPixels[{ y, x }])
            {
                checkValue(pAccess, x, y, constLineColor, nNumberOfQuirks, nNumberOfErrors, true);
            }
            else
            {
                checkValue(pAccess, x, y, constBackgroundColor, nNumberOfQuirks, nNumberOfErrors,
                           true);
            }
        }
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0 || !checkConvexHullProperty(rBitmap, constLineColor, 2, 2))
        aResult = TestResult::Failed;
    return aResult;
}

TestResult OutputDeviceTestCommon::checkOpenBezier(Bitmap& rBitmap)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    std::map<std::pair<int, int>, bool> SetPixels
        = { { { 14, 3 }, true }, { { 14, 4 }, true }, { { 14, 5 }, true }, { { 3, 6 }, true },
            { { 4, 6 }, true },  { { 14, 6 }, true }, { { 4, 7 }, true },  { { 5, 7 }, true },
            { { 13, 7 }, true }, { { 6, 8 }, true },  { { 7, 8 }, true },  { { 12, 8 }, true },
            { { 13, 8 }, true }, { { 8, 9 }, true },  { { 9, 9 }, true },  { { 10, 9 }, true },
            { { 11, 9 }, true }, { { 12, 9 }, true } };

    for (tools::Long x = 0; x < pAccess->Width(); x++)
    {
        for (tools::Long y = 0; y < pAccess->Height(); ++y)
        {
            // coverity[swapped_arguments : FALSE] - this is in the correct order
            if (SetPixels[{ y, x }])
            {
                checkValue(pAccess, x, y, constLineColor, nNumberOfQuirks, nNumberOfErrors, true);
            }
            else
            {
                checkValue(pAccess, x, y, constBackgroundColor, nNumberOfQuirks, nNumberOfErrors,
                           true);
            }
        }
    }
    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0 || !checkConvexHullProperty(rBitmap, constLineColor, 2, 5))
        aResult = TestResult::Failed;
    return aResult;
}

TestResult OutputDeviceTestCommon::checkFilledAsymmetricalDropShape(Bitmap& rBitmap)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    std::map<std::pair<tools::Long, tools::Long>, bool> SetPixels
        = { { { 2, 2 }, true },   { { 3, 2 }, true },   { { 4, 2 }, true },   { { 5, 2 }, true },
            { { 6, 2 }, true },   { { 7, 2 }, true },   { { 8, 2 }, true },   { { 9, 2 }, true },
            { { 10, 2 }, true },  { { 11, 2 }, true },  { { 2, 3 }, true },   { { 3, 3 }, true },
            { { 4, 3 }, true },   { { 5, 3 }, true },   { { 6, 3 }, true },   { { 7, 3 }, true },
            { { 8, 3 }, true },   { { 9, 3 }, true },   { { 10, 3 }, true },  { { 11, 3 }, true },
            { { 12, 3 }, true },  { { 13, 3 }, true },  { { 2, 4 }, true },   { { 3, 4 }, true },
            { { 4, 4 }, true },   { { 5, 4 }, true },   { { 6, 4 }, true },   { { 7, 4 }, true },
            { { 8, 4 }, true },   { { 9, 4 }, true },   { { 10, 4 }, true },  { { 11, 4 }, true },
            { { 12, 4 }, true },  { { 13, 4 }, true },  { { 14, 4 }, true },  { { 15, 4 }, true },
            { { 2, 5 }, true },   { { 3, 5 }, true },   { { 4, 5 }, true },   { { 5, 5 }, true },
            { { 6, 5 }, true },   { { 7, 5 }, true },   { { 8, 5 }, true },   { { 9, 5 }, true },
            { { 10, 5 }, true },  { { 11, 5 }, true },  { { 12, 5 }, true },  { { 13, 5 }, true },
            { { 14, 5 }, true },  { { 15, 5 }, true },  { { 2, 6 }, true },   { { 3, 6 }, true },
            { { 4, 6 }, true },   { { 5, 6 }, true },   { { 6, 6 }, true },   { { 7, 6 }, true },
            { { 8, 6 }, true },   { { 9, 6 }, true },   { { 10, 6 }, true },  { { 11, 6 }, true },
            { { 12, 6 }, true },  { { 13, 6 }, true },  { { 14, 6 }, true },  { { 15, 6 }, true },
            { { 16, 6 }, true },  { { 2, 7 }, true },   { { 3, 7 }, true },   { { 4, 7 }, true },
            { { 5, 7 }, true },   { { 6, 7 }, true },   { { 7, 7 }, true },   { { 8, 7 }, true },
            { { 9, 7 }, true },   { { 10, 7 }, true },  { { 11, 7 }, true },  { { 12, 7 }, true },
            { { 13, 7 }, true },  { { 14, 7 }, true },  { { 15, 7 }, true },  { { 16, 7 }, true },
            { { 2, 8 }, true },   { { 3, 8 }, true },   { { 4, 8 }, true },   { { 5, 8 }, true },
            { { 6, 8 }, true },   { { 7, 8 }, true },   { { 8, 8 }, true },   { { 9, 8 }, true },
            { { 10, 8 }, true },  { { 11, 8 }, true },  { { 12, 8 }, true },  { { 13, 8 }, true },
            { { 14, 8 }, true },  { { 15, 8 }, true },  { { 16, 8 }, true },  { { 17, 8 }, true },
            { { 2, 9 }, true },   { { 3, 9 }, true },   { { 4, 9 }, true },   { { 5, 9 }, true },
            { { 6, 9 }, true },   { { 7, 9 }, true },   { { 8, 9 }, true },   { { 9, 9 }, true },
            { { 10, 9 }, true },  { { 11, 9 }, true },  { { 12, 9 }, true },  { { 13, 9 }, true },
            { { 14, 9 }, true },  { { 15, 9 }, true },  { { 16, 9 }, true },  { { 17, 9 }, true },
            { { 2, 10 }, true },  { { 3, 10 }, true },  { { 4, 10 }, true },  { { 5, 10 }, true },
            { { 6, 10 }, true },  { { 7, 10 }, true },  { { 8, 10 }, true },  { { 9, 10 }, true },
            { { 10, 10 }, true }, { { 11, 10 }, true }, { { 12, 10 }, true }, { { 13, 10 }, true },
            { { 14, 10 }, true }, { { 15, 10 }, true }, { { 16, 10 }, true }, { { 17, 10 }, true },
            { { 2, 11 }, true },  { { 3, 11 }, true },  { { 4, 11 }, true },  { { 5, 11 }, true },
            { { 6, 11 }, true },  { { 7, 11 }, true },  { { 8, 11 }, true },  { { 9, 11 }, true },
            { { 10, 11 }, true }, { { 11, 11 }, true }, { { 12, 11 }, true }, { { 13, 11 }, true },
            { { 14, 11 }, true }, { { 15, 11 }, true }, { { 16, 11 }, true }, { { 17, 11 }, true },
            { { 3, 12 }, true },  { { 4, 12 }, true },  { { 5, 12 }, true },  { { 6, 12 }, true },
            { { 7, 12 }, true },  { { 8, 12 }, true },  { { 9, 12 }, true },  { { 10, 12 }, true },
            { { 11, 12 }, true }, { { 12, 12 }, true }, { { 13, 12 }, true }, { { 14, 12 }, true },
            { { 15, 12 }, true }, { { 16, 12 }, true }, { { 3, 13 }, true },  { { 4, 13 }, true },
            { { 5, 13 }, true },  { { 6, 13 }, true },  { { 7, 13 }, true },  { { 8, 13 }, true },
            { { 9, 13 }, true },  { { 10, 13 }, true }, { { 11, 13 }, true }, { { 12, 13 }, true },
            { { 13, 13 }, true }, { { 14, 13 }, true }, { { 15, 13 }, true }, { { 16, 13 }, true },
            { { 4, 14 }, true },  { { 5, 14 }, true },  { { 6, 14 }, true },  { { 7, 14 }, true },
            { { 8, 14 }, true },  { { 9, 14 }, true },  { { 10, 14 }, true }, { { 11, 14 }, true },
            { { 12, 14 }, true }, { { 13, 14 }, true }, { { 14, 14 }, true }, { { 15, 14 }, true },
            { { 5, 15 }, true },  { { 6, 15 }, true },  { { 7, 15 }, true },  { { 8, 15 }, true },
            { { 9, 15 }, true },  { { 10, 15 }, true }, { { 11, 15 }, true }, { { 12, 15 }, true },
            { { 13, 15 }, true }, { { 14, 15 }, true }, { { 15, 15 }, true }, { { 6, 16 }, true },
            { { 7, 16 }, true },  { { 8, 16 }, true },  { { 9, 16 }, true },  { { 10, 16 }, true },
            { { 11, 16 }, true }, { { 12, 16 }, true }, { { 13, 16 }, true }, { { 8, 17 }, true },
            { { 9, 17 }, true },  { { 10, 17 }, true }, { { 11, 17 }, true } };

    for (tools::Long x = 0; x < pAccess->Width(); x++)
    {
        for (tools::Long y = 0; y < pAccess->Height(); ++y)
        {
            if (SetPixels[{ x, y }])
            {
                checkValue(pAccess, y, x, constFillColor, nNumberOfQuirks, nNumberOfErrors, true);
            }
            else
            {
                checkValue(pAccess, y, x, constBackgroundColor, nNumberOfQuirks, nNumberOfErrors, true);
            }
        }
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

TestResult OutputDeviceTestCommon::checkTextLocation(Bitmap& rBitmap)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;

    //The limit to which error would be tolerated.
    tools::Long textThreshold = 3;
    tools::Long textWidth = 3, textHeight = 8;
    tools::Long deviationX = 0, deviationY = 0;
    tools::Long verticalStart = 0, verticalEnd = 0;
    tools::Long horizontalStart = 0, horizontalEnd = 0;
    tools::Long midX = pAccess->Width() / 2.0;
    tools::Long midY = pAccess->Height() / 2.0;
    bool insideFlag = false;

    //Traversing horizontally
    for (tools::Long x = 0, y = pAccess->Height() / 2.0; x < pAccess->Width(); ++x)
    {
        if (pAccess->GetPixel(y, x) != constBackgroundColor)
        {
            if (!insideFlag)
            {
                horizontalStart = x;
                insideFlag = true;
            }
            else
            {
                horizontalEnd = x;
            }
        }
    }

    deviationX = abs(midX - horizontalStart);
    midY -= midY / 2.0;
    midY += 1;

    insideFlag = false;
    //Traversing vertically
    for (tools::Long x = 0, y = pAccess->Height() / 2.0; x < pAccess->Height(); ++x)
    {
        if (pAccess->GetPixel(x, y) != constBackgroundColor)
        {
            if (!insideFlag)
            {
                verticalStart = x;
                insideFlag = true;
            }
            else
            {
                verticalEnd = x;
            }
        }
    }

    deviationY = abs(midY - verticalStart);

    if (deviationX != 0 || deviationY != 0 || abs(horizontalStart - horizontalEnd) + 1 != textWidth
        || abs(verticalStart - verticalEnd) + 1 != textHeight)
    {
        aResult = TestResult::PassedWithQuirks;
    }

    if (deviationX > textThreshold || deviationY > textThreshold
        || abs((abs(horizontalStart - horizontalEnd) + 1) - textWidth) > textThreshold
        || abs((abs(verticalStart - verticalEnd) + 1) - textHeight) > textThreshold)
    {
        aResult = TestResult::Failed;
    }

    return aResult;
}

TestResult OutputDeviceTestCommon::checkIntersectingRecs(Bitmap& rBitmap, int aLayerNumber,
                                                         Color aExpected)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    for (int x = 4; x <= 19; ++x)
    {
        checkValue(pAccess, x, aLayerNumber, aExpected, nNumberOfQuirks, nNumberOfErrors, true);
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

TestResult OutputDeviceTestCommon::checkEvenOddRuleInIntersectingRecs(Bitmap& rBitmap)
{
    /*
    The even-odd rule would be tested via the below pattern as layers both of the
    constFillColor & constBackgroundColor appears in an even-odd fashion.
     */
    std::vector<Color> aExpectedColors
        = { constBackgroundColor, constBackgroundColor, constLineColor,       constFillColor,
            constFillColor,       constLineColor,       constBackgroundColor, constBackgroundColor,
            constLineColor,       constFillColor,       constFillColor,       constLineColor,
            constBackgroundColor, constBackgroundColor, constLineColor,       constFillColor,
            constFillColor,       constLineColor,       constBackgroundColor, constBackgroundColor,
            constLineColor,       constFillColor,       constLineColor };

    TestResult aReturnValue = TestResult::Passed;
    for (size_t i = 0; i < aExpectedColors.size(); i++)
    {
        TestResult eResult = checkIntersectingRecs(rBitmap, i, aExpectedColors[i]);

        if (eResult == TestResult::Failed)
            aReturnValue = TestResult::Failed;
        if (eResult == TestResult::PassedWithQuirks && aReturnValue != TestResult::Failed)
            aReturnValue = TestResult::PassedWithQuirks;
    }
    return aReturnValue;
}

TestResult OutputDeviceTestCommon::checkOpenPolygon(Bitmap& rBitmap, bool aEnableAA)
{
    std::vector<Color> aExpected = { constBackgroundColor, constLineColor, constLineColor };

    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    for (size_t aLayerNumber = 0; aLayerNumber < aExpected.size(); aLayerNumber++)
    {
        tools::Long startX = aLayerNumber + 1, endX = pAccess->Width() / 2 - aLayerNumber;
        tools::Long startY = aLayerNumber + 2, endY = pAccess->Height() - aLayerNumber - 3;

        for (tools::Long ptX = startX; ptX <= endX; ++ptX)
        {
            if (aEnableAA)
            {
                checkValueAA(pAccess, ptX, endY - (aLayerNumber == 2 ? 2 : 0),
                             aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors);
            }
            else
            {
                checkValue(pAccess, ptX, endY - (aLayerNumber == 2 ? 2 : 0),
                           aExpected[aLayerNumber], nNumberOfQuirks, nNumberOfErrors, true);
            }
        }
        for (tools::Long ptY = startY + (aLayerNumber == 2 ? 2 : 0);
             ptY <= endY - (aLayerNumber == 2 ? 2 : 0); ++ptY)
        {
            if (aEnableAA)
            {
                checkValueAA(pAccess, startX, ptY, aExpected[aLayerNumber], nNumberOfQuirks,
                             nNumberOfErrors);
                checkValueAA(pAccess, endX, ptY, aExpected[aLayerNumber], nNumberOfQuirks,
                             nNumberOfErrors);
            }
            else
            {
                checkValue(pAccess, startX, ptY, aExpected[aLayerNumber], nNumberOfQuirks,
                           nNumberOfErrors, true);
                checkValue(pAccess, endX, ptY, aExpected[aLayerNumber], nNumberOfQuirks,
                           nNumberOfErrors, true);
            }
        }
    }

    if (nNumberOfQuirks > 0)
        aResult = TestResult::PassedWithQuirks;
    if (nNumberOfErrors > 0)
        aResult = TestResult::Failed;
    return aResult;
}

// Check 'count' pixels from (x,y) in (addX,addY) direction, the color values must not decrease.
static bool checkGradient(BitmapScopedWriteAccess& pAccess, int x, int y, int count, int addX, int addY)
{
    const bool bColorize = false;
    Color maxColor = COL_BLACK;
    for( int i = 0; i < count; ++i )
    {
        Color color = pAccess->GetPixel(y, x);
        if( color.GetRed() < maxColor.GetRed() || color.GetGreen() < maxColor.GetGreen() || color.GetBlue() < maxColor.GetBlue())
        {
            if (bColorize)
                pAccess->SetPixel(y, x, COL_RED);
            return false;
        }
        maxColor = color;
        if (bColorize)
            pAccess->SetPixel(y, x, COL_LIGHTGREEN);
        x += addX;
        y += addY;
    }
    return true;
}

TestResult OutputDeviceTestCommon::checkLinearGradient(Bitmap& bitmap)
{
    BitmapScopedWriteAccess pAccess(bitmap);
    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // The lowest line is missing in the default VCL implementation => quirk.
    checkValue(pAccess, 1, 10, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, true, 255 / 10);
    checkValue(pAccess, 10, 10, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, true, 255 / 10);
    for(int y = 1; y < 10; ++y)
    {
        checkValue(pAccess, 1, y, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10);
        checkValue(pAccess, 10, y, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10);
    }
    for(int y = 1; y < 10; ++y)
        if( !checkGradient( pAccess, 10, y, 10, -1, 0 ))
            return TestResult::Failed;
    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return aResult;
}

TestResult OutputDeviceTestCommon::checkLinearGradientAngled(Bitmap& bitmap)
{
    BitmapScopedWriteAccess pAccess(bitmap);
    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // The top-left pixel is not white but gray in the default VCL implementation => quirk.
    checkValue(pAccess, 1, 1, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 50);
    checkValue(pAccess, 10, 10, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 0, 255 / 10); // Bottom-right.
    // Main diagonal.
    if( !checkGradient( pAccess, 10, 10, 10, -1, -1 ))
        return TestResult::Failed;
    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return TestResult::Passed;
}

TestResult OutputDeviceTestCommon::checkLinearGradientBorder(Bitmap& bitmap)
{
    TestResult aResult = TestResult::Passed;
    // Top half is border.
    checkResult(checkFilled(bitmap, tools::Rectangle(Point(1, 1), Size(10, 5)), COL_WHITE), aResult);
    BitmapScopedWriteAccess pAccess(bitmap);
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;
    for(int x = 1; x <= 10; ++x)
    {
        checkValue(pAccess, x, 10, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
        if( !checkGradient( pAccess, x, 10, 5, 0, -1 ))
            return TestResult::Failed;
    }
    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return aResult;
}

TestResult OutputDeviceTestCommon::checkLinearGradientIntensity(Bitmap& bitmap)
{
    BitmapScopedWriteAccess pAccess(bitmap);
    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    for(int x = 1; x <= 10; ++x)
    {
        // The gradient starts at half intensity, i.e. white's 255's are halved.
        checkValue(pAccess, x, 1, Color(128,128,128), nNumberOfQuirks, nNumberOfErrors, false, 10);
        checkValue(pAccess, x, 10, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10);
        if( !checkGradient( pAccess, x, 10, 10, 0, -1 ))
            return TestResult::Failed;
    }
    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return aResult;
}

TestResult OutputDeviceTestCommon::checkLinearGradientSteps(Bitmap& bitmap)
{
    // Reuse the basic linear gradient check.
    TestResult aResult = checkLinearGradient(bitmap);
    // Only 4 steps in the gradient, there should be only 4 colors.
    if( collectColors( bitmap, tools::Rectangle( Point( 1, 1 ), Size( 10, 10 ))).size() != 4 )
        return TestResult::Failed;
    return aResult;
}

TestResult OutputDeviceTestCommon::checkAxialGradient(Bitmap& bitmap)
{
    BitmapScopedWriteAccess pAccess(bitmap);
    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    for(int y = 1; y <= 11; ++y)
    {
        // Middle horizontal line is white, gradients to the sides.
        checkValue(pAccess, 6, y, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
        checkValue(pAccess, 1, y, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
        checkValue(pAccess, 11, y, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
        if( !checkGradient( pAccess, 1, y, 6, 1, 0 ))
            return TestResult::Failed;
        if( !checkGradient( pAccess, 11, y, 6, -1, 0 ))
            return TestResult::Failed;
    }
    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return aResult;
}

TestResult OutputDeviceTestCommon::checkRadialGradient(Bitmap& bitmap)
{
    BitmapScopedWriteAccess pAccess(bitmap);
    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;
    // The default VCL implementation is off-center in the direction to the top-left.
    // This means not all corners will be pure white => quirks.
    checkValue(pAccess, 1, 1, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 3);
    checkValue(pAccess, 1, 10, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
    checkValue(pAccess, 10, 1, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
    checkValue(pAccess, 10, 10, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
    // And not all centers will be pure black => quirks.
    checkValue(pAccess, 5, 5, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
    checkValue(pAccess, 5, 6, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 3);
    checkValue(pAccess, 6, 5, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 3);
    checkValue(pAccess, 6, 6, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 2);
    // Check diagonals, from the offset center.
    if(!checkGradient(pAccess, 5, 5, 5, -1, -1))
        return TestResult::Failed;
    if(!checkGradient(pAccess, 5, 5, 6, 1, 1))
        return TestResult::Failed;
    if(!checkGradient(pAccess, 5, 5, 5, 1, -1))
        return TestResult::Failed;
    if(!checkGradient(pAccess, 5, 5, 5, -1, 1))
        return TestResult::Failed;
    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return aResult;
}

TestResult OutputDeviceTestCommon::checkRadialGradientOfs(Bitmap& bitmap)
{
    BitmapScopedWriteAccess pAccess(bitmap);
    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;
    checkValue(pAccess, 1, 1, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
    checkValue(pAccess, 10, 1, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
    checkValue(pAccess, 1, 10, COL_WHITE, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
    checkValue(pAccess, 10, 10, COL_BLACK, nNumberOfQuirks, nNumberOfErrors, 255 / 10, 255 / 5);
    // Check gradients from the center (=bottom-right corner).
    if(!checkGradient(pAccess, 10, 10, 10, -1, -1))
        return TestResult::Failed;
    if(!checkGradient(pAccess, 10, 10, 10, -1, 0))
        return TestResult::Failed;
    if(!checkGradient(pAccess, 10, 10, 10, 0, -1))
        return TestResult::Failed;
    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return aResult;
}

constexpr int CAPSHRINK = 25;
constexpr int CAPWIDTH = 20;
TestResult OutputDeviceTestCommon::checkLineCap(Bitmap& rBitmap, css::drawing::LineCap lineCap)
{
    BitmapScopedWriteAccess access(rBitmap);
    tools::Rectangle rectangle( Point( 0, 0 ), Size( 101, 101 ));
    rectangle.shrink(CAPSHRINK);
    rectangle = tools::Rectangle( Point(rectangle.LeftCenter().getX(), rectangle.LeftCenter().getY() - CAPWIDTH / 2),
        Point(rectangle.RightCenter().getX(), rectangle.RightCenter().getY() + CAPWIDTH / 2));
    rectangle.shrink(1);
    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // the line itself
    checkValue(access, rectangle.TopLeft(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle.TopRight(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle.BottomLeft(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle.BottomRight(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);

    // the cap in the middle
    Color color = ( lineCap == css::drawing::LineCap_BUTT ) ? constBackgroundColor : constLineColor;
    checkValue(access, rectangle.LeftCenter() - Point(CAPWIDTH/2, 0), color, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle.RightCenter() + Point(CAPWIDTH/2, 0), color, nNumberOfQuirks, nNumberOfErrors, false);

    // the cap corners
    color = ( lineCap == css::drawing::LineCap_SQUARE ) ? constLineColor : constBackgroundColor;
    checkValue(access, rectangle.TopLeft() - Point(CAPWIDTH/2, 0), color, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle.TopRight() + Point(CAPWIDTH/2, 0), color, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle.BottomLeft() - Point(CAPWIDTH/2, 0), color, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle.BottomRight() + Point(CAPWIDTH/2, 0), color, nNumberOfQuirks, nNumberOfErrors, false);

    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return aResult;
}

TestResult OutputDeviceTestCommon::checkLineJoin(Bitmap& rBitmap, basegfx::B2DLineJoin lineJoin)
{
    BitmapScopedWriteAccess access(rBitmap);
    tools::Rectangle rectangle( Point( 0, 0 ), Size( 101, 101 ));
    rectangle.shrink(CAPSHRINK);
    tools::Rectangle rectangle1( Point(rectangle.TopLeft().getX(), rectangle.TopLeft().getY() - CAPWIDTH / 2),
        Point(rectangle.TopRight().getX(), rectangle.TopRight().getY() + CAPWIDTH / 2));
    tools::Rectangle rectangle2( Point(rectangle.TopRight().getX() - CAPWIDTH / 2, rectangle.TopRight().getY()),
        Point(rectangle.BottomRight().getX() + CAPWIDTH / 2, rectangle.BottomRight().getY()));
    rectangle1.shrink(1);
    rectangle2.shrink(1);
    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // the lines themselves
    checkValue(access, rectangle1.TopLeft(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle1.TopRight(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle1.BottomLeft(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle1.BottomRight(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle2.TopLeft(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle2.TopRight(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle2.BottomLeft(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);
    checkValue(access, rectangle2.BottomRight(), constLineColor, nNumberOfQuirks, nNumberOfErrors, false);

    // Only miter has the corner point.
    Color color = ( lineJoin == basegfx::B2DLineJoin::Miter ) ? constLineColor : constBackgroundColor;
    checkValue(access, rectangle2.Right(), rectangle1.Top(), color, nNumberOfQuirks, nNumberOfErrors, false);

    // Round reaches a bit past the diagonal.
    Point midDiagonal = (Point( rectangle2.Right(), rectangle1.Top()) + rectangle.TopRight()) / 2;
    if( lineJoin == basegfx::B2DLineJoin::Round)
        color = constLineColor;
    checkValue(access, midDiagonal + Point( 2, -2 ), color, nNumberOfQuirks, nNumberOfErrors, false);
    // Bevel is the diagonal.
    if( lineJoin == basegfx::B2DLineJoin::Bevel)
        color = constLineColor;
    checkValue(access, midDiagonal + Point( -1, 1 ), color, nNumberOfQuirks, nNumberOfErrors, false);
    // Everything except None has at least some line join.
    checkValue(access, rectangle.TopRight() + Point( 1, -1 ), color, nNumberOfQuirks, nNumberOfErrors, false);

    if (nNumberOfQuirks > 0)
        checkResult(TestResult::PassedWithQuirks, aResult);
    if (nNumberOfErrors > 0)
        checkResult(TestResult::Failed, aResult);
    return aResult;
}


} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
