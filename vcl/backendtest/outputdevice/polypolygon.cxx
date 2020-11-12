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
    aPolygon.SetPoint(Point(rRect.Left() + nOffset, rRect.Top() + nOffset), 0);
    aPolygon.SetPoint(Point(rRect.Right() - nOffset + nFix, rRect.Top() + nOffset), 1);
    aPolygon.SetPoint(Point(rRect.Right() - nOffset + nFix, rRect.Bottom() - nOffset + nFix), 2);
    aPolygon.SetPoint(Point(rRect.Left() + nOffset, rRect.Bottom() - nOffset + nFix), 3);
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

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
