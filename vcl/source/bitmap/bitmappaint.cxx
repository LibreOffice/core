/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/poly.hxx>
#include <tools/helpers.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/alpha.hxx>

#include <bitmapwriteaccess.hxx>
#include <salbmp.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

#include <algorithm>
#include <memory>

bool Bitmap::Erase(const Color& rFillColor)
{
    if (IsEmpty())
        return true;

    if (mxSalBmp)
    {
        // implementation specific replace
        std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
        if (xImpBmp->Create(*mxSalBmp) && xImpBmp->Erase(rFillColor))
        {
            ImplSetSalBitmap(xImpBmp);
            maPrefMapMode = MapMode(MapUnit::MapPixel);
            maPrefSize = xImpBmp->GetSize();
            return true;
        }
    }

    BitmapScopedWriteAccess pWriteAcc(*this);
    bool bRet = false;

    if (pWriteAcc)
    {
        pWriteAcc->Erase(rFillColor);
        bRet = true;
    }

    return bRet;
}

bool Bitmap::Invert()
{
    BitmapScopedWriteAccess pAcc(*this);
    bool bRet = false;

    if (pAcc)
    {
        if (pAcc->HasPalette())
        {
            BitmapPalette aBmpPal(pAcc->GetPalette());
            const sal_uInt16 nCount = aBmpPal.GetEntryCount();

            for (sal_uInt16 i = 0; i < nCount; i++)
            {
                aBmpPal[i].Invert();
            }

            pAcc->SetPalette(aBmpPal);
        }
        else
        {
            const long nWidth = pAcc->Width();
            const long nHeight = pAcc->Height();

            for (long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pAcc->GetScanline(nY);
                for (long nX = 0; nX < nWidth; nX++)
                {
                    BitmapColor aBmpColor = pAcc->GetPixelFromData(pScanline, nX);
                    aBmpColor.Invert();
                    pAcc->SetPixelOnData(pScanline, nX, aBmpColor);
                }
            }
        }

        mxSalBmp->InvalidateChecksum();
        pAcc.reset();
        bRet = true;
    }

    return bRet;
}

bool Bitmap::Mirror(BmpMirrorFlags nMirrorFlags)
{
    bool bHorz(nMirrorFlags & BmpMirrorFlags::Horizontal);
    bool bVert(nMirrorFlags & BmpMirrorFlags::Vertical);
    bool bRet = false;

    if (bHorz && !bVert)
    {
        BitmapScopedWriteAccess pAcc(*this);

        if (pAcc)
        {
            const long nWidth = pAcc->Width();
            const long nHeight = pAcc->Height();
            const long nWidth1 = nWidth - 1;
            const long nWidth_2 = nWidth >> 1;

            for (long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pAcc->GetScanline(nY);
                for (long nX = 0, nOther = nWidth1; nX < nWidth_2; nX++, nOther--)
                {
                    const BitmapColor aTemp(pAcc->GetPixelFromData(pScanline, nX));

                    pAcc->SetPixelOnData(pScanline, nX, pAcc->GetPixelFromData(pScanline, nOther));
                    pAcc->SetPixelOnData(pScanline, nOther, aTemp);
                }
            }

            pAcc.reset();
            bRet = true;
        }
    }
    else if (bVert && !bHorz)
    {
        BitmapScopedWriteAccess pAcc(*this);

        if (pAcc)
        {
            const long nScanSize = pAcc->GetScanlineSize();
            std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nScanSize]);
            const long nHeight = pAcc->Height();
            const long nHeight1 = nHeight - 1;
            const long nHeight_2 = nHeight >> 1;

            for (long nY = 0, nOther = nHeight1; nY < nHeight_2; nY++, nOther--)
            {
                memcpy(pBuffer.get(), pAcc->GetScanline(nY), nScanSize);
                memcpy(pAcc->GetScanline(nY), pAcc->GetScanline(nOther), nScanSize);
                memcpy(pAcc->GetScanline(nOther), pBuffer.get(), nScanSize);
            }

            pAcc.reset();
            bRet = true;
        }
    }
    else if (bHorz && bVert)
    {
        BitmapScopedWriteAccess pAcc(*this);

        if (pAcc)
        {
            const long nWidth = pAcc->Width();
            const long nWidth1 = nWidth - 1;
            const long nHeight = pAcc->Height();
            long nHeight_2 = nHeight >> 1;

            for (long nY = 0, nOtherY = nHeight - 1; nY < nHeight_2; nY++, nOtherY--)
            {
                Scanline pScanline = pAcc->GetScanline(nY);
                Scanline pScanlineOther = pAcc->GetScanline(nOtherY);
                for (long nX = 0, nOtherX = nWidth1; nX < nWidth; nX++, nOtherX--)
                {
                    const BitmapColor aTemp(pAcc->GetPixelFromData(pScanline, nX));

                    pAcc->SetPixelOnData(pScanline, nX,
                                         pAcc->GetPixelFromData(pScanlineOther, nOtherX));
                    pAcc->SetPixelOnData(pScanlineOther, nOtherX, aTemp);
                }
            }

            // if necessary, also mirror the middle line horizontally
            if (nHeight & 1)
            {
                Scanline pScanline = pAcc->GetScanline(nHeight_2);
                for (long nX = 0, nOtherX = nWidth1, nWidth_2 = nWidth >> 1; nX < nWidth_2;
                     nX++, nOtherX--)
                {
                    const BitmapColor aTemp(pAcc->GetPixelFromData(pScanline, nX));
                    pAcc->SetPixelOnData(pScanline, nX, pAcc->GetPixelFromData(pScanline, nOtherX));
                    pAcc->SetPixelOnData(pScanline, nOtherX, aTemp);
                }
            }

            pAcc.reset();
            bRet = true;
        }
    }
    else
        bRet = true;

    return bRet;
}

