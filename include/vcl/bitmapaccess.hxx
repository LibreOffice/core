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

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/salbtype.hxx>
#include <vcl/bitmap.hxx>

// - Access functions -
typedef BitmapColor (*FncGetPixel)(ConstScanline pScanline, long nX, const ColorMask& rMask);
typedef void (*FncSetPixel)(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);

// - BitmapInfoAccess -
class VCL_DLLPUBLIC BitmapInfoAccess
{
    friend class BitmapReadAccess;

public:
    BitmapInfoAccess(Bitmap& rBitmap);
    virtual ~BitmapInfoAccess();

    inline bool operator!() const;

    inline long Width() const;
    inline long Height() const;

    inline bool  IsTopDown() const;
    inline bool  IsBottomUp() const;

    inline sal_uLong GetScanlineFormat() const;
    inline sal_uLong GetScanlineSize() const;

    inline sal_uInt16 GetBitCount() const;
    inline BitmapColor GetBestMatchingColor(const BitmapColor& rBitmapColor);

    inline bool HasPalette() const;
    inline const BitmapPalette& GetPalette() const;
    inline sal_uInt16 GetPaletteEntryCount() const;
    inline const BitmapColor& GetPaletteColor(sal_uInt16 nColor) const;
    inline const BitmapColor& GetBestPaletteColor(const BitmapColor& rBitmapColor) const;
    sal_uInt16 GetBestPaletteIndex(const BitmapColor& rBitmapColor) const;

    inline ColorMask&           GetColorMask() const;

private:
    BitmapInfoAccess()
    {}

    BitmapInfoAccess(const BitmapInfoAccess&)
    {}

    BitmapInfoAccess& operator=(const BitmapInfoAccess&)
    {
        return *this;
    }

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

// - BitmapReadAccess -
class VCL_DLLPUBLIC BitmapReadAccess : public BitmapInfoAccess
{
    friend class BitmapWriteAccess;

public:
    BitmapReadAccess(Bitmap& rBitmap);
    virtual ~BitmapReadAccess();

    inline Scanline GetBuffer() const;
    inline Scanline GetScanline( long nY ) const;

    inline BitmapColor GetPixelFromData( const sal_uInt8* pData, long nX ) const;
    inline void SetPixelOnData( sal_uInt8* pData, long nX, const BitmapColor& rBitmapColor );

    inline BitmapColor GetPixel( long nY, long nX ) const;
    inline BitmapColor GetColor( long nY, long nX ) const;
    inline sal_uInt8 GetPixelIndex( long nY, long nX ) const;
    inline sal_uInt8 GetLuminance( long nY, long nX ) const;

    /** Get the interpolated color at coordinates fY, fX; if outside, return rFallback */
    BitmapColor GetInterpolatedColorWithFallback( double fY, double fX, const BitmapColor& rFallback ) const;

    /** Get the color at coordinates fY, fX; if outside, return rFallback. Automatically does the correct
        inside/outside checks, e.g. static_cast< sal_uInt32 >(-0.25) *is* 0, not -1 and has to be outside */
    BitmapColor GetColorWithFallback( double fY, double fX, const BitmapColor& rFallback ) const;

private:

    BitmapReadAccess()
    {}

    BitmapReadAccess(const BitmapReadAccess&)
        : BitmapInfoAccess()
    {}

    BitmapReadAccess& operator=(const BitmapReadAccess&)
    {
        return *this;
    }

protected:
    Scanline*   mpScanBuf;
    FncGetPixel mFncGetPixel;
    FncSetPixel mFncSetPixel;

    SAL_DLLPRIVATE void ImplInitScanBuffer( Bitmap& rBitmap );
    SAL_DLLPRIVATE void ImplClearScanBuffer();
    SAL_DLLPRIVATE bool ImplSetAccessPointers( sal_uLong nFormat );

public:

    SAL_DLLPRIVATE BitmapBuffer* ImplGetBitmapBuffer() const
    {
        return mpBuffer;
    }

