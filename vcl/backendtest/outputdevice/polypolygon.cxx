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

tools::Polygon createPolygonOffset(tools::Rectangle& rRect, int nOffset)
{
    tools::Polygon aPolygon(4);
    aPolygon.SetPoint(Point(rRect.Left()  + nOffset, rRect.Top()    + nOffset), 0);
    aPolygon.SetPoint(Point(rRect.Right() - nOffset, rRect.Top()    + nOffset), 1);
    aPolygon.SetPoint(Point(rRect.Right() - nOffset, rRect.Bottom() - nOffset), 2);
    aPolygon.SetPoint(Point(rRect.Left()  + nOffset, rRect.Bottom() - nOffset), 3);
    aPolygon.Optimize(PolyOptimizeFlags::CLOSE);
    return aPolygon;
}

} // end anonymous namespace

Bitmap OutputDeviceTestPolyPolygon::setupRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    tools::PolyPolygon aPolyPolygon(2);
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 2));
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 5));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPolyPolygon::setupFilledRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constFillColor);
    mpVirtualDevice->SetFillColor(constFillColor);

    tools::PolyPolygon aPolyPolygon(3);
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 2));
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 4));
    aPolyPolygon.Insert(createPolygonOffset(maVDRectangle, 4));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

}} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
