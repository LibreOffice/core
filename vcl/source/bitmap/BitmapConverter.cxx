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
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapConverter.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>

#include <bitmapwriteaccess.hxx>
#include <impoctree.hxx>

BitmapEx BitmapConverter::execute(BitmapEx const& rBitmapEx)
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const sal_uInt16 nBitCount = aBitmap.GetBitCount();
    bool bRet = false;

    switch (meConversion)
    {
        case BmpConversion::N1BitThreshold:
        {
            BitmapEx aBitmapEx(aBitmap);
            bRet = BitmapFilter::Filter(aBitmapEx, BitmapMonochromeFilter(128));
            aBitmap = aBitmapEx.GetBitmap();
        }
        break;

        case BmpConversion::N4BitGreys:
            bRet = MakeGreyscales(aBitmap, 16);
            break;

        case BmpConversion::N4BitColors:
        {
            if (nBitCount < 4)
                bRet = convertUp(aBitmap, 4);
            else if (nBitCount > 4)
                bRet = convertDown(aBitmap, 4);
            else
                bRet = true;
        }
        break;

        case BmpConversion::N8BitGreys:
            bRet = MakeGreyscales(aBitmap, 256);
            break;

        case BmpConversion::N8BitColors:
        {
            if (nBitCount < 8)
                bRet = convertUp(aBitmap, 8);
            else if (nBitCount > 8)
                bRet = convertDown(aBitmap, 8);
            else
                bRet = true;
        }
        break;

        case BmpConversion::N8BitTrans:
        {
            Color aTrans(BMP_COL_TRANS);

            if (nBitCount < 8)
                bRet = convertUp(aBitmap, 8, &aTrans);
            else
                bRet = convertDown(aBitmap, 8, &aTrans);
        }
        break;

        case BmpConversion::N24Bit:
        {
            if (nBitCount < 24)
                bRet = convertUp(aBitmap, 24);
            else
                bRet = true;
        }
        break;

        case BmpConversion::Ghosted:
            bRet = convertGhosted(aBitmap);
            break;

        default:
            OSL_FAIL("Bitmap::Convert(): Unsupported conversion");
            break;
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

bool BitmapConverter::MakeGreyscales(Bitmap& rBitmap, sal_uInt16 nGreys)
{
    SAL_WARN_IF(nGreys != 16 && nGreys != 256, "vcl", "Only 16 or 256 greyscales are supported!");

    Bitmap::ScopedReadAccess pReadAcc(rBitmap);
    bool bRet = false;

    if (pReadAcc)
    {
        const BitmapPalette& rPal = Bitmap::GetGreyPalette(nGreys);
        sal_uLong nShift = ((nGreys == 16) ? 4UL : 0UL);
        bool bPalDiffers
            = !pReadAcc->HasPalette() || (rPal.GetEntryCount() != pReadAcc->GetPaletteEntryCount());

        if (!bPalDiffers)
            bPalDiffers = (rPal != pReadAcc->GetPalette());

        if (bPalDiffers)
        {
            Bitmap aNewBmp(rBitmap.GetSizePixel(), (nGreys == 16) ? 4 : 8, &rPal);
            BitmapScopedWriteAccess pWriteAcc(aNewBmp);

            if (pWriteAcc)
            {
                const long nWidth = pWriteAcc->Width();
                const long nHeight = pWriteAcc->Height();

                if (pReadAcc->HasPalette())
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            const sal_uInt8 cIndex = pReadAcc->GetIndexFromData(pScanlineRead, nX);
                            pWriteAcc->SetPixelOnData(
                                pScanline, nX,
                                BitmapColor(pReadAcc->GetPaletteColor(cIndex).GetLuminance()
                                            >> nShift));
                        }
                    }
                }
                else if (pReadAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr
                         && pWriteAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal)
                {
                    nShift += 8;

                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pReadScan = pReadAcc->GetScanline(nY);
                        Scanline pWriteScan = pWriteAcc->GetScanline(nY);

                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            const sal_uLong nB = *pReadScan++;
                            const sal_uLong nG = *pReadScan++;
                            const sal_uLong nR = *pReadScan++;

                            *pWriteScan++ = static_cast<sal_uInt8>(
                                (nB * 28UL + nG * 151UL + nR * 77UL) >> nShift);
                        }
                    }
                }
                else if (pReadAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb
                         && pWriteAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal)
                {
                    nShift += 8;

                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pReadScan = pReadAcc->GetScanline(nY);
                        Scanline pWriteScan = pWriteAcc->GetScanline(nY);

                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            const sal_uLong nR = *pReadScan++;
                            const sal_uLong nG = *pReadScan++;
                            const sal_uLong nB = *pReadScan++;

                            *pWriteScan++ = static_cast<sal_uInt8>(
                                (nB * 28UL + nG * 151UL + nR * 77UL) >> nShift);
                        }
                    }
                }
                else
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            pWriteAcc->SetPixelOnData(
                                pScanline, nX,
                                BitmapColor(
                                    pReadAcc->GetPixelFromData(pScanlineRead, nX).GetLuminance()
                                    >> nShift));
                        }
                    }
                }

                pWriteAcc.reset();
                bRet = true;
            }

            pReadAcc.reset();

            if (bRet)
            {
                const MapMode aMap(rBitmap.GetPrefMapMode());
                const Size aSize(rBitmap.GetPrefSize());

                rBitmap = aNewBmp;

                rBitmap.SetPrefMapMode(aMap);
                rBitmap.SetPrefSize(aSize);
            }
        }
        else
        {
            pReadAcc.reset();
            bRet = true;
        }
    }

    return bRet;
}

