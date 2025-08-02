/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmap/BitmapShadowFilter.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapWriteAccess.hxx>

Bitmap BitmapShadowFilter::execute(Bitmap const& rBitmap) const
{
    Bitmap aBitmap(rBitmap);
    BitmapScopedWriteAccess pWriteAccess(aBitmap);

    if (!pWriteAccess)
        return Bitmap();

    for (sal_Int32 y(0), nHeight(pWriteAccess->Height()); y < nHeight; y++)
    {
        Scanline pScanline = pWriteAccess->GetScanline(y);

        for (sal_Int32 x(0), nWidth(pWriteAccess->Width()); x < nWidth; x++)
        {
            const BitmapColor aColor = pWriteAccess->GetColor(y, x);
            sal_uInt16 nLuminance(static_cast<sal_uInt16>(aColor.GetLuminance()) + 1);
            const BitmapColor aDestColor(
                static_cast<sal_uInt8>(
                    (nLuminance * static_cast<sal_uInt16>(maShadowColor.GetRed())) >> 8),
                static_cast<sal_uInt8>(
                    (nLuminance * static_cast<sal_uInt16>(maShadowColor.GetGreen())) >> 8),
                static_cast<sal_uInt8>(
                    (nLuminance * static_cast<sal_uInt16>(maShadowColor.GetBlue())) >> 8));

            pWriteAccess->SetPixelOnData(pScanline, x, aDestColor);
        }
    }

    return aBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
