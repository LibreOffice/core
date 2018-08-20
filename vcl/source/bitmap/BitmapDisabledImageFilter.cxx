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

#include <vcl/bitmapaccess.hxx>

#include <vcl/bitmapwriteaccess.hxx>
#include <BitmapDisabledImageFilter.hxx>

BitmapEx BitmapDisabledImageFilter::execute(BitmapEx const& rBitmapEx)
{
    const Size aSize(rBitmapEx.GetSizePixel());

    // keep disable image at same depth as original where possible, otherwise
    // use 8 bit
    sal_uInt16 nBitCount = rBitmapEx.GetBitCount();
    if (nBitCount < 8)
        nBitCount = 8;
    const BitmapPalette* pPal = nBitCount == 8 ? &Bitmap::GetGreyPalette(256) : nullptr;
    Bitmap aGrey(aSize, nBitCount, pPal);

    AlphaMask aGreyAlpha(aSize);

    Bitmap aBitmap(rBitmapEx.GetBitmap());
    Bitmap::ScopedReadAccess pRead(aBitmap);

    BitmapScopedWriteAccess pGrey(aGrey);
    AlphaScopedWriteAccess pGreyAlpha(aGreyAlpha);

    BitmapEx aReturnBitmap;

    if (rBitmapEx.IsTransparent())
    {
        AlphaMask aBitmapAlpha(rBitmapEx.GetAlpha());
        AlphaMask::ScopedReadAccess pReadAlpha(aBitmapAlpha);

        if (pRead && pReadAlpha && pGrey && pGreyAlpha)
        {
            BitmapColor aGreyAlphaValue(0);

            for (long nY = 0; nY < aSize.Height(); ++nY)
            {
                Scanline pScanAlpha = pGreyAlpha->GetScanline(nY);
                Scanline pScanline = pGrey->GetScanline(nY);
                Scanline pScanReadAlpha = pReadAlpha->GetScanline(nY);

                for (long nX = 0; nX < aSize.Width(); ++nX)
                {
                    const sal_uInt8 nLum(pRead->GetLuminance(nY, nX));
                    BitmapColor aGreyValue(nLum, nLum, nLum);
                    pGrey->SetPixelOnData(pScanline, nX, aGreyValue);

                    const BitmapColor aBitmapAlphaValue(
                        pReadAlpha->GetPixelFromData(pScanReadAlpha, nX));

                    aGreyAlphaValue.SetIndex(
                        sal_uInt8(std::min(aBitmapAlphaValue.GetIndex() + 178ul, 255ul)));
                    pGreyAlpha->SetPixelOnData(pScanAlpha, nX, aGreyAlphaValue);
                }
            }
        }

        pReadAlpha.reset();
        aReturnBitmap = BitmapEx(aGrey, aGreyAlpha);
    }
    else
    {
        if (pRead && pGrey && pGreyAlpha)
        {
            BitmapColor aGreyAlphaValue(0);

            for (long nY = 0; nY < aSize.Height(); ++nY)
            {
                Scanline pScanAlpha = pGreyAlpha->GetScanline(nY);
                Scanline pScanline = pGrey->GetScanline(nY);

                for (long nX = 0; nX < aSize.Width(); ++nX)
                {
                    const sal_uInt8 nLum(pRead->GetLuminance(nY, nX));
                    BitmapColor aGreyValue(nLum, nLum, nLum);
                    pGrey->SetPixelOnData(pScanline, nX, aGreyValue);

                    aGreyAlphaValue.SetIndex(128);
                    pGreyAlpha->SetPixelOnData(pScanAlpha, nX, aGreyAlphaValue);
                }
            }
        }

        aReturnBitmap = BitmapEx(aGrey);
    }

    pRead.reset();
    pGrey.reset();
    pGreyAlpha.reset();

    return aReturnBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
