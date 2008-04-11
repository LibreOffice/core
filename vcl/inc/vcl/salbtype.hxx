/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salbtype.hxx,v $
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

#ifndef _SV_SALBTYPE_HXX
#define _SV_SALBTYPE_HXX
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <osl/endian.h>
#include <tools/debug.hxx>
#include <vcl/salgtype.hxx>
#include <tools/color.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

// ----------
// - Memory -
// ----------

typedef BYTE*       HPBYTE;
typedef HPBYTE      Scanline;
typedef const BYTE* ConstHPBYTE;
typedef ConstHPBYTE ConstScanline;

// --------------------
// - Helper functions -
// --------------------

inline long MinMax( long nVal, long nMin, long nMax );
inline long AlignedWidth4Bytes( long nWidthBits );
inline long FRound( double fVal );

// ------------------
// - Bitmap formats -
// ------------------

#define BMP_FORMAT_BOTTOM_UP                        0x00000000UL
#define BMP_FORMAT_TOP_DOWN                         0x80000000UL

#define BMP_FORMAT_1BIT_MSB_PAL                     0x00000001UL
#define BMP_FORMAT_1BIT_LSB_PAL                     0x00000002UL

#define BMP_FORMAT_4BIT_MSN_PAL                     0x00000004UL
#define BMP_FORMAT_4BIT_LSN_PAL                     0x00000008UL

#define BMP_FORMAT_8BIT_PAL                         0x00000010UL
#define BMP_FORMAT_8BIT_TC_MASK                     0x00000020UL

// #define BMP_FORMAT_16BIT_TC_MASK                 0x00000040UL

#define BMP_FORMAT_24BIT_TC_BGR                     0x00000080UL
#define BMP_FORMAT_24BIT_TC_RGB                     0x00000100UL
#define BMP_FORMAT_24BIT_TC_MASK                    0x00000200UL

#define BMP_FORMAT_32BIT_TC_ABGR                    0x00000400UL
#define BMP_FORMAT_32BIT_TC_ARGB                    0x00000800UL
#define BMP_FORMAT_32BIT_TC_BGRA                    0x00001000UL
#define BMP_FORMAT_32BIT_TC_RGBA                    0x00002000UL
#define BMP_FORMAT_32BIT_TC_MASK                    0x00004000UL

#define BMP_FORMAT_16BIT_TC_MSB_MASK                0x00008000UL
#define BMP_FORMAT_16BIT_TC_LSB_MASK                0x00010000UL

#define BMP_SCANLINE_ADJUSTMENT( Mac_nBmpFormat )   ( (Mac_nBmpFormat) & 0x80000000UL )
#define BMP_SCANLINE_FORMAT( Mac_nBmpFormat )       ( (Mac_nBmpFormat) & 0x7FFFFFFFUL )

// ------------------------------------------------------------------

#define MASK_TO_COLOR( d_nVal, d_RM, d_GM, d_BM, d_RS, d_GS, d_BS, d_Col )                          \
ULONG _def_cR = (BYTE) ( d_RS < 0L ? ( (d_nVal) & d_RM ) << -d_RS : ( (d_nVal) & d_RM ) >> d_RS );  \
ULONG _def_cG = (BYTE) ( d_GS < 0L ? ( (d_nVal) & d_GM ) << -d_GS : ( (d_nVal) & d_GM ) >> d_GS );  \
ULONG _def_cB = (BYTE) ( d_BS < 0L ? ( (d_nVal) & d_BM ) << -d_BS : ( (d_nVal) & d_BM ) >> d_BS );  \
d_Col = BitmapColor( (BYTE) ( _def_cR | ( ( _def_cR & mnROr ) >> mnROrShift ) ),                    \
                     (BYTE) ( _def_cG | ( ( _def_cG & mnGOr ) >> mnGOrShift ) ),                    \
                     (BYTE) ( _def_cB | ( ( _def_cB & mnBOr ) >> mnBOrShift ) ) );

// ------------------------------------------------------------------

