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
#pragma once

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
    BitmapInfoAccess(const Bitmap& rBitmap, BitmapAccessMode nMode = BitmapAccessMode::Info);
    BitmapInfoAccess(const AlphaMask& rBitmap, BitmapAccessMode nMode = BitmapAccessMode::Info);

    virtual ~BitmapInfoAccess();

    bool operator!() const { return mpBuffer == nullptr; }
    explicit operator bool() const { return mpBuffer != nullptr; }

    tools::Long Width() const { return mpBuffer ? mpBuffer->mnWidth : 0L; }

    tools::Long Height() const { return mpBuffer ? mpBuffer->mnHeight : 0L; }

    bool IsTopDown() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer && mpBuffer->meDirection == ScanlineDirection::TopDown;
    }

    bool IsBottomUp() const { return !IsTopDown(); }

    ScanlineFormat GetScanlineFormat() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer ? mpBuffer->meFormat : ScanlineFormat::NONE;
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

    /// Returns the BitmapColor (i.e. palette index) that is either an exact match
    /// of the required color, or failing that, the entry that is the closest i.e. least error
    /// as measured by Color::GetColorError.
    BitmapColor GetBestMatchingColor(const BitmapColor& rBitmapColor) const
    {
        if (HasPalette())
            return BitmapColor(static_cast<sal_uInt8>(GetBestPaletteIndex(rBitmapColor)));
        else
            return rBitmapColor;
    }

    bool HasPalette() const
    {
        const BitmapBuffer* pBuffer = mpBuffer;

        assert(pBuffer && "Access is not valid!");

        return pBuffer && !!pBuffer->maPalette;
    }

    const BitmapPalette& GetPalette() const
    {
        const BitmapBuffer* pBuffer = mpBuffer;

        assert(pBuffer && "Access is not valid!");

        return pBuffer->maPalette;
    }

    sal_uInt16 GetPaletteEntryCount() const
    {
        const BitmapBuffer* pBuffer = mpBuffer;

        assert(HasPalette() && "Bitmap has no palette!");

        return HasPalette() ? pBuffer->maPalette.GetEntryCount() : 0;
    }

    const BitmapColor& GetPaletteColor(sal_uInt16 nColor) const
    {
        const BitmapBuffer* pBuffer = mpBuffer;
        assert(pBuffer && "Access is not valid!");
        assert(HasPalette() && "Bitmap has no palette!");

        return pBuffer->maPalette[nColor];
    }

    /// Returns the BitmapColor (i.e. palette index) that is either an exact match
    /// of the required color, or failing that, the entry that is the closest i.e. least error
    /// as measured by Color::GetColorError.
    sal_uInt16 GetBestPaletteIndex(const BitmapColor& rBitmapColor) const;
    /// Returns the BitmapColor (i.e. palette index) that is an exact match
    /// of the required color. Returns SAL_MAX_UINT16 if nothing found.
    sal_uInt16 GetMatchingPaletteIndex(const BitmapColor& rBitmapColor) const;

    const ColorMask& GetColorMask() const
    {
        const BitmapBuffer* pBuffer = mpBuffer;

        assert(pBuffer && "Access is not valid!");

        return pBuffer->maColorMask;
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

class BitmapScopedInfoAccess
{
public:
    BitmapScopedInfoAccess(const Bitmap& rBitmap)
        : moAccess(rBitmap)
    {
    }
    BitmapScopedInfoAccess(const AlphaMask& rBitmap)
        : moAccess(rBitmap)
    {
    }
    BitmapScopedInfoAccess() {}

    BitmapScopedInfoAccess& operator=(const Bitmap& rBitmap)
    {
        moAccess.emplace(rBitmap);
        return *this;
    }

    BitmapScopedInfoAccess& operator=(const AlphaMask& rBitmap)
    {
        moAccess.emplace(rBitmap);
        return *this;
    }

    bool operator!() const { return !moAccess.has_value() || !*moAccess; }
    explicit operator bool() const { return moAccess && bool(*moAccess); }

    void reset() { moAccess.reset(); }

    BitmapInfoAccess* get() { return moAccess ? &*moAccess : nullptr; }
    const BitmapInfoAccess* get() const { return moAccess ? &*moAccess : nullptr; }

    BitmapInfoAccess* operator->() { return &*moAccess; }
    const BitmapInfoAccess* operator->() const { return &*moAccess; }

    BitmapInfoAccess& operator*() { return *moAccess; }
    const BitmapInfoAccess& operator*() const { return *moAccess; }

private:
    std::optional<BitmapInfoAccess> moAccess;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
