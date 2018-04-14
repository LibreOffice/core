/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/bitmapex.hxx>
#include <vcl/BitmapScaleFilter.hxx>

BitmapEx BitmapScaleFilter::execute(BitmapEx const& rBitmapEx)
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());
    bool bRet = false;

    if (maSize == Size(0, 0))
        bRet = aBitmap.Scale(mfScaleX, mfScaleY, mnScaleFlag);
    else
        bRet = aBitmap.Scale(maSize, mnScaleFlag);

    Bitmap aMask(rBitmapEx.GetMask());

    if (bRet && (rBitmapEx.GetTransparentType() == TransparentType::Bitmap) && !aMask.IsEmpty())
        bRet = aMask.Scale(maSize, mnScaleFlag);

    SAL_WARN_IF(!aMask.IsEmpty() && aBitmap.GetSizePixel() != aMask.GetSizePixel(), "vcl",
                "BitmapEx::Scale(): size mismatch for bitmap and alpha mask.");

    if (bRet)
        return BitmapEx(aBitmap, aMask);

    return BitmapEx();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