#define COLOR_TO_MASK( d_rCol, d_RM, d_GM, d_BM, d_RS, d_GS, d_BS ) \
( ( ( ( d_RS < 0L ) ? ( (UINT32) (d_rCol).GetRed() >> -d_RS ) :     \
    ( (UINT32) (d_rCol).GetRed() << d_RS ) ) & d_RM ) |             \
  ( ( ( d_GS < 0L ) ? ( (UINT32) (d_rCol).GetGreen() >> -d_GS ) :   \
    ( (UINT32) (d_rCol).GetGreen() << d_GS ) ) & d_GM ) |           \
  ( ( ( d_BS < 0L ) ? ( (UINT32) (d_rCol).GetBlue() >> -d_BS ) :    \
    ( (UINT32) (d_rCol).GetBlue() << d_BS ) ) & d_BM ) )

// ---------------
// - BitmapColor -
// ---------------

class Color;

class VCL_DLLPUBLIC BitmapColor
{
private:

// !!! Achtung:
// !!! da auf die Member dieser Klasse via memcpy
// !!! zugegriffen wird, darf diese Klasse weder
// !!! in der Groesse noch der Reihenfolge der
// !!! Member veraendert werden (KA 02.09.97)
    BYTE                mcBlueOrIndex;
    BYTE                mcGreen;
    BYTE                mcRed;
    BYTE                mbIndex;

public:

    inline              BitmapColor();
    inline              BitmapColor( const BitmapColor& rBitmapColor );
    inline              BitmapColor( BYTE cRed, BYTE cGreen, BYTE cBlue );
    inline              BitmapColor( const Color& rColor );
    inline              BitmapColor( BYTE cIndex );
    inline              ~BitmapColor() {};

    inline BOOL         operator==( const BitmapColor& rBitmapColor ) const;
    inline BOOL         operator!=( const BitmapColor& rBitmapColor ) const;
    inline BitmapColor& operator=( const BitmapColor& rBitmapColor );

    inline BOOL         IsIndex() const;

    inline BYTE         GetRed() const;
    inline void         SetRed( BYTE cRed );

    inline BYTE         GetGreen() const;
    inline void         SetGreen( BYTE cGreen );

    inline BYTE         GetBlue() const;
    inline void         SetBlue( BYTE cBlue );

    inline BYTE         GetIndex() const;
    inline void         SetIndex( BYTE cIndex );

    operator            Color() const;
    inline operator     BYTE() const;

    inline BYTE         GetBlueOrIndex() const;

    inline BitmapColor& Invert();

    inline BYTE         GetLuminance() const;
    inline BitmapColor& IncreaseLuminance( BYTE cGreyInc );
    inline BitmapColor& DecreaseLuminance( BYTE cGreyDec );

    inline BitmapColor& Merge( const BitmapColor& rColor, BYTE cTransparency );
    inline BitmapColor& Merge( BYTE cR, BYTE cG, BYTE cB, BYTE cTransparency );

    inline ULONG        GetColorError( const BitmapColor& rBitmapColor ) const;
};

// ---------------
// - BitmapPalette -
// ---------------

class Palette;

class VCL_DLLPUBLIC BitmapPalette
{
    friend class SalBitmap;
    friend class BitmapAccess;

private:

    BitmapColor*                mpBitmapColor;
    USHORT                      mnCount;

//#if 0 // _SOLAR__PRIVATE

public:

    SAL_DLLPRIVATE inline BitmapColor* ImplGetColorBuffer() const;

//#endif // __PRIVATE

public:

    inline                      BitmapPalette();
    inline                      BitmapPalette( const BitmapPalette& rBitmapPalette );
    inline                      BitmapPalette( USHORT nCount );
    inline                      ~BitmapPalette();

    inline BitmapPalette&       operator=( const BitmapPalette& rBitmapPalette );
    inline BOOL                 operator==( const BitmapPalette& rBitmapPalette ) const;
    inline BOOL                 operator!=( const BitmapPalette& rBitmapPalette ) const;
    inline BOOL                 operator!();

    inline USHORT               GetEntryCount() const;
    inline void                 SetEntryCount( USHORT nCount );

    inline const BitmapColor&   operator[]( USHORT nIndex ) const;
    inline BitmapColor&         operator[]( USHORT nIndex );

