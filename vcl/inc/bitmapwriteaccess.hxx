/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_BITMAPWRITEACCESS_HXX
#define INCLUDED_VCL_INC_BITMAPWRITEACCESS_HXX

#include <vcl/alpha.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapaccess.hxx>
#include <boost/optional.hpp>

typedef vcl::ScopedBitmapAccess<BitmapWriteAccess, Bitmap, &Bitmap::AcquireWriteAccess>
    BitmapScopedWriteAccess;

typedef vcl::ScopedBitmapAccess<BitmapWriteAccess, AlphaMask, &AlphaMask::AcquireAlphaWriteAccess>
    AlphaScopedWriteAccess;

class VCL_DLLPUBLIC BitmapWriteAccess : public BitmapReadAccess
{
public:
    BitmapWriteAccess(Bitmap& rBitmap);
    virtual ~BitmapWriteAccess() override;

    void CopyScanline(long nY, const BitmapReadAccess& rReadAcc);
    void CopyScanline(long nY, ConstScanline aSrcScanline, ScanlineFormat nSrcScanlineFormat,
                      sal_uInt32 nSrcScanlineSize);

    void CopyBuffer(const BitmapReadAccess& rReadAcc);

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

    void SetPixel(long nY, long nX, const BitmapColor& rBitmapColor)
    {
        assert(mpBuffer && "Access is not valid!");
        assert(nX < mpBuffer->mnWidth && "x-coordinate out of range!");
        assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

        mFncSetPixel(GetScanline(nY), nX, rBitmapColor, maColorMask);
    }

    void SetPixelIndex(long nY, long nX, sal_uInt8 cIndex)
    {
        SetPixel(nY, nX, BitmapColor(cIndex));
    }

    void SetLineColor(const Color& rColor);

    void SetFillColor();
    void SetFillColor(const Color& rColor);

    void Erase(const Color& rColor);

    void DrawLine(const Point& rStart, const Point& rEnd);

    void FillRect(const tools::Rectangle& rRect);
    void DrawRect(const tools::Rectangle& rRect);

private:
    boost::optional<BitmapColor> mpLineColor;
    boost::optional<BitmapColor> mpFillColor;

    BitmapWriteAccess() = delete;
    BitmapWriteAccess(const BitmapWriteAccess&) = delete;
    BitmapWriteAccess& operator=(const BitmapWriteAccess&) = delete;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
