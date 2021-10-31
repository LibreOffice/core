/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmapex.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapShadowFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

BitmapEx BitmapShadowFilter::execute(BitmapEx const& rBitmapEx) const
{
    BitmapEx aBitmapEx(rBitmapEx);
    BitmapScopedWriteAccess pWriteAccess(const_cast<Bitmap&>(aBitmapEx.GetBitmap()));

    if (!pWriteAccess)
        return rBitmapEx;

    for (sal_Int32 y(0); y < sal_Int32(pWriteAccess->Height()); y++)
    {
        Scanline pScanline = pWriteAccess->GetScanline(y);

        for (sal_Int32 x(0); x < sal_Int32(pWriteAccess->Width()); x++)
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

    return aBitmapEx;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