    inline USHORT               GetBestIndex( const BitmapColor& rCol ) const;
};

// ---------------
// - ColorMask -
// ---------------

class VCL_DLLPUBLIC ColorMask
{
    ULONG               mnRMask;
    ULONG               mnGMask;
    ULONG               mnBMask;
    long                mnRShift;
    long                mnGShift;
    long                mnBShift;
    ULONG               mnROrShift;
    ULONG               mnGOrShift;
    ULONG               mnBOrShift;
    ULONG               mnROr;
    ULONG               mnGOr;
    ULONG               mnBOr;

    SAL_DLLPRIVATE inline long ImplCalcMaskShift( ULONG nMask, ULONG& rOr, ULONG& rOrShift ) const;

public:

    inline              ColorMask( ULONG nRedMask = 0UL, ULONG nGreenMask = 0UL, ULONG nBlueMask = 0UL );
    inline              ~ColorMask() {}

    inline ULONG        GetRedMask() const;
    inline ULONG        GetGreenMask() const;
    inline ULONG        GetBlueMask() const;

    inline void         GetColorFor8Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor8Bit( const BitmapColor& rColor, HPBYTE pPixel ) const;

    inline void         GetColorFor16BitMSB( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor16BitMSB( const BitmapColor& rColor, HPBYTE pPixel ) const;
    inline void         GetColorFor16BitLSB( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor16BitLSB( const BitmapColor& rColor, HPBYTE pPixel ) const;

    inline void         GetColorFor24Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor24Bit( const BitmapColor& rColor, HPBYTE pPixel ) const;

    inline void         GetColorFor32Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const;
    inline void         SetColorFor32Bit( const BitmapColor& rColor, HPBYTE pPixel ) const;
};

// ---------------
// - BitmapBuffer -
// ---------------

struct VCL_DLLPUBLIC BitmapBuffer
{
    ULONG           mnFormat;
    long            mnWidth;
    long            mnHeight;
    long            mnScanlineSize;
    USHORT          mnBitCount;
    ColorMask       maColorMask;
    BitmapPalette   maPalette;
    BYTE*           mpBits;

                    BitmapBuffer(){}
                    ~BitmapBuffer() {}
};

// ---------------------
// - StretchAndConvert -
// ---------------------

VCL_DLLPUBLIC BitmapBuffer* StretchAndConvert( const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
                                               ULONG nDstBitmapFormat, BitmapPalette* pDstPal = NULL, ColorMask* pDstMask = NULL );

// ---------------
// - Inlines -
// ---------------

inline long MinMax( long nVal, long nMin, long nMax )
{
    return( nVal >= nMin ? ( nVal <= nMax ? nVal : nMax ) : nMin );
}

// ------------------------------------------------------------------

inline long AlignedWidth4Bytes( long nWidthBits )
{
    return( ( ( nWidthBits + 31 ) >> 5 ) << 2 );
}

// ------------------------------------------------------------------

inline long FRound( double fVal )
{
    return( fVal > 0.0 ? (long) ( fVal + 0.5 ) : -(long) ( -fVal + 0.5 ) );
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor() :
            mcBlueOrIndex   ( 0 ),
            mcGreen         ( 0 ),
            mcRed           ( 0 ),
            mbIndex         ( FALSE )
{
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor( BYTE cRed, BYTE cGreen, BYTE cBlue ) :
            mcBlueOrIndex   ( cBlue ),
            mcGreen         ( cGreen ),
            mcRed           ( cRed ),
            mbIndex         ( FALSE )
{
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor( const BitmapColor& rBitmapColor ) :
            mcBlueOrIndex   ( rBitmapColor.mcBlueOrIndex ),
            mcGreen         ( rBitmapColor.mcGreen ),
            mcRed           ( rBitmapColor.mcRed ),
            mbIndex         ( rBitmapColor.mbIndex )
{
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor( const Color& rColor ) :
            mcBlueOrIndex   ( rColor.GetBlue() ),
            mcGreen         ( rColor.GetGreen() ),
            mcRed           ( rColor.GetRed() ),
            mbIndex         ( 0 )
{
}

// ------------------------------------------------------------------

inline BitmapColor::BitmapColor( BYTE cIndex ) :
            mcBlueOrIndex   ( cIndex ),
            mcGreen         ( 0 ),
            mcRed           ( 0 ),
            mbIndex         ( TRUE )
{
}

// ------------------------------------------------------------------

inline BOOL BitmapColor::operator==( const BitmapColor& rBitmapColor ) const
{
    return( ( mcBlueOrIndex == rBitmapColor.mcBlueOrIndex ) &&
            ( mbIndex ? rBitmapColor.mbIndex :
            ( mcGreen == rBitmapColor.mcGreen && mcRed == rBitmapColor.mcRed ) ) );
}

// ------------------------------------------------------------------

inline BOOL BitmapColor::operator!=( const BitmapColor& rBitmapColor ) const
{
    return !( *this == rBitmapColor );
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::operator=( const BitmapColor& rBitmapColor )
{
    mcBlueOrIndex = rBitmapColor.mcBlueOrIndex;
    mcGreen = rBitmapColor.mcGreen;
    mcRed = rBitmapColor.mcRed;
    mbIndex = rBitmapColor.mbIndex;

    return *this;
}

// ------------------------------------------------------------------

inline BOOL BitmapColor::IsIndex() const
{
    return mbIndex;
}

// ------------------------------------------------------------------

inline BYTE BitmapColor::GetRed() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcRed;
}

// ------------------------------------------------------------------

inline void BitmapColor::SetRed( BYTE cRed )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcRed = cRed;
}

// ------------------------------------------------------------------

inline BYTE BitmapColor::GetGreen() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcGreen;
}

// ------------------------------------------------------------------

inline void BitmapColor::SetGreen( BYTE cGreen )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcGreen = cGreen;
}

// ------------------------------------------------------------------

inline BYTE BitmapColor::GetBlue() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return mcBlueOrIndex;
}

// ------------------------------------------------------------------

inline void BitmapColor::SetBlue( BYTE cBlue )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = cBlue;
}

// ------------------------------------------------------------------

inline BYTE BitmapColor::GetIndex() const
{
    DBG_ASSERT( mbIndex, "Pixel represents color values!" );
    return mcBlueOrIndex;
}

// ------------------------------------------------------------------

inline void BitmapColor::SetIndex( BYTE cIndex )
{
    DBG_ASSERT( mbIndex, "Pixel represents color values!" );
    mcBlueOrIndex = cIndex;
}

// ------------------------------------------------------------------

inline BitmapColor::operator Color() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return Color( mcRed, mcGreen, mcBlueOrIndex );
}

