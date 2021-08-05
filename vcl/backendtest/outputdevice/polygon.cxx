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

namespace vcl::test
{
namespace
{
void drawPolygonOffset(OutputDevice& rDevice, tools::Rectangle const& rRect, int nOffset,
                       int nFix = 0)
{
    // Note: According to https://lists.freedesktop.org/archives/libreoffice/2019-November/083709.html
    // filling polygons always skips the right-most and bottom-most pixels, in order to avoid
    // overlaps when drawing adjacent polygons. Specifying nFix = 1 allows to visually compensate
    // for this by making the polygon explicitly larger.
    tools::Polygon aPolygon(4);
    int nMidOffset = rRect.GetWidth() / 2;
    aPolygon.SetPoint(Point(rRect.Left()  + nOffset - (nOffset+1)/2, rRect.Top()    + nOffset - 1), 0);
    aPolygon.SetPoint(Point(rRect.Right() - nMidOffset + nFix - nOffset/3, rRect.Top()    + nOffset - 1), 1);
    aPolygon.SetPoint(Point(rRect.Right() - nMidOffset + nFix - nOffset/3, rRect.Bottom() - nOffset + nFix + 1), 2);
    aPolygon.SetPoint(Point(rRect.Left()  + nOffset - (nOffset+1)/2, rRect.Bottom() - nOffset + nFix + 1), 3);
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

    if (useLineColor)
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
    OutputDeviceTestCommon::createDiamondPoints(maVDRectangle, 4, aPoint1, aPoint2, aPoint3,
                                                aPoint4);

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
        maVDRectangle, aHorizontalLinePoint1, aHorizontalLinePoint2, aVerticalLinePoint1,
        aVerticalLinePoint2, aDiagonalLinePoint1, aDiagonalLinePoint2);

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
        maVDRectangle, aHorizontalLinePoint1, aHorizontalLinePoint2, aVerticalLinePoint1,
        aVerticalLinePoint2, aDiagonalLinePoint1, aDiagonalLinePoint2);

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

Bitmap OutputDeviceTestPolygon::setupDropShape()
{
    initialSetup(21, 21, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    mpVirtualDevice->DrawPolygon(OutputDeviceTestCommon::createDropShapePolygon());

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupAADropShape()
{
    initialSetup(21, 21, constBackgroundColor, true);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    mpVirtualDevice->DrawPolygon(OutputDeviceTestCommon::createDropShapePolygon());

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupHalfEllipse(bool aEnableAA)
{
    initialSetup(19, 21, constBackgroundColor, aEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    mpVirtualDevice->DrawPolyLine(
        tools::Polygon(OutputDeviceTestCommon::createHalfEllipsePolygon()));

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupClosedBezier()
{
    initialSetup(21, 16, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    mpVirtualDevice->DrawPolyLine(OutputDeviceTestCommon::createClosedBezierLoop(maVDRectangle));

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupFilledAsymmetricalDropShape()
{
    initialSetup(21, 21, constBackgroundColor);

    mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(constFillColor);

    mpVirtualDevice->DrawPolygon(OutputDeviceTestCommon::createDropShapePolygon());

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupRectangleOnSize1028()
{
    initialSetup(1028, 1028, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPolygonOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPolygonOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolygon::setupRectangleOnSize4096()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPolygonOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPolygonOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