bool BitmapConverter::convertUp(Bitmap& rBitmap, sal_uInt16 nBitCount, Color const* pExtColor)
{
    SAL_WARN_IF(nBitCount <= rBitmap.GetBitCount(), "vcl", "New BitCount must be greater!");

    Bitmap::ScopedReadAccess pReadAcc(rBitmap);
    bool bRet = false;

    if (pReadAcc)
    {
        BitmapPalette aPalette;
        Bitmap aNewBmp(rBitmap.GetSizePixel(), nBitCount,
                       pReadAcc->HasPalette() ? &pReadAcc->GetPalette() : &aPalette);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const long nWidth = pWriteAcc->Width();
            const long nHeight = pWriteAcc->Height();

            if (pWriteAcc->HasPalette())
            {
                const BitmapPalette& rOldPalette = pReadAcc->GetPalette();
                const sal_uInt16 nOldCount = rOldPalette.GetEntryCount();
                assert(nOldCount <= (1 << rBitmap.GetBitCount()));

                aPalette.SetEntryCount(1 << nBitCount);

                for (sal_uInt16 i = 0; i < nOldCount; i++)
                    aPalette[i] = rOldPalette[i];

                if (pExtColor)
                    aPalette[aPalette.GetEntryCount() - 1] = *pExtColor;

                pWriteAcc->SetPalette(aPalette);

                for (long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; nX++)
                    {
                        pWriteAcc->SetPixelOnData(pScanline, nX,
                                                  pReadAcc->GetPixelFromData(pScanlineRead, nX));
                    }
                }
            }
            else
            {
                if (pReadAcc->HasPalette())
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            pWriteAcc->SetPixelOnData(
                                pScanline, nX,
                                pReadAcc->GetPaletteColor(
                                    pReadAcc->GetIndexFromData(pScanlineRead, nX)));
                        }
                    }
                }
                else
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            pWriteAcc->SetPixelOnData(
                                pScanline, nX, pReadAcc->GetPixelFromData(pScanlineRead, nX));
                        }
                    }
                }
            }
            bRet = true;
        }

        if (bRet)
        {
            const MapMode aMap(rBitmap.GetPrefMapMode());
            const Size aSize(rBitmap.GetPrefSize());

            rBitmap = aNewBmp;

            rBitmap.SetPrefMapMode(aMap);
            rBitmap.SetPrefSize(aSize);
        }
    }

    return bRet;
}

