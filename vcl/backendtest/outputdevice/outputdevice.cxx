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

namespace vcl::test {

Bitmap OutputDeviceTestAnotherOutDev::setupDrawOutDev()
{
    ScopedVclPtrInstance<VirtualDevice> pSourceDev;
    Size aSourceSize(9, 9);
    pSourceDev->SetOutputSizePixel(aSourceSize);
    pSourceDev->SetBackground(Wallpaper(constFillColor));
    pSourceDev->Erase();

    initialSetup(13, 13, constBackgroundColor);

    mpVirtualDevice->DrawOutDev(Point(2, 2), aSourceSize, Point(), aSourceSize, *pSourceDev);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

Bitmap OutputDeviceTestAnotherOutDev::setupXOR()
{
    initialSetup(13, 13, constBackgroundColor);

    tools::Rectangle aDrawRectangle(maVDRectangle);
    aDrawRectangle.shrink(2);

    tools::Rectangle aScissorRectangle(maVDRectangle);
    aScissorRectangle.shrink(4);

    mpVirtualDevice->SetRasterOp(RasterOp::Xor);
    mpVirtualDevice->SetFillColor(constFillColor);
    mpVirtualDevice->DrawRect(aDrawRectangle);

    mpVirtualDevice->SetRasterOp(RasterOp::N0);
    mpVirtualDevice->SetFillColor(COL_BLACK);
    mpVirtualDevice->DrawRect(aScissorRectangle);

    mpVirtualDevice->SetRasterOp(RasterOp::Xor);
    mpVirtualDevice->SetFillColor(constFillColor);
    mpVirtualDevice->DrawRect(aDrawRectangle);

    mpVirtualDevice->SetRasterOp(RasterOp::Xor);
    mpVirtualDevice->SetLineColor(constFillColor);
    mpVirtualDevice->SetFillColor();
    // Rectangle drawn twice is a no-op.
    aDrawRectangle = maVDRectangle;
    mpVirtualDevice->DrawRect(aDrawRectangle);
    mpVirtualDevice->DrawRect(aDrawRectangle);
    // Rectangle drawn three times is like drawing once.
    aDrawRectangle.shrink(1);
    mpVirtualDevice->DrawRect(aDrawRectangle);
    mpVirtualDevice->DrawRect(aDrawRectangle);
    mpVirtualDevice->DrawRect(aDrawRectangle);

    return mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
}

TestResult OutputDeviceTestAnotherOutDev::checkDrawOutDev(Bitmap& rBitmap)
{
    std::vector<Color> aExpected
    {
        constBackgroundColor, constBackgroundColor,
        constFillColor, constFillColor, constFillColor, constFillColor, constFillColor
    };
    return checkRectangles(rBitmap, aExpected);
}

TestResult OutputDeviceTestAnotherOutDev::checkXOR(Bitmap& rBitmap)
{
    Color xorColor( constBackgroundColor.GetRed() ^ constFillColor.GetRed(),
                    constBackgroundColor.GetGreen() ^ constFillColor.GetGreen(),
                    constBackgroundColor.GetBlue() ^ constFillColor.GetBlue());
    std::vector<Color> aExpected
    {
        constBackgroundColor, xorColor,
        constBackgroundColor, constBackgroundColor,
        constFillColor, constFillColor,
        constFillColor
    };
    return checkRectangles(rBitmap, aExpected);
}

} // end namespace vcl::test

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
