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
#include <vcl/BitmapPopArtFilter.hxx>

#include <bitmapwriteaccess.hxx>

BitmapEx BitmapPopArtFilter::execute(BitmapEx const& rBitmapEx) const
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
            std::vector<PopArtEntry> aPopArtTable(nEntryCount);

            for (n = 0; n < nEntryCount; n++)
            {
                PopArtEntry& rEntry = aPopArtTable[n];
                rEntry.mnIndex = static_cast<sal_uInt16>(n);
                rEntry.mnCount = 0;
            }

            // get pixel count for each palette entry
            for (long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                for (long nX = 0; nX < nWidth; nX++)
                {
                    aPopArtTable[pWriteAcc->GetIndexFromData(pScanline, nX)].mnCount++;
                }
            }

            // sort table
            std::sort(aPopArtTable.begin(), aPopArtTable.end(),
                      [](const PopArtEntry& lhs, const PopArtEntry& rhs) {
                          int nRet;

                          if (lhs.mnCount < rhs.mnCount)
                          {
                              nRet = 1;
                          }
                          else if (lhs.mnCount == rhs.mnCount)
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
                if (aPopArtTable[n].mnCount)
                    nLastEntry = n;
            }

            // rotate palette (one entry)
            const BitmapColor aFirstCol(pWriteAcc->GetPaletteColor(
                sal::static_int_cast<sal_uInt16>(aPopArtTable[0].mnIndex)));

            for (nFirstEntry = 0; nFirstEntry < nLastEntry; nFirstEntry++)
            {
                pWriteAcc->SetPaletteColor(
                    sal::static_int_cast<sal_uInt16>(aPopArtTable[nFirstEntry].mnIndex),
                    pWriteAcc->GetPaletteColor(
                        sal::static_int_cast<sal_uInt16>(aPopArtTable[nFirstEntry + 1].mnIndex)));
            }

            pWriteAcc->SetPaletteColor(
                sal::static_int_cast<sal_uInt16>(aPopArtTable[nLastEntry].mnIndex), aFirstCol);

            // cleanup
            pWriteAcc.reset();
            bRet = true;
        }
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
