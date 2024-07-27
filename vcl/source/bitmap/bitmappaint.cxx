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
#include <vcl/alpha.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <salbmp.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

#include <algorithm>
#include <memory>

static BitmapColor UpdatePaletteForNewColor(BitmapScopedWriteAccess& pAcc,
                                            const sal_uInt16 nActColors,
                                            const sal_uInt16 nMaxColors, const tools::Long nHeight,
                                            const tools::Long nWidth,
                                            const BitmapColor& rWantedColor);

bool Bitmap::Erase(const Color& rFillColor)
{
    if (IsEmpty())
        return true;

    // implementation specific replace
    std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
    if (xImpBmp->Create(*mxSalBmp) && xImpBmp->Erase(rFillColor))
    {
        ImplSetSalBitmap(xImpBmp);
        maPrefMapMode = MapMode(MapUnit::MapPixel);
        maPrefSize = xImpBmp->GetSize();
        return true;
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
    if (!mxSalBmp)
        return false;

    // try optimised call, much faster on Skia
    if (mxSalBmp->Invert())
    {
        mxSalBmp->InvalidateChecksum();
        return true;
    }

    BitmapScopedWriteAccess pWriteAcc(*this);
    const tools::Long nWidth = pWriteAcc->Width();
    const tools::Long nHeight = pWriteAcc->Height();

    if (pWriteAcc->HasPalette())
    {
        const sal_uInt16 nActColors = pWriteAcc->GetPaletteEntryCount();

        if (pWriteAcc->GetPalette().IsGreyPalette8Bit())
        {
            // For alpha masks, we need to actually invert the underlying data
            // or the optimisations elsewhere do not always work right. If this is a bottleneck,
            // probably better to try improving it inside the mxSalBmp->Invert() call above.
            for (tools::Long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                for (tools::Long nX = 0; nX < nWidth; nX++)
                {
                    BitmapColor aBmpColor = pWriteAcc->GetPixelFromData(pScanline, nX);
                    aBmpColor.SetIndex(0xff - aBmpColor.GetIndex());
                    pWriteAcc->SetPixelOnData(pScanline, nX, aBmpColor);
                }
            }
        }
        else
        {
            for (sal_uInt16 i = 0; i < nActColors; ++i)
            {
                BitmapColor aBmpColor = pWriteAcc->GetPaletteColor(i);
                aBmpColor.Invert();
                pWriteAcc->SetPaletteColor(i, aBmpColor);
            }
        }
    }
    else
    {
        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
            {
                BitmapColor aBmpColor = pWriteAcc->GetPixelFromData(pScanline, nX);
                aBmpColor.Invert();
                pWriteAcc->SetPixelOnData(pScanline, nX, aBmpColor);
            }
        }
    }
    mxSalBmp->InvalidateChecksum();

    return true;
}

namespace
{
// Put each scanline's content horizontally mirrored into the other one.
// (optimized version accessing pixel values directly).
template <int bitCount>
void mirrorScanlines(Scanline scanline1, Scanline scanline2, tools::Long nWidth)
{
    constexpr int byteCount = bitCount / 8;
    Scanline pos1 = scanline1;
    Scanline pos2 = scanline2 + (nWidth - 1) * byteCount; // last in second scanline
    sal_uInt8 tmp[byteCount];
    for (tools::Long i = 0; i < nWidth; ++i)
    {
        memcpy(tmp, pos1, byteCount);
        memcpy(pos1, pos2, byteCount);
        memcpy(pos2, tmp, byteCount);
        pos1 += byteCount;
        pos2 -= byteCount;
    }
}
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
            const tools::Long nWidth = pAcc->Width();
            const tools::Long nHeight = pAcc->Height();
            const tools::Long nWidth1 = nWidth - 1;
            const tools::Long nWidth_2 = nWidth / 2;
            const tools::Long nSecondHalf = nWidth - nWidth_2;

