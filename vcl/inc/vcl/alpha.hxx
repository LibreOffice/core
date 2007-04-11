/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: alpha.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:47:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_ALPHA_HXX
#define _SV_ALPHA_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

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
