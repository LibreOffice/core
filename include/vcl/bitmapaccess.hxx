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

#ifndef INCLUDED_VCL_BMPACC_HXX
#define INCLUDED_VCL_BMPACC_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>
#include <vcl/Scanline.hxx>
#include <vcl/BitmapBuffer.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapAccessMode.hxx>

typedef BitmapColor (*FncGetPixel)(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
typedef void (*FncSetPixel)(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);

class VCL_DLLPUBLIC BitmapInfoAccess
{
    friend class BitmapReadAccess;

public:
    BitmapInfoAccess(Bitmap& rBitmap, BitmapAccessMode nMode = BitmapAccessMode::Info);
    virtual ~BitmapInfoAccess();

    bool operator!() const
    {
        return mpBuffer == nullptr;
    }

    tools::Long Width() const
    {
        return mpBuffer ? mpBuffer->mnWidth : 0L;
    }

    tools::Long Height() const
    {
        return mpBuffer ? mpBuffer->mnHeight : 0L;
    }

    bool IsTopDown() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer && (mpBuffer->mnFormat & ScanlineFormat::TopDown);
    }

    bool IsBottomUp() const
    {
        return !IsTopDown();
    }

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

    const BitmapColor& GetPaletteColor( sal_uInt16 nColor ) const
    {
        const BitmapBuffer* pBuffer = mpBuffer;

        assert(pBuffer && "Access is not valid!");
        assert(HasPalette() && "Bitmap has no palette!");

        return pBuffer->maPalette[nColor];
    }

    const BitmapColor& GetBestPaletteColor(const BitmapColor& rBitmapColor) const
    {
        return GetPaletteColor(GetBestPaletteIndex(rBitmapColor));
    }

    sal_uInt16 GetBestPaletteIndex(const BitmapColor& rBitmapColor) const;

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


class VCL_DLLPUBLIC BitmapReadAccess : public BitmapInfoAccess
{
    friend class BitmapWriteAccess;

public:
    BitmapReadAccess(Bitmap& rBitmap, BitmapAccessMode nMode = BitmapAccessMode::Read);
    virtual ~BitmapReadAccess() override;

    Scanline GetBuffer() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer ? mpBuffer->mpBits : nullptr;
    }

    Scanline GetScanline(tools::Long nY) const
    {
        assert(mpBuffer && "Access is not valid!");
        assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

        if (mpBuffer->mnFormat & ScanlineFormat::TopDown)
        {
            return mpBuffer->mpBits + (nY * mpBuffer->mnScanlineSize);
        }
        return mpBuffer->mpBits + ((mpBuffer->mnHeight - 1 - nY) * mpBuffer->mnScanlineSize);
    }

    BitmapColor GetPixelFromData(const sal_uInt8* pData, tools::Long nX) const
    {
        assert(pData && "Access is not valid!");

        return mFncGetPixel( pData, nX, maColorMask );
    }

    sal_uInt8 GetIndexFromData(const sal_uInt8* pData, tools::Long nX) const
    {
        return GetPixelFromData( pData, nX ).GetIndex();
    }

    void SetPixelOnData(sal_uInt8* pData, tools::Long nX, const BitmapColor& rBitmapColor)
    {
        assert(pData && "Access is not valid!");

        mFncSetPixel(pData, nX, rBitmapColor, maColorMask);
    }

    BitmapColor GetPixel(tools::Long nY, tools::Long nX) const
    {
        assert(mpBuffer && "Access is not valid!");
        assert(nX < mpBuffer->mnWidth && "x-coordinate out of range!");
        assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

        return mFncGetPixel(GetScanline(nY), nX, maColorMask );
    }

    BitmapColor GetColor(tools::Long nY, tools::Long nX) const
    {
        if (HasPalette())
        {
            const BitmapBuffer* pBuffer = mpBuffer;
            return pBuffer->maPalette[GetPixelIndex(nY, nX)];
        }
        else
            return GetPixel(nY, nX);
    }

    sal_uInt8 GetPixelIndex(tools::Long nY, tools::Long nX) const
    {
        return GetPixel(nY, nX).GetIndex();
    }

    /** Get the interpolated color at coordinates fY, fX; if outside, return rFallback */
    BitmapColor GetInterpolatedColorWithFallback( double fY, double fX, const BitmapColor& rFallback ) const;

    /** Get the color at coordinates fY, fX; if outside, return rFallback. Automatically does the correct
        inside/outside checks, e.g. static_cast< sal_uInt32 >(-0.25) *is* 0, not -1 and has to be outside */
    BitmapColor GetColorWithFallback( double fY, double fX, const BitmapColor& rFallback ) const;

private:
    BitmapReadAccess(const BitmapReadAccess&) = delete;
    BitmapReadAccess& operator=(const BitmapReadAccess&) = delete;

protected:
    FncGetPixel mFncGetPixel;
    FncSetPixel mFncSetPixel;

public:

    SAL_DLLPRIVATE BitmapBuffer* ImplGetBitmapBuffer() const
    {
        return mpBuffer;
    }

    static BitmapColor GetPixelForN1BitMsbPal(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN1BitLsbPal(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN4BitMsnPal(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN4BitLsnPal(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN8BitPal(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN24BitTcBgr(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN24BitTcRgb(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcAbgr(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcXbgr(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcArgb(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcXrgb(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcBgra(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcBgrx(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcRgba(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcRgbx(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcMask(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask);

    static void SetPixelForN1BitMsbPal(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN1BitLsbPal(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN4BitMsnPal(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN4BitLsnPal(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN8BitPal(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN24BitTcBgr(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN24BitTcRgb(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcAbgr(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcXbgr(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcArgb(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcXrgb(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcBgra(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcBgrx(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcRgba(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcRgbx(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcMask(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);

    static FncGetPixel GetPixelFunction( ScanlineFormat nFormat );
    static FncSetPixel SetPixelFunction( ScanlineFormat nFormat );
};


#endif // INCLUDED_VCL_BMPACC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
