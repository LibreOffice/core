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
#include <vcl/BitmapPopArtFilter.hxx>

#include <vcl/bitmapwriteaccess.hxx>

#include <cstdlib>

BitmapEx BitmapPopArtFilter::execute(BitmapEx const& rBitmapEx)
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    bool bRet = (aBitmap.GetBitCount() <= 8) || aBitmap.Convert(BmpConversion::N8BitColors);

    if (bRet)
    {
        bRet = false;

        BitmapScopedWriteAccess pWriteAcc(aBitmap);

        if (pWriteAcc)
        {
            const long nWidth = pWriteAcc->Width();
            const long nHeight = pWriteAcc->Height();
            const int nEntryCount = 1 << pWriteAcc->GetBitCount();
            int n = 0;
            PopArtEntry* pPopArtTable = new PopArtEntry[nEntryCount];

            for (n = 0; n < nEntryCount; n++)
            {
                PopArtEntry& rEntry = pPopArtTable[n];
                rEntry.mnIndex = static_cast<sal_uInt16>(n);
                rEntry.mnCount = 0;
            }

            // get pixel count for each palette entry
            for (long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                for (long nX = 0; nX < nWidth; nX++)
                {
                    pPopArtTable[pWriteAcc->GetIndexFromData(pScanline, nX)].mnCount++;
                }
            }

            // sort table
            std::qsort(pPopArtTable, nEntryCount, sizeof(PopArtEntry),
                       [](const void* p1, const void* p2) {
                           int nRet;

                           if (static_cast<PopArtEntry const*>(p1)->mnCount
                               < static_cast<PopArtEntry const*>(p2)->mnCount)
                           {
                               nRet = 1;
                           }
                           else if (static_cast<PopArtEntry const*>(p1)->mnCount
                                    == static_cast<PopArtEntry const*>(p2)->mnCount)
                           {
                               nRet = 0;
                           }
                           else
                           {
                               nRet = -1;
                           }

                           return nRet;
                       });

            // get last used entry
            sal_uLong nFirstEntry;
            sal_uLong nLastEntry = 0;

            for (n = 0; n < nEntryCount; n++)
            {
                if (pPopArtTable[n].mnCount)
                    nLastEntry = n;
            }

            // rotate palette (one entry)
            const BitmapColor aFirstCol(pWriteAcc->GetPaletteColor(
                sal::static_int_cast<sal_uInt16>(pPopArtTable[0].mnIndex)));

            for (nFirstEntry = 0; nFirstEntry < nLastEntry; nFirstEntry++)
            {
                pWriteAcc->SetPaletteColor(
                    sal::static_int_cast<sal_uInt16>(pPopArtTable[nFirstEntry].mnIndex),
                    pWriteAcc->GetPaletteColor(
                        sal::static_int_cast<sal_uInt16>(pPopArtTable[nFirstEntry + 1].mnIndex)));
            }

            pWriteAcc->SetPaletteColor(
                sal::static_int_cast<sal_uInt16>(pPopArtTable[nLastEntry].mnIndex), aFirstCol);

            // cleanup
            delete[] pPopArtTable;
            pWriteAcc.reset();
            bRet = true;
        }
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
