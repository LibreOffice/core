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

#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/BitmapLightenFilter.hxx>

Bitmap BitmapLightenFilter::execute(Bitmap const& rBitmap) const
{
    const Size aSize(rBitmap.GetSizePixel());

    Bitmap aDarkBitmap(aSize,
                       rBitmap.HasAlpha() ? vcl::PixelFormat::N32_BPP : vcl::PixelFormat::N24_BPP);

    BitmapScopedReadAccess pRead(rBitmap);
    BitmapScopedWriteAccess pWrite(aDarkBitmap);

    if (pRead && pWrite)
    {
        for (sal_Int32 nY = 0; nY < sal_Int32(aSize.Height()); ++nY)
        {
            Scanline pScanline = pWrite->GetScanline(nY);
            Scanline pScanlineRead = pRead->GetScanline(nY);
            for (sal_Int32 nX = 0; nX < sal_Int32(aSize.Width()); ++nX)
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

    return aDarkBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
