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
#include <vcl/BitmapMedianFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

#define S2(a, b)                                                                                   \
    {                                                                                              \
        tools::Long t;                                                                             \
        if ((t = b - a) < 0)                                                                       \
        {                                                                                          \
            a += t;                                                                                \
            b -= t;                                                                                \
        }                                                                                          \
    }
#define MN3(a, b, c)                                                                               \
    S2(a, b);                                                                                      \
    S2(a, c);
#define MX3(a, b, c)                                                                               \
    S2(b, c);                                                                                      \
    S2(a, c);
#define MNMX3(a, b, c)                                                                             \
    MX3(a, b, c);                                                                                  \
    S2(a, b);
#define MNMX4(a, b, c, d)                                                                          \
    S2(a, b);                                                                                      \
    S2(c, d);                                                                                      \
    S2(a, c);                                                                                      \
    S2(b, d);
#define MNMX5(a, b, c, d, e)                                                                       \
    S2(a, b);                                                                                      \
    S2(c, d);                                                                                      \
    MN3(a, c, e);                                                                                  \
    MX3(b, d, e);
#define MNMX6(a, b, c, d, e, f)                                                                    \
    S2(a, d);                                                                                      \
    S2(b, e);                                                                                      \
    S2(c, f);                                                                                      \
    MN3(a, b, c);                                                                                  \
    MX3(d, e, f);

BitmapEx BitmapMedianFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

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
            tools::Long nY, nX, i;
            tools::Long nR1, nR2, nR3, nR4, nR5, nR6, nR7, nR8, nR9;
            tools::Long nG1, nG2, nG3, nG4, nG5, nG6, nG7, nG8, nG9;
            tools::Long nB1, nB2, nB3, nB4, nB5, nB6, nB7, nB8, nB9;

            // create column LUT
            for (i = 0; i < nWidth2; i++)
                pColm[i] = (i > 0) ? (i - 1) : 0;

            pColm[nWidth + 1] = pColm[nWidth];

            // create row LUT
            for (i = 0; i < nHeight2; i++)
                pRows[i] = (i > 0) ? (i - 1) : 0;

            pRows[nHeight + 1] = pRows[nHeight];

            // read first three rows of bitmap color
            if (nHeight2 > 2)
            {
                for (i = 0; i < nWidth2; i++)
                {
                    pColRow1[i] = pReadAcc->GetColor(pRows[0], pColm[i]);
                    pColRow2[i] = pReadAcc->GetColor(pRows[1], pColm[i]);
                    pColRow3[i] = pReadAcc->GetColor(pRows[2], pColm[i]);
                }
            }

            // do median filtering
            for (nY = 0; nY < nHeight;)
            {
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                for (nX = 0; nX < nWidth; nX++)
                {
                    pColor = pRowTmp1 + nX;
                    nR1 = pColor->GetRed();
                    nG1 = pColor->GetGreen();
                    nB1 = pColor->GetBlue();
                    nR2 = (++pColor)->GetRed();
                    nG2 = pColor->GetGreen();
                    nB2 = pColor->GetBlue();
                    nR3 = (++pColor)->GetRed();
                    nG3 = pColor->GetGreen();
                    nB3 = pColor->GetBlue();

                    pColor = pRowTmp2 + nX;
                    nR4 = pColor->GetRed();
                    nG4 = pColor->GetGreen();
                    nB4 = pColor->GetBlue();
                    nR5 = (++pColor)->GetRed();
                    nG5 = pColor->GetGreen();
                    nB5 = pColor->GetBlue();
                    nR6 = (++pColor)->GetRed();
                    nG6 = pColor->GetGreen();
                    nB6 = pColor->GetBlue();

                    pColor = pRowTmp3 + nX;
                    nR7 = pColor->GetRed();
                    nG7 = pColor->GetGreen();
                    nB7 = pColor->GetBlue();
                    nR8 = (++pColor)->GetRed();
                    nG8 = pColor->GetGreen();
                    nB8 = pColor->GetBlue();
                    nR9 = (++pColor)->GetRed();
                    nG9 = pColor->GetGreen();
                    nB9 = pColor->GetBlue();

                    MNMX6(nR1, nR2, nR3, nR4, nR5, nR6);
                    MNMX5(nR7, nR2, nR3, nR4, nR5);
                    MNMX4(nR8, nR2, nR3, nR4);
                    MNMX3(nR9, nR2, nR3);

                    MNMX6(nG1, nG2, nG3, nG4, nG5, nG6);
                    MNMX5(nG7, nG2, nG3, nG4, nG5);
                    MNMX4(nG8, nG2, nG3, nG4);
                    MNMX3(nG9, nG2, nG3);

                    MNMX6(nB1, nB2, nB3, nB4, nB5, nB6);
                    MNMX5(nB7, nB2, nB3, nB4, nB5);
                    MNMX4(nB8, nB2, nB3, nB4);
                    MNMX3(nB9, nB2, nB3);

                    // set destination color
                    pWriteAcc->SetPixelOnData(pScanline, nX,
                                              BitmapColor(static_cast<sal_uInt8>(nR2),
                                                          static_cast<sal_uInt8>(nG2),
                                                          static_cast<sal_uInt8>(nB2)));
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
                        pRowTmp3[i] = pReadAcc->GetColor(pRows[nY + 2], pColm[i]);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