bool Bitmap::Rotate(long nAngle10, const Color& rFillColor)
{
    bool bRet = false;

    nAngle10 %= 3600;
    nAngle10 = (nAngle10 < 0) ? (3599L + nAngle10) : nAngle10;

    if (!nAngle10)
        bRet = true;
    else if (nAngle10 == 1800)
        bRet = Mirror(BmpMirrorFlags::Horizontal | BmpMirrorFlags::Vertical);
    else
    {
        ScopedReadAccess pReadAcc(*this);
        Bitmap aRotatedBmp;

        if (pReadAcc)
        {
            const Size aSizePix(GetSizePixel());

            if (nAngle10 == 900 || nAngle10 == 2700)
            {
                const Size aNewSizePix(aSizePix.Height(), aSizePix.Width());
                Bitmap aNewBmp(aNewSizePix, GetBitCount(), &pReadAcc->GetPalette());
                BitmapScopedWriteAccess pWriteAcc(aNewBmp);

                if (pWriteAcc)
                {
                    const long nWidth = aSizePix.Width();
                    const long nWidth1 = nWidth - 1;
                    const long nHeight = aSizePix.Height();
                    const long nHeight1 = nHeight - 1;
                    const long nNewWidth = aNewSizePix.Width();
                    const long nNewHeight = aNewSizePix.Height();

                    if (nAngle10 == 900)
                    {
                        for (long nY = 0, nOtherX = nWidth1; nY < nNewHeight; nY++, nOtherX--)
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nY);
                            for (long nX = 0, nOtherY = 0; nX < nNewWidth; nX++)
                            {
                                pWriteAcc->SetPixelOnData(pScanline, nX,
                                                          pReadAcc->GetPixel(nOtherY++, nOtherX));
                            }
                        }
                    }
                    else if (nAngle10 == 2700)
                    {
                        for (long nY = 0, nOtherX = 0; nY < nNewHeight; nY++, nOtherX++)
                        {
                            Scanline pScanline = pWriteAcc->GetScanline(nY);
                            for (long nX = 0, nOtherY = nHeight1; nX < nNewWidth; nX++)
                            {
                                pWriteAcc->SetPixelOnData(pScanline, nX,
                                                          pReadAcc->GetPixel(nOtherY--, nOtherX));
                            }
                        }
                    }

                    pWriteAcc.reset();
                }

                aRotatedBmp = aNewBmp;
            }
            else
            {
                Point aTmpPoint;
                tools::Rectangle aTmpRectangle(aTmpPoint, aSizePix);
                tools::Polygon aPoly(aTmpRectangle);
                aPoly.Rotate(aTmpPoint, static_cast<sal_uInt16>(nAngle10));

                tools::Rectangle aNewBound(aPoly.GetBoundRect());
                const Size aNewSizePix(aNewBound.GetSize());
                Bitmap aNewBmp(aNewSizePix, GetBitCount(), &pReadAcc->GetPalette());
                BitmapScopedWriteAccess pWriteAcc(aNewBmp);

                if (pWriteAcc)
                {
                    const BitmapColor aFillColor(pWriteAcc->GetBestMatchingColor(rFillColor));
                    const double fCosAngle = cos(nAngle10 * F_PI1800);
                    const double fSinAngle = sin(nAngle10 * F_PI1800);
                    const double fXMin = aNewBound.Left();
                    const double fYMin = aNewBound.Top();
                    const long nWidth = aSizePix.Width();
                    const long nHeight = aSizePix.Height();
                    const long nNewWidth = aNewSizePix.Width();
                    const long nNewHeight = aNewSizePix.Height();
                    long nX;
                    long nY;
                    long nRotX;
                    long nRotY;
                    std::unique_ptr<long[]> pCosX(new long[nNewWidth]);
                    std::unique_ptr<long[]> pSinX(new long[nNewWidth]);
                    std::unique_ptr<long[]> pCosY(new long[nNewHeight]);
                    std::unique_ptr<long[]> pSinY(new long[nNewHeight]);

                    for (nX = 0; nX < nNewWidth; nX++)
                    {
                        const double fTmp = (fXMin + nX) * 64.;

                        pCosX[nX] = FRound(fCosAngle * fTmp);
                        pSinX[nX] = FRound(fSinAngle * fTmp);
                    }

                    for (nY = 0; nY < nNewHeight; nY++)
                    {
                        const double fTmp = (fYMin + nY) * 64.;

                        pCosY[nY] = FRound(fCosAngle * fTmp);
                        pSinY[nY] = FRound(fSinAngle * fTmp);
                    }

                    for (nY = 0; nY < nNewHeight; nY++)
                    {
                        long nSinY = pSinY[nY];
                        long nCosY = pCosY[nY];
                        Scanline pScanline = pWriteAcc->GetScanline(nY);

                        for (nX = 0; nX < nNewWidth; nX++)
                        {
                            nRotX = (pCosX[nX] - nSinY) >> 6;
                            nRotY = (pSinX[nX] + nCosY) >> 6;

                            if ((nRotX > -1) && (nRotX < nWidth) && (nRotY > -1)
                                && (nRotY < nHeight))
                            {
                                pWriteAcc->SetPixelOnData(pScanline, nX,
                                                          pReadAcc->GetPixel(nRotY, nRotX));
                            }
                            else
                            {
                                pWriteAcc->SetPixelOnData(pScanline, nX, aFillColor);
                            }
                        }
                    }

                    pWriteAcc.reset();
                }

                aRotatedBmp = aNewBmp;
            }

            pReadAcc.reset();
        }

        bRet = !!aRotatedBmp;
        if (bRet)
            ReassignWithSize(aRotatedBmp);
    }

    return bRet;
};

