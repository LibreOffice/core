/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/helpers.hxx>

#include <vcl/bitmap/BitmapConvolutionMatrixFilter.hxx>
#include <vcl/bitmap/BitmapSharpenFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <array>

Bitmap BitmapConvolutionMatrixFilter::execute(Bitmap const& rBitmap) const
{
    Bitmap aBitmap(rBitmap);

    BitmapScopedReadAccess pReadAcc(aBitmap);
    if (!pReadAcc)
        return Bitmap();

    Bitmap aNewBmp(aBitmap.GetSizePixel(), vcl::PixelFormat::N24_BPP);
    BitmapScopedWriteAccess pWriteAcc(aNewBmp);
    if (!pWriteAcc)
        return Bitmap();

    std::array<std::array<sal_Int32, 256>, 9> aCoeff;

    // create LUT of products of matrix value and possible color component values
    for (sal_Int32 nY = 0; nY < 9; nY++)
    {
        for (sal_Int32 nX = 0, nTmp = 0, nMatrixVal = mrMatrix[nY]; nX < 256;
             nX++, nTmp += nMatrixVal)
        {
            aCoeff[nY][nX] = nTmp;
        }
    }

    const sal_Int32 nWidth = pWriteAcc->Width();
    const sal_Int32 nWidth2 = nWidth + 2;
    std::unique_ptr<sal_Int32[]> pColm(new sal_Int32[nWidth2]);

    // create column LUT
    for (sal_Int32 nColIdx = 0; nColIdx < nWidth2; nColIdx++)
    {
        pColm[nColIdx] = (nColIdx > 0) ? (nColIdx - 1) : 0;
    }

    pColm[nWidth + 1] = pColm[nWidth];

    const sal_Int32 nHeight = pWriteAcc->Height();
    const sal_Int32 nHeight2 = nHeight + 2;
    std::unique_ptr<sal_Int32[]> pRows(new sal_Int32[nHeight2]);

    // create row LUT
    for (sal_Int32 nRowIdx = 0; nRowIdx < nHeight2; nRowIdx++)
    {
        pRows[nRowIdx] = (nRowIdx > 0) ? (nRowIdx - 1) : 0;
    }

    pRows[nHeight + 1] = pRows[nHeight];

    std::unique_ptr<BitmapColor[]> pColRow1(new BitmapColor[nWidth2]);
    std::unique_ptr<BitmapColor[]> pColRow2(new BitmapColor[nWidth2]);
    std::unique_ptr<BitmapColor[]> pColRow3(new BitmapColor[nWidth2]);
    BitmapColor* pRowTmp1 = pColRow1.get();
    BitmapColor* pRowTmp2 = pColRow2.get();
    BitmapColor* pRowTmp3 = pColRow3.get();

    // read first three rows of bitmap color
    for (sal_Int32 nRowIdx = 0; nRowIdx < nWidth2; nRowIdx++)
    {
        pColRow1[nRowIdx] = pReadAcc->GetColor(pRows[0], pColm[nRowIdx]);
        pColRow2[nRowIdx] = pReadAcc->GetColor(pRows[1], pColm[nRowIdx]);
        pColRow3[nRowIdx] = pReadAcc->GetColor(pRows[2], pColm[nRowIdx]);
    }

    // do convolution
    for (sal_Int32 nY = 0; nY < nHeight;)
    {
        Scanline pScanline = pWriteAcc->GetScanline(nY);
        for (sal_Int32 nX = 0; nX < nWidth; nX++)
        {
            // first row
            sal_Int32* pTmp = aCoeff[0].data();
            BitmapColor* pColor = pRowTmp1 + nX;
            sal_Int32 nSumR = pTmp[pColor->GetRed()];
            sal_Int32 nSumG = pTmp[pColor->GetGreen()];
            sal_Int32 nSumB = pTmp[pColor->GetBlue()];

            pTmp = aCoeff[1].data();
            nSumR += pTmp[(++pColor)->GetRed()];
            nSumG += pTmp[pColor->GetGreen()];
            nSumB += pTmp[pColor->GetBlue()];

            pTmp = aCoeff[2].data();
            nSumR += pTmp[(++pColor)->GetRed()];
            nSumG += pTmp[pColor->GetGreen()];
            nSumB += pTmp[pColor->GetBlue()];

            // second row
            pTmp = aCoeff[3].data();
            pColor = pRowTmp2 + nX;
            nSumR += pTmp[pColor->GetRed()];
            nSumG += pTmp[pColor->GetGreen()];
            nSumB += pTmp[pColor->GetBlue()];

            pTmp = aCoeff[4].data();
            nSumR += pTmp[(++pColor)->GetRed()];
            nSumG += pTmp[pColor->GetGreen()];
            nSumB += pTmp[pColor->GetBlue()];

            pTmp = aCoeff[5].data();
            nSumR += pTmp[(++pColor)->GetRed()];
            nSumG += pTmp[pColor->GetGreen()];
            nSumB += pTmp[pColor->GetBlue()];

            // third row
            pTmp = aCoeff[6].data();
            pColor = pRowTmp3 + nX;
            nSumR += pTmp[pColor->GetRed()];
            nSumG += pTmp[pColor->GetGreen()];
            nSumB += pTmp[pColor->GetBlue()];

            pTmp = aCoeff[7].data();
            nSumR += pTmp[(++pColor)->GetRed()];
            nSumG += pTmp[pColor->GetGreen()];
            nSumB += pTmp[pColor->GetBlue()];

            pTmp = aCoeff[8].data();
            nSumR += pTmp[(++pColor)->GetRed()];
            nSumG += pTmp[pColor->GetGreen()];
            nSumB += pTmp[pColor->GetBlue()];

            const sal_Int32 nDivisor = 8;

            // calculate destination color
            pWriteAcc->SetPixelOnData(
                pScanline, nX,
                BitmapColor(static_cast<sal_uInt8>(
                                std::clamp(nSumR / nDivisor, sal_Int32(0), sal_Int32(255))),
                            static_cast<sal_uInt8>(
                                std::clamp(nSumG / nDivisor, sal_Int32(0), sal_Int32(255))),
                            static_cast<sal_uInt8>(
                                std::clamp(nSumB / nDivisor, sal_Int32(0), sal_Int32(255)))));
        }

        if (++nY < nHeight)
        {
            if (pRowTmp1 == pColRow1.get())
            {
                pRowTmp1 = pColRow2.get();
                pRowTmp2 = pColRow3.get();
                pRowTmp3 = pColRow1.get();
            }
            else if (pRowTmp1 == pColRow2.get())
            {
                pRowTmp1 = pColRow3.get();
                pRowTmp2 = pColRow1.get();
                pRowTmp3 = pColRow2.get();
            }
            else
            {
                pRowTmp1 = pColRow1.get();
                pRowTmp2 = pColRow2.get();
                pRowTmp3 = pColRow3.get();
            }

            for (sal_Int32 i = 0; i < nWidth2; i++)
            {
                pRowTmp3[i] = pReadAcc->GetColor(pRows[nY + 2], pColm[i]);
            }
        }
    }

    pWriteAcc.reset();
    pReadAcc.reset();

    const MapMode aMap(aBitmap.GetPrefMapMode());
    const Size aPrefSize(aBitmap.GetPrefSize());

    aBitmap = std::move(aNewBmp);

    aBitmap.SetPrefMapMode(aMap);
    aBitmap.SetPrefSize(aPrefSize);

    return aBitmap;
}

const sal_Int32 g_SharpenMatrix[] = { -1, -1, -1, -1, 16, -1, -1, -1, -1 };

BitmapSharpenFilter::BitmapSharpenFilter()
    : BitmapConvolutionMatrixFilter(g_SharpenMatrix)
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