            switch (pAcc->GetBitCount())
            {
                // Special-case these, swap the halves of scanlines while mirroring them.
                case 32:
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                        mirrorScanlines<32>(pAcc->GetScanline(nY),
                                            pAcc->GetScanline(nY) + 4 * nSecondHalf, nWidth_2);
                    break;
                case 24:
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                        mirrorScanlines<24>(pAcc->GetScanline(nY),
                                            pAcc->GetScanline(nY) + 3 * nSecondHalf, nWidth_2);
                    break;
                case 8:
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                        mirrorScanlines<8>(pAcc->GetScanline(nY),
                                           pAcc->GetScanline(nY) + nSecondHalf, nWidth_2);
                    break;
                default:
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pAcc->GetScanline(nY);
                        for (tools::Long nX = 0, nOther = nWidth1; nX < nWidth_2; nX++, nOther--)
                        {
                            const BitmapColor aTemp(pAcc->GetPixelFromData(pScanline, nX));

                            pAcc->SetPixelOnData(pScanline, nX,
                                                 pAcc->GetPixelFromData(pScanline, nOther));
                            pAcc->SetPixelOnData(pScanline, nOther, aTemp);
                        }
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
            const tools::Long nScanSize = pAcc->GetScanlineSize();
            std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nScanSize]);
            const tools::Long nHeight = pAcc->Height();
            const tools::Long nHeight1 = nHeight - 1;
            const tools::Long nHeight_2 = nHeight >> 1;

            for (tools::Long nY = 0, nOther = nHeight1; nY < nHeight_2; nY++, nOther--)
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
            const tools::Long nWidth = pAcc->Width();
            const tools::Long nWidth1 = nWidth - 1;
            const tools::Long nHeight = pAcc->Height();
            tools::Long nHeight_2 = nHeight / 2;
            const tools::Long nWidth_2 = nWidth / 2;
            const tools::Long nSecondHalf = nWidth - nWidth_2;

            switch (pAcc->GetBitCount())
            {
                case 32:
                    for (tools::Long nY = 0, nOtherY = nHeight - 1; nY < nHeight_2; nY++, nOtherY--)
                        mirrorScanlines<32>(pAcc->GetScanline(nY), pAcc->GetScanline(nOtherY),
                                            nWidth);
                    if (nHeight & 1)
                        mirrorScanlines<32>(pAcc->GetScanline(nHeight_2),
                                            pAcc->GetScanline(nHeight_2) + 4 * nSecondHalf,
                                            nWidth_2);
                    break;
                case 24:
                    for (tools::Long nY = 0, nOtherY = nHeight - 1; nY < nHeight_2; nY++, nOtherY--)
                        mirrorScanlines<24>(pAcc->GetScanline(nY), pAcc->GetScanline(nOtherY),
                                            nWidth);
                    if (nHeight & 1)
                        mirrorScanlines<24>(pAcc->GetScanline(nHeight_2),
                                            pAcc->GetScanline(nHeight_2) + 3 * nSecondHalf,
                                            nWidth_2);
                    break;
                case 8:
                    for (tools::Long nY = 0, nOtherY = nHeight - 1; nY < nHeight_2; nY++, nOtherY--)
                        mirrorScanlines<8>(pAcc->GetScanline(nY), pAcc->GetScanline(nOtherY),
                                           nWidth);
                    if (nHeight & 1)
                        mirrorScanlines<8>(pAcc->GetScanline(nHeight_2),
                                           pAcc->GetScanline(nHeight_2) + nSecondHalf, nWidth_2);
                    break;
                default:
                    for (tools::Long nY = 0, nOtherY = nHeight - 1; nY < nHeight_2; nY++, nOtherY--)
                    {
                        Scanline pScanline = pAcc->GetScanline(nY);
                        Scanline pScanlineOther = pAcc->GetScanline(nOtherY);
                        for (tools::Long nX = 0, nOtherX = nWidth1; nX < nWidth; nX++, nOtherX--)
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
                        for (tools::Long nX = 0, nOtherX = nWidth1; nX < nWidth_2; nX++, nOtherX--)
                        {
                            const BitmapColor aTemp(pAcc->GetPixelFromData(pScanline, nX));
                            pAcc->SetPixelOnData(pScanline, nX,
                                                 pAcc->GetPixelFromData(pScanline, nOtherX));
                            pAcc->SetPixelOnData(pScanline, nOtherX, aTemp);
                        }
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

bool Bitmap::Rotate(Degree10 nAngle10, const Color& rFillColor)
{
    nAngle10 %= 3600_deg10;
    nAngle10 = (nAngle10 < 0_deg10) ? (Degree10(3599) + nAngle10) : nAngle10;

    if (!nAngle10)
        return true;
    if (nAngle10 == 1800_deg10)
        return Mirror(BmpMirrorFlags::Horizontal | BmpMirrorFlags::Vertical);

    BitmapScopedReadAccess pReadAcc(*this);
    if (!pReadAcc)
        return false;

    Bitmap aRotatedBmp;
    bool bRet = false;
    const Size aSizePix(GetSizePixel());

    if (nAngle10 == 900_deg10 || nAngle10 == 2700_deg10)
    {
        const Size aNewSizePix(aSizePix.Height(), aSizePix.Width());
        Bitmap aNewBmp(aNewSizePix, getPixelFormat(), &pReadAcc->GetPalette());
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const tools::Long nWidth = aSizePix.Width();
            const tools::Long nWidth1 = nWidth - 1;
            const tools::Long nHeight = aSizePix.Height();
            const tools::Long nHeight1 = nHeight - 1;
            const tools::Long nNewWidth = aNewSizePix.Width();
            const tools::Long nNewHeight = aNewSizePix.Height();

            if (nAngle10 == 900_deg10)
            {
                for (tools::Long nY = 0, nOtherX = nWidth1; nY < nNewHeight; nY++, nOtherX--)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    for (tools::Long nX = 0, nOtherY = 0; nX < nNewWidth; nX++)
                    {
                        pWriteAcc->SetPixelOnData(pScanline, nX,
                                                  pReadAcc->GetPixel(nOtherY++, nOtherX));
                    }
                }
            }
            else if (nAngle10 == 2700_deg10)
            {
                for (tools::Long nY = 0, nOtherX = 0; nY < nNewHeight; nY++, nOtherX++)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    for (tools::Long nX = 0, nOtherY = nHeight1; nX < nNewWidth; nX++)
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
        aPoly.Rotate(aTmpPoint, nAngle10);

        tools::Rectangle aNewBound(aPoly.GetBoundRect());
        const Size aNewSizePix(aNewBound.GetSize());
        Bitmap aNewBmp(aNewSizePix, getPixelFormat(), &pReadAcc->GetPalette());
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const BitmapColor aFillColor(pWriteAcc->GetBestMatchingColor(rFillColor));
            const double fCosAngle = cos(toRadians(nAngle10));
            const double fSinAngle = sin(toRadians(nAngle10));
            const double fXMin = aNewBound.Left();
            const double fYMin = aNewBound.Top();
            const sal_Int32 nWidth = aSizePix.Width();
            const sal_Int32 nHeight = aSizePix.Height();
            const sal_Int32 nNewWidth = aNewSizePix.Width();
            const sal_Int32 nNewHeight = aNewSizePix.Height();
            // we store alternating values of cos/sin. We do this instead of
            // separate arrays to improve cache hit.
            std::unique_ptr<sal_Int32[]> pCosSinX(new sal_Int32[nNewWidth * 2]);
            std::unique_ptr<sal_Int32[]> pCosSinY(new sal_Int32[nNewHeight * 2]);

            for (sal_Int32 nIdx = 0, nX = 0; nX < nNewWidth; nX++)
            {
                const double fTmp = (fXMin + nX) * 64;

                pCosSinX[nIdx++] = std::round(fCosAngle * fTmp);
                pCosSinX[nIdx++] = std::round(fSinAngle * fTmp);
            }

            for (sal_Int32 nIdx = 0, nY = 0; nY < nNewHeight; nY++)
            {
                const double fTmp = (fYMin + nY) * 64;

                pCosSinY[nIdx++] = std::round(fCosAngle * fTmp);
                pCosSinY[nIdx++] = std::round(fSinAngle * fTmp);
            }

            for (sal_Int32 nCosSinYIdx = 0, nY = 0; nY < nNewHeight; nY++)
            {
                sal_Int32 nCosY = pCosSinY[nCosSinYIdx++];
                sal_Int32 nSinY = pCosSinY[nCosSinYIdx++];
                Scanline pScanline = pWriteAcc->GetScanline(nY);

                for (sal_Int32 nCosSinXIdx = 0, nX = 0; nX < nNewWidth; nX++)
                {
                    sal_Int32 nRotX = (pCosSinX[nCosSinXIdx++] - nSinY) >> 6;
                    sal_Int32 nRotY = (pCosSinX[nCosSinXIdx++] + nCosY) >> 6;

                    if ((nRotX > -1) && (nRotX < nWidth) && (nRotY > -1) && (nRotY < nHeight))
                    {
                        pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPixel(nRotY, nRotX));
                    }
                    else
                    {
                        pWriteAcc->SetPixelOnData(pScanline, nX, aFillColor);
                    }
                }
            }

            pWriteAcc.reset();
        }

        aRotatedBmp = std::move(aNewBmp);
    }

    pReadAcc.reset();

    bRet = !aRotatedBmp.IsEmpty();
    if (bRet)
        ReassignWithSize(aRotatedBmp);

    return bRet;
};