// ------------------------------------------------------------------

inline BitmapColor::operator BYTE() const
{
    DBG_ASSERT( mbIndex, "Pixel represents color values!" );
    return mcBlueOrIndex;
}

// ------------------------------------------------------------------

inline BYTE BitmapColor::GetBlueOrIndex() const
{
    // #i47518# Yield a value regardless of mbIndex
    return mcBlueOrIndex;
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::Invert()
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = ~mcBlueOrIndex, mcGreen = ~mcGreen, mcRed = ~mcRed;

    return *this;
}

// ------------------------------------------------------------------

inline BYTE BitmapColor::GetLuminance() const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    return( (BYTE) ( ( mcBlueOrIndex * 28UL + mcGreen * 151UL + mcRed * 77UL ) >> 8UL ) );
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::IncreaseLuminance( BYTE cGreyInc )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = (BYTE) MinMax( (long) mcBlueOrIndex + cGreyInc, 0L, 255L );
    mcGreen = (BYTE) MinMax( (long) mcGreen + cGreyInc, 0L, 255L );
    mcRed = (BYTE) MinMax( (long) mcRed + cGreyInc, 0L, 255L );

    return *this;
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::DecreaseLuminance( BYTE cGreyDec )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = (BYTE) MinMax( (long) mcBlueOrIndex - cGreyDec, 0L, 255L );
    mcGreen = (BYTE) MinMax( (long) mcGreen - cGreyDec, 0L, 255L );
    mcRed = (BYTE) MinMax( (long) mcRed - cGreyDec, 0L, 255L );

    return *this;
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::Merge( const BitmapColor& rBitmapColor, BYTE cTransparency )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    DBG_ASSERT( !rBitmapColor.mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = COLOR_CHANNEL_MERGE( mcBlueOrIndex, rBitmapColor.mcBlueOrIndex, cTransparency );
    mcGreen = COLOR_CHANNEL_MERGE( mcGreen, rBitmapColor.mcGreen, cTransparency );
    mcRed = COLOR_CHANNEL_MERGE( mcRed, rBitmapColor.mcRed, cTransparency );

    return *this;
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapColor::Merge( BYTE cR, BYTE cG, BYTE cB, BYTE cTransparency )
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    mcBlueOrIndex = COLOR_CHANNEL_MERGE( mcBlueOrIndex, cB, cTransparency );
    mcGreen = COLOR_CHANNEL_MERGE( mcGreen, cG, cTransparency );
    mcRed = COLOR_CHANNEL_MERGE( mcRed, cR, cTransparency );

    return *this;
}

