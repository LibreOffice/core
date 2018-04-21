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
#include <vcl/bitmapaccess.hxx>
#include <vcl/alpha.hxx>

#include <BitmapBlendFilter.hxx>
#include <bitmapwriteaccess.hxx>

BitmapEx BitmapBlendFilter::execute(BitmapEx const& aBitmapEx)
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();

    // Convert to a truecolor bitmap, if we're a paletted one. There's room
    // for tradeoff decision here, maybe later for an overload (or a flag)
    if (aBitmap.GetBitCount() <= 8)
        aBitmap.Convert(BmpConversion::N24Bit);

    AlphaMask::ScopedReadAccess pAlphaAcc(const_cast<AlphaMask&>(mrAlphaMask));

    BitmapScopedWriteAccess pAcc(aBitmap);
    bool bRet = false;

    if (pAlphaAcc && pAcc)
    {
        const long nWidth = std::min(pAlphaAcc->Width(), pAcc->Width());
        const long nHeight = std::min(pAlphaAcc->Height(), pAcc->Height());

        for (long nY = 0; nY < nHeight; ++nY)
        {
            Scanline pScanline = pAcc->GetScanline(nY);
            Scanline pScanlineAlpha = pAlphaAcc->GetScanline(nY);
            for (long nX = 0; nX < nWidth; ++nX)
            {
                pAcc->SetPixelOnData(
                    pScanline, nX,
                    pAcc->GetPixelFromData(pScanline, nX)
                        .Merge(mrBackgroundColor,
                               255 - pAlphaAcc->GetIndexFromData(pScanlineAlpha, nX)));
            }
        }

        bRet = true;
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
