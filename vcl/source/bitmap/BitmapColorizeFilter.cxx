/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/color.hxx>
#include <tools/helpers.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <bitmap/BitmapColorizeFilter.hxx>

BitmapEx BitmapColorizeFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap = rBitmapEx.GetBitmap();
    BitmapScopedWriteAccess pWriteAccess(aBitmap);

    if (!pWriteAccess)
        return rBitmapEx;

    BitmapColor aBitmapColor;
    const tools::Long nW = pWriteAccess->Width();
    const tools::Long nH = pWriteAccess->Height();
    std::vector<sal_uInt8> aMapR(256);
    std::vector<sal_uInt8> aMapG(256);
    std::vector<sal_uInt8> aMapB(256);
    tools::Long nX;
    tools::Long nY;

    const sal_uInt8 cR = maColor.GetRed();
    const sal_uInt8 cG = maColor.GetGreen();
    const sal_uInt8 cB = maColor.GetBlue();

    for (nX = 0; nX < 256; ++nX)
    {
        aMapR[nX] = MinMax((nX + cR) / 2, 0, 255);
        aMapG[nX] = MinMax((nX + cG) / 2, 0, 255);
        aMapB[nX] = MinMax((nX + cB) / 2, 0, 255);
    }

    if (pWriteAccess->HasPalette())
    {
        for (sal_uInt16 i = 0, nCount = pWriteAccess->GetPaletteEntryCount(); i < nCount; i++)
        {
            const BitmapColor& rCol = pWriteAccess->GetPaletteColor(i);
            aBitmapColor.SetRed(aMapR[rCol.GetRed()]);
            aBitmapColor.SetGreen(aMapG[rCol.GetGreen()]);
            aBitmapColor.SetBlue(aMapB[rCol.GetBlue()]);
            pWriteAccess->SetPaletteColor(i, aBitmapColor);
        }
    }
    else if (pWriteAccess->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr)
    {
        for (nY = 0; nY < nH; ++nY)
        {
            Scanline pScan = pWriteAccess->GetScanline(nY);

            for (nX = 0; nX < nW; ++nX)
            {
                *pScan = aMapB[*pScan];
                pScan++;
                *pScan = aMapG[*pScan];
                pScan++;
                *pScan = aMapR[*pScan];
                pScan++;
            }
        }
    }
    else
    {
        for (nY = 0; nY < nH; ++nY)
        {
            Scanline pScanline = pWriteAccess->GetScanline(nY);
            for (nX = 0; nX < nW; ++nX)
            {
                aBitmapColor = pWriteAccess->GetPixelFromData(pScanline, nX);
                aBitmapColor.SetRed(aMapR[aBitmapColor.GetRed()]);
                aBitmapColor.SetGreen(aMapG[aBitmapColor.GetGreen()]);
                aBitmapColor.SetBlue(aMapB[aBitmapColor.GetBlue()]);
                pWriteAccess->SetPixelOnData(pScanline, nX, aBitmapColor);
            }
        }
    }

    return rBitmapEx;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
