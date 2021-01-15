/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <o3tl/safeint.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/bitmapex.hxx>

namespace vcl::bitmap
{
/**
 * Intended to be used to feed into CreateFromData to create a BitmapEx. RGB data format.
 */
class VCL_DLLPUBLIC RawBitmap
{
    friend BitmapEx VCL_DLLPUBLIC CreateFromData(RawBitmap&& rawBitmap);
    std::unique_ptr<sal_uInt8[]> mpData;
    Size maSize;
    sal_uInt8 mnBitCount;

public:
    RawBitmap(Size const& rSize, sal_uInt8 nBitCount)
        : maSize(rSize)
        , mnBitCount(nBitCount)
    {
        assert(nBitCount == 24 || nBitCount == 32);
        sal_Int32 nRowSize, nDataSize;
        if (o3tl::checked_multiply<sal_Int32>(rSize.getWidth(), nBitCount / 8, nRowSize)
            || o3tl::checked_multiply<sal_Int32>(nRowSize, rSize.getHeight(), nDataSize)
            || nDataSize < 0)
        {
            throw std::bad_alloc();
        }
        mpData.reset(new sal_uInt8[nDataSize]);
    }
    void SetPixel(tools::Long nY, tools::Long nX, Color nColor)
    {
        tools::Long p = (nY * maSize.getWidth() + nX) * (mnBitCount / 8);
        mpData[p++] = nColor.GetRed();
        mpData[p++] = nColor.GetGreen();
        mpData[p++] = nColor.GetBlue();
        if (mnBitCount == 32)
            mpData[p] = 255 - nColor.GetAlpha();
    }
    Color GetPixel(tools::Long nY, tools::Long nX) const
    {
        tools::Long p = (nY * maSize.getWidth() + nX) * mnBitCount / 8;
        if (mnBitCount == 24)
            return Color(mpData[p], mpData[p + 1], mpData[p + 2]);
        else
            return Color(ColorTransparency, mpData[p + 3], mpData[p], mpData[p + 1], mpData[p + 2]);
    }
    // so we don't accidentally leave any code in that uses palette color indexes
    void SetPixel(tools::Long nY, tools::Long nX, BitmapColor nColor) = delete;
    tools::Long Height() { return maSize.Height(); }
    tools::Long Width() { return maSize.Width(); }
    sal_uInt8 GetBitCount() const { return mnBitCount; }
};

} // end vcl::bitmap

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
