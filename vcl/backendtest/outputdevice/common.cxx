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
#include <bitmapwriteaccess.hxx>
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

TestResult checkRect(Bitmap& rBitmap, int aLayerNumber, Color aExpectedColor)
{
    BitmapScopedWriteAccess pAccess(rBitmap);
    long nHeight = pAccess->Height();
    long nWidth = pAccess->Width();

    long firstX = 0 + aLayerNumber;
    long firstY = 0 + aLayerNumber;

    long lastX = nWidth  - aLayerNumber - 1;
    long lastY = nHeight - aLayerNumber - 1;

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // check corner quirks
    checkValue(pAccess, firstX, firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, firstX, lastY,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  lastY,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);

    for (long y = firstY + 1; y <= lastY - 1; y++)
    {
        checkValue(pAccess, firstX, y,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        checkValue(pAccess, lastX,  y,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
    }
    for (long x = firstX + 1; x <= lastX - 1; x++)
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
    long nWidth  = pAccess->Width();
    long nHeight = pAccess->Height();

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // check horizontal line
    {
        long startX = 4;
        long endX   = nWidth - 2;

        long y = 1;

        checkValue(pAccess, startX, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);
        checkValue(pAccess, endX,   y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);

        for (long x = startX + 1; x <= endX - 1; x++)
        {
            checkValue(pAccess, x, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false, nColorThresh);
        }
    }

    // check vertical line
    {
        long startY = 4;
        long endY   = nHeight - 2;

        long x = 1;

        checkValue(pAccess, x, startY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);
        checkValue(pAccess, x, endY,   aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);

        for (long y = startY + 1; y <= endY - 1; y++)
        {
            checkValue(pAccess, x, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false, nColorThresh);
        }
    }

    // check diagonal line
    {
        long startX = 1;
        long endX   = nWidth - 2;

        long startY = 1;
        long endY   = nHeight - 2;

        long x = startX;
        long y = startY;

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
    long nHeight = pAccess->Height();
    long nWidth = pAccess->Width();

    long midX = nWidth  / 2;
    long midY = nHeight / 2;

    long firstX = aLayerNumber;
    long lastX  = nWidth - aLayerNumber - 1;

    long firstY = aLayerNumber;
    long lastY  = nHeight - aLayerNumber - 1;

    long offsetFromMid = 0;

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    checkValue(pAccess, firstX, midY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  midY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, midX, firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, midX,  lastY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);

    offsetFromMid = 1;
    for (long x = firstX + 1; x <= midX - 1; x++)
    {
        checkValue(pAccess, x, midY - offsetFromMid, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        checkValue(pAccess, x, midY + offsetFromMid, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);

        offsetFromMid++;
    }

    offsetFromMid = midY - aLayerNumber - 1;

    for (long x = midX + 1; x <= lastX - 1; x++)
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

void OutputDeviceTestCommon::initialSetup(long nWidth, long nHeight, Color aColor, bool bEnableAA, bool bAlphaVirtualDevice)
{
    if (bAlphaVirtualDevice)
        mpVirtualDevice = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    else
        mpVirtualDevice = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);

    maVDRectangle = tools::Rectangle(Point(), Size (nWidth, nHeight));
    mpVirtualDevice->SetOutputSizePixel(maVDRectangle.GetSize());
    if (bEnableAA)
        mpVirtualDevice->SetAntialiasing(AntialiasingFlags::EnableB2dDraw | AntialiasingFlags::PixelSnapHairline);
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

TestResult OutputDeviceTestCommon::checkRectangleAA(Bitmap& aBitmap)
{
    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor, constLineColor,
        constBackgroundColor, constBackgroundColor, constLineColor, constBackgroundColor
    };
    return checkRectangles(aBitmap, aExpected);
}

TestResult OutputDeviceTestCommon::checkFilledRectangle(Bitmap& aBitmap, bool useLineColor)
{
    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor,
        useLineColor ? constLineColor : constFillColor,
        constFillColor, constFillColor, constFillColor, constFillColor
    };
    return checkRectangles(aBitmap, aExpected);
}

TestResult OutputDeviceTestCommon::checkFilled(Bitmap& rBitmap, tools::Rectangle aRectangle, Color aExpectedColor)
{
    BitmapScopedWriteAccess pAccess(rBitmap);

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    for (long y = aRectangle.Top(); y < aRectangle.Top() + aRectangle.GetHeight(); y++)
    {
        for (long x = aRectangle.Left(); x < aRectangle.Left() + aRectangle.GetWidth(); x++)
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
    long midPointX = rRect.Left() + (rRect.Right()  - rRect.Left()) / 2.0;
    long midPointY = rRect.Top()  + (rRect.Bottom() - rRect.Top())  / 2.0;

    rPoint1 = Point(midPointX         , midPointY - nOffset);
    rPoint2 = Point(midPointX + nOffset, midPointY         );
    rPoint3 = Point(midPointX         , midPointY + nOffset);
    rPoint4 = Point(midPointX - nOffset, midPointY         );
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

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
