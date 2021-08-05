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
void drawRectOffset(OutputDevice& rDevice, tools::Rectangle const& rRect, int nOffset)
{
    int nMidOffset = rRect.Left() + (rRect.Right() - rRect.Left()) / 2;
    rDevice.DrawRect(tools::Rectangle(rRect.Left()  + nOffset - (nOffset+1)/2, rRect.Top()    + nOffset - 1,
                                      rRect.Right() - nMidOffset - nOffset/3,
                                      rRect.Bottom() - nOffset + 1));
}

void drawInvertOffset(OutputDevice& rDevice, tools::Rectangle const& rRect, int nOffset,
                      InvertFlags eFlags)
{
    tools::Rectangle aRectangle(rRect.Left() + nOffset, rRect.Top() + nOffset,
                                rRect.Right() - nOffset, rRect.Bottom() - nOffset);
    rDevice.Invert(aRectangle, eFlags);
}

} // end anonymous namespace

Bitmap OutputDeviceTestRect::setupFilledRectangle(bool useLineColor)
{
    initialSetup(13, 13, constBackgroundColor);

    if (useLineColor)
        mpVirtualDevice->SetLineColor(constLineColor);
    else
        mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(constFillColor);

    drawRectOffset(*mpVirtualDevice, maVDRectangle, 2);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestRect::setupRectangle(bool bEnableAA)
{
    initialSetup(13, 13, constBackgroundColor, bEnableAA);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawRectOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawRectOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestRect::setupInvert_NONE()
{
    initialSetup(20, 20, COL_WHITE);

    mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(COL_LIGHTRED);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(2, 2), Size(8, 8)));
    mpVirtualDevice->SetFillColor(COL_LIGHTGREEN);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(10, 2), Size(8, 8)));
    mpVirtualDevice->SetFillColor(COL_LIGHTBLUE);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(2, 10), Size(8, 8)));

    drawInvertOffset(*mpVirtualDevice, maVDRectangle, 2, InvertFlags::NONE);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestRect::setupInvert_N50()
{
    initialSetup(20, 20, COL_WHITE);

    mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(COL_LIGHTRED);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(2, 2), Size(8, 8)));
    mpVirtualDevice->SetFillColor(COL_LIGHTGREEN);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(10, 2), Size(8, 8)));
    mpVirtualDevice->SetFillColor(COL_LIGHTBLUE);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(2, 10), Size(8, 8)));

    drawInvertOffset(*mpVirtualDevice, maVDRectangle, 2, InvertFlags::N50);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestRect::setupInvert_TrackFrame()
{
    initialSetup(20, 20, COL_WHITE);

    mpVirtualDevice->SetLineColor();
    mpVirtualDevice->SetFillColor(COL_LIGHTRED);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(2, 2), Size(8, 8)));
    mpVirtualDevice->SetFillColor(COL_LIGHTGREEN);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(10, 2), Size(8, 8)));
    mpVirtualDevice->SetFillColor(COL_LIGHTBLUE);
    mpVirtualDevice->DrawRect(tools::Rectangle(Point(2, 10), Size(8, 8)));

    drawInvertOffset(*mpVirtualDevice, maVDRectangle, 2, InvertFlags::TrackFrame);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestRect::setupRectangleOnSize1028()
{
    initialSetup(1028, 1028, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawRectOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawRectOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestRect::setupRectangleOnSize4096()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    drawRectOffset(*mpVirtualDevice, maVDRectangle, 2);
    drawRectOffset(*mpVirtualDevice, maVDRectangle, 5);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
