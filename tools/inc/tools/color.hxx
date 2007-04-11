/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: color.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:08:56 $
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
#ifndef _TOOLS_COLOR_HXX
#define _TOOLS_COLOR_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

class SvStream;
class ResId;

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

// --------------------
// - ColorCount-Types -
// --------------------

#define COLCOUNT_MONOCHROM          ((ULONG)2)
#define COLCOUNT_16                 ((ULONG)16)
#define COLCOUNT_256                ((ULONG)256)
#define COLCOUNT_HICOLOR1           (((ULONG)0x00007FFF)+1)
#define COLCOUNT_HICOLOR2           (((ULONG)0x0000FFFF)+1)
#define COLCOUNT_TRUECOLOR          (((ULONG)0x00FFFFFF)+1)

// ---------------
// - Color-Types -
// ---------------

typedef UINT32 ColorData;
#define RGB_COLORDATA( r,g,b )      ((ColorData)(((UINT32)((UINT8)(b))))|(((UINT32)((UINT8)(g)))<<8)|(((UINT32)((UINT8)(r)))<<16))
#define TRGB_COLORDATA( t,r,g,b )   ((ColorData)(((UINT32)((UINT8)(b))))|(((UINT32)((UINT8)(g)))<<8)|(((UINT32)((UINT8)(r)))<<16)|(((UINT32)((UINT8)(t)))<<24))
#define COLORDATA_RED( n )          ((UINT8)((n)>>16))
#define COLORDATA_GREEN( n )        ((UINT8)(((UINT16)(n)) >> 8))
#define COLORDATA_BLUE( n )         ((UINT8)(n))
#define COLORDATA_TRANSPARENCY( n ) ((UINT8)((n)>>24))
#define COLORDATA_RGB( n )          ((ColorData)((n) & 0x00FFFFFF))

#define COL_BLACK                   RGB_COLORDATA( 0x00, 0x00, 0x00 )
#define COL_BLUE                    RGB_COLORDATA( 0x00, 0x00, 0x80 )
#define COL_GREEN                   RGB_COLORDATA( 0x00, 0x80, 0x00 )
#define COL_CYAN                    RGB_COLORDATA( 0x00, 0x80, 0x80 )
#define COL_RED                     RGB_COLORDATA( 0x80, 0x00, 0x00 )
#define COL_MAGENTA                 RGB_COLORDATA( 0x80, 0x00, 0x80 )
#define COL_BROWN                   RGB_COLORDATA( 0x80, 0x80, 0x00 )
#define COL_GRAY                    RGB_COLORDATA( 0x80, 0x80, 0x80 )
#define COL_LIGHTGRAY               RGB_COLORDATA( 0xC0, 0xC0, 0xC0 )
#define COL_LIGHTBLUE               RGB_COLORDATA( 0x00, 0x00, 0xFF )
#define COL_LIGHTGREEN              RGB_COLORDATA( 0x00, 0xFF, 0x00 )
#define COL_LIGHTCYAN               RGB_COLORDATA( 0x00, 0xFF, 0xFF )
#define COL_LIGHTRED                RGB_COLORDATA( 0xFF, 0x00, 0x00 )
#define COL_LIGHTMAGENTA            RGB_COLORDATA( 0xFF, 0x00, 0xFF )
#define COL_YELLOW                  RGB_COLORDATA( 0xFF, 0xFF, 0x00 )
#define COL_WHITE                   RGB_COLORDATA( 0xFF, 0xFF, 0xFF )
#define COL_TRANSPARENT             TRGB_COLORDATA( 0xFF, 0xFF, 0xFF, 0xFF )
#define COL_AUTO                    (UINT32)0xFFFFFFFF

#define COLOR_CHANNEL_MERGE( _def_cDst, _def_cSrc, _def_cSrcTrans ) \
    ((BYTE)((((long)(_def_cDst)-(_def_cSrc))*(_def_cSrcTrans)+(((_def_cSrc)<<8L)|(_def_cDst)))>>8L))

// ---------
// - Color -
// ---------

class TOOLS_DLLPUBLIC Color
{
protected:
    ColorData           mnColor;

public:
                        Color() { mnColor = COL_BLACK; }
                        Color( ColorData nColor ) { mnColor = nColor; }
                        Color( UINT8 nRed, UINT8 nGreen, UINT8 nBlue )
                            { mnColor = RGB_COLORDATA( nRed, nGreen, nBlue ); }
                        Color( UINT8 nTransparency, UINT8 nRed, UINT8 nGreen, UINT8 nBlue )
                            { mnColor = TRGB_COLORDATA( nTransparency, nRed, nGreen, nBlue ); }
                        Color( const ResId& rResId );
                         // This ctor is defined in svtools, not tools!