Bitmap Bitmap::CreateMask(const Color& rTransColor, sal_uInt8 nTol) const
{
    ScopedReadAccess pReadAcc(const_cast<Bitmap&>(*this));

    // Historically LO used 1bpp masks, but 8bpp masks are much faster,
    // better supported by hardware, and the memory savings are not worth
    // it anymore.
    // TODO: Possibly remove the 1bpp code later.
    constexpr bool use8BitMask = true;

    if (!nTol && pReadAcc
        && (pReadAcc->GetScanlineFormat() == ScanlineFormat::N1BitLsbPal
            || pReadAcc->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal)
        && pReadAcc->GetBestMatchingColor(COL_WHITE) == pReadAcc->GetBestMatchingColor(rTransColor))
    {
        // if we're a 1 bit pixel already, and the transcolor matches the color that would replace it
        // already, then just return a copy
        return *this;
    }

    Bitmap aNewBmp(GetSizePixel(), use8BitMask ? 8 : 1,
                   use8BitMask ? &Bitmap::GetGreyPalette(256) : nullptr);
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    bool bRet = false;

    if (pWriteAcc && pReadAcc)
    {
        const long nWidth = pReadAcc->Width();
        const long nHeight = pReadAcc->Height();
        const BitmapColor aBlack(pWriteAcc->GetBestMatchingColor(COL_BLACK));
        const BitmapColor aWhite(pWriteAcc->GetBestMatchingColor(COL_WHITE));

        if (!nTol)
        {
            const BitmapColor aTest(pReadAcc->GetBestMatchingColor(rTransColor));

            if (pWriteAcc->GetBitCount() == 1
                && (pReadAcc->GetScanlineFormat() == ScanlineFormat::N4BitMsnPal
                    || pReadAcc->GetScanlineFormat() == ScanlineFormat::N4BitLsnPal))
            {
                // optimized for 4Bit-MSN/LSN source palette
                const sal_uInt8 cTest = aTest.GetIndex();
                const long nShiftInit
                    = ((pReadAcc->GetScanlineFormat() == ScanlineFormat::N4BitMsnPal) ? 4 : 0);

                if (pWriteAcc->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal
                    && aWhite.GetIndex() == 1)
                {
                    // optimized for 1Bit-MSB destination palette
                    for (long nY = 0; nY < nHeight; ++nY)
                    {
                        Scanline pSrc = pReadAcc->GetScanline(nY);
                        Scanline pDst = pWriteAcc->GetScanline(nY);
                        for (long nX = 0, nShift = nShiftInit; nX < nWidth; nX++, nShift ^= 4)
                        {
                            if (cTest == ((pSrc[nX >> 1] >> nShift) & 0x0f))
                                pDst[nX >> 3] |= 1 << (7 - (nX & 7));
                            else
                                pDst[nX >> 3] &= ~(1 << (7 - (nX & 7)));
                        }
                    }
                }
                else
                {
                    for (long nY = 0; nY < nHeight; ++nY)
                    {
                        Scanline pSrc = pReadAcc->GetScanline(nY);
                        Scanline pDst = pWriteAcc->GetScanline(nY);
                        for (long nX = 0, nShift = nShiftInit; nX < nWidth; nX++, nShift ^= 4)
                        {
                            if (cTest == ((pSrc[nX >> 1] >> nShift) & 0x0f))
                                pWriteAcc->SetPixelOnData(pDst, nX, aWhite);
                            else
                                pWriteAcc->SetPixelOnData(pDst, nX, aBlack);
                        }
                    }
                }
            }
            else if (pWriteAcc->GetBitCount() == 1
                     && pReadAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal)
            {
                // optimized for 8Bit source palette
                const sal_uInt8 cTest = aTest.GetIndex();

                if (pWriteAcc->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal
                    && aWhite.GetIndex() == 1)
                {
                    // optimized for 1Bit-MSB destination palette
                    for (long nY = 0; nY < nHeight; ++nY)
                    {
                        Scanline pSrc = pReadAcc->GetScanline(nY);
                        Scanline pDst = pWriteAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; ++nX)
                        {
                            if (cTest == pSrc[nX])
                                pDst[nX >> 3] |= 1 << (7 - (nX & 7));
                            else
                                pDst[nX >> 3] &= ~(1 << (7 - (nX & 7)));
                        }
                    }
                }
                else
                {
                    for (long nY = 0; nY < nHeight; ++nY)
                    {
                        Scanline pSrc = pReadAcc->GetScanline(nY);
                        Scanline pDst = pWriteAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; ++nX)
                        {
                            if (cTest == pSrc[nX])
                                pWriteAcc->SetPixelOnData(pDst, nX, aWhite);
                            else
                                pWriteAcc->SetPixelOnData(pDst, nX, aBlack);
                        }
                    }
                }
            }
            else if (pWriteAcc->GetScanlineFormat() == pReadAcc->GetScanlineFormat()
                     && aWhite.GetIndex() == 1
                     && (pReadAcc->GetScanlineFormat() == ScanlineFormat::N1BitLsbPal
                         || pReadAcc->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal))
            {
                for (long nY = 0; nY < nHeight; ++nY)
                {
                    Scanline pSrc = pReadAcc->GetScanline(nY);
                    Scanline pDst = pWriteAcc->GetScanline(nY);
                    assert(pWriteAcc->GetScanlineSize() == pReadAcc->GetScanlineSize());
                    const long nScanlineSize = pWriteAcc->GetScanlineSize();
                    for (long nX = 0; nX < nScanlineSize; ++nX)
                        pDst[nX] = ~pSrc[nX];
                }
            }
            else if (use8BitMask && pWriteAcc->GetBitCount() == 8
                     && pReadAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal)
            {
                // optimized for 8Bit source palette
                const sal_uInt8 cTest = aTest.GetIndex();

                for (long nY = 0; nY < nHeight; ++nY)
                {
                    Scanline pSrc = pReadAcc->GetScanline(nY);
                    Scanline pDst = pWriteAcc->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; ++nX)
                    {
                        if (cTest == pSrc[nX])
                            pDst[nX] = aWhite.GetIndex();
                        else
                            pDst[nX] = aBlack.GetIndex();
                    }
                }
            }
            else
            {
                // not optimized
                for (long nY = 0; nY < nHeight; ++nY)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; ++nX)
                    {
                        if (aTest == pReadAcc->GetPixelFromData(pScanlineRead, nX))
                            pWriteAcc->SetPixelOnData(pScanline, nX, aWhite);
                        else
                            pWriteAcc->SetPixelOnData(pScanline, nX, aBlack);
                    }
                }
            }
        }
        else
        {
            BitmapColor aCol;
            long nR, nG, nB;
            const long nMinR = MinMax<long>(rTransColor.GetRed() - nTol, 0, 255);
            const long nMaxR = MinMax<long>(rTransColor.GetRed() + nTol, 0, 255);
            const long nMinG = MinMax<long>(rTransColor.GetGreen() - nTol, 0, 255);
            const long nMaxG = MinMax<long>(rTransColor.GetGreen() + nTol, 0, 255);
            const long nMinB = MinMax<long>(rTransColor.GetBlue() - nTol, 0, 255);
            const long nMaxB = MinMax<long>(rTransColor.GetBlue() + nTol, 0, 255);

            if (pReadAcc->HasPalette())
            {
                for (long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; nX++)
                    {
                        aCol = pReadAcc->GetPaletteColor(
                            pReadAcc->GetIndexFromData(pScanlineRead, nX));
                        nR = aCol.GetRed();
                        nG = aCol.GetGreen();
                        nB = aCol.GetBlue();

                        if (nMinR <= nR && nMaxR >= nR && nMinG <= nG && nMaxG >= nG && nMinB <= nB
                            && nMaxB >= nB)
                        {
                            pWriteAcc->SetPixelOnData(pScanline, nX, aWhite);
                        }
                        else
                        {
                            pWriteAcc->SetPixelOnData(pScanline, nX, aBlack);
                        }
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
                        aCol = pReadAcc->GetPixelFromData(pScanlineRead, nX);
                        nR = aCol.GetRed();
                        nG = aCol.GetGreen();
                        nB = aCol.GetBlue();

                        if (nMinR <= nR && nMaxR >= nR && nMinG <= nG && nMaxG >= nG && nMinB <= nB
                            && nMaxB >= nB)
                        {
                            pWriteAcc->SetPixelOnData(pScanline, nX, aWhite);
                        }
                        else
                        {
                            pWriteAcc->SetPixelOnData(pScanline, nX, aBlack);
                        }
                    }
                }
            }
        }

        bRet = true;
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    if (bRet)
    {
        aNewBmp.maPrefSize = maPrefSize;
        aNewBmp.maPrefMapMode = maPrefMapMode;
    }
    else
        aNewBmp = Bitmap();

    return aNewBmp;
}

