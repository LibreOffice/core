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

#include <memory>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/salbtype.hxx>
#include <vcl/bitmap.hxx>

typedef BitmapColor (*FncGetPixel)(ConstScanline pScanline, long nX, const ColorMask& rMask);
typedef void (*FncSetPixel)(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);


class VCL_DLLPUBLIC BitmapInfoAccess
{
    friend class BitmapReadAccess;

public:
    BitmapInfoAccess(Bitmap& rBitmap);
    virtual ~BitmapInfoAccess();

    bool operator!() const
    {
        return mpBuffer == nullptr;
    }

    long Width() const
    {
        return mpBuffer ? mpBuffer->mnWidth : 0L;
    }

    long Height() const
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

    sal_uLong GetScanlineSize() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer ? mpBuffer->mnScanlineSize : 0UL;
    }

    sal_uInt16 GetBitCount() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer ? mpBuffer->mnBitCount : 0;
    }

    BitmapColor GetBestMatchingColor(const BitmapColor& rBitmapColor)
    {
        if (HasPalette())
            return BitmapColor((sal_uInt8) GetBestPaletteIndex(rBitmapColor));
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

    const BitmapColor& GetPaletteColor( sal_uInt16 nColor ) const
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

    SAL_DLLPRIVATE void ImplCreate(Bitmap& rBitmap);
    SAL_DLLPRIVATE void ImplDestroy();

protected:
    BitmapInfoAccess(Bitmap& rBitmap, BitmapAccessMode nMode);
};


class VCL_DLLPUBLIC BitmapReadAccess : public BitmapInfoAccess
{
    friend class BitmapWriteAccess;

public:
    BitmapReadAccess(Bitmap& rBitmap);
    virtual ~BitmapReadAccess() override;

    Scanline GetBuffer() const
    {
        assert(mpBuffer && "Access is not valid!");

        return mpBuffer ? mpBuffer->mpBits : nullptr;
    }

    Scanline GetScanline(long nY) const
    {
        assert(mpBuffer && "Access is not valid!");
        assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

        if (mpBuffer->mnFormat & ScanlineFormat::TopDown)
        {
            return mpBuffer->mpBits + (nY * mpBuffer->mnScanlineSize);
        }
        return mpBuffer->mpBits + ((mpBuffer->mnHeight - 1 - nY) * mpBuffer->mnScanlineSize);
    }

    BitmapColor GetPixelFromData(const sal_uInt8* pData, long nX) const
    {
        assert(pData && "Access is not valid!");

        return mFncGetPixel( pData, nX, maColorMask );
    }

    void SetPixelOnData(sal_uInt8* pData, long nX, const BitmapColor& rBitmapColor)
    {
        assert(pData && "Access is not valid!");

        mFncSetPixel(pData, nX, rBitmapColor, maColorMask);
    }

    BitmapColor GetPixel(long nY, long nX) const
    {
        assert(mpBuffer && "Access is not valid!");
        assert(nX < mpBuffer->mnWidth && "x-coordinate out of range!");
        assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

        return mFncGetPixel(GetScanline(nY), nX, maColorMask );
    }

    BitmapColor GetColor(long nY, long nX) const
    {
        if (HasPalette())
            return mpBuffer->maPalette[GetPixelIndex(nY, nX)];
        else
            return GetPixel(nY, nX);
    }

    sal_uInt8 GetPixelIndex(long nY, long nX) const
    {
        return GetPixel(nY, nX).GetBlueOrIndex();
    }

    sal_uInt8 GetLuminance(long nY, long nX) const
    {
        return GetColor(nY, nX).GetLuminance();
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

    SAL_DLLPRIVATE void ImplInitScanBuffer( Bitmap const & rBitmap );
    SAL_DLLPRIVATE bool ImplSetAccessPointers( ScanlineFormat nFormat );

public:

    SAL_DLLPRIVATE BitmapBuffer* ImplGetBitmapBuffer() const
    {
        return mpBuffer;
    }

    static BitmapColor GetPixelForN1BitMsbPal(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN1BitLsbPal(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN4BitMsnPal(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN4BitLsnPal(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN8BitPal(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN8BitTcMask(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN16BitTcMsbMask(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN16BitTcLsbMask(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN24BitTcBgr(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN24BitTcRgb(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcAbgr(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcArgb(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcBgra(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcRgba(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelForN32BitTcMask(ConstScanline pScanline, long nX, const ColorMask& rMask);

    static void SetPixelForN1BitMsbPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN1BitLsbPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN4BitMsnPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN4BitLsnPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN8BitPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN8BitTcMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN16BitTcMsbMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN16BitTcLsbMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN24BitTcBgr(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN24BitTcRgb(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcAbgr(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcArgb(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcBgra(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcRgba(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelForN32BitTcMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);

protected:
    BitmapReadAccess(Bitmap& rBitmap, BitmapAccessMode nMode);
};


class VCL_DLLPUBLIC BitmapWriteAccess : public BitmapReadAccess
{
public:
    BitmapWriteAccess(Bitmap& rBitmap);
    virtual ~BitmapWriteAccess() override;

    void CopyScanline(long nY, const BitmapReadAccess& rReadAcc);
    void CopyScanline(long nY,
                      ConstScanline aSrcScanline,
                      ScanlineFormat nSrcScanlineFormat,
                      sal_uLong nSrcScanlineSize);

    void CopyBuffer( const BitmapReadAccess& rReadAcc );

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

    std::unique_ptr<BitmapColor> mpLineColor;
    std::unique_ptr<BitmapColor> mpFillColor;

    BitmapWriteAccess() = delete;
    BitmapWriteAccess(const BitmapWriteAccess&) = delete;
    BitmapWriteAccess& operator=(const BitmapWriteAccess&) = delete;
};

#endif // INCLUDED_VCL_BMPACC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
