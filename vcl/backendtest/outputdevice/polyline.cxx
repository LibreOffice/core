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

namespace vcl {
namespace test {

namespace
{

void drawPolyLineOffset(OutputDevice& rDevice, tools::Rectangle& rRect, int nOffset)
{
    tools::Polygon aPolygon(4);
    aPolygon.SetPoint(Point(rRect.Left()  + nOffset, rRect.Top()    + nOffset), 0);
    aPolygon.SetPoint(Point(rRect.Right() - nOffset, rRect.Top()    + nOffset), 1);
    aPolygon.SetPoint(Point(rRect.Right() - nOffset, rRect.Bottom() - nOffset), 2);
    aPolygon.SetPoint(Point(rRect.Left()  + nOffset, rRect.Bottom() - nOffset), 3);
    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    rDevice.DrawPolyLine(aPolygon);
}

} // end anonymous namespace

Bitmap OutputDeviceTestPolyLine::setupRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLine::setupDiamond()
{
    initialSetup(11, 11, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    Point aPoint1, aPoint2, aPoint3, aPoint4;
    OutputDeviceTestCommon::createDiamondPoints(maVDRectangle, 4, aPoint1, aPoint2, aPoint3, aPoint4);

    tools::Polygon aPolygon(4);

    aPolygon.SetPoint(aPoint1, 0);
    aPolygon.SetPoint(aPoint2, 1);
    aPolygon.SetPoint(aPoint3, 2);
    aPolygon.SetPoint(aPoint4, 3);
    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    mpVirtualDevice->DrawPolyLine(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLine::setupLines()
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

    tools::Polygon aHorizontalPolygon(2);
    aHorizontalPolygon.SetPoint(aHorizontalLinePoint1, 0);
    aHorizontalPolygon.SetPoint(aHorizontalLinePoint2, 1);
    mpVirtualDevice->DrawPolyLine(aHorizontalPolygon);

    tools::Polygon aVerticalPolygon(2);
    aVerticalPolygon.SetPoint(aVerticalLinePoint1, 0);
    aVerticalPolygon.SetPoint(aVerticalLinePoint2, 1);
    mpVirtualDevice->DrawPolyLine(aVerticalPolygon);

    tools::Polygon aDiagonalPolygon(2);
    aDiagonalPolygon.SetPoint(aDiagonalLinePoint1, 0);
    aDiagonalPolygon.SetPoint(aDiagonalLinePoint2, 1);
    mpVirtualDevice->DrawPolyLine(aDiagonalPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLine::setupAALines()
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

    tools::Polygon aHorizontalPolygon(2);
    aHorizontalPolygon.SetPoint(aHorizontalLinePoint1, 0);
    aHorizontalPolygon.SetPoint(aHorizontalLinePoint2, 1);
    mpVirtualDevice->DrawPolyLine(aHorizontalPolygon);

    tools::Polygon aVerticalPolygon(2);
    aVerticalPolygon.SetPoint(aVerticalLinePoint1, 0);
    aVerticalPolygon.SetPoint(aVerticalLinePoint2, 1);
    mpVirtualDevice->DrawPolyLine(aVerticalPolygon);

    tools::Polygon aDiagonalPolygon(2);
    aDiagonalPolygon.SetPoint(aDiagonalLinePoint1, 0);
    aDiagonalPolygon.SetPoint(aDiagonalLinePoint2, 1);
    mpVirtualDevice->DrawPolyLine(aDiagonalPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

}} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
