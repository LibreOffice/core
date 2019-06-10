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
#include <vcl/bitmapex.hxx>

namespace vcl
{
namespace test
{
namespace
{
void drawPolyLineOffset(OutputDevice& rDevice, tools::Rectangle const& rRect, int nOffset)
{
    basegfx::B2DPolygon aPolygon{
        basegfx::B2DPoint(rRect.Left() + nOffset, rRect.Top() + nOffset),
        basegfx::B2DPoint(rRect.Right() - nOffset, rRect.Top() + nOffset),
        basegfx::B2DPoint(rRect.Right() - nOffset, rRect.Bottom() - nOffset),
        basegfx::B2DPoint(rRect.Left() + nOffset, rRect.Bottom() - nOffset),
    };
    aPolygon.setClosed(true);

    rDevice.DrawPolyLine(aPolygon, 0.0); // draw hairline
}

} // end anonymous namespace

Bitmap OutputDeviceTestPolyLineB2D::setupRectangle(bool bEnableAA)
{
    initialSetup(13, 13, constBackgroundColor, bEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPolyLineOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmapEx(maVDRectangle.TopLeft(), maVDRectangle.GetSize())
        .GetBitmap();
}
}
} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
