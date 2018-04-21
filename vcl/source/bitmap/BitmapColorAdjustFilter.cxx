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
#include <vcl/BitmapColorAdjustFilter.hxx>
#include <vcl/bitmapaccess.hxx>

#include <bitmapwriteaccess.hxx>

#include <cstdlib>

BitmapEx BitmapColorAdjustFilter::execute(BitmapEx const& aBitmapEx)
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();

    bool bRet = true;

    // nothing to do => return quickly
    if (!(!mnLuminancePercent && !mnContrastPercent && !mnChannelRPercent && !mnChannelGPercent
          && !mnChannelBPercent && (mfGamma == 1.0) && !mbInvert))
    {
        bRet = false;

        BitmapScopedWriteAccess pAcc(aBitmap);

        if (pAcc)
        {
            BitmapColor aCol;
            const long nW = pAcc->Width();
            const long nH = pAcc->Height();
            std::unique_ptr<sal_uInt8[]> cMapR(new sal_uInt8[256]);
            std::unique_ptr<sal_uInt8[]> cMapG(new sal_uInt8[256]);
            std::unique_ptr<sal_uInt8[]> cMapB(new sal_uInt8[256]);
            double fM, fROff, fGOff, fBOff, fOff;

            // calculate slope
            if (mnContrastPercent >= 0)
                fM = 128.0 / (128.0 - 1.27 * MinMax(mnContrastPercent, 0, 100));
            else
                fM = (128.0 + 1.27 * MinMax(mnContrastPercent, -100, 0)) / 128.0;

            if (!mbMsoBrightness)
                // total offset = luminance offset + contrast offset
                fOff = MinMax(mnLuminancePercent, -100, 100) * 2.55 + 128.0 - fM * 128.0;
            else
                fOff = MinMax(mnLuminancePercent, -100, 100) * 2.55;

            // channel offset = channel offset + total offset
            fROff = mnChannelRPercent * 2.55 + fOff;
            fGOff = mnChannelGPercent * 2.55 + fOff;
            fBOff = mnChannelBPercent * 2.55 + fOff;

            // calculate gamma value
            mfGamma = (mfGamma <= 0.0 || mfGamma > 10.0) ? 1.0 : (1.0 / mfGamma);
            const bool bGamma = (mfGamma != 1.0);

            // create mapping table
            for (long nX = 0; nX < 256; nX++)
            {
                if (!mbMsoBrightness)
                {
                    cMapR[nX] = static_cast<sal_uInt8>(MinMax(FRound(nX * fM + fROff), 0, 255));
                    cMapG[nX] = static_cast<sal_uInt8>(MinMax(FRound(nX * fM + fGOff), 0, 255));
                    cMapB[nX] = static_cast<sal_uInt8>(MinMax(FRound(nX * fM + fBOff), 0, 255));
                }
                else
                {
                    // LO simply uses (in a somewhat optimized form) "newcolor = (oldcolor-128)*contrast+brightness+128"
                    // as the formula, i.e. contrast first, brightness afterwards. MSOffice, for whatever weird reason,
                    // use neither first, but apparently it applies half of brightness before contrast and half afterwards.
                    cMapR[nX] = static_cast<sal_uInt8>(
                        MinMax(FRound((nX + fROff / 2 - 128) * fM + 128 + fROff / 2), 0, 255));
                    cMapG[nX] = static_cast<sal_uInt8>(
                        MinMax(FRound((nX + fGOff / 2 - 128) * fM + 128 + fGOff / 2), 0, 255));
                    cMapB[nX] = static_cast<sal_uInt8>(
                        MinMax(FRound((nX + fBOff / 2 - 128) * fM + 128 + fBOff / 2), 0, 255));
                }
                if (bGamma)
                {
                    cMapR[nX] = GAMMA(cMapR[nX], mfGamma);
                    cMapG[nX] = GAMMA(cMapG[nX], mfGamma);
                    cMapB[nX] = GAMMA(cMapB[nX], mfGamma);
                }

                if (mbInvert)
                {
                    cMapR[nX] = ~cMapR[nX];
                    cMapG[nX] = ~cMapG[nX];
                    cMapB[nX] = ~cMapB[nX];
                }
            }

            // do modifying
            if (pAcc->HasPalette())
            {
                BitmapColor aNewCol;

                for (sal_uInt16 i = 0, nCount = pAcc->GetPaletteEntryCount(); i < nCount; i++)
                {
                    const BitmapColor& rCol = pAcc->GetPaletteColor(i);
                    aNewCol.SetRed(cMapR[rCol.GetRed()]);
                    aNewCol.SetGreen(cMapG[rCol.GetGreen()]);
                    aNewCol.SetBlue(cMapB[rCol.GetBlue()]);
                    pAcc->SetPaletteColor(i, aNewCol);
                }
            }
            else if (pAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr)
            {
                for (long nY = 0; nY < nH; nY++)
                {
                    Scanline pScan = pAcc->GetScanline(nY);

                    for (long nX = 0; nX < nW; nX++)
                    {
                        *pScan = cMapB[*pScan];
                        pScan++;
                        *pScan = cMapG[*pScan];
                        pScan++;
                        *pScan = cMapR[*pScan];
                        pScan++;
                    }
                }
            }
            else if (pAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb)
            {
                for (long nY = 0; nY < nH; nY++)
                {
                    Scanline pScan = pAcc->GetScanline(nY);

                    for (long nX = 0; nX < nW; nX++)
                    {
                        *pScan = cMapR[*pScan];
                        pScan++;
                        *pScan = cMapG[*pScan];
                        pScan++;
                        *pScan = cMapB[*pScan];
                        pScan++;
                    }
                }
            }
            else
            {
                for (long nY = 0; nY < nH; nY++)
                {
                    Scanline pScanline = pAcc->GetScanline(nY);
                    for (long nX = 0; nX < nW; nX++)
                    {
                        aCol = pAcc->GetPixelFromData(pScanline, nX);
                        aCol.SetRed(cMapR[aCol.GetRed()]);
                        aCol.SetGreen(cMapG[aCol.GetGreen()]);
                        aCol.SetBlue(cMapB[aCol.GetBlue()]);
                        pAcc->SetPixelOnData(pScanline, nX, aCol);
                    }
                }
            }

            pAcc.reset();
            bRet = true;
        }
    }

    if (bRet)
        return BitmapEx(aBitmap);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