vcl::Region Bitmap::CreateRegion(const Color& rColor, const tools::Rectangle& rRect) const
{
    vcl::Region aRegion;
    tools::Rectangle aRect(rRect);
    ScopedReadAccess pReadAcc(const_cast<Bitmap&>(*this));

    aRect.Intersection(tools::Rectangle(Point(), GetSizePixel()));
    aRect.Justify();

    if (pReadAcc)
    {
        const long nLeft = aRect.Left();
        const long nTop = aRect.Top();
        const long nRight = aRect.Right();
        const long nBottom = aRect.Bottom();
        const BitmapColor aMatch(pReadAcc->GetBestMatchingColor(rColor));

        std::vector<long> aLine;
        long nYStart(nTop);
        long nY(nTop);

        for (; nY <= nBottom; nY++)
        {
            std::vector<long> aNewLine;
            long nX(nLeft);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);

            for (; nX <= nRight;)
            {
                while ((nX <= nRight) && (aMatch != pReadAcc->GetPixelFromData(pScanlineRead, nX)))
                    nX++;

                if (nX <= nRight)
                {
                    aNewLine.push_back(nX);

                    while ((nX <= nRight)
                           && (aMatch == pReadAcc->GetPixelFromData(pScanlineRead, nX)))
                    {
                        nX++;
                    }

                    aNewLine.push_back(nX - 1);
                }
            }

            if (aNewLine != aLine)
            {
                // need to write aLine, it's different from the next line
                if (!aLine.empty())
                {
                    tools::Rectangle aSubRect;

                    // enter y values and proceed ystart
                    aSubRect.SetTop(nYStart);
                    aSubRect.SetBottom(nY ? nY - 1 : 0);

                    for (size_t a(0); a < aLine.size();)
                    {
                        aSubRect.SetLeft(aLine[a++]);
                        aSubRect.SetRight(aLine[a++]);
                        aRegion.Union(aSubRect);
                    }
                }

                // copy line as new line
                aLine = aNewLine;
                nYStart = nY;
            }
        }

        // write last line if used
        if (!aLine.empty())
        {
            tools::Rectangle aSubRect;

            // enter y values
            aSubRect.SetTop(nYStart);
            aSubRect.SetBottom(nY ? nY - 1 : 0);

            for (size_t a(0); a < aLine.size();)
            {
                aSubRect.SetLeft(aLine[a++]);
                aSubRect.SetRight(aLine[a++]);
                aRegion.Union(aSubRect);
            }
        }

        pReadAcc.reset();
    }
    else
    {
        aRegion = aRect;
    }

    return aRegion;
}

