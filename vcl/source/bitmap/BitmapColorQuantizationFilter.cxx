/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <algorithm>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapColorQuantizationFilter.hxx>

#include <bitmapwriteaccess.hxx>

#include <cstdlib>

BitmapEx BitmapColorQuantizationFilter::execute(BitmapEx const& aBitmapEx) const
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();

    bool bRet = false;

    if (aBitmap.GetColorCount() <= sal_Int64(mnNewColorCount))
    {
        bRet = true;
    }
    else
    {
        Bitmap::ScopedReadAccess pRAcc(aBitmap);
        sal_uInt16 nBitCount;

        auto const cappedNewColorCount = std::min(mnNewColorCount, sal_uInt16(256));

        if (cappedNewColorCount < 17)
            nBitCount = 4;
        else
            nBitCount = 8;

        if (pRAcc)
        {
            const sal_uInt32 nValidBits = 4;
            const sal_uInt32 nRightShiftBits = 8 - nValidBits;
            const sal_uInt32 nLeftShiftBits1 = nValidBits;
            const sal_uInt32 nLeftShiftBits2 = nValidBits << 1;
            const sal_uInt32 nColorsPerComponent = 1 << nValidBits;
            const sal_uInt32 nColorOffset = 256 / nColorsPerComponent;
            const sal_uInt32 nTotalColors
                = nColorsPerComponent * nColorsPerComponent * nColorsPerComponent;
            const tools::Long nWidth = pRAcc->Width();
            const tools::Long nHeight = pRAcc->Height();
            std::unique_ptr<PopularColorCount[]> pCountTable(new PopularColorCount[nTotalColors]);

            memset(pCountTable.get(), 0, nTotalColors * sizeof(PopularColorCount));

            for (tools::Long nR = 0, nIndex = 0; nR < 256; nR += nColorOffset)
            {
                for (tools::Long nG = 0; nG < 256; nG += nColorOffset)
                {
                    for (tools::Long nB = 0; nB < 256; nB += nColorOffset)
                    {
                        pCountTable[nIndex].mnIndex = nIndex;
                        nIndex++;
                    }
                }
            }

            if (pRAcc->HasPalette())
            {
                for (tools::Long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanlineRead = pRAcc->GetScanline(nY);
                    for (tools::Long nX = 0; nX < nWidth; nX++)
                    {
                        const BitmapColor& rCol
                            = pRAcc->GetPaletteColor(pRAcc->GetIndexFromData(pScanlineRead, nX));
                        pCountTable[((static_cast<sal_uInt32>(rCol.GetRed()) >> nRightShiftBits)
                                     << nLeftShiftBits2)
                                    | ((static_cast<sal_uInt32>(rCol.GetGreen()) >> nRightShiftBits)
                                       << nLeftShiftBits1)
                                    | (static_cast<sal_uInt32>(rCol.GetBlue()) >> nRightShiftBits)]
                            .mnCount++;
                    }
                }
            }
            else
            {
                for (tools::Long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanlineRead = pRAcc->GetScanline(nY);
                    for (tools::Long nX = 0; nX < nWidth; nX++)
                    {
                        const BitmapColor aCol(pRAcc->GetPixelFromData(pScanlineRead, nX));
                        pCountTable[((static_cast<sal_uInt32>(aCol.GetRed()) >> nRightShiftBits)
                                     << nLeftShiftBits2)
                                    | ((static_cast<sal_uInt32>(aCol.GetGreen()) >> nRightShiftBits)
                                       << nLeftShiftBits1)
                                    | (static_cast<sal_uInt32>(aCol.GetBlue()) >> nRightShiftBits)]
                            .mnCount++;
                    }
                }
            }

            BitmapPalette aNewPal(cappedNewColorCount);

            std::qsort(pCountTable.get(), nTotalColors, sizeof(PopularColorCount),
                       [](const void* p1, const void* p2) {
                           int nRet;

                           if (static_cast<PopularColorCount const*>(p1)->mnCount
                               < static_cast<PopularColorCount const*>(p2)->mnCount)
                               nRet = 1;
                           else if (static_cast<PopularColorCount const*>(p1)->mnCount
                                    == static_cast<PopularColorCount const*>(p2)->mnCount)
                               nRet = 0;
                           else
                               nRet = -1;

                           return nRet;
                       });

            for (sal_uInt16 n = 0; n < cappedNewColorCount; n++)
            {
                const PopularColorCount& rPop = pCountTable[n];
                aNewPal[n] = BitmapColor(
                    static_cast<sal_uInt8>((rPop.mnIndex >> nLeftShiftBits2) << nRightShiftBits),
                    static_cast<sal_uInt8>(
                        ((rPop.mnIndex >> nLeftShiftBits1) & (nColorsPerComponent - 1))
                        << nRightShiftBits),
                    static_cast<sal_uInt8>((rPop.mnIndex & (nColorsPerComponent - 1))
                                           << nRightShiftBits));
            }

            Bitmap aNewBmp(aBitmap.GetSizePixel(), nBitCount, &aNewPal);
            BitmapScopedWriteAccess pWAcc(aNewBmp);

            if (pWAcc)
            {
                BitmapColor aDstCol(sal_uInt8(0));
                std::unique_ptr<sal_uInt8[]> pIndexMap(new sal_uInt8[nTotalColors]);

                for (tools::Long nR = 0, nIndex = 0; nR < 256; nR += nColorOffset)
                {
                    for (tools::Long nG = 0; nG < 256; nG += nColorOffset)
                    {
                        for (tools::Long nB = 0; nB < 256; nB += nColorOffset)
                        {
                            pIndexMap[nIndex++] = static_cast<sal_uInt8>(aNewPal.GetBestIndex(
                                BitmapColor(static_cast<sal_uInt8>(nR), static_cast<sal_uInt8>(nG),
                                            static_cast<sal_uInt8>(nB))));
                        }
                    }
                }

                if (pRAcc->HasPalette())
                {
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWAcc->GetScanline(nY);
                        Scanline pScanlineRead = pRAcc->GetScanline(nY);
                        for (tools::Long nX = 0; nX < nWidth; nX++)
                        {
                            const BitmapColor& rCol = pRAcc->GetPaletteColor(
                                pRAcc->GetIndexFromData(pScanlineRead, nX));
                            aDstCol.SetIndex(pIndexMap[((static_cast<sal_uInt32>(rCol.GetRed())
                                                         >> nRightShiftBits)
                                                        << nLeftShiftBits2)
                                                       | ((static_cast<sal_uInt32>(rCol.GetGreen())
                                                           >> nRightShiftBits)
                                                          << nLeftShiftBits1)
                                                       | (static_cast<sal_uInt32>(rCol.GetBlue())
                                                          >> nRightShiftBits)]);
                            pWAcc->SetPixelOnData(pScanline, nX, aDstCol);
                        }
                    }
                }
                else
                {
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWAcc->GetScanline(nY);
                        Scanline pScanlineRead = pRAcc->GetScanline(nY);

                        for (tools::Long nX = 0; nX < nWidth; nX++)
                        {
                            const BitmapColor aCol(pRAcc->GetPixelFromData(pScanlineRead, nX));
                            aDstCol.SetIndex(pIndexMap[((static_cast<sal_uInt32>(aCol.GetRed())
                                                         >> nRightShiftBits)
                                                        << nLeftShiftBits2)
                                                       | ((static_cast<sal_uInt32>(aCol.GetGreen())
                                                           >> nRightShiftBits)
                                                          << nLeftShiftBits1)
                                                       | (static_cast<sal_uInt32>(aCol.GetBlue())
                                                          >> nRightShiftBits)]);
                            pWAcc->SetPixelOnData(pScanline, nX, aDstCol);
                        }
                    }
                }

                pWAcc.reset();
                bRet = true;
            }

            pCountTable.reset();
            pRAcc.reset();

            if (bRet)
            {
                const MapMode aMap(aBitmap.GetPrefMapMode());
                const Size aSize(aBitmap.GetPrefSize());

                aBitmap = aNewBmp;

                aBitmap.SetPrefMapMode(aMap);
                aBitmap.SetPrefSize(aSize);
            }
        }
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
