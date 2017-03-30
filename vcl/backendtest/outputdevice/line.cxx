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

void drawLineOffset(OutputDevice& rDevice, tools::Rectangle& rRect, int nOffset)
{
    Point aLeftTop     (rRect.Left()  + nOffset, rRect.Top()    + nOffset);
    Point aRightTop    (rRect.Right() - nOffset, rRect.Top()    + nOffset);
    Point aLeftBottom  (rRect.Left()  + nOffset, rRect.Bottom() - nOffset);
    Point aRightBottom (rRect.Right() - nOffset, rRect.Bottom() - nOffset);

    rDevice.DrawLine(aLeftTop,     aRightTop);
    rDevice.DrawLine(aRightTop,    aRightBottom);
    rDevice.DrawLine(aRightBottom, aLeftBottom);
    rDevice.DrawLine(aLeftBottom,  aLeftTop);
}

} // end anonymous namespace

Bitmap OutputDeviceTestLine::setupRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawLineOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawLineOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestLine::setupDiamond()
{
    initialSetup(11, 11, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constFillColor);
    mpVirtualDevice->SetFillColor();

    Point aPoint1, aPoint2, aPoint3, aPoint4;
    OutputDeviceTestCommon::createDiamondPoints(maVDRectangle, 4, aPoint1, aPoint2, aPoint3, aPoint4);

    mpVirtualDevice->DrawLine(aPoint1, aPoint2);
    mpVirtualDevice->DrawLine(aPoint2, aPoint3);
    mpVirtualDevice->DrawLine(aPoint3, aPoint4);
    mpVirtualDevice->DrawLine(aPoint4, aPoint1);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestLine::setupLines()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    Point aHorizontalLinePoint1, aHorizontalLinePoint2;
    Point aVerticalLinePoint1, aVerticalLinePoint2;
    Point aDiagonalLinePoint1, aDiagonalLinePoint2;

    OutputDeviceTestCommon::createHorizontalVerticalDiagonalLinePoints(
                          maVDRectangle, aHorizontalLinePoint1, aHorizontalLinePoint2,
                                         aVerticalLinePoint1,   aVerticalLinePoint2,
                                         aDiagonalLinePoint1,   aDiagonalLinePoint2);

    mpVirtualDevice->DrawLine(aHorizontalLinePoint1, aHorizontalLinePoint2);
    mpVirtualDevice->DrawLine(aVerticalLinePoint1,   aVerticalLinePoint2);
    mpVirtualDevice->DrawLine(aDiagonalLinePoint1,   aDiagonalLinePoint2);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestLine::setupAALines()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetAntialiasing(AntialiasingFlags::EnableB2dDraw);
    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    Point aHorizontalLinePoint1, aHorizontalLinePoint2;
    Point aVerticalLinePoint1, aVerticalLinePoint2;
    Point aDiagonalLinePoint1, aDiagonalLinePoint2;

    OutputDeviceTestCommon::createHorizontalVerticalDiagonalLinePoints(
                          maVDRectangle, aHorizontalLinePoint1, aHorizontalLinePoint2,
                                         aVerticalLinePoint1,   aVerticalLinePoint2,
                                         aDiagonalLinePoint1,   aDiagonalLinePoint2);

    mpVirtualDevice->DrawLine(aHorizontalLinePoint1, aHorizontalLinePoint2);
    mpVirtualDevice->DrawLine(aVerticalLinePoint1,   aVerticalLinePoint2);
    mpVirtualDevice->DrawLine(aDiagonalLinePoint1,   aDiagonalLinePoint2);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

}} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
