/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <tools/helpers.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapEmbossGreyFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

#include <algorithm>

BitmapEx BitmapEmbossGreyFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    bool bRet = aBitmap.ImplMakeGreyscales(256);

    if (bRet)
    {
        bRet = false;

        Bitmap::ScopedReadAccess pReadAcc(aBitmap);

        if (pReadAcc)
        {
            Bitmap aNewBmp(aBitmap.GetSizePixel(), vcl::PixelFormat::N8_BPP,
                           &pReadAcc->GetPalette());
            BitmapScopedWriteAccess pWriteAcc(aNewBmp);

            if (pWriteAcc)
            {
                BitmapColor aGrey(sal_uInt8(0));
                const tools::Long nWidth = pWriteAcc->Width();
                const tools::Long nHeight = pWriteAcc->Height();
                tools::Long nGrey11, nGrey12, nGrey13;
                tools::Long nGrey21, nGrey22, nGrey23;
                tools::Long nGrey31, nGrey32, nGrey33;
                double fAzim = basegfx::deg2rad(mnAzimuthAngle100 * 0.01);
                double fElev = basegfx::deg2rad(mnElevationAngle100 * 0.01);
                std::unique_ptr<tools::Long[]> pHMap(new tools::Long[nWidth + 2]);
                std::unique_ptr<tools::Long[]> pVMap(new tools::Long[nHeight + 2]);
                tools::Long nX, nY, nNx, nNy, nDotL;
                const tools::Long nLx = FRound(cos(fAzim) * cos(fElev) * 255.0);
                const tools::Long nLy = FRound(sin(fAzim) * cos(fElev) * 255.0);
                const tools::Long nLz = FRound(sin(fElev) * 255.0);
                const auto nZ2 = ((6 * 255) / 4) * ((6 * 255) / 4);
                const tools::Long nNzLz = ((6 * 255) / 4) * nLz;
                const sal_uInt8 cLz
                    = static_cast<sal_uInt8>(std::clamp(nLz, tools::Long(0), tools::Long(255)));

                // fill mapping tables
                pHMap[0] = 0;

                for (nX = 1; nX <= nWidth; nX++)
                {
                    pHMap[nX] = nX - 1;
                }

                pHMap[nWidth + 1] = nWidth - 1;

                pVMap[0] = 0;

                for (nY = 1; nY <= nHeight; nY++)
                {
                    pVMap[nY] = nY - 1;
                }

                pVMap[nHeight + 1] = nHeight - 1;

                for (nY = 0; nY < nHeight; nY++)
                {
                    nGrey11 = pReadAcc->GetPixel(pVMap[nY], pHMap[0]).GetIndex();
                    nGrey12 = pReadAcc->GetPixel(pVMap[nY], pHMap[1]).GetIndex();
                    nGrey13 = pReadAcc->GetPixel(pVMap[nY], pHMap[2]).GetIndex();
                    nGrey21 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[0]).GetIndex();
                    nGrey22 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[1]).GetIndex();
                    nGrey23 = pReadAcc->GetPixel(pVMap[nY + 1], pHMap[2]).GetIndex();
                    nGrey31 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[0]).GetIndex();
                    nGrey32 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[1]).GetIndex();
                    nGrey33 = pReadAcc->GetPixel(pVMap[nY + 2], pHMap[2]).GetIndex();

                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    for (nX = 0; nX < nWidth; nX++)
                    {
                        nNx = nGrey11 + nGrey21 + nGrey31 - nGrey13 - nGrey23 - nGrey33;
                        nNy = nGrey31 + nGrey32 + nGrey33 - nGrey11 - nGrey12 - nGrey13;

                        if (!nNx && !nNy)
                        {
                            aGrey.SetIndex(cLz);
                        }
                        else if ((nDotL = nNx * nLx + nNy * nLy + nNzLz) < 0)
                        {
                            aGrey.SetIndex(0);
                        }
                        else
                        {
                            const double fGrey
                                = nDotL / sqrt(static_cast<double>(nNx * nNx + nNy * nNy + nZ2));
                            aGrey.SetIndex(static_cast<sal_uInt8>(std::clamp(fGrey, 0.0, 255.0)));
                        }

                        pWriteAcc->SetPixelOnData(pScanline, nX, aGrey);

                        if (nX < (nWidth - 1))
                        {
                            const tools::Long nNextX = pHMap[nX + 3];

                            nGrey11 = nGrey12;
                            nGrey12 = nGrey13;
                            nGrey13 = pReadAcc->GetPixel(pVMap[nY], nNextX).GetIndex();
                            nGrey21 = nGrey22;
                            nGrey22 = nGrey23;
                            nGrey23 = pReadAcc->GetPixel(pVMap[nY + 1], nNextX).GetIndex();
                            nGrey31 = nGrey32;
                            nGrey32 = nGrey33;
                            nGrey33 = pReadAcc->GetPixel(pVMap[nY + 2], nNextX).GetIndex();
                        }
                    }
                }

                pHMap.reset();
                pVMap.reset();
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
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
