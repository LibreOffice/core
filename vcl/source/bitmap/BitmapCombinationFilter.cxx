/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/color.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapCombinationFilter.hxx>
#include <vcl/bitmapaccess.hxx>

#include <bitmapwriteaccess.hxx>

#include <cstdlib>

BitmapEx BitmapCombinationFilter::execute(BitmapEx const& aBitmapEx)
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();

    Bitmap::ScopedReadAccess pMaskAcc(const_cast<Bitmap&>(mrMask));
    BitmapScopedWriteAccess pAcc(aBitmap);
    bool bRet = false;

    if (pMaskAcc && pAcc)
    {
        const long nWidth = std::min(pMaskAcc->Width(), pAcc->Width());
        const long nHeight = std::min(pMaskAcc->Height(), pAcc->Height());
        const Color aColBlack(COL_BLACK);
        BitmapColor aPixel;
        BitmapColor aMaskPixel;
        const BitmapColor aWhite(pAcc->GetBestMatchingColor(COL_WHITE));
        const BitmapColor aBlack(pAcc->GetBestMatchingColor(aColBlack));
        const BitmapColor aMaskBlack(pMaskAcc->GetBestMatchingColor(aColBlack));

        switch (meCombine)
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

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
