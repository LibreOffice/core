/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <basegfx/color/bcolortools.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapConvolutionMatrixFilter.hxx>
#include <vcl/BitmapSharpenFilter.hxx>

#include <vcl/bitmapwriteaccess.hxx>

BitmapEx BitmapConvolutionMatrixFilter::execute(BitmapEx const& rBitmapEx)
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    const long nDivisor = 8;
    Bitmap::ScopedReadAccess pReadAcc(aBitmap);
    bool bRet = false;

    if (pReadAcc)
    {
        Bitmap aNewBmp(aBitmap.GetSizePixel(), 24);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const long nWidth = pWriteAcc->Width(), nWidth2 = nWidth + 2;
            const long nHeight = pWriteAcc->Height(), nHeight2 = nHeight + 2;
            long* pColm = new long[nWidth2];
            long* pRows = new long[nHeight2];
            BitmapColor* pColRow1
                = reinterpret_cast<BitmapColor*>(new sal_uInt8[sizeof(BitmapColor) * nWidth2]);
            BitmapColor* pColRow2
                = reinterpret_cast<BitmapColor*>(new sal_uInt8[sizeof(BitmapColor) * nWidth2]);
            BitmapColor* pColRow3
                = reinterpret_cast<BitmapColor*>(new sal_uInt8[sizeof(BitmapColor) * nWidth2]);
            BitmapColor* pRowTmp1 = pColRow1;
            BitmapColor* pRowTmp2 = pColRow2;
            BitmapColor* pRowTmp3 = pColRow3;
            BitmapColor* pColor;
            long nY, nX, i, nSumR, nSumG, nSumB, nMatrixVal, nTmp;
            long(*pKoeff)[256] = new long[9][256];
            long* pTmp;

            // create LUT of products of matrix value and possible color component values
            for (nY = 0; nY < 9; nY++)
            {
                for (nX = nTmp = 0, nMatrixVal = mrMatrix[nY]; nX < 256; nX++, nTmp += nMatrixVal)
                {
                    pKoeff[nY][nX] = nTmp;
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
                    nSumR = (pTmp = pKoeff[0])[(pColor = pRowTmp1 + nX)->GetRed()];
                    nSumG = pTmp[pColor->GetGreen()];
                    nSumB = pTmp[pColor->GetBlue()];

                    nSumR += (pTmp = pKoeff[1])[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    nSumR += (pTmp = pKoeff[2])[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    // second row
                    nSumR += (pTmp = pKoeff[3])[(pColor = pRowTmp2 + nX)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    nSumR += (pTmp = pKoeff[4])[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    nSumR += (pTmp = pKoeff[5])[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    // third row
                    nSumR += (pTmp = pKoeff[6])[(pColor = pRowTmp3 + nX)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    nSumR += (pTmp = pKoeff[7])[(++pColor)->GetRed()];
                    nSumG += pTmp[pColor->GetGreen()];
                    nSumB += pTmp[pColor->GetBlue()];

                    nSumR += (pTmp = pKoeff[8])[(++pColor)->GetRed()];
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
                    if (pRowTmp1 == pColRow1)
                    {
                        pRowTmp1 = pColRow2;
                        pRowTmp2 = pColRow3;
                        pRowTmp3 = pColRow1;
                    }
                    else if (pRowTmp1 == pColRow2)
                    {
                        pRowTmp1 = pColRow3;
                        pRowTmp2 = pColRow1;
                        pRowTmp3 = pColRow2;
                    }
                    else
                    {
                        pRowTmp1 = pColRow1;
                        pRowTmp2 = pColRow2;
                        pRowTmp3 = pColRow3;
                    }

                    for (i = 0; i < nWidth2; i++)
                    {
                        pRowTmp3[i] = pReadAcc->GetColor(pRows[nY + 2], pColm[i]);
                    }
                }
            }

            delete[] pKoeff;
            delete[] reinterpret_cast<sal_uInt8*>(pColRow1);
            delete[] reinterpret_cast<sal_uInt8*>(pColRow2);
            delete[] reinterpret_cast<sal_uInt8*>(pColRow3);
            delete[] pColm;
            delete[] pRows;

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

static const long g_SharpenMatrix[] = { -1, -1, -1, -1, 16, -1, -1, -1, -1 };

BitmapSharpenFilter::BitmapSharpenFilter()
    : BitmapConvolutionMatrixFilter(g_SharpenMatrix)
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
