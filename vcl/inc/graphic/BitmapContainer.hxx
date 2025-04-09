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
#include <vcl/bitmapex.hxx>

struct ImpSwapInfo;

class SAL_DLLPUBLIC_RTTI BitmapContainer final
{
public:
    BitmapEx maBitmapEx;

    BitmapContainer() = default;

    BitmapContainer(BitmapEx const& rBitmapEx)
        : maBitmapEx(rBitmapEx)
    {
    }

    bool operator==(const BitmapContainer& rOther) const { return maBitmapEx == rOther.maBitmapEx; }

    void createSwapInfo(ImpSwapInfo& rSwapInfo);

    bool isAlpha() { return maBitmapEx.IsAlpha(); }

    const BitmapEx& getBitmapExRef() const { return maBitmapEx; }

    Size getPrefSize() const
    {
        Size aSize = maBitmapEx.GetPrefSize();
        if (!aSize.Width() || !aSize.Height())
            aSize = maBitmapEx.GetSizePixel();
        return aSize;
    }

    MapMode getPrefMapMode() const
    {
        const Size aSize = maBitmapEx.GetPrefSize();
        if (aSize.Width() && aSize.Height())
            return maBitmapEx.GetPrefMapMode();
        return {};
    }

    sal_uInt64 getSizeBytes() { return maBitmapEx.GetSizeBytes(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
