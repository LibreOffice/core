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

#include <cmath>
#include <vector>

namespace vcl::test {

namespace
{

void drawPolyLineOffset(OutputDevice& rDevice, tools::Rectangle const & rRect, int nOffset)
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

Bitmap OutputDeviceTestPolyLine::setupRectangle(bool bEnableAA)
{
    initialSetup(13, 13, constBackgroundColor, bEnableAA);

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

    mpVirtualDevice->SetAntialiasing(AntialiasingFlags::Enable);
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

Bitmap OutputDeviceTestPolyLine::setupBezier()
{
    initialSetup(21, 21, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    std::vector<Point> aDiamondPoints;
    Point aPoint1, aPoint2, aPoint3, aPoint4;
    OutputDeviceTestCommon::createAndAppendDiamondPoints(maVDRectangle, 8, aPoint1, aPoint2, aPoint3, aPoint4, aDiamondPoints);

    tools::Long distanceFromCoordinate = 4*(abs(aDiamondPoints[0].getY()-aDiamondPoints[2].getY())/2)*((sqrt(2)-1)/3);
    tools::Long X,Y;
    int aPointNumber = 0;

    tools::Polygon aPolygon(16);

    aPolygon.SetPoint(aPoint1,0);
    aPolygon.SetFlags(0,PolyFlags::Normal);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetPoint(Point(X+distanceFromCoordinate,Y),1);
    aPolygon.SetFlags(1,PolyFlags::Control);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetPoint(Point(X,Y-distanceFromCoordinate),2);
    aPolygon.SetFlags(2,PolyFlags::Control);
    aPolygon.SetPoint(aPoint2,3);

    aPolygon.SetFlags(3,PolyFlags::Normal);
    aPolygon.SetPoint(aPoint2,4);
    aPolygon.SetFlags(4,PolyFlags::Normal);
    aPolygon.SetPoint(Point(X,Y+distanceFromCoordinate),5);
    aPolygon.SetFlags(5,PolyFlags::Control);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetPoint(Point(X+distanceFromCoordinate,Y),6);
    aPolygon.SetFlags(6,PolyFlags::Control);
    aPolygon.SetPoint(aPoint3,7);
    aPolygon.SetFlags(7,PolyFlags::Normal);

    aPolygon.SetPoint(aPoint3,8);
    aPolygon.SetFlags(8,PolyFlags::Normal);
    aPolygon.SetPoint(Point(X-distanceFromCoordinate,Y),9);
    aPolygon.SetFlags(9,PolyFlags::Control);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetPoint(Point(X,Y+distanceFromCoordinate),10);
    aPolygon.SetFlags(10,PolyFlags::Control);
    aPolygon.SetPoint(aPoint4,11);
    aPolygon.SetFlags(11,PolyFlags::Normal);

    aPolygon.SetPoint(aPoint4,12);
    aPolygon.SetFlags(12,PolyFlags::Normal);
    aPolygon.SetPoint(Point(X,Y-distanceFromCoordinate),13);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetFlags(13,PolyFlags::Control);
    aPolygon.SetPoint(Point(X-distanceFromCoordinate,Y),14);
    aPolygon.SetFlags(14,PolyFlags::Control);
    aPolygon.SetPoint(aPoint1,15);
    aPolygon.SetFlags(15,PolyFlags::Normal);

    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    mpVirtualDevice->DrawPolyLine(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLine::setupAABezier()
{
    initialSetup(21, 21, constBackgroundColor,true);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    std::vector<Point> aDiamondPoints;
    Point aPoint1, aPoint2, aPoint3, aPoint4;
    OutputDeviceTestCommon::createAndAppendDiamondPoints(maVDRectangle, 8, aPoint1, aPoint2, aPoint3, aPoint4, aDiamondPoints);

    tools::Long distanceFromCoordinate = 4*(abs(aDiamondPoints[0].getY()-aDiamondPoints[2].getY())/2)*((sqrt(2)-1)/3);
    tools::Long X,Y;
    int aPointNumber = 0;

    tools::Polygon aPolygon(16);

    aPolygon.SetPoint(aPoint1,0);
    aPolygon.SetFlags(0,PolyFlags::Normal);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetPoint(Point(X+distanceFromCoordinate,Y),1);
    aPolygon.SetFlags(1,PolyFlags::Control);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetPoint(Point(X,Y-distanceFromCoordinate),2);
    aPolygon.SetFlags(2,PolyFlags::Control);
    aPolygon.SetPoint(aPoint2,3);

    aPolygon.SetFlags(3,PolyFlags::Normal);
    aPolygon.SetPoint(aPoint2,4);
    aPolygon.SetFlags(4,PolyFlags::Normal);
    aPolygon.SetPoint(Point(X,Y+distanceFromCoordinate),5);
    aPolygon.SetFlags(5,PolyFlags::Control);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetPoint(Point(X+distanceFromCoordinate,Y),6);
    aPolygon.SetFlags(6,PolyFlags::Control);
    aPolygon.SetPoint(aPoint3,7);
    aPolygon.SetFlags(7,PolyFlags::Normal);

    aPolygon.SetPoint(aPoint3,8);
    aPolygon.SetFlags(8,PolyFlags::Normal);
    aPolygon.SetPoint(Point(X-distanceFromCoordinate,Y),9);
    aPolygon.SetFlags(9,PolyFlags::Control);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetPoint(Point(X,Y+distanceFromCoordinate),10);
    aPolygon.SetFlags(10,PolyFlags::Control);
    aPolygon.SetPoint(aPoint4,11);
    aPolygon.SetFlags(11,PolyFlags::Normal);

    aPolygon.SetPoint(aPoint4,12);
    aPolygon.SetFlags(12,PolyFlags::Normal);
    aPolygon.SetPoint(Point(X,Y-distanceFromCoordinate),13);
    X=aDiamondPoints[aPointNumber].getX();
    Y=aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber+1)%4;
    aPolygon.SetFlags(13,PolyFlags::Control);
    aPolygon.SetPoint(Point(X-distanceFromCoordinate,Y),14);
    aPolygon.SetFlags(14,PolyFlags::Control);
    aPolygon.SetPoint(aPoint1,15);
    aPolygon.SetFlags(15,PolyFlags::Normal);

    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    mpVirtualDevice->DrawPolyLine(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyLine::setupEllipse()
{
    initialSetup(19, 21, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    int offset = 8;
    tools::Long mPtX = maVDRectangle.Left() + (maVDRectangle.Right() - maVDRectangle.Left()) / 2.0;
    tools::Long mPtY = maVDRectangle.Top() + (maVDRectangle.Bottom() - maVDRectangle.Top()) / 2.0;
    Point aPoint1 = Point(mPtX + offset, mPtY + 2);
    Point aPoint2 = Point(mPtX - offset, mPtY + 2);

    tools::Polygon aPolygon(4);

    aPolygon.SetPoint(aPoint1,0);
    aPolygon.SetPoint(Point(mPtX + offset,mPtY - offset - 2),1);
    aPolygon.SetFlags(1,PolyFlags::Control);
    aPolygon.SetPoint(Point(mPtX - offset, mPtY - offset - 2),2);
    aPolygon.SetFlags(2,PolyFlags::Control);
    aPolygon.SetPoint(aPoint2,3);

    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    mpVirtualDevice->DrawPolyLine(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}
} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
