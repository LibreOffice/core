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

namespace vcl
{
namespace test
{
namespace
{
basegfx::B2DPolygon createPolygonOffset(tools::Rectangle const& rRect, int nOffset)
{
    basegfx::B2DPolygon aPolygon{
        basegfx::B2DPoint(rRect.Left() + nOffset, rRect.Top() + nOffset),
        basegfx::B2DPoint(rRect.Right() - nOffset, rRect.Top() + nOffset),
        basegfx::B2DPoint(rRect.Right() - nOffset, rRect.Bottom() - nOffset),
        basegfx::B2DPoint(rRect.Left() + nOffset, rRect.Bottom() - nOffset),
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

Bitmap OutputDeviceTestPolyPolygonB2D::setupFilledRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(constFillColor);

    basegfx::B2DPolyPolygon aPolyPolygon(createPolygonOffset(maVDRectangle, 2));

    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}
}
} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