Bitmap Bitmap::CreateMask(const Color& rTransColor) const
{
    BitmapScopedReadAccess pReadAcc(*this);
    if (!pReadAcc)
        return Bitmap();

    // Historically LO used 1bpp masks, but 8bpp masks are much faster,
    // better supported by hardware, and the memory savings are not worth
    // it anymore.
    // TODO: Possibly remove the 1bpp code later.

    if ((pReadAcc->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal)
        && pReadAcc->GetBestMatchingColor(COL_WHITE) == pReadAcc->GetBestMatchingColor(rTransColor))
    {
        // if we're a 1 bit pixel already, and the transcolor matches the color that would replace it
        // already, then just return a copy
        return *this;
    }

    auto ePixelFormat = vcl::PixelFormat::N8_BPP;
    Bitmap aNewBmp(GetSizePixel(), ePixelFormat, &Bitmap::GetGreyPalette(256));
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return Bitmap();

    const tools::Long nWidth = pReadAcc->Width();
    const tools::Long nHeight = pReadAcc->Height();
    const BitmapColor aBlack(pWriteAcc->GetBestMatchingColor(COL_BLACK));
    const BitmapColor aWhite(pWriteAcc->GetBestMatchingColor(COL_WHITE));

    const BitmapColor aTest(pReadAcc->GetBestMatchingColor(rTransColor));

    if (pWriteAcc->GetScanlineFormat() == pReadAcc->GetScanlineFormat() && aWhite.GetIndex() == 1
        && (pReadAcc->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal))
    {
        for (tools::Long nY = 0; nY < nHeight; ++nY)
        {
            Scanline pSrc = pReadAcc->GetScanline(nY);
            Scanline pDst = pWriteAcc->GetScanline(nY);
            assert(pWriteAcc->GetScanlineSize() == pReadAcc->GetScanlineSize());
            const tools::Long nScanlineSize = pWriteAcc->GetScanlineSize();
            for (tools::Long nX = 0; nX < nScanlineSize; ++nX)
                pDst[nX] = ~pSrc[nX];
        }
    }
    else if (pReadAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal)
    {
        // optimized for 8Bit source palette
        const sal_uInt8 cTest = aTest.GetIndex();

        for (tools::Long nY = 0; nY < nHeight; ++nY)
        {
            Scanline pSrc = pReadAcc->GetScanline(nY);
            Scanline pDst = pWriteAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; ++nX)
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
        for (tools::Long nY = 0; nY < nHeight; ++nY)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; ++nX)
            {
                if (aTest == pReadAcc->GetPixelFromData(pScanlineRead, nX))
                    pWriteAcc->SetPixelOnData(pScanline, nX, aWhite);
                else
                    pWriteAcc->SetPixelOnData(pScanline, nX, aBlack);
            }
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    aNewBmp.maPrefSize = maPrefSize;
    aNewBmp.maPrefMapMode = maPrefMapMode;

    return aNewBmp;
}