bool BitmapConverter::convertDown(Bitmap& rBitmap, sal_uInt16 nBitCount, Color const* pExtColor)
{
    SAL_WARN_IF(nBitCount > rBitmap.GetBitCount(), "vcl",
                "New BitCount must be lower ( or equal when pExtColor is set )!");

    Bitmap::ScopedReadAccess pReadAcc(rBitmap);
    bool bRet = false;

    if (pReadAcc)
    {
        BitmapPalette aPalette;
        Bitmap aNewBmp(rBitmap.GetSizePixel(), nBitCount, &aPalette);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const sal_uInt16 nCount = 1 << nBitCount;
            const long nWidth = pWriteAcc->Width();
            const long nWidth1 = nWidth - 1;
            const long nHeight = pWriteAcc->Height();
            Octree aOctree(*pReadAcc, pExtColor ? (nCount - 1) : nCount);
            aPalette = aOctree.GetPalette();
            InverseColorMap aColorMap(aPalette);
            BitmapColor aColor;
            ImpErrorQuad aErrQuad;
            std::vector<ImpErrorQuad> aErrQuad1(nWidth);
            std::vector<ImpErrorQuad> aErrQuad2(nWidth);
            ImpErrorQuad* pQLine1 = aErrQuad1.data();
            ImpErrorQuad* pQLine2 = nullptr;
            long nYTmp = 0;
            sal_uInt8 cIndex;
            bool bQ1 = true;

            if (pExtColor)
            {
                aPalette.SetEntryCount(aPalette.GetEntryCount() + 1);
                aPalette[aPalette.GetEntryCount() - 1] = *pExtColor;
            }

            // set Black/White always, if we have enough space
            if (aPalette.GetEntryCount() < (nCount - 1))
            {
                aPalette.SetEntryCount(aPalette.GetEntryCount() + 2);
                aPalette[aPalette.GetEntryCount() - 2] = COL_BLACK;
                aPalette[aPalette.GetEntryCount() - 1] = COL_WHITE;
            }

            pWriteAcc->SetPalette(aPalette);

            for (long nY = 0; nY < std::min(nHeight, 2L); nY++, nYTmp++)
            {
                pQLine2 = !nY ? aErrQuad1.data() : aErrQuad2.data();
                Scanline pScanlineRead = pReadAcc->GetScanline(nYTmp);
                for (long nX = 0; nX < nWidth; nX++)
                {
                    if (pReadAcc->HasPalette())
                        pQLine2[nX] = pReadAcc->GetPaletteColor(
                            pReadAcc->GetIndexFromData(pScanlineRead, nX));
                    else
                        pQLine2[nX] = pReadAcc->GetPixelFromData(pScanlineRead, nX);
                }
            }

            assert(pQLine2 || nHeight == 0);

            for (long nY = 0; nY < nHeight; nY++, nYTmp++)
            {
                // first pixel in the line
                cIndex = static_cast<sal_uInt8>(
                    aColorMap.GetBestPaletteIndex(pQLine1[0].ImplGetColor()));
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                pWriteAcc->SetPixelOnData(pScanline, 0, BitmapColor(cIndex));

                long nX;
                for (nX = 1; nX < nWidth1; nX++)
                {
                    aColor = pQLine1[nX].ImplGetColor();
                    cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(aColor));
                    aErrQuad = (ImpErrorQuad(aColor) -= pWriteAcc->GetPaletteColor(cIndex));
                    pQLine1[++nX].ImplAddColorError7(aErrQuad);
                    pQLine2[nX--].ImplAddColorError1(aErrQuad);
                    pQLine2[nX--].ImplAddColorError5(aErrQuad);
                    pQLine2[nX++].ImplAddColorError3(aErrQuad);
                    pWriteAcc->SetPixelOnData(pScanline, nX, BitmapColor(cIndex));
                }

                // Last RowPixel
                if (nX < nWidth)
                {
                    cIndex = static_cast<sal_uInt8>(
                        aColorMap.GetBestPaletteIndex(pQLine1[nWidth1].ImplGetColor()));
                    pWriteAcc->SetPixelOnData(pScanline, nX, BitmapColor(cIndex));
                }

                // Refill/copy row buffer
                pQLine1 = pQLine2;
                bQ1 = !bQ1;
                pQLine2 = bQ1 ? aErrQuad2.data() : aErrQuad1.data();

                if (nYTmp < nHeight)
                {
                    Scanline pScanlineRead = pReadAcc->GetScanline(nYTmp);
                    for (nX = 0; nX < nWidth; nX++)
                    {
                        if (pReadAcc->HasPalette())
                            pQLine2[nX] = pReadAcc->GetPaletteColor(
                                pReadAcc->GetIndexFromData(pScanlineRead, nX));
                        else
                            pQLine2[nX] = pReadAcc->GetPixelFromData(pScanlineRead, nX);
                    }
                }
            }

            bRet = true;
        }

        if (bRet)
        {
            const MapMode aMap(rBitmap.GetPrefMapMode());
            const Size aSize(rBitmap.GetPrefSize());

            rBitmap = aNewBmp;

            rBitmap.SetPrefMapMode(aMap);
            rBitmap.SetPrefSize(aSize);
        }
    }

    return bRet;
}