bool Bitmap::Replace(const Bitmap& rMask, const Color& rReplaceColor)
{
    ScopedReadAccess pMaskAcc(const_cast<Bitmap&>(rMask));
    BitmapScopedWriteAccess pAcc(*this);
    bool bRet = false;

    if (pMaskAcc && pAcc)
    {
        const long nWidth = std::min(pMaskAcc->Width(), pAcc->Width());
        const long nHeight = std::min(pMaskAcc->Height(), pAcc->Height());
        const BitmapColor aMaskWhite(pMaskAcc->GetBestMatchingColor(COL_WHITE));
        BitmapColor aReplace;

        if (pAcc->HasPalette())
        {
            const sal_uInt16 nActColors = pAcc->GetPaletteEntryCount();
            const sal_uInt16 nMaxColors = 1 << pAcc->GetBitCount();

            // default to the nearest color
            aReplace = pAcc->GetBestMatchingColor(rReplaceColor);

            // for paletted images without a matching palette entry
            // look for an unused palette entry (NOTE: expensive!)
            if (pAcc->GetPaletteColor(aReplace.GetIndex()) != BitmapColor(rReplaceColor))
            {
                // if the palette has empty entries use the last one
                if (nActColors < nMaxColors)
                {
                    pAcc->SetPaletteEntryCount(nActColors + 1);
                    pAcc->SetPaletteColor(nActColors, rReplaceColor);
                    aReplace = BitmapColor(static_cast<sal_uInt8>(nActColors));
                }
                else
                {
                    std::unique_ptr<bool[]> pFlags(new bool[nMaxColors]);

                    // Set all entries to false
                    std::fill(pFlags.get(), pFlags.get() + nMaxColors, false);

                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
                            pFlags[pAcc->GetIndexFromData(pScanline, nX)] = true;
                    }

                    for (sal_uInt16 i = 0; i < nMaxColors; i++)
                    {
                        // Hurray, we do have an unused entry
                        if (!pFlags[i])
                        {
                            pAcc->SetPaletteColor(i, rReplaceColor);
                            aReplace = BitmapColor(static_cast<sal_uInt8>(i));
                        }
                    }
                }
            }
        }
        else
            aReplace = rReplaceColor;

        for (long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pAcc->GetScanline(nY);
            Scanline pScanlineMask = pMaskAcc->GetScanline(nY);
            for (long nX = 0; nX < nWidth; nX++)
            {
                if (pMaskAcc->GetPixelFromData(pScanlineMask, nX) == aMaskWhite)
                    pAcc->SetPixelOnData(pScanline, nX, aReplace);
            }
        }

        bRet = true;
    }

    return bRet;
}

