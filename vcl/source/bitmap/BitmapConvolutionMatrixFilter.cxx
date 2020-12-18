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

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapConvolutionMatrixFilter.hxx>
#include <vcl/BitmapSharpenFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

#include <array>

BitmapEx BitmapConvolutionMatrixFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const tools::Long nDivisor = 8;
    Bitmap::ScopedReadAccess pReadAcc(aBitmap);
    bool bRet = false;

    if (pReadAcc)
    {
        Bitmap aNewBmp(aBitmap.GetSizePixel(), 24);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const tools::Long nWidth = pWriteAcc->Width(), nWidth2 = nWidth + 2;
            const tools::Long nHeight = pWriteAcc->Height(), nHeight2 = nHeight + 2;
            std::unique_ptr<tools::Long[]> pColm(new tools::Long[nWidth2]);
            std::unique_ptr<tools::Long[]> pRows(new tools::Long[nHeight2]);
            std::unique_ptr<BitmapColor[]> pColRow1(new BitmapColor[nWidth2]);
            std::unique_ptr<BitmapColor[]> pColRow2(new BitmapColor[nWidth2]);
            std::unique_ptr<BitmapColor[]> pColRow3(new BitmapColor[nWidth2]);
            BitmapColor* pRowTmp1 = pColRow1.get();
            BitmapColor* pRowTmp2 = pColRow2.get();
            BitmapColor* pRowTmp3 = pColRow3.get();
            BitmapColor* pColor;
            tools::Long nY, nX, i, nSumR, nSumG, nSumB, nMatrixVal, nTmp;
            std::array<std::array<sal_Int32, 256>, 9> aKoeff;
            sal_Int32* pTmp;

            // create LUT of products of matrix value and possible color component values
            for (nY = 0; nY < 9; nY++)
            {
                for (nX = nTmp = 0, nMatrixVal = mrMatrix[nY]; nX < 256; nX++, nTmp += nMatrixVal)
                {
                    aKoeff[nY][nX] = nTmp;
                }
            }

            // create column LUT
            for (i = 0; i < nWidth2; i++)
            {
                pColm[i] = (i > 0) ? (i - 1) : 0;
            }

            pColm[nWidth + 1] = pColm[nWidth];

            // create row LUT
            for (i = 0; i < nHeight2; i++)
            {
                pRows[i] = (i > 0) ? (i - 1) : 0;
            }

            pRows[nHeight + 1] = pRows[nHeight];

            // read first three rows of bitmap color
            for (i = 0; i < nWidth2; i++)
            {
                pColRow1[i] = pReadAcc->GetColor(pRows[0], pColm[i]);
                pColRow2[i] = pReadAcc->GetColor(pRows[1], pColm[i]);
                pColRow3[i] = pReadAcc->GetColor(pRows[2], pColm[i]);
            }

            // do convolution
            for (nY = 0; nY < nHeight;)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                for (nX = 0; nX < nWidth; nX++)
                {
                    // first row
                    pTmp = aKoeff[0].data();
                    pColor = pRowTmp1 + nX;
                    nSumR = pTmp[pColor->GetRed()];
                    nSumG = pTmp[pColor->GetGreen()];
                    nSumB = pTmp[pColor->GetBlue()];

                    pTmp = aKoeff[1].data();
                    nSumR += pTmp[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    pTmp = aKoeff[2].data();
                    nSumR += pTmp[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    // second row
                    pTmp = aKoeff[3].data();
                    pColor = pRowTmp2 + nX;
                    nSumR += pTmp[pColor->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    pTmp = aKoeff[4].data();
                    nSumR += pTmp[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    pTmp = aKoeff[5].data();
                    nSumR += pTmp[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    // third row
                    pTmp = aKoeff[6].data();
                    pColor = pRowTmp3 + nX;
                    nSumR += pTmp[pColor->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    pTmp = aKoeff[7].data();
                    nSumR += pTmp[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    pTmp = aKoeff[8].data();
                    nSumR += pTmp[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    // calculate destination color
                    pWriteAcc->SetPixelOnData(
                        pScanline, nX,
                        BitmapColor(static_cast<sal_uInt8>(MinMax(nSumR / nDivisor, 0, 255)),
                                    static_cast<sal_uInt8>(MinMax(nSumG / nDivisor, 0, 255)),
                                    static_cast<sal_uInt8>(MinMax(nSumB / nDivisor, 0, 255))));
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

                    for (i = 0; i < nWidth2; i++)
                    {
                        pRowTmp3[i] = pReadAcc->GetColor(pRows[nY + 2], pColm[i]);
                    }
                }
            }

            pWriteAcc.reset();

            bRet = true;
        }

        pReadAcc.reset();

        if (bRet)
        {
            const MapMode aMap(aBitmap.GetPrefMapMode());
            const Size aPrefSize(aBitmap.GetPrefSize());

            aBitmap = aNewBmp;

            aBitmap.SetPrefMapMode(aMap);
            aBitmap.SetPrefSize(aPrefSize);
        }
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

const sal_Int32 g_SharpenMatrix[] = { -1, -1, -1, -1, 16, -1, -1, -1, -1 };

BitmapSharpenFilter::BitmapSharpenFilter()
    : BitmapConvolutionMatrixFilter(g_SharpenMatrix)
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
