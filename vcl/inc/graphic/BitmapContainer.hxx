/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>

struct SwapInfo;

class SAL_DLLPUBLIC_RTTI BitmapContainer final
{
public:
    Bitmap maBitmap;

    BitmapContainer() = default;

    BitmapContainer(Bitmap const& rBitmap)
        : maBitmap(rBitmap)
    {
    }

    bool operator==(const BitmapContainer& rOther) const { return maBitmap == rOther.maBitmap; }

    void createSwapInfo(SwapInfo& rSwapInfo);

    bool isAlpha() { return maBitmap.HasAlpha(); }

    const Bitmap& getBitmapRef() const { return maBitmap; }

    Size getPrefSize() const
    {
        Size aSize = maBitmap.GetPrefSize();
        if (!aSize.Width() || !aSize.Height())
            aSize = maBitmap.GetSizePixel();
        return aSize;
    }

    MapMode getPrefMapMode() const
    {
        const Size aSize = maBitmap.GetPrefSize();
        if (aSize.Width() && aSize.Height())
            return maBitmap.GetPrefMapMode();
        return {};
    }

    sal_uInt64 getSizeBytes() { return maBitmap.GetSizeBytes(); }

    BitmapChecksum getChecksum() const { return maBitmap.GetChecksum(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
