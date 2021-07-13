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

namespace vcl::test {

namespace
{

void drawPolygonOffset(OutputDevice& rDevice, tools::Rectangle const & rRect, int nOffset, int nFix = 0)
{
    // Note: According to https://lists.freedesktop.org/archives/libreoffice/2019-November/083709.html
    // filling polygons always skips the right-most and bottom-most pixels, in order to avoid
    // overlaps when drawing adjacent polygons. Specifying nFix = 1 allows to visually compensate
    // for this by making the polygon explicitly larger.
    tools::Polygon aPolygon(4);
    aPolygon.SetPoint(Point(rRect.Left()  + nOffset, rRect.Top()    + nOffset), 0);
    aPolygon.SetPoint(Point(rRect.Right() - nOffset + nFix, rRect.Top()    + nOffset), 1);
    aPolygon.SetPoint(Point(rRect.Right() - nOffset + nFix, rRect.Bottom() - nOffset + nFix), 2);
    aPolygon.SetPoint(Point(rRect.Left()  + nOffset, rRect.Bottom() - nOffset + nFix), 3);
    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    rDevice.DrawPolygon(aPolygon);
}

} // end anonymous namespace

Bitmap OutputDeviceTestPolygon::setupRectangle(bool bEnableAA)
{
    initialSetup(13, 13, constBackgroundColor, bEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPolygonOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPolygonOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupFilledRectangle(bool useLineColor)
{
    initialSetup(13, 13, constBackgroundColor);

    if(useLineColor)
        mpVirtualDevice->SetLineColor(constLineColor);
    else
        mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(constFillColor);
    drawPolygonOffset(*mpVirtualDevice, maVDRectangle, 2, useLineColor ? 0 : 1);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupDiamond()
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

    mpVirtualDevice->DrawPolygon(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupLines()
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
    mpVirtualDevice->DrawPolygon(aHorizontalPolygon);

    tools::Polygon aVerticalPolygon(2);
    aVerticalPolygon.SetPoint(aVerticalLinePoint1, 0);
    aVerticalPolygon.SetPoint(aVerticalLinePoint2, 1);
    mpVirtualDevice->DrawPolygon(aVerticalPolygon);

    tools::Polygon aDiagonalPolygon(2);
    aDiagonalPolygon.SetPoint(aDiagonalLinePoint1, 0);
    aDiagonalPolygon.SetPoint(aDiagonalLinePoint2, 1);
    mpVirtualDevice->DrawPolygon(aDiagonalPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupAALines()
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
    mpVirtualDevice->DrawPolygon(aHorizontalPolygon);

    tools::Polygon aVerticalPolygon(2);
    aVerticalPolygon.SetPoint(aVerticalLinePoint1, 0);
    aVerticalPolygon.SetPoint(aVerticalLinePoint2, 1);
    mpVirtualDevice->DrawPolygon(aVerticalPolygon);

    tools::Polygon aDiagonalPolygon(2);
    aDiagonalPolygon.SetPoint(aDiagonalLinePoint1, 0);
    aDiagonalPolygon.SetPoint(aDiagonalLinePoint2, 1);
    mpVirtualDevice->DrawPolygon(aDiagonalPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupBezier()
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

    mpVirtualDevice->DrawPolygon(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupAABezier()
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

    mpVirtualDevice->DrawPolygon(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupEllipse()
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

Bitmap OutputDeviceTestPolygon::setupClosedBezier()
{
    initialSetup(21, 16, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    tools::Long minX = maVDRectangle.Left();
    tools::Long maxX = maVDRectangle.Right() - 2;
    tools::Long minY = maVDRectangle.Top();
    tools::Long maxY = maVDRectangle.Bottom() - 2;

    tools::Polygon aPolygon(4);

    aPolygon.SetPoint(Point((maxX/2.0),maxY),0);
    aPolygon.SetFlags(0,PolyFlags::Normal);
    aPolygon.SetPoint(Point(maxX,minY),1);
    aPolygon.SetFlags(1,PolyFlags::Control);
    aPolygon.SetPoint(Point(minX,minY),2);
    aPolygon.SetFlags(2,PolyFlags::Control);
    aPolygon.SetPoint(Point((maxX/2.0),maxY),3);
    aPolygon.SetFlags(3,PolyFlags::Normal);

    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    mpVirtualDevice->DrawPolyLine(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupFilledAsymmetricalCircle()
{
    initialSetup(21, 18, constBackgroundColor);

    mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(constFillColor);

    std::vector<Point> aDiamondPoints;
    Point aPoint1, aPoint2, aPoint3, aPoint4;
    OutputDeviceTestCommon::createAndAppendDiamondPoints(maVDRectangle, 8, aPoint1, aPoint2,
                                                         aPoint3, aPoint4, aDiamondPoints);

    tools::Long distanceFromCoordinate
        = 4 * (abs(aDiamondPoints[0].getY() - aDiamondPoints[2].getY()) / 2) * ((sqrt(2) - 1) / 3);
    tools::Long X, Y;
    int aPointNumber = 0;

    tools::Polygon aPolygon(16);

    aPolygon.SetPoint(aPoint1, 0);
    aPolygon.SetFlags(0, PolyFlags::Normal);
    X = aDiamondPoints[aPointNumber].getX();
    Y = aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber + 1) % 4;
    aPolygon.SetPoint(Point(X + distanceFromCoordinate, Y), 1);
    aPolygon.SetFlags(1, PolyFlags::Control);
    X = aDiamondPoints[aPointNumber].getX();
    Y = aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber + 1) % 4;
    aPolygon.SetPoint(Point(X + 2, Y - distanceFromCoordinate), 2);
    aPolygon.SetFlags(2, PolyFlags::Control);
    aPolygon.SetPoint(aPoint2, 3);

    aPolygon.SetFlags(3, PolyFlags::Normal);
    aPolygon.SetPoint(aPoint2, 4);
    aPolygon.SetFlags(4, PolyFlags::Normal);
    aPolygon.SetPoint(Point(X, Y + distanceFromCoordinate), 5);
    aPolygon.SetFlags(5, PolyFlags::Control);
    X = aDiamondPoints[aPointNumber].getX();
    Y = aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber + 1) % 4;
    aPolygon.SetPoint(Point(X + distanceFromCoordinate, Y + 2), 6);
    aPolygon.SetFlags(6, PolyFlags::Control);
    aPolygon.SetPoint(aPoint3, 7);
    aPolygon.SetFlags(7, PolyFlags::Normal);

    aPolygon.SetPoint(aPoint3, 8);
    aPolygon.SetFlags(8, PolyFlags::Normal);
    aPolygon.SetPoint(Point(X - distanceFromCoordinate, Y), 9);
    aPolygon.SetFlags(9, PolyFlags::Control);
    X = aDiamondPoints[aPointNumber].getX();
    Y = aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber + 1) % 4;
    aPolygon.SetPoint(Point(X, Y + distanceFromCoordinate - 2), 10);
    aPolygon.SetFlags(10, PolyFlags::Control);
    aPolygon.SetPoint(aPoint4, 11);
    aPolygon.SetFlags(11, PolyFlags::Normal);

    aPolygon.SetPoint(aPoint4, 12);
    aPolygon.SetFlags(12, PolyFlags::Normal);
    aPolygon.SetPoint(Point(X - 2, Y - distanceFromCoordinate), 13);
    X = aDiamondPoints[aPointNumber].getX();
    Y = aDiamondPoints[aPointNumber].getY();
    aPointNumber = (aPointNumber + 1) % 4;
    aPolygon.SetFlags(13, PolyFlags::Control);
    aPolygon.SetPoint(Point(X - distanceFromCoordinate - 2, Y), 14);
    aPolygon.SetFlags(14, PolyFlags::Control);
    aPolygon.SetPoint(aPoint1, 15);
    aPolygon.SetFlags(15, PolyFlags::Normal);

    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);

    mpVirtualDevice->DrawPolygon(aPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}
} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
