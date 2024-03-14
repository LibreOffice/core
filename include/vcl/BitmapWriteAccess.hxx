/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#pragma once

#include <vcl/alpha.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <optional>

class VCL_DLLPUBLIC BitmapWriteAccess final : public BitmapReadAccess
{
public:
    BitmapWriteAccess(Bitmap& rBitmap);
    BitmapWriteAccess(AlphaMask& rBitmap);
    virtual ~BitmapWriteAccess() override;

    SAL_DLLPRIVATE void CopyScanline(tools::Long nY, const BitmapReadAccess& rReadAcc);
    SAL_DLLPRIVATE void CopyScanline(tools::Long nY, ConstScanline aSrcScanline,
                                     ScanlineFormat nSrcScanlineFormat,
                                     sal_uInt32 nSrcScanlineSize);

    void SetPalette(const BitmapPalette& rPalette)
    {
        assert(mpBuffer && "Access is not valid!");

        mpBuffer->maPalette = rPalette;
    }

    void SetPaletteEntryCount(sal_uInt16 nCount)
    {
        assert(mpBuffer && "Access is not valid!");

        mpBuffer->maPalette.SetEntryCount(nCount);
    }

    void SetPaletteColor(sal_uInt16 nColor, const BitmapColor& rBitmapColor)
    {
        assert(mpBuffer && "Access is not valid!");
        assert(HasPalette() && "Bitmap has no palette!");

        mpBuffer->maPalette[nColor] = rBitmapColor;
    }

    void SetPixel(tools::Long nY, tools::Long nX, const BitmapColor& rBitmapColor)
    {
        assert(mpBuffer && "Access is not valid!");
        assert(nX < mpBuffer->mnWidth && "x-coordinate out of range!");
        assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

        mFncSetPixel(GetScanline(nY), nX, rBitmapColor, maColorMask);
    }

    void SetPixelIndex(tools::Long nY, tools::Long nX, sal_uInt8 cIndex)
    {
        SetPixel(nY, nX, BitmapColor(cIndex));
    }

    void SetLineColor(const Color& rColor);

    void SetFillColor();
    void SetFillColor(const Color& rColor);

    void Erase(const Color& rColor);

    SAL_DLLPRIVATE void DrawLine(const Point& rStart, const Point& rEnd);

    void FillRect(const tools::Rectangle& rRect);
    void DrawRect(const tools::Rectangle& rRect);

private:
    std::optional<BitmapColor> mpLineColor;
    std::optional<BitmapColor> mpFillColor;

    BitmapWriteAccess() = delete;
    BitmapWriteAccess(const BitmapWriteAccess&) = delete;
    BitmapWriteAccess& operator=(const BitmapWriteAccess&) = delete;
};

class BitmapScopedWriteAccess
{
public:
    BitmapScopedWriteAccess(Bitmap& rBitmap)
        : moAccess(rBitmap)
    {
    }
    BitmapScopedWriteAccess(AlphaMask& rBitmap)
        : moAccess(rBitmap)
    {
    }
    BitmapScopedWriteAccess() {}

    BitmapScopedWriteAccess& operator=(Bitmap& rBitmap)
    {
        moAccess.emplace(rBitmap);
        return *this;
    }

    BitmapScopedWriteAccess& operator=(AlphaMask& rBitmap)
    {
        moAccess.emplace(rBitmap);
        return *this;
    }

    bool operator!() const { return !moAccess.has_value() || !*moAccess; }
    explicit operator bool() const { return moAccess && bool(*moAccess); }

    void reset() { moAccess.reset(); }

    BitmapWriteAccess* get() { return moAccess ? &*moAccess : nullptr; }
    const BitmapWriteAccess* get() const { return moAccess ? &*moAccess : nullptr; }

    BitmapWriteAccess* operator->() { return &*moAccess; }
    const BitmapWriteAccess* operator->() const { return &*moAccess; }

    BitmapWriteAccess& operator*() { return *moAccess; }
    const BitmapWriteAccess& operator*() const { return *moAccess; }

private:
    std::optional<BitmapWriteAccess> moAccess;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