Bitmap Bitmap::CreateMask(const Color& rTransColor, sal_uInt8 nTol) const
{
    if (nTol == 0)
        return CreateMask(rTransColor);

    BitmapScopedReadAccess pReadAcc(*this);
    if (!pReadAcc)
        return Bitmap();

    // Historically LO used 1bpp masks, but 8bpp masks are much faster,
    // better supported by hardware, and the memory savings are not worth
    // it anymore.
    // TODO: Possibly remove the 1bpp code later.

    auto ePixelFormat = vcl::PixelFormat::N8_BPP;
    Bitmap aNewBmp(GetSizePixel(), ePixelFormat, &Bitmap::GetGreyPalette(256));
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return Bitmap();

    const tools::Long nWidth = pReadAcc->Width();
    const tools::Long nHeight = pReadAcc->Height();
    const BitmapColor aBlack(pWriteAcc->GetBestMatchingColor(COL_BLACK));
    const BitmapColor aWhite(pWriteAcc->GetBestMatchingColor(COL_WHITE));

    BitmapColor aCol;
    tools::Long nR, nG, nB;
    const tools::Long nMinR = std::clamp<tools::Long>(rTransColor.GetRed() - nTol, 0, 255);
    const tools::Long nMaxR = std::clamp<tools::Long>(rTransColor.GetRed() + nTol, 0, 255);
    const tools::Long nMinG = std::clamp<tools::Long>(rTransColor.GetGreen() - nTol, 0, 255);
    const tools::Long nMaxG = std::clamp<tools::Long>(rTransColor.GetGreen() + nTol, 0, 255);
    const tools::Long nMinB = std::clamp<tools::Long>(rTransColor.GetBlue() - nTol, 0, 255);
    const tools::Long nMaxB = std::clamp<tools::Long>(rTransColor.GetBlue() + nTol, 0, 255);

    if (pReadAcc->HasPalette())
    {
        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
            {
                aCol = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX));
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
        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
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

    pWriteAcc.reset();
    pReadAcc.reset();

    aNewBmp.maPrefSize = maPrefSize;
    aNewBmp.maPrefMapMode = maPrefMapMode;

    return aNewBmp;
}