// ------------------------------------------------------------------

inline ULONG BitmapColor::GetColorError( const BitmapColor& rBitmapColor ) const
{
    DBG_ASSERT( !mbIndex, "Pixel represents index into colortable!" );
    DBG_ASSERT( !rBitmapColor.mbIndex, "Pixel represents index into colortable!" );
    return( (ULONG) ( labs( mcBlueOrIndex - rBitmapColor.mcBlueOrIndex ) +
                      labs( mcGreen - rBitmapColor.mcGreen ) +
                      labs( mcRed - rBitmapColor.mcRed ) ) );
}

// ------------------------------------------------------------------

inline BitmapPalette::BitmapPalette() :
            mpBitmapColor   ( NULL ),
            mnCount         ( 0 )
{
}

// ------------------------------------------------------------------

inline BitmapPalette::BitmapPalette( const BitmapPalette& rBitmapPalette ) :
            mnCount( rBitmapPalette.mnCount )
{
    if( mnCount )
    {
        const ULONG nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = (BitmapColor*) new BYTE[ nSize ];
        memcpy( mpBitmapColor, rBitmapPalette.mpBitmapColor, nSize );
    }
    else
        mpBitmapColor = NULL;
}

// ------------------------------------------------------------------

inline BitmapPalette::BitmapPalette( USHORT nCount ) :
            mnCount( nCount )
{
    if( mnCount )
    {
        const ULONG nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = (BitmapColor*) new BYTE[ nSize ];
        memset( mpBitmapColor, 0, nSize );
    }
    else
        mpBitmapColor = NULL;
}

// ------------------------------------------------------------------

inline BitmapPalette::~BitmapPalette()
{
    delete[] (BYTE*) mpBitmapColor;
}

// ------------------------------------------------------------------

inline BitmapPalette& BitmapPalette::operator=( const BitmapPalette& rBitmapPalette )
{
    delete[] (BYTE*) mpBitmapColor;
    mnCount = rBitmapPalette.mnCount;

    if( mnCount )
    {
        const ULONG nSize = mnCount * sizeof( BitmapColor );
        mpBitmapColor = (BitmapColor*) new BYTE[ nSize ];
        memcpy( mpBitmapColor, rBitmapPalette.mpBitmapColor, nSize );
    }
    else
        mpBitmapColor = NULL;

    return *this;
}

// ------------------------------------------------------------------

