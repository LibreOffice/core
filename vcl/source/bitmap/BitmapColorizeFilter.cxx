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

#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/BitmapColorizeFilter.hxx>

BitmapEx BitmapColorizeFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap = rBitmapEx.GetBitmap();
    BitmapScopedWriteAccess pWriteAccess(aBitmap);

    if (!pWriteAccess)
        return rBitmapEx;

    std::vector<sal_uInt8> aMapR(256);
    std::vector<sal_uInt8> aMapG(256);
    std::vector<sal_uInt8> aMapB(256);

    const sal_uInt8 cR = maColor.GetRed();
    const sal_uInt8 cG = maColor.GetGreen();
    const sal_uInt8 cB = maColor.GetBlue();

    for (sal_Int32 nX = 0; nX < 256; ++nX)
    {
        aMapR[nX] = std::clamp((nX + cR) / 2, sal_Int32(0), sal_Int32(255));
        aMapG[nX] = std::clamp((nX + cG) / 2, sal_Int32(0), sal_Int32(255));
        aMapB[nX] = std::clamp((nX + cB) / 2, sal_Int32(0), sal_Int32(255));
    }

    BitmapColor aBitmapColor;

    if (pWriteAccess->HasPalette())
    {
        for (sal_uInt16 nPaletteIdx = 0, nCount = pWriteAccess->GetPaletteEntryCount();
             nPaletteIdx < nCount; nPaletteIdx++)
        {
            const BitmapColor& rCol = pWriteAccess->GetPaletteColor(nPaletteIdx);
            aBitmapColor.SetRed(aMapR[rCol.GetRed()]);
            aBitmapColor.SetGreen(aMapG[rCol.GetGreen()]);
            aBitmapColor.SetBlue(aMapB[rCol.GetBlue()]);
            pWriteAccess->SetPaletteColor(nPaletteIdx, aBitmapColor);
        }
    }
    else if (pWriteAccess->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr)
    {
        const sal_Int32 nW = pWriteAccess->Width();
        const sal_Int32 nH = pWriteAccess->Height();

        for (sal_Int32 nY = 0; nY < nH; ++nY)
        {
            Scanline pScan = pWriteAccess->GetScanline(nY);

            for (sal_Int32 nX = 0; nX < nW; ++nX)
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
        const sal_Int32 nW = pWriteAccess->Width();
        const sal_Int32 nH = pWriteAccess->Height();

        for (sal_Int32 nY = 0; nY < nH; ++nY)
        {
            Scanline pScanline = pWriteAccess->GetScanline(nY);
            for (sal_Int32 nX = 0; nX < nW; ++nX)
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