AlphaMask Bitmap::CreateAlphaMask(const Color& rTransColor) const
{
    BitmapScopedReadAccess pReadAcc(*this);
    if (!pReadAcc)
        return AlphaMask();

    // Historically LO used 1bpp masks, but 8bpp masks are much faster,
    // better supported by hardware, and the memory savings are not worth
    // it anymore.

    if ((pReadAcc->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal)
        && pReadAcc->GetBestMatchingColor(COL_ALPHA_TRANSPARENT)
               == pReadAcc->GetBestMatchingColor(rTransColor))
    {
        // if we're a 1 bit pixel already, and the transcolor matches the color that would replace it
        // already, then just return a copy
        return AlphaMask(*this);
    }

    AlphaMask aNewBmp(GetSizePixel());
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return AlphaMask();

    const tools::Long nWidth = pReadAcc->Width();
    const tools::Long nHeight = pReadAcc->Height();
    const BitmapColor aOpaqueColor(pWriteAcc->GetBestMatchingColor(COL_ALPHA_OPAQUE));
    const BitmapColor aTransparentColor(pWriteAcc->GetBestMatchingColor(COL_ALPHA_TRANSPARENT));

    const BitmapColor aTest(pReadAcc->GetBestMatchingColor(rTransColor));

    if (pReadAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal)
    {
        // optimized for 8Bit source palette
        const sal_uInt8 cTest = aTest.GetIndex();

        for (tools::Long nY = 0; nY < nHeight; ++nY)
        {
            Scanline pSrc = pReadAcc->GetScanline(nY);
            Scanline pDst = pWriteAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; ++nX)
            {
                if (cTest == pSrc[nX])
                    pDst[nX] = aTransparentColor.GetIndex();
                else
                    pDst[nX] = aOpaqueColor.GetIndex();
            }
        }
    }
    else
    {
        // not optimized
        for (tools::Long nY = 0; nY < nHeight; ++nY)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; ++nX)
            {
                if (aTest == pReadAcc->GetPixelFromData(pScanlineRead, nX))
                    pWriteAcc->SetPixelOnData(pScanline, nX, aTransparentColor);
                else
                    pWriteAcc->SetPixelOnData(pScanline, nX, aOpaqueColor);
            }
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    aNewBmp.SetPrefSize(maPrefSize);
    aNewBmp.SetPrefMapMode(maPrefMapMode);

    return aNewBmp;
}

AlphaMask Bitmap::CreateAlphaMask(const Color& rTransColor, sal_uInt8 nTol) const
{
    if (nTol == 0)
        return CreateAlphaMask(rTransColor);

    BitmapScopedReadAccess pReadAcc(*this);
    if (!pReadAcc)
        return AlphaMask();

    // Historically LO used 1bpp masks, but 8bpp masks are much faster,
    // better supported by hardware, and the memory savings are not worth
    // it anymore.
    // TODO: Possibly remove the 1bpp code later.

    AlphaMask aNewBmp(GetSizePixel());
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return AlphaMask();

    const tools::Long nWidth = pReadAcc->Width();
    const tools::Long nHeight = pReadAcc->Height();
    const BitmapColor aOpaqueColor(pWriteAcc->GetBestMatchingColor(COL_ALPHA_OPAQUE));
    const BitmapColor aTransparentColor(pWriteAcc->GetBestMatchingColor(COL_ALPHA_TRANSPARENT));

    BitmapColor aCol;
    tools::Long nR, nG, nB;
    const tools::Long nMinR = std::clamp<tools::Long>(rTransColor.GetRed() - nTol, 0, 255);
    const tools::Long nMaxR = std::clamp<tools::Long>(rTransColor.GetRed() + nTol, 0, 255);
    const tools::Long nMinG = std::clamp<tools::Long>(rTransColor.GetGreen() - nTol, 0, 255);
    const tools::Long nMaxG = std::clamp<tools::Long>(rTransColor.GetGreen() + nTol, 0, 255);
    const tools::Long nMinB = std::clamp<tools::Long>(rTransColor.GetBlue() - nTol, 0, 255);
    const tools::Long nMaxB = std::clamp<tools::Long>(rTransColor.GetBlue() + nTol, 0, 255);

    if (pReadAcc->HasPalette())
    {
        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
            {
                aCol = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX));
                nR = aCol.GetRed();
                nG = aCol.GetGreen();
                nB = aCol.GetBlue();

                if (nMinR <= nR && nMaxR >= nR && nMinG <= nG && nMaxG >= nG && nMinB <= nB
                    && nMaxB >= nB)
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aTransparentColor);
                }
                else
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aOpaqueColor);
                }
            }
        }
    }
    else
    {
        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            Scanline pScanlineRead = pReadAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
            {
                aCol = pReadAcc->GetPixelFromData(pScanlineRead, nX);
                nR = aCol.GetRed();
                nG = aCol.GetGreen();
                nB = aCol.GetBlue();

                if (nMinR <= nR && nMaxR >= nR && nMinG <= nG && nMaxG >= nG && nMinB <= nB
                    && nMaxB >= nB)
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aTransparentColor);
                }
                else
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aOpaqueColor);
                }
            }
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    aNewBmp.SetPrefSize(maPrefSize);
    aNewBmp.SetPrefMapMode(maPrefMapMode);

    return aNewBmp;
}

