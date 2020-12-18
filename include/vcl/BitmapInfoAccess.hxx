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

#ifndef INCLUDED_VCL_BITMAPINFOACCESS_HXX
#define INCLUDED_VCL_BITMAPINFOACCESS_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>
#include <vcl/Scanline.hxx>
#include <vcl/BitmapBuffer.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapAccessMode.hxx>

bool Bitmap32IsPreMultipled();

typedef BitmapColor (*FncGetPixel)(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
typedef void (*FncSetPixel)(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor,
                            const ColorMask& rMask);

class VCL_DLLPUBLIC BitmapInfoAccess
{
    friend class BitmapReadAccess;

public:
    BitmapInfoAccess(Bitmap& rBitmap, BitmapAccessMode nMode = BitmapAccessMode::Info);
    virtual ~BitmapInfoAccess();

    bool operator!() const { return mpBuffer == nullptr; }

    tools::Long Width() const { return mpBuffer ? mpBuffer->mnWidth : 0L; }

    tools::Long Height() const { return mpBuffer ? mpBuffer->mnHeight : 0L; }

    bool IsTopDown() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer && (mpBuffer->mnFormat & ScanlineFormat::TopDown);
    }

    bool IsBottomUp() const { return !IsTopDown(); }

    ScanlineFormat GetScanlineFormat() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer ? RemoveScanline(mpBuffer->mnFormat) : ScanlineFormat::NONE;
    }

    sal_uInt32 GetScanlineSize() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer ? mpBuffer->mnScanlineSize : 0;
    }

    sal_uInt16 GetBitCount() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer ? mpBuffer->mnBitCount : 0;
    }

    BitmapColor GetBestMatchingColor(const BitmapColor& rBitmapColor)
    {
        if (HasPalette())
            return BitmapColor(static_cast<sal_uInt8>(GetBestPaletteIndex(rBitmapColor)));
        else
            return rBitmapColor;
    }

    bool HasPalette() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer && !!mpBuffer->maPalette;
    }

    const BitmapPalette& GetPalette() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer->maPalette;
    }

    sal_uInt16 GetPaletteEntryCount() const
    {
        assert(HasPalette() && "Bitmap has no palette!");

        return HasPalette() ? mpBuffer->maPalette.GetEntryCount() : 0;
    }

    const BitmapColor& GetPaletteColor(sal_uInt16 nColor) const
    {
        assert(mpBuffer && "Access is not valid!");
        assert(HasPalette() && "Bitmap has no palette!");

        return mpBuffer->maPalette[nColor];
    }

    const BitmapColor& GetBestPaletteColor(const BitmapColor& rBitmapColor) const
    {
        return GetPaletteColor(GetBestPaletteIndex(rBitmapColor));
    }

    sal_uInt16 GetBestPaletteIndex(const BitmapColor& rBitmapColor) const;

    ColorMask& GetColorMask() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer->maColorMask;
    }

private:
    BitmapInfoAccess(const BitmapInfoAccess&) = delete;
    BitmapInfoAccess& operator=(const BitmapInfoAccess&) = delete;

protected:
    Bitmap maBitmap;
    BitmapBuffer* mpBuffer;
    ColorMask maColorMask;
    BitmapAccessMode mnAccessMode;
};

#endif // INCLUDED_VCL_BITMAPINFOACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
