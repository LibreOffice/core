/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <basegfx/color/bcolortools.hxx>

#include <bitmapwriteaccess.hxx>
#include <BitmapLightenFilter.hxx>

BitmapEx BitmapLightenFilter::execute(BitmapEx const& rBitmapEx) const
{
    const Size aSize(rBitmapEx.GetSizePixel());

    Bitmap aBitmap(rBitmapEx.GetBitmap());
    Bitmap aDarkBitmap(aSize, 24);

    Bitmap::ScopedReadAccess pRead(aBitmap);
    BitmapScopedWriteAccess pWrite(aDarkBitmap);

    if (pRead && pWrite)
    {
        for (tools::Long nY = 0; nY < aSize.Height(); ++nY)
        {
            Scanline pScanline = pWrite->GetScanline(nY);
            Scanline pScanlineRead = pRead->GetScanline(nY);
            for (tools::Long nX = 0; nX < aSize.Width(); ++nX)
            {
                BitmapColor aBmpColor
                    = pRead->HasPalette()
                          ? pRead->GetPaletteColor(pRead->GetIndexFromData(pScanlineRead, nX))
                          : pRead->GetPixelFromData(pScanlineRead, nX);
                aBmpColor.Invert();
                basegfx::BColor aBColor(aBmpColor.getBColor());
                aBColor = basegfx::utils::rgb2hsl(aBColor);

                double fHue = aBColor.getRed();
                fHue += 180.0;

                while (fHue > 360.0)
                {
                    fHue -= 360.0;
                }

                aBColor.setRed(fHue);

                aBColor = basegfx::utils::hsl2rgb(aBColor);
                aBmpColor.SetRed((aBColor.getRed() * 255.0) + 0.5);
                aBmpColor.SetGreen((aBColor.getGreen() * 255.0) + 0.5);
                aBmpColor.SetBlue((aBColor.getBlue() * 255.0) + 0.5);

                pWrite->SetPixelOnData(pScanline, nX, aBmpColor);
            }
        }
    }
    pWrite.reset();
    pRead.reset();

    return BitmapEx(aDarkBitmap, rBitmapEx.GetAlpha());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
