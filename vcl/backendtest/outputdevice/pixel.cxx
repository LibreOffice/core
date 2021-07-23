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
void drawPixelOffset(OutputDevice& rDevice, tools::Rectangle const& rRect, int nOffset)
{
    for (tools::Long x = 0 + nOffset; x < (rRect.GetWidth() - nOffset); ++x)
    {
        tools::Long y1 = nOffset;
        tools::Long y2 = rRect.GetHeight() - nOffset - 1;

        rDevice.DrawPixel(Point(x, y1));
        rDevice.DrawPixel(Point(x, y2));
    }

    for (tools::Long y = 0 + nOffset; y < (rRect.GetHeight() - nOffset); ++y)
    {
        tools::Long x1 = nOffset;
        tools::Long x2 = rRect.GetWidth() - nOffset - 1;

        rDevice.DrawPixel(Point(x1, y));
        rDevice.DrawPixel(Point(x2, y));
    }
}

} // end anonymous namespace

Bitmap OutputDeviceTestPixel::setupRectangle(bool bEnableAA)
{
    initialSetup(13, 13, constBackgroundColor, bEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPixelOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPixelOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPixel::setupRectangleOnSize1028()
{
    initialSetup(1028, 1028, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPixelOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPixelOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestPixel::setupRectangleOnSize4096()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPixelOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPixelOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
