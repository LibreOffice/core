/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/BitmapDisabledImageFilter.hxx>

Bitmap BitmapDisabledImageFilter::execute(Bitmap const& rBitmap) const
{
    const Size aSize(rBitmap.GetSizePixel());

    // keep disable image at same depth as original where possible, otherwise
    // use 8 bit
    auto ePixelFormat = rBitmap.getPixelFormat();
    if (sal_uInt16(ePixelFormat) < 8)
        ePixelFormat = vcl::PixelFormat::N8_BPP;

    const BitmapPalette* pPal
        = vcl::isPalettePixelFormat(ePixelFormat) ? &Bitmap::GetGreyPalette(256) : nullptr;
    Bitmap aGrey(aSize, ePixelFormat, pPal);
    BitmapScopedWriteAccess pGrey(aGrey);

    BitmapScopedReadAccess pRead(rBitmap);
    if (pRead && pGrey)
    {
        for (sal_Int32 nY = 0; nY < sal_Int32(aSize.Height()); ++nY)
        {
            Scanline pGreyScan = pGrey->GetScanline(nY);
            Scanline pReadScan = pRead->GetScanline(nY);

            for (sal_Int32 nX = 0; nX < sal_Int32(aSize.Width()); ++nX)
            {
                // Get the luminance from RGB color and remap the value from 0-255 to 160-224
                const BitmapColor aColor = pRead->GetPixelFromData(pReadScan, nX);
                sal_uInt8 nLum(aColor.GetLuminance() / 4 + 160);
                BitmapColor aGreyValue(ColorAlpha, nLum, nLum, nLum, aColor.GetAlpha());
                pGrey->SetPixelOnData(pGreyScan, nX, aGreyValue);
            }
        }
    }

    return aGrey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
