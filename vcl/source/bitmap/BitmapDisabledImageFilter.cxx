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

#include <vcl/bitmapaccess.hxx>

#include <bitmapwriteaccess.hxx>
#include <BitmapDisabledImageFilter.hxx>

BitmapEx BitmapDisabledImageFilter::execute(BitmapEx const& rBitmapEx) const
{
    const Size aSize(rBitmapEx.GetSizePixel());

    // keep disable image at same depth as original where possible, otherwise
    // use 8 bit
    sal_uInt16 nBitCount = rBitmapEx.GetBitCount();
    if (nBitCount < 8)
        nBitCount = 8;

    const BitmapPalette* pPal = nBitCount == 8 ? &Bitmap::GetGreyPalette(256) : nullptr;
    Bitmap aGrey(aSize, nBitCount, pPal);
    BitmapScopedWriteAccess pGrey(aGrey);

    BitmapEx aReturnBitmap;
    Bitmap aReadBitmap(rBitmapEx.GetBitmap());
    Bitmap::ScopedReadAccess pRead(aReadBitmap);
    if (pRead && pGrey)
    {
        for (long nY = 0; nY < aSize.Height(); ++nY)
        {
            Scanline pGreyScan = pGrey->GetScanline(nY);
            Scanline pReadScan = pRead->GetScanline(nY);

            for (long nX = 0; nX < aSize.Width(); ++nX)
            {
                // Get the luminance from RGB color and remap the value from 0-255 to 160-224
                const BitmapColor aColor = pRead->GetPixelFromData(pReadScan, nX);
                sal_uInt8 nLum(aColor.GetLuminance() / 4 + 160);
                BitmapColor aGreyValue(nLum, nLum, nLum);
                pGrey->SetPixelOnData(pGreyScan, nX, aGreyValue);
            }
        }
    }

    if (rBitmapEx.IsTransparent())
    {
        aReturnBitmap = BitmapEx(aGrey, rBitmapEx.GetAlpha());
    }
    else
        aReturnBitmap = BitmapEx(aGrey);

    return aReturnBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