bool Bitmap::Replace(const AlphaMask& rAlpha, const Color& rMergeColor)
{
    Bitmap aNewBmp(GetSizePixel(), 24);
    ScopedReadAccess pAcc(*this);
    AlphaMask::ScopedReadAccess pAlphaAcc(const_cast<AlphaMask&>(rAlpha));
    BitmapScopedWriteAccess pNewAcc(aNewBmp);
    bool bRet = false;

    if (pAcc && pAlphaAcc && pNewAcc)
    {
        BitmapColor aCol;
        const long nWidth = std::min(pAlphaAcc->Width(), pAcc->Width());
        const long nHeight = std::min(pAlphaAcc->Height(), pAcc->Height());

        for (long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pNewAcc->GetScanline(nY);
            Scanline pScanlineAlpha = pAlphaAcc->GetScanline(nY);
            for (long nX = 0; nX < nWidth; nX++)
            {
                aCol = pAcc->GetColor(nY, nX);
                aCol.Merge(rMergeColor, 255 - pAlphaAcc->GetIndexFromData(pScanlineAlpha, nX));
                pNewAcc->SetPixelOnData(pScanline, nX, aCol);
            }
        }

        bRet = true;
    }

    pAcc.reset();
    pAlphaAcc.reset();
    pNewAcc.reset();

    if (bRet)
    {
        const MapMode aMap(maPrefMapMode);
        const Size aSize(maPrefSize);

        *this = aNewBmp;

        maPrefMapMode = aMap;
        maPrefSize = aSize;
    }

    return bRet;
}

