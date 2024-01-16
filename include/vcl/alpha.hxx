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

#ifndef INCLUDED_VCL_ALPHA_HXX
#define INCLUDED_VCL_ALPHA_HXX

#include <sal/config.h>

#include <utility>

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>

class BitmapEx;

class VCL_DLLPUBLIC AlphaMask final
{
public:
                AlphaMask();
    explicit    AlphaMask( const Bitmap& rBitmap );
                AlphaMask( const AlphaMask& rAlphaMask );
                AlphaMask( AlphaMask&& rAlphaMask );
    explicit    AlphaMask( const Size& rSizePixel, const sal_uInt8* pEraseTransparency = nullptr );
                ~AlphaMask();

    AlphaMask&  operator=( const Bitmap& rBitmap );
    AlphaMask&  operator=( const AlphaMask& rAlphaMask ) { maBitmap = rAlphaMask.maBitmap; return *this; }
    AlphaMask&  operator=( AlphaMask&& rAlphaMask ) noexcept { maBitmap = std::move(rAlphaMask.maBitmap); return *this; }
    bool        operator==( const AlphaMask& rAlphaMask ) const { return maBitmap == rAlphaMask.maBitmap; }
    bool        operator!=( const AlphaMask& rAlphaMask ) const { return maBitmap != rAlphaMask.maBitmap; }

    Bitmap const & GetBitmap() const { return maBitmap; }

    void        Erase( sal_uInt8 cTransparency );
    void        BlendWith(const AlphaMask& rOther);

    /** Perform boolean OR operation with another alpha-mask

        @param rMask
        The mask bitmap in the selected combine operation

        @return true, if the operation was completed successfully.
     */
    bool        AlphaCombineOr( const AlphaMask& rMask );

    // check if alpha is used, returns true if at least one pixel has transparence
    bool        hasAlpha() const;

    bool IsEmpty() const { return maBitmap.IsEmpty(); }

    void SetEmpty() { maBitmap.SetEmpty(); }

    vcl::PixelFormat getPixelFormat() const { return maBitmap.getPixelFormat(); }

    sal_Int64 GetSizeBytes() const { return maBitmap.GetSizeBytes(); }

    Size GetSizePixel() const { return maBitmap.GetSizePixel(); }

    void SetPrefSize( const Size& rSize ) { maBitmap.SetPrefSize(rSize); }

    void SetPrefMapMode( const MapMode& rMapMode ) { maBitmap.SetPrefMapMode(rMapMode); }

    BitmapChecksum GetChecksum() const { return maBitmap.GetChecksum(); }

    bool Invert();

    bool Mirror( BmpMirrorFlags nMirrorFlags ) { return maBitmap.Mirror(nMirrorFlags); }

    bool Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag = BmpScaleFlag::Default ) { return maBitmap.Scale(rNewSize, nScaleFlag); }

    bool Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag = BmpScaleFlag::Default )
    { return maBitmap.Scale(rScaleX, rScaleY, nScaleFlag); }

    bool Convert( BmpConversion eConversion ) { return maBitmap.Convert(eConversion); }

    vcl::Region CreateRegion( const Color& rColor, const tools::Rectangle& rRect ) const { return maBitmap.CreateRegion(rColor, rRect); }

    bool Rotate( Degree10 nAngle10, const Color& rFillColor ) { return maBitmap.Rotate(nAngle10, rFillColor); }

    bool Crop( const tools::Rectangle& rRectPixel ) { return maBitmap.Crop(rRectPixel); }

    bool Expand( sal_Int32 nDX, sal_Int32 nDY, const Color* pInitColor = nullptr ) { return maBitmap.Expand(nDX, nDY, pInitColor); }

    bool CopyPixel( const tools::Rectangle& rRectDst,
                    const tools::Rectangle& rRectSrc,
                    const AlphaMask& rBmpSrc )
    { return maBitmap.CopyPixel(rRectDst, rRectSrc, rBmpSrc.maBitmap); }

    bool CopyPixel_AlphaOptimized(
                    const tools::Rectangle& rRectDst,
                    const tools::Rectangle& rRectSrc,
                    const AlphaMask& rBmpSrc )
    { return maBitmap.CopyPixel_AlphaOptimized(rRectDst, rRectSrc, rBmpSrc); }

private:
    friend class BitmapEx;
    friend class ::OutputDevice;
    friend bool VCL_DLLPUBLIC ReadDIBBitmapEx(BitmapEx& rTarget, SvStream& rIStm, bool bFileHeader, bool bMSOFormat);

    Bitmap maBitmap;
};

#endif // INCLUDED_VCL_ALPHA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
