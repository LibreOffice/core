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
tools::Polygon createPolygonOffset(tools::Rectangle const& rRect, int nOffset, int nFix = 0)
{
    // Note: According to https://lists.freedesktop.org/archives/libreoffice/2019-November/083709.html
    // filling polygons always skips the right-most and bottom-most pixels, in order to avoid
    // overlaps when drawing adjacent polygons. Specifying nFix = 1 allows to visually compensate
    // for this by making the polygon explicitly larger.
    tools::Polygon aPolygon(4);
    int nMidOffset = rRect.GetWidth() / 2;
    aPolygon.SetPoint(Point(rRect.Left() + nOffset - (nOffset + 1) / 2, rRect.Top() + nOffset - 1),
                      0);
    aPolygon.SetPoint(
        Point(rRect.Right() - nMidOffset + nFix - nOffset / 3, rRect.Top() + nOffset - 1), 1);
    aPolygon.SetPoint(
        Point(rRect.Right() - nMidOffset + nFix - nOffset / 3, rRect.Bottom() - nOffset + nFix + 1),
        2);
    aPolygon.SetPoint(
        Point(rRect.Left() + nOffset - (nOffset + 1) / 2, rRect.Bottom() - nOffset + nFix + 1), 3);
    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);
    return aPolygon;
}

} // end anonymous namespace

Bitmap OutputDeviceTestPolyPolygon::setupRectangle(bool bEnableAA)
{
    initialSetup(13, 13, constBackgroundColor, bEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    tools::PolyPolygon aPolyPolygon(2);
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 2));
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 5));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygon::setupFilledRectangle(bool useLineColor)
{
    initialSetup(13, 13, constBackgroundColor);

    if (useLineColor)
        mpVirtualDevice->SetLineColor(constLineColor);
    else
        mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(constFillColor);

    tools::PolyPolygon aPolyPolygon(1);
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 2, useLineColor ? 0 : 1));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygon::setupIntersectingRectangles()
{
    initialSetup(24, 24, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor(constFillColor);

    tools::PolyPolygon aPolyPolygon(4);

    int nOffset = 2, nFix = 1;
    tools::Polygon aPolygon1(4), aPolygon2(4), aPolygon3(4), aPolygon4(4);

    /*
        The intersection between different rectangles has been
        achieved by stacking them on top of each other and decreasing and
        increasing the top and bottom offset accordingly to the rectangle
        keeping the left and the right offset intact which in turn coalesced
        them to each other helping in achieving multiple intersecting rectangles.
        The desired color fill pattern is then achieved by setting the fill
        color which in turn would fill the shape with the provided color
        in accordance to the even-odd filling rule.
    */

    //Rect - 1
    aPolygon1.SetPoint(
        Point(maVDRectangle.Left() + nOffset + nFix, maVDRectangle.Top() + (nOffset - 1) + nFix),
        0);
    aPolygon1.SetPoint(Point(maVDRectangle.Right() - (nOffset + 2) + nFix,
                             maVDRectangle.Top() + (nOffset - 1) + nFix),
                       1);
    aPolygon1.SetPoint(Point(maVDRectangle.Right() - (nOffset + 2) + nFix,
                             maVDRectangle.Bottom() - (nOffset + 8) + nFix),
                       2);
    aPolygon1.SetPoint(
        Point(maVDRectangle.Left() + nOffset + nFix, maVDRectangle.Bottom() - (nOffset + 8) + nFix),
        3);
    aPolyPolygon.Insert(aPolygon1);

    //Rect - 2
    aPolygon2.SetPoint(
        Point(maVDRectangle.Left() + nOffset + nFix, maVDRectangle.Top() + (nOffset + 2) + nFix),
        0);
    aPolygon2.SetPoint(Point(maVDRectangle.Right() - (nOffset + 2) + nFix,
                             maVDRectangle.Top() + (nOffset + 2) + nFix),
                       1);
    aPolygon2.SetPoint(Point(maVDRectangle.Right() - (nOffset + 2) + nFix,
                             maVDRectangle.Bottom() - (nOffset + 5) + nFix),
                       2);
    aPolygon2.SetPoint(
        Point(maVDRectangle.Left() + nOffset + nFix, maVDRectangle.Bottom() - (nOffset + 5) + nFix),
        3);
    aPolyPolygon.Insert(aPolygon2);

    //Rect - 3
    aPolygon3.SetPoint(
        Point(maVDRectangle.Left() + nOffset + nFix, maVDRectangle.Top() + (nOffset + 5) + nFix),
        0);
    aPolygon3.SetPoint(Point(maVDRectangle.Right() - (nOffset + 2) + nFix,
                             maVDRectangle.Top() + (nOffset + 5) + nFix),
                       1);
    aPolygon3.SetPoint(Point(maVDRectangle.Right() - (nOffset + 2) + nFix,
                             maVDRectangle.Bottom() - (nOffset + 2) + nFix),
                       2);
    aPolygon3.SetPoint(
        Point(maVDRectangle.Left() + nOffset + nFix, maVDRectangle.Bottom() - (nOffset + 2) + nFix),
        3);
    aPolyPolygon.Insert(aPolygon3);

    //Rect - 4
    aPolygon4.SetPoint(
        Point(maVDRectangle.Left() + nOffset + nFix, maVDRectangle.Top() + (nOffset + 8) + nFix),
        0);
    aPolygon4.SetPoint(Point(maVDRectangle.Right() - (nOffset + 2) + nFix,
                             maVDRectangle.Top() + (nOffset + 8) + nFix),
                       1);
    aPolygon4.SetPoint(Point(maVDRectangle.Right() - (nOffset + 2) + nFix,
                             maVDRectangle.Bottom() - nOffset + nFix),
                       2);
    aPolygon4.SetPoint(
        Point(maVDRectangle.Left() + nOffset + nFix, maVDRectangle.Bottom() - nOffset + nFix), 3);
    aPolyPolygon.Insert(aPolygon4);

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygon::setupRectangleOnSize1028()
{
    initialSetup(1028, 1028, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    tools::PolyPolygon aPolyPolygon(2);
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 2));
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 5));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygon::setupRectangleOnSize4096()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    tools::PolyPolygon aPolyPolygon(2);
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 2));
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 5));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