bool Bitmap::Replace(const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol)
{
    if (mxSalBmp)
    {
        // implementation specific replace
        std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
        if (xImpBmp->Create(*mxSalBmp) && xImpBmp->Replace(rSearchColor, rReplaceColor, nTol))
        {
            ImplSetSalBitmap(xImpBmp);
            maPrefMapMode = MapMode(MapUnit::MapPixel);
            maPrefSize = xImpBmp->GetSize();
            return true;
        }
    }

    // Bitmaps with 1 bit color depth can cause problems if they have other entries than black/white
    // in their palette
    if (GetBitCount() == 1)
        Convert(BmpConversion::N4BitColors);

    BitmapScopedWriteAccess pAcc(*this);
    bool bRet = false;

    if (pAcc)
    {
        const long nMinR = MinMax<long>(rSearchColor.GetRed() - nTol, 0, 255);
        const long nMaxR = MinMax<long>(rSearchColor.GetRed() + nTol, 0, 255);
        const long nMinG = MinMax<long>(rSearchColor.GetGreen() - nTol, 0, 255);
        const long nMaxG = MinMax<long>(rSearchColor.GetGreen() + nTol, 0, 255);
        const long nMinB = MinMax<long>(rSearchColor.GetBlue() - nTol, 0, 255);
        const long nMaxB = MinMax<long>(rSearchColor.GetBlue() + nTol, 0, 255);

        if (pAcc->HasPalette())
        {
            for (sal_uInt16 i = 0, nPalCount = pAcc->GetPaletteEntryCount(); i < nPalCount; i++)
            {
                const BitmapColor& rCol = pAcc->GetPaletteColor(i);

                if (nMinR <= rCol.GetRed() && nMaxR >= rCol.GetRed() && nMinG <= rCol.GetGreen()
                    && nMaxG >= rCol.GetGreen() && nMinB <= rCol.GetBlue()
                    && nMaxB >= rCol.GetBlue())
                {
                    pAcc->SetPaletteColor(i, rReplaceColor);
                }
            }
        }
        else
        {
            BitmapColor aCol;
            const BitmapColor aReplace(pAcc->GetBestMatchingColor(rReplaceColor));

            for (long nY = 0, nHeight = pAcc->Height(); nY < nHeight; nY++)
            {
                Scanline pScanline = pAcc->GetScanline(nY);
                for (long nX = 0, nWidth = pAcc->Width(); nX < nWidth; nX++)
                {
                    aCol = pAcc->GetPixelFromData(pScanline, nX);

                    if (nMinR <= aCol.GetRed() && nMaxR >= aCol.GetRed() && nMinG <= aCol.GetGreen()
                        && nMaxG >= aCol.GetGreen() && nMinB <= aCol.GetBlue()
                        && nMaxB >= aCol.GetBlue())
                    {
                        pAcc->SetPixelOnData(pScanline, nX, aReplace);
                    }
                }
            }
        }

        pAcc.reset();
        bRet = true;
    }

    return bRet;
}

bool Bitmap::Replace(const Color* pSearchColors, const Color* pReplaceColors, sal_uLong nColorCount,
                     sal_uInt8 const* pTols)
{
    // Bitmaps with 1 bit color depth can cause problems if they have other entries than black/white
    // in their palette
    if (GetBitCount() == 1)
        Convert(BmpConversion::N4BitColors);

    BitmapScopedWriteAccess pAcc(*this);
    bool bRet = false;

    if (pAcc)
    {
        std::unique_ptr<long[]> pMinR(new long[nColorCount]);
        std::unique_ptr<long[]> pMaxR(new long[nColorCount]);
        std::unique_ptr<long[]> pMinG(new long[nColorCount]);
        std::unique_ptr<long[]> pMaxG(new long[nColorCount]);
        std::unique_ptr<long[]> pMinB(new long[nColorCount]);
        std::unique_ptr<long[]> pMaxB(new long[nColorCount]);

        if (pTols)
        {
            for (sal_uLong i = 0; i < nColorCount; i++)
            {
                const Color& rCol = pSearchColors[i];
                const sal_uInt8 nTol = pTols[i];

                pMinR[i] = MinMax<long>(rCol.GetRed() - nTol, 0, 255);
                pMaxR[i] = MinMax<long>(rCol.GetRed() + nTol, 0, 255);
                pMinG[i] = MinMax<long>(rCol.GetGreen() - nTol, 0, 255);
                pMaxG[i] = MinMax<long>(rCol.GetGreen() + nTol, 0, 255);
                pMinB[i] = MinMax<long>(rCol.GetBlue() - nTol, 0, 255);
                pMaxB[i] = MinMax<long>(rCol.GetBlue() + nTol, 0, 255);
            }
        }
        else
        {
            for (sal_uLong i = 0; i < nColorCount; i++)
            {
                const Color& rCol = pSearchColors[i];

                pMinR[i] = rCol.GetRed();
                pMaxR[i] = rCol.GetRed();
                pMinG[i] = rCol.GetGreen();
                pMaxG[i] = rCol.GetGreen();
                pMinB[i] = rCol.GetBlue();
                pMaxB[i] = rCol.GetBlue();
            }
        }

        if (pAcc->HasPalette())
        {
            for (sal_uInt16 nEntry = 0, nPalCount = pAcc->GetPaletteEntryCount();
                 nEntry < nPalCount; nEntry++)
            {
                const BitmapColor& rCol = pAcc->GetPaletteColor(nEntry);

                for (sal_uLong i = 0; i < nColorCount; i++)
                {
                    if (pMinR[i] <= rCol.GetRed() && pMaxR[i] >= rCol.GetRed()
                        && pMinG[i] <= rCol.GetGreen() && pMaxG[i] >= rCol.GetGreen()
                        && pMinB[i] <= rCol.GetBlue() && pMaxB[i] >= rCol.GetBlue())
                    {
                        pAcc->SetPaletteColor(nEntry, pReplaceColors[i]);
                        break;
                    }
                }
            }
        }
        else
        {
            BitmapColor aCol;
            std::unique_ptr<BitmapColor[]> pReplaces(new BitmapColor[nColorCount]);

            for (sal_uLong i = 0; i < nColorCount; i++)
                pReplaces[i] = pAcc->GetBestMatchingColor(pReplaceColors[i]);

            for (long nY = 0, nHeight = pAcc->Height(); nY < nHeight; nY++)
            {
                Scanline pScanline = pAcc->GetScanline(nY);
                for (long nX = 0, nWidth = pAcc->Width(); nX < nWidth; nX++)
                {
                    aCol = pAcc->GetPixelFromData(pScanline, nX);

                    for (sal_uLong i = 0; i < nColorCount; i++)
                    {
                        if (pMinR[i] <= aCol.GetRed() && pMaxR[i] >= aCol.GetRed()
                            && pMinG[i] <= aCol.GetGreen() && pMaxG[i] >= aCol.GetGreen()
                            && pMinB[i] <= aCol.GetBlue() && pMaxB[i] >= aCol.GetBlue())
                        {
                            pAcc->SetPixelOnData(pScanline, nX, pReplaces[i]);
                            break;
                        }
                    }
                }
            }
        }

        pAcc.reset();
        bRet = true;
    }

    return bRet;
}

