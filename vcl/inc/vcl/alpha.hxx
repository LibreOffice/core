/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_ALPHA_HXX
#define _SV_ALPHA_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>

// -------------
// - AlphaMask -
// -------------

class ImageList;
class BitmapEx;

class VCL_DLLPUBLIC AlphaMask : private Bitmap
{
    friend class BitmapEx;
    friend class OutputDevice;
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream&, const ImageList& );

private:

    SAL_DLLPRIVATE const Bitmap&    ImplGetBitmap() const;
    SAL_DLLPRIVATE void             ImplSetBitmap( const Bitmap& rBitmap );

public:

                            AlphaMask();
                            AlphaMask( const Bitmap& rBitmap );
                            AlphaMask( const AlphaMask& rAlphaMask );
                            AlphaMask( const Size& rSizePixel, sal_uInt8* pEraseTransparency = NULL );
                            ~AlphaMask();

    AlphaMask&              operator=( const Bitmap& rBitmap );
    AlphaMask&              operator=( const AlphaMask& rAlphaMask ) { return (AlphaMask&) Bitmap::operator=( rAlphaMask ); }
    sal_Bool                    operator!() const { return Bitmap::operator!(); }
    sal_Bool                    operator==( const AlphaMask& rAlphaMask ) const { return Bitmap::operator==( rAlphaMask ); }
    sal_Bool                    operator!=( const AlphaMask& rAlphaMask ) const { return Bitmap::operator!=( rAlphaMask ); }

    const MapMode&          GetPrefMapMode() const { return Bitmap::GetPrefMapMode(); }
    void                    SetPrefMapMode( const MapMode& rMapMode ) { Bitmap::SetPrefMapMode( rMapMode ); }

    const Size&             GetPrefSize() const { return Bitmap::GetPrefSize(); }
    void                    SetPrefSize( const Size& rSize ) { Bitmap::SetPrefSize( rSize ); }

    Size                    GetSizePixel() const { return Bitmap::GetSizePixel(); }
    void                    SetSizePixel( const Size& rNewSize ) { Bitmap::SetSizePixel( rNewSize ); }

    sal_uLong                   GetSizeBytes() const { return Bitmap::GetSizeBytes(); }
    sal_uLong                   GetChecksum() const { return Bitmap::GetChecksum(); }

    Bitmap                  GetBitmap() const;

public:

    sal_Bool                    Crop( const Rectangle& rRectPixel );
    sal_Bool                    Expand( sal_uLong nDX, sal_uLong nDY, sal_uInt8* pInitTransparency = NULL );
    sal_Bool                    CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc, const AlphaMask* pAlphaSrc = NULL );
    sal_Bool                    Erase( sal_uInt8 cTransparency );
    sal_Bool                    Invert();
    sal_Bool                    Mirror( sal_uLong nMirrorFlags );
    sal_Bool                    Scale( const Size& rNewSize, sal_uLong nScaleFlag = BMP_SCALE_FAST );
    sal_Bool                    Scale( const double& rScaleX, const double& rScaleY, sal_uLong nScaleFlag = BMP_SCALE_FAST );
    sal_Bool                    Rotate( long nAngle10, sal_uInt8 cFillTransparency );
    sal_Bool                    Replace( const Bitmap& rMask, sal_uInt8 rReplaceTransparency );
    sal_Bool                    Replace( sal_uInt8 cSearchTransparency, sal_uInt8 cReplaceTransparency, sal_uLong nTol = 0UL );
    sal_Bool                    Replace( sal_uInt8* pSearchTransparencies, sal_uInt8* pReplaceTransparencies,
                                     sal_uLong nColorCount, sal_uLong* pTols = NULL );

public:

    BitmapReadAccess*       AcquireReadAccess() { return Bitmap::AcquireReadAccess(); }
    BitmapWriteAccess*      AcquireWriteAccess() { return Bitmap::AcquireWriteAccess(); }
    void                    ReleaseAccess( BitmapReadAccess* pAccess );

public:

    sal_Bool                    Read( SvStream& rIStm, sal_Bool bFileHeader = sal_True ) { return Bitmap::Read( rIStm, bFileHeader ); }
    sal_Bool                    Write( SvStream& rOStm, sal_Bool bCompressed = sal_True, sal_Bool bFileHeader = sal_True ) const { return Bitmap::Write( rOStm, bCompressed, bFileHeader ); }

    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const BitmapEx& rBitmapEx );
    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, BitmapEx& rBitmapEx );
};

#endif // _SV_ALPHA_HXX