bool BitmapConverter::convertGhosted(Bitmap& rBitmap)
{
    Bitmap aNewBmp;
    Bitmap::ScopedReadAccess pR(rBitmap);
    bool bRet = false;

    if (pR)
    {
        if (pR->HasPalette())
        {
            BitmapPalette aNewPal(pR->GetPaletteEntryCount());

            for (long i = 0, nCount = aNewPal.GetEntryCount(); i < nCount; i++)
            {
                const BitmapColor& rOld = pR->GetPaletteColor(static_cast<sal_uInt16>(i));
                aNewPal[static_cast<sal_uInt16>(i)]
                    = BitmapColor((rOld.GetRed() >> 1) | 0x80, (rOld.GetGreen() >> 1) | 0x80,
                                  (rOld.GetBlue() >> 1) | 0x80);
            }

            aNewBmp = Bitmap(rBitmap.GetSizePixel(), rBitmap.GetBitCount(), &aNewPal);
            BitmapScopedWriteAccess pW(aNewBmp);

            if (pW)
            {
                pW->CopyBuffer(*pR);
                bRet = true;
            }
        }
        else
        {
            aNewBmp = Bitmap(rBitmap.GetSizePixel(), 24);

            BitmapScopedWriteAccess pW(aNewBmp);

            if (pW)
            {
                const long nWidth = pR->Width(), nHeight = pR->Height();

                for (long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pW->GetScanline(nY);
                    Scanline pScanlineRead = pR->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; nX++)
                    {
                        const BitmapColor aOld(pR->GetPixelFromData(pScanlineRead, nX));
                        pW->SetPixelOnData(pScanline, nX,
                                           BitmapColor((aOld.GetRed() >> 1) | 0x80,
                                                       (aOld.GetGreen() >> 1) | 0x80,
                                                       (aOld.GetBlue() >> 1) | 0x80));
                    }
                }

                bRet = true;
            }
        }

        pR.reset();
    }

    if (bRet)
    {
        const MapMode aMap(rBitmap.GetPrefMapMode());
        const Size aSize(rBitmap.GetPrefSize());

        rBitmap = aNewBmp;

        rBitmap.SetPrefMapMode(aMap);
        rBitmap.SetPrefSize(aSize);
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
