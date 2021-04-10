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
#include <vcl/BitmapSolarizeFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

BitmapEx BitmapSolarizeFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());
    bool bRet = false;
    BitmapScopedWriteAccess pWriteAcc(aBitmap);

    if (pWriteAcc)
    {
        if (pWriteAcc->HasPalette())
        {
            const BitmapPalette& rPal = pWriteAcc->GetPalette();

            for (sal_uInt16 i = 0, nCount = rPal.GetEntryCount(); i < nCount; i++)
            {
                if (rPal[i].GetLuminance() >= mcSolarGreyThreshold)
                {
                    BitmapColor aCol(rPal[i]);
                    aCol.Invert();
                    pWriteAcc->SetPaletteColor(i, aCol);
                }
            }
        }
        else
        {
            BitmapColor aCol;
            const tools::Long nWidth = pWriteAcc->Width();
            const tools::Long nHeight = pWriteAcc->Height();

            for (tools::Long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                for (tools::Long nX = 0; nX < nWidth; nX++)
                {
                    aCol = pWriteAcc->GetPixelFromData(pScanline, nX);

                    if (aCol.GetLuminance() >= mcSolarGreyThreshold)
                    {
                        aCol.Invert();
                        pWriteAcc->SetPixelOnData(pScanline, nX, aCol);
                    }
                }
            }
        }

        pWriteAcc.reset();
        bRet = true;
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
