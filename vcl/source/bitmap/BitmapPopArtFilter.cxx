/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/solar.h>

#include <vcl/bitmap/BitmapPopArtFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

Bitmap BitmapPopArtFilter::execute(Bitmap const& rBitmap) const
{
    Bitmap aBitmap(rBitmap);

    bool bConvert = isPalettePixelFormat(aBitmap.getPixelFormat())
                    || aBitmap.Convert(BmpConversion::N8BitColors);

    if (!bConvert)
        return Bitmap();

    BitmapScopedWriteAccess pWriteAcc(aBitmap);

    if (!pWriteAcc)
        return Bitmap();

    const sal_Int32 nWidth = pWriteAcc->Width();
    const sal_Int32 nHeight = pWriteAcc->Height();
    const sal_uInt16 nEntryCount = 1 << pWriteAcc->GetBitCount();
    sal_uInt16 n = 0;
    std::vector<PopArtEntry> aPopArtTable(nEntryCount);

    for (n = 0; n < nEntryCount; n++)
    {
        PopArtEntry& rEntry = aPopArtTable[n];
        rEntry.mnIndex = n;
        rEntry.mnCount = 0;
    }

    // get pixel count for each palette entry
    for (sal_Int32 nY = 0; nY < nHeight; nY++)
    {
        Scanline pScanline = pWriteAcc->GetScanline(nY);
        for (sal_Int32 nX = 0; nX < nWidth; nX++)
        {
            aPopArtTable[pWriteAcc->GetIndexFromData(pScanline, nX)].mnCount++;
            assert(aPopArtTable[pWriteAcc->GetIndexFromData(pScanline, nX)].mnCount != 0);
        }
    }

    // sort table
    std::sort(
        aPopArtTable.begin(), aPopArtTable.end(),
        [](const PopArtEntry& lhs, const PopArtEntry& rhs) { return lhs.mnCount > rhs.mnCount; });

    // get last used entry
    sal_uInt16 nFirstEntry;
    sal_uInt16 nLastEntry = 0;

    for (n = 0; n < nEntryCount; n++)
    {
        if (aPopArtTable[n].mnCount)
            nLastEntry = n;
    }

    // rotate palette (one entry)
    const BitmapColor aFirstCol(pWriteAcc->GetPaletteColor(aPopArtTable[0].mnIndex));

    for (nFirstEntry = 0; nFirstEntry < nLastEntry; nFirstEntry++)
    {
        pWriteAcc->SetPaletteColor(
            aPopArtTable[nFirstEntry].mnIndex,
            pWriteAcc->GetPaletteColor(aPopArtTable[nFirstEntry + 1].mnIndex));
    }

    pWriteAcc->SetPaletteColor(aPopArtTable[nLastEntry].mnIndex, aFirstCol);

    // cleanup
    pWriteAcc.reset();

    return aBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
