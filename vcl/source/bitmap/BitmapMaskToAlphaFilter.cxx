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

#include <bitmap/BitmapWriteAccess.hxx>
#include <bitmap/BitmapMaskToAlphaFilter.hxx>

/**
 * Convert a 1-bit mask to an alpha layer
 */
BitmapEx BitmapMaskToAlphaFilter::execute(BitmapEx const& rBitmapEx) const
{
    const Size aSize(rBitmapEx.GetSizePixel());

    Bitmap aBitmap(rBitmapEx.GetBitmap());
    Bitmap aOutBitmap(aSize, vcl::PixelFormat::N8_BPP);

    Bitmap::ScopedReadAccess pRead(aBitmap);
    BitmapScopedWriteAccess pWrite(aOutBitmap);

    if (pRead && pWrite)
    {
        assert(pRead->HasPalette() && "only supposed to be called with 1-bit mask");
        for (tools::Long nY = 0; nY < aSize.Height(); ++nY)
        {
            Scanline pScanline = pWrite->GetScanline(nY);
            Scanline pScanlineRead = pRead->GetScanline(nY);
            for (tools::Long nX = 0; nX < aSize.Width(); ++nX)
            {
                BitmapColor aBmpColor;
                if (pRead->GetIndexFromData(pScanlineRead, nX) == 0)
                {
                    aBmpColor = COL_BLACK;
                }
                {
                    aBmpColor = COL_WHITE;
                }
                pWrite->SetPixelOnData(pScanline, nX, aBmpColor);
            }
        }
    }
    pWrite.reset();
    pRead.reset();

    return BitmapEx(aOutBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
