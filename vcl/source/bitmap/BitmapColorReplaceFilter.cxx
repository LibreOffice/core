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
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapColorReplaceFilter.hxx>
#include <vcl/bitmapex.hxx>

#include <bitmapwriteaccess.hxx>

BitmapEx BitmapColorReplaceFilter::execute(BitmapEx const& aBitmapEx) const
{
    Bitmap aBitmap = aBitmapEx.GetBitmap();

    if (BitmapScopedWriteAccess pWriteAcc{ aBitmap })
    {
        const BitmapColor aReplaceWhat(pWriteAcc->GetBestMatchingColor(m_aReplaceWhat));
        const BitmapColor aReplaceTo(pWriteAcc->GetBestMatchingColor(m_aReplaceTo));
        const long nWidth = pWriteAcc->Width();
        const long nHeight = pWriteAcc->Height();

        for (long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pWriteAcc->GetScanline(nY);
            for (long nX = 0; nX < nWidth; nX++)
            {
                if (pWriteAcc->GetPixelFromData(pScanline, nX) == aReplaceWhat)
                {
                    pWriteAcc->SetPixelOnData(pScanline, nX, aReplaceTo);
                }
            }
        }
    }

    return BitmapEx(aBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