    static BitmapColor GetPixelFor_1BIT_MSB_PAL(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_1BIT_LSB_PAL(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_4BIT_MSN_PAL(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_4BIT_LSN_PAL(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_8BIT_PAL(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_8BIT_TC_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_16BIT_TC_MSB_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_16BIT_TC_LSB_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_24BIT_TC_BGR(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_24BIT_TC_RGB(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_24BIT_TC_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_32BIT_TC_ABGR(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_32BIT_TC_ARGB(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_32BIT_TC_BGRA(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_32BIT_TC_RGBA(ConstScanline pScanline, long nX, const ColorMask& rMask);
    static BitmapColor GetPixelFor_32BIT_TC_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask);

    static void SetPixelFor_1BIT_MSB_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_1BIT_LSB_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_4BIT_MSN_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_4BIT_LSN_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_8BIT_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_8BIT_TC_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_16BIT_TC_MSB_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_16BIT_TC_LSB_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_24BIT_TC_BGR(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_24BIT_TC_RGB(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_24BIT_TC_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_32BIT_TC_ABGR(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_32BIT_TC_ARGB(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_32BIT_TC_BGRA(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_32BIT_TC_RGBA(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);
    static void SetPixelFor_32BIT_TC_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask);

protected:
    BitmapReadAccess(Bitmap& rBitmap, BitmapAccessMode nMode);
};

// - BitmapWriteAccess -
class VCL_DLLPUBLIC BitmapWriteAccess : public BitmapReadAccess
{
public:
    BitmapWriteAccess(Bitmap& rBitmap);
    virtual ~BitmapWriteAccess();

    void CopyScanline(long nY, const BitmapReadAccess& rReadAcc);
    void CopyScanline(long nY,
                      ConstScanline aSrcScanline,
                      sal_uLong nSrcScanlineFormat,
                      sal_uLong nSrcScanlineSize);

    void CopyBuffer( const BitmapReadAccess& rReadAcc );

    inline void SetPalette(const BitmapPalette& rPalette);
    inline void SetPaletteEntryCount(sal_uInt16 nCount);
    inline void SetPaletteColor(sal_uInt16 nColor, const BitmapColor& rBitmapColor);

    inline void SetPixel(long nY, long nX, const BitmapColor& rBitmapColor);
    inline void SetPixelIndex(long nY, long nX, sal_uInt8 cIndex);

    void SetLineColor(const Color& rColor);

    void SetFillColor();
    void SetFillColor(const Color& rColor);

    void Erase(const Color& rColor);

    void DrawLine(const Point& rStart, const Point& rEnd);

    void FillRect(const Rectangle& rRect);
    void DrawRect(const Rectangle& rRect);

private:

    std::unique_ptr<BitmapColor> mpLineColor;
    std::unique_ptr<BitmapColor> mpFillColor;

    BitmapWriteAccess()
    {}

    BitmapWriteAccess(const BitmapWriteAccess&)
        : BitmapReadAccess()
    {}

    BitmapWriteAccess& operator=(const BitmapWriteAccess&)
    {
        return *this;
    }
};

// - Inlines -
inline bool BitmapInfoAccess::operator!() const
{
    return mpBuffer == nullptr;
}

inline long BitmapInfoAccess::Width() const
{
    return mpBuffer ? mpBuffer->mnWidth : 0L;
}

inline long BitmapInfoAccess::Height() const
{
    return mpBuffer ? mpBuffer->mnHeight : 0L;
}

inline bool BitmapInfoAccess::IsTopDown() const
{
    assert(mpBuffer && "Access is not valid!");

    return mpBuffer && (BMP_SCANLINE_ADJUSTMENT(mpBuffer->mnFormat) == BMP_FORMAT_TOP_DOWN);
}

inline bool BitmapInfoAccess::IsBottomUp() const
{
    return !IsTopDown();
}

inline sal_uLong BitmapInfoAccess::GetScanlineFormat() const
{
    assert(mpBuffer && "Access is not valid!");

    return mpBuffer ? BMP_SCANLINE_FORMAT(mpBuffer->mnFormat) : 0UL;
}

inline sal_uLong BitmapInfoAccess::GetScanlineSize() const
{
    assert(mpBuffer && "Access is not valid!");

    return mpBuffer ? mpBuffer->mnScanlineSize : 0UL;
}

inline sal_uInt16  BitmapInfoAccess::GetBitCount() const
{
    assert(mpBuffer && "Access is not valid!");

    return mpBuffer ? mpBuffer->mnBitCount : 0;
}

inline BitmapColor BitmapInfoAccess::GetBestMatchingColor(const BitmapColor& rBitmapColor)
{
    if (HasPalette())
        return BitmapColor((sal_uInt8) GetBestPaletteIndex(rBitmapColor));
    else
        return rBitmapColor;
}

inline bool BitmapInfoAccess::HasPalette() const
{
    assert(mpBuffer && "Access is not valid!");

    return mpBuffer && !!mpBuffer->maPalette;
}

inline const BitmapPalette& BitmapInfoAccess::GetPalette() const
{
    assert(mpBuffer && "Access is not valid!");

    return mpBuffer->maPalette;
}

inline sal_uInt16 BitmapInfoAccess::GetPaletteEntryCount() const
{
    assert(HasPalette() && "Bitmap has no palette!");

    return HasPalette() ? mpBuffer->maPalette.GetEntryCount() : 0;
}

inline const BitmapColor& BitmapInfoAccess::GetPaletteColor( sal_uInt16 nColor ) const
{
    assert(mpBuffer && "Access is not valid!");
    assert(HasPalette() && "Bitmap has no palette!");

    return mpBuffer->maPalette[nColor];
}

inline const BitmapColor& BitmapInfoAccess::GetBestPaletteColor(const BitmapColor& rBitmapColor) const
{
    return GetPaletteColor(GetBestPaletteIndex(rBitmapColor));
}



inline ColorMask& BitmapInfoAccess::GetColorMask() const
{
    assert(mpBuffer && "Access is not valid!");

    return mpBuffer->maColorMask;
}

inline Scanline BitmapReadAccess::GetBuffer() const
{
    assert(mpBuffer && "Access is not valid!");

    return mpBuffer ? mpBuffer->mpBits : nullptr;
}

inline Scanline BitmapReadAccess::GetScanline(long nY) const
{
    assert(mpBuffer && mpScanBuf && "Access is not valid!");
    assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

    return mpScanBuf[nY];
}

inline BitmapColor BitmapReadAccess::GetPixel(long nY, long nX) const
{
    assert(mpBuffer && mpScanBuf && "Access is not valid!");
    assert(nX >= 0 && "x-coordinate out of range!");
    assert(nX < mpBuffer->mnWidth && "x-coordinate out of range!");
    assert(nY >= 0 && "y-coordinate out of range!");
    assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

    return mFncGetPixel(mpScanBuf[nY], nX, maColorMask );
}

inline sal_uInt8 BitmapReadAccess::GetPixelIndex(long nY, long nX) const
{
    return GetPixel(nY, nX).GetBlueOrIndex();
}

inline BitmapColor BitmapReadAccess::GetPixelFromData(const sal_uInt8* pData, long nX) const
{
    assert(pData && "Access is not valid!");

    return mFncGetPixel( pData, nX, maColorMask );
}

inline void BitmapReadAccess::SetPixelOnData(sal_uInt8* pData, long nX, const BitmapColor& rBitmapColor)
{
    assert(pData && "Access is not valid!");

    mFncSetPixel(pData, nX, rBitmapColor, maColorMask);
}

inline BitmapColor BitmapReadAccess::GetColor(long nY, long nX) const
{
    if (HasPalette())
        return mpBuffer->maPalette[GetPixelIndex(nY, nX)];
    else
        return GetPixel(nY, nX);
}

inline sal_uInt8 BitmapReadAccess::GetLuminance(long nY, long nX) const
{
    return GetColor(nY, nX).GetLuminance();
}

inline void BitmapWriteAccess::SetPalette(const BitmapPalette& rPalette)
{
    assert(mpBuffer && "Access is not valid!");

    mpBuffer->maPalette = rPalette;
}

inline void BitmapWriteAccess::SetPaletteEntryCount(sal_uInt16 nCount)
{
    assert(mpBuffer && "Access is not valid!");

    mpBuffer->maPalette.SetEntryCount(nCount);
}

inline void BitmapWriteAccess::SetPaletteColor(sal_uInt16 nColor, const BitmapColor& rBitmapColor)
{
    assert(mpBuffer && "Access is not valid!");
    assert(HasPalette() && "Bitmap has no palette!");

    mpBuffer->maPalette[nColor] = rBitmapColor;
}

inline void BitmapWriteAccess::SetPixel(long nY, long nX, const BitmapColor& rBitmapColor)
{
    assert(mpBuffer && "Access is not valid!");
    assert(nX < mpBuffer->mnWidth && "x-coordinate out of range!");
    assert(nY < mpBuffer->mnHeight && "y-coordinate out of range!");

    mFncSetPixel(mpScanBuf[nY], nX, rBitmapColor, maColorMask);
}

inline void BitmapWriteAccess::SetPixelIndex(long nY, long nX, sal_uInt8 cIndex)
{
    SetPixel(nY, nX, BitmapColor(cIndex));
}

#endif // INCLUDED_VCL_BMPACC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
