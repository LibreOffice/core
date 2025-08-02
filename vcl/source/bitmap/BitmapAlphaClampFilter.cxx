/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/bitmap/BitmapAlphaClampFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

Bitmap BitmapAlphaClampFilter::execute(Bitmap const& rBitmap) const
{
    if (!rBitmap.HasAlpha())
        return rBitmap;

    Bitmap aNewBitmap(rBitmap);
    {
        BitmapScopedWriteAccess pWriteAcc(aNewBitmap);
        const Size aSize(aNewBitmap.GetSizePixel());

        for (sal_Int32 nY = 0; nY < sal_Int32(aSize.Height()); ++nY)
        {
            Scanline pScan = pWriteAcc->GetScanline(nY);

            for (sal_Int32 nX = 0; nX < sal_Int32(aSize.Width()); ++nX)
            {
                BitmapColor aBitmapValue(pWriteAcc->GetPixelFromData(pScan, nX));
                if ((255 - aBitmapValue.GetAlpha()) > mcThreshold)
                {
                    aBitmapValue.SetAlpha(0);
                    pWriteAcc->SetPixelOnData(pScan, nX, aBitmapValue);
                }
            }
        }
    }

    return aNewBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
