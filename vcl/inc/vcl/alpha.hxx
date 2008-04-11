/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: alpha.hxx,v $
 * $Revision: 1.3 $
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
                            AlphaMask( const Size& rSizePixel, BYTE* pEraseTransparency = NULL );
                            ~AlphaMask();

    AlphaMask&              operator=( const Bitmap& rBitmap );
    AlphaMask&              operator=( const AlphaMask& rAlphaMask ) { return (AlphaMask&) Bitmap::operator=( rAlphaMask ); }
    BOOL                    operator!() const { return Bitmap::operator!(); }
    BOOL                    operator==( const AlphaMask& rAlphaMask ) const { return Bitmap::operator==( rAlphaMask ); }
    BOOL                    operator!=( const AlphaMask& rAlphaMask ) const { return Bitmap::operator!=( rAlphaMask ); }

    const MapMode&          GetPrefMapMode() const { return Bitmap::GetPrefMapMode(); }
    void                    SetPrefMapMode( const MapMode& rMapMode ) { Bitmap::SetPrefMapMode( rMapMode ); }

    const Size&             GetPrefSize() const { return Bitmap::GetPrefSize(); }
    void                    SetPrefSize( const Size& rSize ) { Bitmap::SetPrefSize( rSize ); }

    Size                    GetSizePixel() const { return Bitmap::GetSizePixel(); }
    void                    SetSizePixel( const Size& rNewSize ) { Bitmap::SetSizePixel( rNewSize ); }

    ULONG                   GetSizeBytes() const { return Bitmap::GetSizeBytes(); }
    ULONG                   GetChecksum() const { return Bitmap::GetChecksum(); }

    Bitmap                  GetBitmap() const;

public:

    BOOL                    Crop( const Rectangle& rRectPixel );
    BOOL                    Expand( ULONG nDX, ULONG nDY, BYTE* pInitTransparency = NULL );
    BOOL                    CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc, const AlphaMask* pAlphaSrc = NULL );
    BOOL                    Erase( BYTE cTransparency );
    BOOL                    Invert();
    BOOL                    Mirror( ULONG nMirrorFlags );
    BOOL                    Scale( const Size& rNewSize, ULONG nScaleFlag = BMP_SCALE_FAST );
    BOOL                    Scale( const double& rScaleX, const double& rScaleY, ULONG nScaleFlag = BMP_SCALE_FAST );
    BOOL                    Rotate( long nAngle10, BYTE cFillTransparency );
    BOOL                    Replace( const Bitmap& rMask, BYTE rReplaceTransparency );
    BOOL                    Replace( BYTE cSearchTransparency, BYTE cReplaceTransparency, ULONG nTol = 0UL );
    BOOL                    Replace( BYTE* pSearchTransparencies, BYTE* pReplaceTransparencies,
                                     ULONG nColorCount, ULONG* pTols = NULL );

public:

    BitmapReadAccess*       AcquireReadAccess() { return Bitmap::AcquireReadAccess(); }
    BitmapWriteAccess*      AcquireWriteAccess() { return Bitmap::AcquireWriteAccess(); }
    void                    ReleaseAccess( BitmapReadAccess* pAccess );

public:

    BOOL                    Read( SvStream& rIStm, BOOL bFileHeader = TRUE ) { return Bitmap::Read( rIStm, bFileHeader ); }
    BOOL                    Write( SvStream& rOStm, BOOL bCompressed = TRUE, BOOL bFileHeader = TRUE ) const { return Bitmap::Write( rOStm, bCompressed, bFileHeader ); }

    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const BitmapEx& rBitmapEx );
    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, BitmapEx& rBitmapEx );
};

#endif // _SV_ALPHA_HXX