inline BOOL BitmapPalette::operator==( const BitmapPalette& rBitmapPalette ) const
{
    BOOL bRet = FALSE;

    if( rBitmapPalette.mnCount == mnCount )
    {
        bRet = TRUE;

        for( USHORT i = 0; i < mnCount; i++ )
        {
            if( mpBitmapColor[ i ] != rBitmapPalette.mpBitmapColor[ i ] )
            {
                bRet = FALSE;
                break;
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

inline BOOL BitmapPalette::operator!=( const BitmapPalette& rBitmapPalette ) const
{
    return !( *this == rBitmapPalette );
}

// ------------------------------------------------------------------

inline BOOL BitmapPalette::operator!()
{
    return( !mnCount || !mpBitmapColor );
}

// ------------------------------------------------------------------

inline USHORT BitmapPalette::GetEntryCount() const
{
    return mnCount;
}

// ------------------------------------------------------------------

inline void BitmapPalette::SetEntryCount( USHORT nCount )
{
    if( !nCount )
    {
        delete[] (BYTE*) mpBitmapColor;
        mpBitmapColor = NULL;
        mnCount = 0;
    }
    else if( nCount != mnCount )
    {
        const ULONG nNewSize = nCount * sizeof( BitmapColor );
        const ULONG nMinSize = Min( mnCount, nCount ) * sizeof( BitmapColor );
        BYTE*       pNewColor = new BYTE[ nNewSize ];

        if ( nMinSize && mpBitmapColor )
            memcpy( pNewColor, mpBitmapColor, nMinSize );
        delete[] (BYTE*) mpBitmapColor;
        memset( pNewColor + nMinSize, 0, nNewSize - nMinSize );
        mpBitmapColor = (BitmapColor*) pNewColor;
        mnCount = nCount;
    }
}

// ------------------------------------------------------------------

inline const BitmapColor& BitmapPalette::operator[]( USHORT nIndex ) const
{
    DBG_ASSERT( nIndex < mnCount, "Palette index is out of range!" );
    return mpBitmapColor[ nIndex ];
}

// ------------------------------------------------------------------

inline BitmapColor& BitmapPalette::operator[]( USHORT nIndex )
{
    DBG_ASSERT( nIndex < mnCount, "Palette index is out of range!" );
    return mpBitmapColor[ nIndex ];
}

// ------------------------------------------------------------------

//#if 0 // _SOLAR__PRIVATE
inline BitmapColor* BitmapPalette::ImplGetColorBuffer() const
{
    DBG_ASSERT( mpBitmapColor, "No color buffer available!" );
    return mpBitmapColor;
}
//#endif
// ------------------------------------------------------------------

inline USHORT BitmapPalette::GetBestIndex( const BitmapColor& rCol ) const
{
    USHORT nRetIndex = 0;

    if( mpBitmapColor && mnCount )
    {
        BOOL bFound = FALSE;

        for( long j = 0L; ( j < mnCount ) && !bFound; j++ )
            if( rCol == mpBitmapColor[ j ] )
                nRetIndex = ( (USHORT) j ), bFound = TRUE;

        if( !bFound )
        {
            long nActErr, nLastErr = rCol.GetColorError( mpBitmapColor[ nRetIndex = mnCount - 1 ] );

            for( long i = nRetIndex - 1; i >= 0L; i-- )
                if ( ( nActErr = rCol.GetColorError( mpBitmapColor[ i ] ) ) < nLastErr )
                    nLastErr = nActErr, nRetIndex = (USHORT) i;
        }
    }

    return nRetIndex;
}

// ------------------------------------------------------------------

inline ColorMask::ColorMask( ULONG nRedMask, ULONG nGreenMask, ULONG nBlueMask ) :
            mnRMask( nRedMask ),
            mnGMask( nGreenMask ),
            mnBMask( nBlueMask ),
            mnROrShift( 0L ),
            mnGOrShift( 0L ),
            mnBOrShift( 0L ),
            mnROr( 0L ),
            mnGOr( 0L ),
            mnBOr( 0L )
{
    mnRShift = ( mnRMask ? ImplCalcMaskShift( mnRMask, mnROr, mnROrShift ) : 0L );
    mnGShift = ( mnGMask ? ImplCalcMaskShift( mnGMask, mnGOr, mnGOrShift ) : 0L );
    mnBShift = ( mnBMask ? ImplCalcMaskShift( mnBMask, mnBOr, mnBOrShift ) : 0L );
}

// ------------------------------------------------------------------

inline long ColorMask::ImplCalcMaskShift( ULONG nMask, ULONG& rOr, ULONG& rOrShift ) const
{
    long    nShift;
    long    nRet;
    ULONG   nLen = 0UL;

    // bei welchen Bits faengt die Maske an
    for( nShift = 31L; ( nShift >= 0L ) && !( nMask & ( 1 << (ULONG) nShift ) ); nShift-- )
    {}

    nRet = nShift;

    // XXX Anzahl der gesetzten Bits ermitteln => nach rechts bis Null laufen
    while( ( nShift >= 0L ) && ( nMask & ( 1 << (ULONG) nShift ) ) )
    {
        nShift--;
        nLen++;
    }

    rOrShift = 8L - nLen;
    rOr = (BYTE) ( ( 0xffUL >> nLen ) << rOrShift );

    return( nRet -= 7 );
}

// ------------------------------------------------------------------

inline ULONG ColorMask::GetRedMask() const
{
    return mnRMask;
}

// ------------------------------------------------------------------

inline ULONG ColorMask::GetGreenMask() const
{
    return mnGMask;
}

// ------------------------------------------------------------------

inline ULONG ColorMask::GetBlueMask() const
{
    return mnBMask;
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorFor8Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
    const UINT32 nVal = *pPixel;
    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor8Bit( const BitmapColor& rColor, HPBYTE pPixel ) const
{
    *pPixel = (BYTE) COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorFor16BitMSB( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const UINT32 nVal = *(UINT16*) pPixel;
#else
    const UINT32 nVal = pPixel[ 1 ] | ( (UINT32) pPixel[ 0 ] << 8UL );
#endif

    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor16BitMSB( const BitmapColor& rColor, HPBYTE pPixel ) const
{
    const UINT16 nVal = (UINT16)COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );

#ifdef OSL_BIGENDIAN
    *(UINT16*) pPixel = nVal;
#else
    pPixel[ 0 ] = (BYTE)(nVal >> 8U);
    pPixel[ 1 ] = (BYTE) nVal;
#endif
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorFor16BitLSB( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const UINT32 nVal = pPixel[ 0 ] | ( (UINT32) pPixel[ 1 ] << 8UL );
#else
    const UINT32 nVal = *(UINT16*) pPixel;
#endif

    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor16BitLSB( const BitmapColor& rColor, HPBYTE pPixel ) const
{
    const UINT16 nVal = (UINT16)COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );

#ifdef OSL_BIGENDIAN
    pPixel[ 0 ] = (BYTE) nVal;
    pPixel[ 1 ] = (BYTE)(nVal >> 8U);
#else
    *(UINT16*) pPixel = nVal;
#endif
}


// ------------------------------------------------------------------

inline void ColorMask::GetColorFor24Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
    const UINT32 nVal = pPixel[ 0 ] | ( (UINT32) pPixel[ 1 ] << 8UL ) | ( (UINT32) pPixel[ 2 ] << 16UL );
    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor24Bit( const BitmapColor& rColor, HPBYTE pPixel ) const
{
    const UINT32 nVal = COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );
    pPixel[ 0 ] = (BYTE) nVal; pPixel[ 1 ] = (BYTE) ( nVal >> 8UL ); pPixel[ 2 ] = (BYTE) ( nVal >> 16UL );
}

// ------------------------------------------------------------------

inline void ColorMask::GetColorFor32Bit( BitmapColor& rColor, ConstHPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const UINT32 nVal = (UINT32) pPixel[ 0 ] | ( (UINT32) pPixel[ 1 ] << 8UL ) |
                        ( (UINT32) pPixel[ 2 ] << 16UL ) | ( (UINT32) pPixel[ 3 ] << 24UL );
#else
    const UINT32 nVal = *(UINT32*) pPixel;
#endif

    MASK_TO_COLOR( nVal, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift, rColor );
}

// ------------------------------------------------------------------

inline void ColorMask::SetColorFor32Bit( const BitmapColor& rColor, HPBYTE pPixel ) const
{
#ifdef OSL_BIGENDIAN
    const UINT32 nVal = COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );
    pPixel[ 0 ] = (BYTE) nVal; pPixel[ 1 ] = (BYTE) ( nVal >> 8UL );
    pPixel[ 2 ] = (BYTE) ( nVal >> 16UL ); pPixel[ 3 ] = (BYTE) ( nVal >> 24UL );
#else
    *(UINT32*) pPixel = COLOR_TO_MASK( rColor, mnRMask, mnGMask, mnBMask, mnRShift, mnGShift, mnBShift );
#endif
}

#endif // _SV_SALBTYPE_HXX
