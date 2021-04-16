/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmapex.hxx>
#include <vcl/BitmapAlphaClampFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

BitmapEx BitmapAlphaClampFilter::execute(BitmapEx const& rBitmapEx) const
{
    if (!rBitmapEx.IsAlpha())
        return rBitmapEx;

    AlphaMask aBitmapAlpha(rBitmapEx.GetAlpha());
    {
        AlphaScopedWriteAccess pWriteAlpha(aBitmapAlpha);
        const Size aSize(rBitmapEx.GetSizePixel());

        for (sal_Int32 nY = 0; nY < sal_Int32(aSize.Height()); ++nY)
        {
            Scanline pScanAlpha = pWriteAlpha->GetScanline(nY);

            for (sal_Int32 nX = 0; nX < sal_Int32(aSize.Width()); ++nX)
            {
                BitmapColor aBitmapAlphaValue(pWriteAlpha->GetPixelFromData(pScanAlpha, nX));
                if ((255 - aBitmapAlphaValue.GetIndex()) > mcThreshold)
                {
                    aBitmapAlphaValue.SetIndex(0);
                    pWriteAlpha->SetPixelOnData(pScanAlpha, nX, aBitmapAlphaValue);
                }
            }
        }
    }

    return BitmapEx(rBitmapEx.GetBitmap(), aBitmapAlpha);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
