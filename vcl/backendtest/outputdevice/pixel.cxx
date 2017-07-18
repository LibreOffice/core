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

void drawPixelOffset(OutputDevice& rDevice, tools::Rectangle const & rRect, int nOffset)
{
    for (long x = 0 + nOffset; x < (rRect.GetWidth() - nOffset); ++x)
    {
        long y1 = nOffset;
        long y2 = rRect.GetHeight() - nOffset - 1;

        rDevice.DrawPixel(Point(x, y1));
        rDevice.DrawPixel(Point(x, y2));
    }

    for (long y = 0 + nOffset; y < (rRect.GetHeight() - nOffset); ++y)
    {
        long x1 = nOffset;
        long x2 = rRect.GetWidth() - nOffset - 1;

        rDevice.DrawPixel(Point(x1, y));
        rDevice.DrawPixel(Point(x2, y));
    }
}

} // end anonymous namespace

Bitmap OutputDeviceTestPixel::setupRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawPixelOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawPixelOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

}} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
