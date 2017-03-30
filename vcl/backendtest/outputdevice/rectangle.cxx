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
    void drawRectOffset(OutputDevice& rDevice, tools::Rectangle& rRect, int nOffset)
    {
        rDevice.DrawRect(tools::Rectangle(rRect.Left()  + nOffset, rRect.Top()    + nOffset,
                                   rRect.Right() - nOffset, rRect.Bottom() - nOffset));

    }
} // end anonymous namespace

Bitmap OutputDeviceTestRect::setupFilledRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constFillColor);
    mpVirtualDevice->SetFillColor(constFillColor);

    drawRectOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawRectOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestRect::setupRectangle()
{
    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawRectOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawRectOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

}} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
