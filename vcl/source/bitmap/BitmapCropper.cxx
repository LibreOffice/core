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
