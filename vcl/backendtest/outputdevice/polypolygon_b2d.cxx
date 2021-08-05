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
basegfx::B2DPolygon createPolygonOffset(tools::Rectangle const& rRect, int nOffset, int nFix = 0)
{
    // Note: According to https://lists.freedesktop.org/archives/libreoffice/2019-November/083709.html
    // filling polygons always skips the right-most and bottom-most pixels, in order to avoid
    // overlaps when drawing adjacent polygons. Specifying nFix = 1 allows to visually compensate
    // for this by making the polygon explicitly larger.
    int nMidOffset = rRect.GetWidth() / 2;
    basegfx::B2DPolygon aPolygon{
        basegfx::B2DPoint(rRect.Left() + nOffset - (nOffset + 1) / 2, rRect.Top() + nOffset - 1),
        basegfx::B2DPoint(rRect.Right() - nMidOffset - nOffset / 3 + nFix,
                          rRect.Top() + nOffset - 1),
        basegfx::B2DPoint(rRect.Right() - nMidOffset - nOffset / 3 + nFix,
                          rRect.Bottom() - nOffset + 1 + nFix),
        basegfx::B2DPoint(rRect.Left() + nOffset - (nOffset + 1) / 2,
                          rRect.Bottom() - nOffset + 1 + nFix),
    };
    aPolygon.setClosed(true);
    return aPolygon;
}

} // end anonymous namespace

Bitmap OutputDeviceTestPolyPolygonB2D::setupRectangle(bool bEnableAA)
{
    initialSetup(13, 13, constBackgroundColor, bEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    basegfx::B2DPolyPolygon aPolyPolygon;
    aPolyPolygon.append(createPolygonOffset(maVDRectangle, 2));
    aPolyPolygon.append(createPolygonOffset(maVDRectangle, 5));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygonB2D::setupFilledRectangle(bool useLineColor)
{
    initialSetup(13, 13, constBackgroundColor);

    if (useLineColor)
        mpVirtualDevice->SetLineColor(constLineColor);
    else
        mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(constFillColor);

    basegfx::B2DPolyPolygon aPolyPolygon(
        createPolygonOffset(maVDRectangle, 2, useLineColor ? 0 : 1));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygonB2D::setupIntersectingRectangles()
{
    initialSetup(24, 24, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor(constFillColor);

    basegfx::B2DPolyPolygon aPolyPolygon;

    int nOffset = 2, nFix = 1;
    basegfx::B2DPolygon aPolygon1, aPolygon2, aPolygon3, aPolygon4;

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
    aPolygon1.append(basegfx::B2DPoint(maVDRectangle.Left() + nOffset + nFix,
                                       maVDRectangle.Top() + (nOffset - 1) + nFix));
    aPolygon1.append(basegfx::B2DPoint(maVDRectangle.Right() - (nOffset + 2) + nFix,
                                       maVDRectangle.Top() + (nOffset - 1) + nFix));
    aPolygon1.append(basegfx::B2DPoint(maVDRectangle.Right() - (nOffset + 2) + nFix,
                                       maVDRectangle.Bottom() - (nOffset + 8) + nFix));
    aPolygon1.append(basegfx::B2DPoint(maVDRectangle.Left() + nOffset + nFix,
                                       maVDRectangle.Bottom() - (nOffset + 8) + nFix));
    aPolygon1.setClosed(true);
    aPolyPolygon.append(aPolygon1);

    //Rect - 2
    aPolygon2.append(basegfx::B2DPoint(maVDRectangle.Left() + nOffset + nFix,
                                       maVDRectangle.Top() + (nOffset + 2) + nFix));
    aPolygon2.append(basegfx::B2DPoint(maVDRectangle.Right() - (nOffset + 2) + nFix,
                                       maVDRectangle.Top() + (nOffset + 2) + nFix));
    aPolygon2.append(basegfx::B2DPoint(maVDRectangle.Right() - (nOffset + 2) + nFix,
                                       maVDRectangle.Bottom() - (nOffset + 5) + nFix));
    aPolygon2.append(basegfx::B2DPoint(maVDRectangle.Left() + nOffset + nFix,
                                       maVDRectangle.Bottom() - (nOffset + 5) + nFix));
    aPolygon2.setClosed(true);
    aPolyPolygon.append(aPolygon2);

    //Rect - 3
    aPolygon3.append(basegfx::B2DPoint(maVDRectangle.Left() + nOffset + nFix,
                                       maVDRectangle.Top() + (nOffset + 5) + nFix));
    aPolygon3.append(basegfx::B2DPoint(maVDRectangle.Right() - (nOffset + 2) + nFix,
                                       maVDRectangle.Top() + (nOffset + 5) + nFix));
    aPolygon3.append(basegfx::B2DPoint(maVDRectangle.Right() - (nOffset + 2) + nFix,
                                       maVDRectangle.Bottom() - (nOffset + 2) + nFix));
    aPolygon3.append(basegfx::B2DPoint(maVDRectangle.Left() + nOffset + nFix,
                                       maVDRectangle.Bottom() - (nOffset + 2) + nFix));
    aPolygon3.setClosed(true);
    aPolyPolygon.append(aPolygon3);

    //Rect - 4
    aPolygon4.append(basegfx::B2DPoint(maVDRectangle.Left() + nOffset + nFix,
                                       maVDRectangle.Top() + (nOffset + 8) + nFix));
    aPolygon4.append(basegfx::B2DPoint(maVDRectangle.Right() - (nOffset + 2) + nFix,
                                       maVDRectangle.Top() + (nOffset + 8) + nFix));
    aPolygon4.append(basegfx::B2DPoint(maVDRectangle.Right() - (nOffset + 2) + nFix,
                                       maVDRectangle.Bottom() - nOffset + nFix));
    aPolygon4.append(basegfx::B2DPoint(maVDRectangle.Left() + nOffset + nFix,
                                       maVDRectangle.Bottom() - nOffset + nFix));
    aPolygon4.setClosed(true);
    aPolyPolygon.append(aPolygon4);

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygonB2D::setupRectangleOnSize1028()
{
    initialSetup(1028, 1028, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    basegfx::B2DPolyPolygon aPolyPolygon;
    aPolyPolygon.append(createPolygonOffset(maVDRectangle, 2));
    aPolyPolygon.append(createPolygonOffset(maVDRectangle, 5));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygonB2D::setupRectangleOnSize4096()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    basegfx::B2DPolyPolygon aPolyPolygon;
    aPolyPolygon.append(createPolygonOffset(maVDRectangle, 2));
    aPolyPolygon.append(createPolygonOffset(maVDRectangle, 5));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