bool Bitmap::CombineSimple(const Bitmap& rMask, BmpCombine eCombine)
{
    ScopedReadAccess pMaskAcc(const_cast<Bitmap&>(rMask));
    BitmapScopedWriteAccess pAcc(*this);
    bool bRet = false;

    if (pMaskAcc && pAcc)
    {
        const long nWidth = std::min(pMaskAcc->Width(), pAcc->Width());
        const long nHeight = std::min(pMaskAcc->Height(), pAcc->Height());
        const Color aColBlack(COL_BLACK);
        const BitmapColor aWhite(pAcc->GetBestMatchingColor(COL_WHITE));
        const BitmapColor aBlack(pAcc->GetBestMatchingColor(aColBlack));
        const BitmapColor aMaskBlack(pMaskAcc->GetBestMatchingColor(aColBlack));

        switch (eCombine)
        {
            case BmpCombine::And:
            {
                for (long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pAcc->GetScanline(nY);
                    Scanline pScanlineMask = pMaskAcc->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; nX++)
                    {
                        if (pMaskAcc->GetPixelFromData(pScanlineMask, nX) != aMaskBlack
                            && pAcc->GetPixelFromData(pScanline, nX) != aBlack)
                        {
                            pAcc->SetPixelOnData(pScanline, nX, aWhite);
                        }
                        else
                        {
                            pAcc->SetPixelOnData(pScanline, nX, aBlack);
                        }
                    }
                }
            }
            break;

            case BmpCombine::Or:
            {
                for (long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pAcc->GetScanline(nY);
                    Scanline pScanlineMask = pMaskAcc->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; nX++)
                    {
                        if (pMaskAcc->GetPixelFromData(pScanlineMask, nX) != aMaskBlack
                            || pAcc->GetPixelFromData(pScanline, nX) != aBlack)
                        {
                            pAcc->SetPixelOnData(pScanline, nX, aWhite);
                        }
                        else
                        {
                            pAcc->SetPixelOnData(pScanline, nX, aBlack);
                        }
                    }
                }
            }
            break;
        }

        bRet = true;
    }

    return bRet;
}

// TODO: Have a look at OutputDevice::ImplDrawAlpha() for some
// optimizations. Might even consolidate the code here and there.
bool Bitmap::Blend(const AlphaMask& rAlpha, const Color& rBackgroundColor)
{
    // Convert to a truecolor bitmap, if we're a paletted one. There's room for tradeoff decision here,
    // maybe later for an overload (or a flag)
    if (GetBitCount() <= 8)
        Convert(BmpConversion::N24Bit);

    AlphaMask::ScopedReadAccess pAlphaAcc(const_cast<AlphaMask&>(rAlpha));

    BitmapScopedWriteAccess pAcc(*this);
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
                BitmapColor aBmpColor = pAcc->GetPixelFromData(pScanline, nX);
                aBmpColor.Merge(rBackgroundColor,
                                255 - pAlphaAcc->GetIndexFromData(pScanlineAlpha, nX));
                pAcc->SetPixelOnData(pScanline, nX, aBmpColor);
            }
        }

        bRet = true;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