    void                SetRed( UINT8 nRed );
    UINT8               GetRed() const { return COLORDATA_RED( mnColor ); }
    void                SetGreen( UINT8 nGreen );
    UINT8               GetGreen() const { return COLORDATA_GREEN( mnColor ); }
    void                SetBlue( UINT8 nBlue );
    UINT8               GetBlue() const { return COLORDATA_BLUE( mnColor ); }
    void                SetTransparency( UINT8 nTransparency );
    UINT8               GetTransparency() const { return COLORDATA_TRANSPARENCY( mnColor ); }

    void                SetColor( ColorData nColor ) { mnColor = nColor; }
    ColorData           GetColor() const { return mnColor; }
    ColorData           GetRGBColor() const { return COLORDATA_RGB( mnColor ); }

    UINT8               GetColorError( const Color& rCompareColor ) const;

    UINT8               GetLuminance() const;
    void                IncreaseLuminance( UINT8 cLumInc );
    void                DecreaseLuminance( UINT8 cLumDec );

    void                IncreaseContrast( UINT8 cContInc );
    void                DecreaseContrast( UINT8 cContDec );

    void                Invert();

    void                Merge( const Color& rMergeColor, BYTE cTransparency );

    BOOL                IsRGBEqual( const Color& rColor ) const;

    // comparison with luminance thresholds
    BOOL                IsDark()    const;
    BOOL                IsBright()  const;

    // color space conversion tools
    // the range for h/s/b is:
    // Hue: 0-360 degree
    // Saturation: 0-100 %
    // Brightness: 0-100 %
    static ColorData    HSBtoRGB( USHORT nHue, USHORT nSat, USHORT nBri );
    void                RGBtoHSB( USHORT& nHue, USHORT& nSat, USHORT& nBri ) const;

    BOOL                operator==( const Color& rColor ) const
                            { return (mnColor == rColor.mnColor); }
    BOOL                operator!=( const Color& rColor ) const
                            { return !(Color::operator==( rColor )); }

    SvStream&           Read( SvStream& rIStm, BOOL bNewFormat = TRUE );
    SvStream&           Write( SvStream& rOStm, BOOL bNewFormat = TRUE );

    TOOLS_DLLPUBLIC friend SvStream&    operator>>( SvStream& rIStream, Color& rColor );
    TOOLS_DLLPUBLIC friend SvStream&    operator<<( SvStream& rOStream, const Color& rColor );
};

inline void Color::SetRed( UINT8 nRed )
{
    mnColor &= 0xFF00FFFF;
    mnColor |= ((UINT32)nRed)<<16;
}

inline void Color::SetGreen( UINT8 nGreen )
{
    mnColor &= 0xFFFF00FF;
    mnColor |= ((UINT16)nGreen)<<8;
}

inline void Color::SetBlue( UINT8 nBlue )
{
    mnColor &= 0xFFFFFF00;
    mnColor |= nBlue;
}

inline void Color::SetTransparency( UINT8 nTransparency )
{
    mnColor &= 0x00FFFFFF;
    mnColor |= ((UINT32)nTransparency)<<24;
}

inline BOOL Color::IsRGBEqual( const Color& rColor ) const
{
    return (COLORDATA_RGB( mnColor ) == COLORDATA_RGB( rColor.mnColor ));
}

inline UINT8 Color::GetLuminance() const
{
    return( (UINT8) ( ( COLORDATA_BLUE( mnColor ) * 28UL +
                        COLORDATA_GREEN( mnColor ) * 151UL +
                        COLORDATA_RED( mnColor ) * 77UL ) >> 8UL ) );
}

inline void Color::Merge( const Color& rMergeColor, BYTE cTransparency )
{
    SetRed( COLOR_CHANNEL_MERGE( COLORDATA_RED( mnColor ), COLORDATA_RED( rMergeColor.mnColor ), cTransparency ) );
    SetGreen( COLOR_CHANNEL_MERGE( COLORDATA_GREEN( mnColor ), COLORDATA_GREEN( rMergeColor.mnColor ), cTransparency ) );
    SetBlue( COLOR_CHANNEL_MERGE( COLORDATA_BLUE( mnColor ), COLORDATA_BLUE( rMergeColor.mnColor ), cTransparency ) );
}

#endif // _TOOLS_COLOR_HXX