vcl::Region Bitmap::CreateRegion(const Color& rColor, const tools::Rectangle& rRect) const
{
    tools::Rectangle aRect(rRect);
    BitmapScopedReadAccess pReadAcc(*this);

    aRect.Intersection(tools::Rectangle(Point(), GetSizePixel()));
    aRect.Normalize();

    if (!pReadAcc)
        return vcl::Region(aRect);

    vcl::Region aRegion;
    const tools::Long nLeft = aRect.Left();
    const tools::Long nTop = aRect.Top();
    const tools::Long nRight = aRect.Right();
    const tools::Long nBottom = aRect.Bottom();
    const BitmapColor aMatch(pReadAcc->GetBestMatchingColor(rColor));

    std::vector<tools::Long> aLine;
    tools::Long nYStart(nTop);
    tools::Long nY(nTop);

    for (; nY <= nBottom; nY++)
    {
        std::vector<tools::Long> aNewLine;
        tools::Long nX(nLeft);
        Scanline pScanlineRead = pReadAcc->GetScanline(nY);

        for (; nX <= nRight;)
        {
            while ((nX <= nRight) && (aMatch != pReadAcc->GetPixelFromData(pScanlineRead, nX)))
                nX++;

            if (nX <= nRight)
            {
                aNewLine.push_back(nX);

                while ((nX <= nRight) && (aMatch == pReadAcc->GetPixelFromData(pScanlineRead, nX)))
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
            aLine = std::move(aNewLine);
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

    return aRegion;
}

bool Bitmap::ReplaceMask(const AlphaMask& rMask, const Color& rReplaceColor)
{
    BitmapScopedReadAccess pMaskAcc(rMask);
    BitmapScopedWriteAccess pAcc(*this);

    if (!pMaskAcc || !pAcc)
        return false;

    const tools::Long nWidth = std::min(pMaskAcc->Width(), pAcc->Width());
    const tools::Long nHeight = std::min(pMaskAcc->Height(), pAcc->Height());
    const BitmapColor aMaskWhite(pMaskAcc->GetBestMatchingColor(COL_WHITE));
    BitmapColor aReplace;

    if (pAcc->HasPalette())
    {
        const sal_uInt16 nActColors = pAcc->GetPaletteEntryCount();
        const sal_uInt16 nMaxColors = 1 << pAcc->GetBitCount();

        aReplace = UpdatePaletteForNewColor(pAcc, nActColors, nMaxColors, nHeight, nWidth,
                                            BitmapColor(rReplaceColor));
    }
    else
        aReplace = rReplaceColor;

    for (tools::Long nY = 0; nY < nHeight; nY++)
    {
        Scanline pScanline = pAcc->GetScanline(nY);
        Scanline pScanlineMask = pMaskAcc->GetScanline(nY);
        for (tools::Long nX = 0; nX < nWidth; nX++)
        {
            if (pMaskAcc->GetPixelFromData(pScanlineMask, nX) == aMaskWhite)
                pAcc->SetPixelOnData(pScanline, nX, aReplace);
        }
    }

    return true;
}

bool Bitmap::Replace(const AlphaMask& rAlpha, const Color& rMergeColor)
{
    Bitmap aNewBmp(GetSizePixel(), vcl::PixelFormat::N24_BPP);
    BitmapScopedReadAccess pAcc(*this);
    BitmapScopedReadAccess pAlphaAcc(rAlpha);
    BitmapScopedWriteAccess pNewAcc(aNewBmp);

    if (!pAcc || !pAlphaAcc || !pNewAcc)
        return false;

    BitmapColor aCol;
    const tools::Long nWidth = std::min(pAlphaAcc->Width(), pAcc->Width());
    const tools::Long nHeight = std::min(pAlphaAcc->Height(), pAcc->Height());

    for (tools::Long nY = 0; nY < nHeight; nY++)
    {
        Scanline pScanline = pNewAcc->GetScanline(nY);
        Scanline pScanlineAlpha = pAlphaAcc->GetScanline(nY);
        for (tools::Long nX = 0; nX < nWidth; nX++)
        {
            aCol = pAcc->GetColor(nY, nX);
            aCol.Merge(rMergeColor, pAlphaAcc->GetIndexFromData(pScanlineAlpha, nX));
            pNewAcc->SetPixelOnData(pScanline, nX, aCol);
        }
    }

    pAcc.reset();
    pAlphaAcc.reset();
    pNewAcc.reset();

    const MapMode aMap(maPrefMapMode);
    const Size aSize(maPrefSize);

    *this = std::move(aNewBmp);

    maPrefMapMode = aMap;
    maPrefSize = aSize;

    return true;
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

    BitmapScopedWriteAccess pAcc(*this);
    if (!pAcc)
        return false;

    const tools::Long nMinR = std::clamp<tools::Long>(rSearchColor.GetRed() - nTol, 0, 255);
    const tools::Long nMaxR = std::clamp<tools::Long>(rSearchColor.GetRed() + nTol, 0, 255);
    const tools::Long nMinG = std::clamp<tools::Long>(rSearchColor.GetGreen() - nTol, 0, 255);
    const tools::Long nMaxG = std::clamp<tools::Long>(rSearchColor.GetGreen() + nTol, 0, 255);
    const tools::Long nMinB = std::clamp<tools::Long>(rSearchColor.GetBlue() - nTol, 0, 255);
    const tools::Long nMaxB = std::clamp<tools::Long>(rSearchColor.GetBlue() + nTol, 0, 255);

    if (pAcc->HasPalette())
    {
        for (sal_uInt16 i = 0, nPalCount = pAcc->GetPaletteEntryCount(); i < nPalCount; i++)
        {
            const BitmapColor& rCol = pAcc->GetPaletteColor(i);

            if (nMinR <= rCol.GetRed() && nMaxR >= rCol.GetRed() && nMinG <= rCol.GetGreen()
                && nMaxG >= rCol.GetGreen() && nMinB <= rCol.GetBlue() && nMaxB >= rCol.GetBlue())
            {
                pAcc->SetPaletteColor(i, rReplaceColor);
            }
        }
    }
    else
    {
        BitmapColor aCol;
        const BitmapColor aReplace(pAcc->GetBestMatchingColor(rReplaceColor));

        for (tools::Long nY = 0, nHeight = pAcc->Height(); nY < nHeight; nY++)
        {
            Scanline pScanline = pAcc->GetScanline(nY);
            for (tools::Long nX = 0, nWidth = pAcc->Width(); nX < nWidth; nX++)
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

    return true;
}

bool Bitmap::Replace(const Color* pSearchColors, const Color* pReplaceColors, size_t nColorCount,
                     sal_uInt8 const* pTols)
{
    BitmapScopedWriteAccess pAcc(*this);
    if (!pAcc)
        return false;

    std::vector<sal_uInt8> aMinR(nColorCount);
    std::vector<sal_uInt8> aMaxR(nColorCount);
    std::vector<sal_uInt8> aMinG(nColorCount);
    std::vector<sal_uInt8> aMaxG(nColorCount);
    std::vector<sal_uInt8> aMinB(nColorCount);
    std::vector<sal_uInt8> aMaxB(nColorCount);

    if (pTols)
    {
        for (size_t i = 0; i < nColorCount; ++i)
        {
            const Color& rCol = pSearchColors[i];
            const sal_uInt8 nTol = pTols[i];

            aMinR[i] = std::clamp(rCol.GetRed() - nTol, 0, 255);
            aMaxR[i] = std::clamp(rCol.GetRed() + nTol, 0, 255);
            aMinG[i] = std::clamp(rCol.GetGreen() - nTol, 0, 255);
            aMaxG[i] = std::clamp(rCol.GetGreen() + nTol, 0, 255);
            aMinB[i] = std::clamp(rCol.GetBlue() - nTol, 0, 255);
            aMaxB[i] = std::clamp(rCol.GetBlue() + nTol, 0, 255);
        }
    }
    else
    {
        for (size_t i = 0; i < nColorCount; ++i)
        {
            const Color& rCol = pSearchColors[i];

            aMinR[i] = rCol.GetRed();
            aMaxR[i] = rCol.GetRed();
            aMinG[i] = rCol.GetGreen();
            aMaxG[i] = rCol.GetGreen();
            aMinB[i] = rCol.GetBlue();
            aMaxB[i] = rCol.GetBlue();
        }
    }

    if (pAcc->HasPalette())
    {
        for (sal_uInt16 nEntry = 0, nPalCount = pAcc->GetPaletteEntryCount(); nEntry < nPalCount;
             nEntry++)
        {
            const BitmapColor& rCol = pAcc->GetPaletteColor(nEntry);

            for (size_t i = 0; i < nColorCount; ++i)
            {
                if (aMinR[i] <= rCol.GetRed() && aMaxR[i] >= rCol.GetRed()
                    && aMinG[i] <= rCol.GetGreen() && aMaxG[i] >= rCol.GetGreen()
                    && aMinB[i] <= rCol.GetBlue() && aMaxB[i] >= rCol.GetBlue())
                {
                    pAcc->SetPaletteColor(nEntry, pReplaceColors[i]);
                    break;
                }
            }
        }
    }
    else
    {
        std::vector<BitmapColor> aReplaces(nColorCount);

        for (size_t i = 0; i < nColorCount; ++i)
            aReplaces[i] = pAcc->GetBestMatchingColor(pReplaceColors[i]);

        for (tools::Long nY = 0, nHeight = pAcc->Height(); nY < nHeight; nY++)
        {
            Scanline pScanline = pAcc->GetScanline(nY);
            for (tools::Long nX = 0, nWidth = pAcc->Width(); nX < nWidth; nX++)
            {
                BitmapColor aCol = pAcc->GetPixelFromData(pScanline, nX);

                for (size_t i = 0; i < nColorCount; ++i)
                {
                    if (aMinR[i] <= aCol.GetRed() && aMaxR[i] >= aCol.GetRed()
                        && aMinG[i] <= aCol.GetGreen() && aMaxG[i] >= aCol.GetGreen()
                        && aMinB[i] <= aCol.GetBlue() && aMaxB[i] >= aCol.GetBlue())
                    {
                        pAcc->SetPixelOnData(pScanline, nX, aReplaces[i]);
                        break;
                    }
                }
            }
        }
    }

    pAcc.reset();

    return true;
}

// TODO: Have a look at OutputDevice::ImplDrawAlpha() for some
// optimizations. Might even consolidate the code here and there.
bool Bitmap::Blend(const AlphaMask& rAlpha, const Color& rBackgroundColor)
{
    // Convert to a truecolor bitmap, if we're a paletted one. There's room for tradeoff decision here,
    // maybe later for an overload (or a flag)
    if (vcl::isPalettePixelFormat(getPixelFormat()))
        Convert(BmpConversion::N24Bit);

    BitmapScopedReadAccess pAlphaAcc(rAlpha);

    BitmapScopedWriteAccess pAcc(*this);

    if (!pAlphaAcc || !pAcc)
        return false;

    const tools::Long nWidth = std::min(pAlphaAcc->Width(), pAcc->Width());
    const tools::Long nHeight = std::min(pAlphaAcc->Height(), pAcc->Height());

    for (tools::Long nY = 0; nY < nHeight; ++nY)
    {
        Scanline pScanline = pAcc->GetScanline(nY);
        Scanline pScanlineAlpha = pAlphaAcc->GetScanline(nY);
        for (tools::Long nX = 0; nX < nWidth; ++nX)
        {
            BitmapColor aBmpColor = pAcc->GetPixelFromData(pScanline, nX);
            aBmpColor.Merge(rBackgroundColor, pAlphaAcc->GetIndexFromData(pScanlineAlpha, nX));
            pAcc->SetPixelOnData(pScanline, nX, aBmpColor);
        }
    }

    return true;
}

static BitmapColor UpdatePaletteForNewColor(BitmapScopedWriteAccess& pAcc,
                                            const sal_uInt16 nActColors,
                                            const sal_uInt16 nMaxColors, const tools::Long nHeight,
                                            const tools::Long nWidth,
                                            const BitmapColor& rWantedColor)
{
    // default to the nearest color
    sal_uInt16 aReplacePalIndex = pAcc->GetMatchingPaletteIndex(rWantedColor);
    if (aReplacePalIndex != SAL_MAX_UINT16)
        return BitmapColor(static_cast<sal_uInt8>(aReplacePalIndex));

    // for paletted images without a matching palette entry

    // if the palette has empty entries use the last one
    if (nActColors < nMaxColors)
    {
        pAcc->SetPaletteEntryCount(nActColors + 1);
        pAcc->SetPaletteColor(nActColors, rWantedColor);
        return BitmapColor(static_cast<sal_uInt8>(nActColors));
    }

    // look for an unused palette entry (NOTE: expensive!)
    std::unique_ptr<bool[]> pFlags(new bool[nMaxColors]);

    // Set all entries to false
    std::fill(pFlags.get(), pFlags.get() + nMaxColors, false);

    for (tools::Long nY = 0; nY < nHeight; nY++)
    {
        Scanline pScanline = pAcc->GetScanline(nY);
        for (tools::Long nX = 0; nX < nWidth; nX++)
            pFlags[pAcc->GetIndexFromData(pScanline, nX)] = true;
    }

    for (sal_uInt16 i = 0; i < nMaxColors; i++)
    {
        // Hurray, we do have an unused entry
        if (!pFlags[i])
        {
            pAcc->SetPaletteColor(i, rWantedColor);
            return BitmapColor(static_cast<sal_uInt8>(i));
        }
    }
    assert(false && "found nothing");
    return BitmapColor(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
