/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapDuoToneFilter.hxx>

#include <bitmapwriteaccess.hxx>

static sal_uInt8 lcl_getDuotoneColorComponent(sal_uInt8 base, sal_uInt16 color1, sal_uInt16 color2)
{
    color2 = color2 * base / 0xFF;
    color1 = color1 * (0xFF - base) / 0xFF;

    return static_cast<sal_uInt8>(color1 + color2);
}

BitmapEx BitmapDuoToneFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const tools::Long nWidth = aBitmap.GetSizePixel().Width();
    const tools::Long nHeight = aBitmap.GetSizePixel().Height();

    Bitmap aResultBitmap(aBitmap.GetSizePixel(), 24);
    Bitmap::ScopedReadAccess pReadAcc(aBitmap);
    BitmapScopedWriteAccess pWriteAcc(aResultBitmap);
    const BitmapColor aColorOne(static_cast<sal_uInt8>(mnColorOne >> 16),
                                static_cast<sal_uInt8>(mnColorOne >> 8),
                                static_cast<sal_uInt8>(mnColorOne));
    const BitmapColor aColorTwo(static_cast<sal_uInt8>(mnColorTwo >> 16),
                                static_cast<sal_uInt8>(mnColorTwo >> 8),
                                static_cast<sal_uInt8>(mnColorTwo));

    for (tools::Long x = 0; x < nWidth; x++)
    {
        for (tools::Long y = 0; y < nHeight; y++)
        {
            BitmapColor aColor = pReadAcc->GetColor(y, x);
            sal_uInt8 nLuminance = aColor.GetLuminance();
            BitmapColor aResultColor(
                lcl_getDuotoneColorComponent(nLuminance, aColorOne.GetRed(), aColorTwo.GetRed()),
                lcl_getDuotoneColorComponent(nLuminance, aColorOne.GetGreen(),
                                             aColorTwo.GetGreen()),
                lcl_getDuotoneColorComponent(nLuminance, aColorOne.GetBlue(), aColorTwo.GetBlue()));
            pWriteAcc->SetPixel(y, x, aResultColor);
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();
    aBitmap.ReassignWithSize(aResultBitmap);

    return BitmapEx(aBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
