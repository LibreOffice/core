/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapProcessor.hxx>
#include <vcl/bmpacc.hxx>
#include <basegfx/color/bcolortools.hxx>

BitmapEx BitmapProcessor::createLightImage(const BitmapEx& rBitmapEx)
{
    const Size aSize(rBitmapEx.GetSizePixel());

    Bitmap aBitmap(rBitmapEx.GetBitmap());
    Bitmap aDarkBitmap(aSize, 24);

    BitmapReadAccess* pRead(aBitmap.AcquireReadAccess());
    BitmapWriteAccess* pWrite(aDarkBitmap.AcquireWriteAccess());

    if (pRead && pWrite)
    {
        for (long nY = 0; nY < aSize.Height(); ++nY)
        {
            for (long nX = 0; nX < aSize.Width(); ++nX)
            {
                BitmapColor aColor = pRead->HasPalette() ?
                                        pRead->GetPaletteColor(pRead->GetPixelIndex(nY, nX)) :
                                        pRead->GetPixel(nY, nX);
                basegfx::BColor aBColor(Color(aColor.Invert()).getBColor());
                aBColor = basegfx::tools::rgb2hsl(aBColor);

                double fHue = aBColor.getRed();
                fHue += 180.0;
                while (fHue > 360.0)
                    fHue -= 360.0;
                aBColor.setRed(fHue);

                aBColor = basegfx::tools::hsl2rgb(aBColor);
                aColor.SetRed(((aBColor.getRed()   * 255.0) + 0.5));
                aColor.SetGreen(((aBColor.getGreen() * 255.0) + 0.5));
                aColor.SetBlue(((aBColor.getBlue()  * 255.0) + 0.5));

                pWrite->SetPixel(nY, nX, aColor);
            }
        }
    }
    Bitmap::ReleaseAccess(pWrite);
    Bitmap::ReleaseAccess(pRead);

    return BitmapEx(aDarkBitmap, rBitmapEx.GetAlpha());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
