/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "test/outputdevice.hxx"

namespace vcl {
namespace test {

namespace
{

int deltaColor(BitmapColor aColor1, BitmapColor aColor2)
{
    int deltaR = std::abs(aColor1.GetRed()   - aColor2.GetRed());
    int deltaG = std::abs(aColor1.GetGreen() - aColor2.GetGreen());
    int deltaB = std::abs(aColor1.GetBlue()  - aColor2.GetBlue());

    return std::max(std::max(deltaR, deltaG), deltaB);
}

void checkValue(Bitmap::ScopedWriteAccess& pAccess, int x, int y, Color aExpected,
                      int& nNumberOfQuirks, int& nNumberOfErrors, bool bQuirkMode, int nColorDeltaThresh = 0)
{
    const bool bColorize = false;
    Color aColor = pAccess->GetPixel(y, x);
    int nColorDelta = deltaColor(aColor, aExpected);

    if (nColorDelta <= nColorDeltaThresh)
    {
        if (bColorize)
            pAccess->SetPixel(y, x, Color(COL_LIGHTGREEN));
    }
    else if (bQuirkMode)
    {
        nNumberOfQuirks++;
        if (bColorize)
            pAccess->SetPixel(y, x, Color(COL_YELLOW));
    }
    else
    {
        nNumberOfErrors++;
        if (bColorize)
            pAccess->SetPixel(y, x, Color(COL_LIGHTRED));
    }
}

TestResult checkRect(Bitmap& rBitmap, int aLayerNumber, Color aExpectedColor)
{
    Bitmap::ScopedWriteAccess pAccess(rBitmap);
    sal_Int32 nHeight = pAccess->Height();
    sal_Int32 nWidth = pAccess->Width();

    sal_Int32 firstX = 0 + aLayerNumber;
    sal_Int32 firstY = 0 + aLayerNumber;

    sal_Int32 lastX = nWidth  - aLayerNumber - 1;
    sal_Int32 lastY = nHeight - aLayerNumber - 1;

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // check corner quirks
    checkValue(pAccess, firstX, firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, firstX, lastY,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  lastY,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);

    for (sal_Int32 y = firstY + 1; y <= lastY - 1; y++)
    {
        checkValue(pAccess, firstX, y,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        checkValue(pAccess, lastX,  y,  aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
    }
    for (sal_Int32 x = firstX + 1; x <= lastX - 1; x++)
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
    Bitmap::ScopedWriteAccess pAccess(rBitmap);
    sal_Int32 nWidth  = pAccess->Width();
    sal_Int32 nHeight = pAccess->Height();

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    // check horizontal line
    {
        sal_Int32 startX = 4;
        sal_Int32 endX   = nWidth - 2;

        sal_Int32 y = 1;

        checkValue(pAccess, startX, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);
        checkValue(pAccess, endX,   y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);

        for (int x = startX + 1; x <= endX - 1; x++)
        {
            checkValue(pAccess, x, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false, nColorThresh);
        }
    }

    // check vertical line
    {
        sal_Int32 startY = 4;
        sal_Int32 endY   = nHeight - 2;

        sal_Int32 x = 1;

        checkValue(pAccess, x, startY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);
        checkValue(pAccess, x, endY,   aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true, nColorThresh);

        for (int y = startY + 1; y <= endY - 1; y++)
        {
            checkValue(pAccess, x, y, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false, nColorThresh);
        }
    }

    // check diagonal line
    {
        sal_Int32 startX = 1;
        sal_Int32 endX   = nWidth - 2;

        sal_Int32 startY = 1;
        sal_Int32 endY   = nHeight - 2;

        sal_Int32 x = startX;
        sal_Int32 y = startY;

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
    Bitmap::ScopedWriteAccess pAccess(rBitmap);
    sal_Int32 nHeight = pAccess->Height();
    sal_Int32 nWidth = pAccess->Width();

    sal_Int32 midX = nWidth  / 2;
    sal_Int32 midY = nHeight / 2;

    sal_Int32 firstX = aLayerNumber;
    sal_Int32 lastX  = nWidth - aLayerNumber - 1;

    sal_Int32 firstY = aLayerNumber;
    sal_Int32 lastY  = nHeight - aLayerNumber - 1;

    sal_Int32 offsetFromMid = 0;

    TestResult aResult = TestResult::Passed;
    int nNumberOfQuirks = 0;
    int nNumberOfErrors = 0;

    checkValue(pAccess, firstX, midY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, lastX,  midY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, midX, firstY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);
    checkValue(pAccess, midX,  lastY, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, true);

    offsetFromMid = 1;
    for (sal_Int32 x = firstX + 1; x <= midX - 1; x++)
    {
        checkValue(pAccess, x, midY - offsetFromMid, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);
        checkValue(pAccess, x, midY + offsetFromMid, aExpectedColor, nNumberOfQuirks, nNumberOfErrors, false);

        offsetFromMid++;
    }

    offsetFromMid = midY - aLayerNumber - 1;

    for (sal_Int32 x = midX + 1; x <= lastX - 1; x++)
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
const Color OutputDeviceTestCommon::constFillColor(COL_LIGHTBLUE);

OutputDeviceTestCommon::OutputDeviceTestCommon()
    : mpVirtualDevice(VclPtr<VirtualDevice>::Create())
{}

void OutputDeviceTestCommon::initialSetup(sal_Int32 nWidth, sal_Int32 nHeight, Color aColor)
{
    maVDRectangle = tools::Rectangle(Point(), Size (nWidth, nHeight));
    mpVirtualDevice->SetOutputSizePixel(maVDRectangle.GetSize());
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

TestResult OutputDeviceTestCommon::checkRectangle(Bitmap& aBitmap)
{
    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor, constLineColor,
        constBackgroundColor, constBackgroundColor, constLineColor, constBackgroundColor
    };
    return checkRectangles(aBitmap, aExpected);
}

TestResult OutputDeviceTestCommon::checkFilledRectangle(Bitmap& aBitmap)
{
    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor,
        constFillColor, constFillColor, constFillColor, constFillColor, constFillColor
    };
    return checkRectangles(aBitmap, aExpected);
}

TestResult OutputDeviceTestCommon::checkRectangles(Bitmap& aBitmap, std::vector<Color>& aExpectedColors)
{
    TestResult aReturnValue = TestResult::Passed;
    for (size_t i = 0; i < aExpectedColors.size(); i++)
    {
        switch(checkRect(aBitmap, i, aExpectedColors[i]))
        {
            case TestResult::Failed:
                return TestResult::Failed;
            case TestResult::PassedWithQuirks:
                aReturnValue = TestResult::PassedWithQuirks;
                break;
            default:
                break;
        }

    }
    return aReturnValue;
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
    sal_Int32 midPointX = rRect.Left() + (rRect.Right()  - rRect.Left()) / 2.0;
    sal_Int32 midPointY = rRect.Top()  + (rRect.Bottom() - rRect.Top())  / 2.0;

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

}} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
