/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapCropper.hxx>

/** Crop the bitmap

    @param rRectPixel
    A rectangle specifying the crop amounts on all four sides of
    the bitmap. If the upper left corner of the bitmap is assigned
    (0,0), then this method cuts out the given rectangle from the
    bitmap. Note that the rectangle is clipped to the bitmap's
    dimension, i.e. negative left,top rectangle coordinates or
    exceeding width or height is ignored.
 */
BitmapEx BitmapCropper::execute(BitmapEx const& rBitmapEx)
{
    Bitmap aBitmap = rBitmapEx.GetBitmap();

    bool bRet = aBitmap.Crop(maCropRect);

    Bitmap aMask = rBitmapEx.GetMask();

    if (bRet && (rBitmapEx.GetTransparentType() == TransparentType::Bitmap) && !!aMask)
        bRet = aMask.Crop(maCropRect);

    SAL_WARN_IF(!!aMask && aBitmap.GetSizePixel() != aMask.GetSizePixel(), "vcl",
                "BitmapEx::Crop(): size mismatch for bitmap and alpha mask.");

    if (bRet)
        return BitmapEx(aBitmap, aMask);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
