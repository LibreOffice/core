/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <stdlib.h>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <tools/rc.h>

// -----------
// - Inlines -
// -----------

static inline long _FRound( double fVal )
{
    return( fVal > 0.0 ? (long) ( fVal + 0.5 ) : -(long) ( -fVal + 0.5 ) );
}

// ---------
// - Color -
// ---------

Color::Color( const ResId& rResId )
{
    rResId.SetRT( RSC_COLOR );
    ResMgr* pResMgr = rResId.GetResMgr();
    if ( pResMgr && pResMgr->GetResource( rResId ) )
    {
        // Header ueberspringen
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        // Daten laden
        sal_uInt16 nRed     = pResMgr->ReadShort();
        sal_uInt16 nGreen   = pResMgr->ReadShort();
        sal_uInt16 nBlue    = pResMgr->ReadShort();
        // one more historical sal_uIntPtr
        pResMgr->ReadLong();

        // RGB-Farbe
        mnColor = RGB_COLORDATA( nRed>>8, nGreen>>8, nBlue>>8 );
    }
    else
    {
        mnColor = RGB_COLORDATA( 0, 0, 0 );
    }
}
sal_uInt8 Color::GetColorError( const Color& rCompareColor ) const
{
    const long nErrAbs = labs( (long) rCompareColor.GetRed() - GetRed() ) +
                         labs( (long) rCompareColor.GetGreen() - GetGreen() ) +
                         labs( (long) rCompareColor.GetBlue() - GetBlue() );

    return (sal_uInt8) _FRound( nErrAbs * 0.3333333333 );
}

// -----------------------------------------------------------------------

void Color::IncreaseLuminance( sal_uInt8 cLumInc )
{
    SetRed( (sal_uInt8) SAL_BOUND( (long) COLORDATA_RED( mnColor ) + cLumInc, 0L, 255L ) );
    SetGreen( (sal_uInt8) SAL_BOUND( (long) COLORDATA_GREEN( mnColor ) + cLumInc, 0L, 255L ) );
    SetBlue( (sal_uInt8) SAL_BOUND( (long) COLORDATA_BLUE( mnColor ) + cLumInc, 0L, 255L ) );
}

// -----------------------------------------------------------------------

void Color::DecreaseLuminance( sal_uInt8 cLumDec )
{
    SetRed( (sal_uInt8) SAL_BOUND( (long) COLORDATA_RED( mnColor ) - cLumDec, 0L, 255L ) );
    SetGreen( (sal_uInt8) SAL_BOUND( (long) COLORDATA_GREEN( mnColor ) - cLumDec, 0L, 255L ) );
    SetBlue( (sal_uInt8) SAL_BOUND( (long) COLORDATA_BLUE( mnColor ) - cLumDec, 0L, 255L ) );
}

// -----------------------------------------------------------------------

void Color::IncreaseContrast( sal_uInt8 cContInc )
{
    if( cContInc)
    {
        const double fM = 128.0 / ( 128.0 - 0.4985 * cContInc );
        const double fOff = 128.0 - fM * 128.0;

        SetRed( (sal_uInt8) SAL_BOUND( _FRound( COLORDATA_RED( mnColor ) * fM + fOff ), 0L, 255L ) );
        SetGreen( (sal_uInt8) SAL_BOUND( _FRound( COLORDATA_GREEN( mnColor ) * fM + fOff ), 0L, 255L ) );
        SetBlue( (sal_uInt8) SAL_BOUND( _FRound( COLORDATA_BLUE( mnColor ) * fM + fOff ), 0L, 255L ) );
    }
}

// -----------------------------------------------------------------------

void Color::DecreaseContrast( sal_uInt8 cContDec )
{
    if( cContDec )
    {
        const double fM = ( 128.0 - 0.4985 * cContDec ) / 128.0;
        const double fOff = 128.0 - fM * 128.0;

        SetRed( (sal_uInt8) SAL_BOUND( _FRound( COLORDATA_RED( mnColor ) * fM + fOff ), 0L, 255L ) );
        SetGreen( (sal_uInt8) SAL_BOUND( _FRound( COLORDATA_GREEN( mnColor ) * fM + fOff ), 0L, 255L ) );
        SetBlue( (sal_uInt8) SAL_BOUND( _FRound( COLORDATA_BLUE( mnColor ) * fM + fOff ), 0L, 255L ) );
    }
}

// -----------------------------------------------------------------------

void Color::Invert()
{
    SetRed( ~COLORDATA_RED( mnColor ) );
    SetGreen( ~COLORDATA_GREEN( mnColor ) );
    SetBlue( ~COLORDATA_BLUE( mnColor ) );
}

// -----------------------------------------------------------------------

sal_Bool Color::IsDark() const
{
    return GetLuminance() <= 38;
}

// -----------------------------------------------------------------------

sal_Bool Color::IsBright() const
{
    return GetLuminance() >= 245;
}

// -----------------------------------------------------------------------
// color space conversion
// -----------------------------------------------------------------------

void Color::RGBtoHSB( sal_uInt16& nHue, sal_uInt16& nSat, sal_uInt16& nBri ) const
{
    sal_uInt8 c[3];
    sal_uInt8 cMax, cMin;

    c[0] = GetRed();
    c[1] = GetGreen();
    c[2] = GetBlue();

    cMax = c[0];
    if( c[1] > cMax )
        cMax = c[1];
    if( c[2] > cMax )
        cMax = c[2];

    // Brightness = max(R, G, B);
    nBri = cMax * 100 / 255;

    cMin = c[0];
    if( c[1] < cMin )
        cMin = c[1];
    if( c[2] < cMin )
        cMin = c[2];

    sal_uInt8 cDelta = cMax - cMin;

    // Saturation = max - min / max
    if( nBri > 0 )
        nSat = cDelta * 100 / cMax;
    else
        nSat = 0;

    if( nSat == 0 )
        nHue = 0; // Default = undefined
    else
    {
        double dHue = 0.0;

        if( c[0] == cMax )
        {
            dHue = (double)( c[1] - c[2] ) / (double)cDelta;
        }
        else if( c[1] == cMax )
        {
            dHue = 2.0 + (double)( c[2] - c[0] ) / (double)cDelta;
        }
        else if ( c[2] == cMax )
        {
            dHue = 4.0 + (double)( c[0] - c[1] ) / (double)cDelta;
        }
        dHue *= 60.0;

        if( dHue < 0.0 )
            dHue += 360.0;

        nHue = (sal_uInt16) dHue;
    }
}

ColorData Color::HSBtoRGB( sal_uInt16 nHue, sal_uInt16 nSat, sal_uInt16 nBri )
{
    sal_uInt8 cR=0,cG=0,cB=0;
    sal_uInt8 nB = (sal_uInt8) ( nBri * 255 / 100 );

    if( nSat == 0 )
    {
        cR = nB;
        cG = nB;
        cB = nB;
    }
    else
    {
        double dH = nHue;
        double f;
        sal_uInt16 n;
        if( dH == 360.0 )
            dH = 0.0;

        dH /= 60.0;
        n = (sal_uInt16) dH;
        f = dH - n;

        sal_uInt8 a = (sal_uInt8) ( nB * ( 100 - nSat ) / 100 );
        sal_uInt8 b = (sal_uInt8) ( nB * ( 100 - ( (double)nSat * f ) ) / 100 );
        sal_uInt8 c = (sal_uInt8) ( nB * ( 100 - ( (double)nSat * ( 1.0 - f ) ) ) / 100 );

        switch( n )
        {
            case 0: cR = nB;    cG = c;     cB = a;     break;
            case 1: cR = b;     cG = nB;    cB = a;     break;
            case 2: cR = a;     cG = nB;    cB = c;     break;
            case 3: cR = a;     cG = b;     cB = nB;    break;
            case 4: cR = c;     cG = a;     cB = nB;    break;
            case 5: cR = nB;    cG = a;     cB = b;     break;
        }
    }

    return RGB_COLORDATA( cR, cG, cB );
}

// -----------------------------------------------------------------------

SvStream& Color::Read( SvStream& rIStm, sal_Bool bNewFormat )
{
    if ( bNewFormat )
        rIStm >> mnColor;
    else
        rIStm >> *this;

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& Color::Write( SvStream& rOStm, sal_Bool bNewFormat )
{
    if ( bNewFormat )
        rOStm << mnColor;
    else
        rOStm << *this;

    return rOStm;
}

// -----------------------------------------------------------------------

#define COL_NAME_USER       ((sal_uInt16)0x8000)
#define COL_RED_1B          ((sal_uInt16)0x0001)
#define COL_RED_2B          ((sal_uInt16)0x0002)
#define COL_GREEN_1B        ((sal_uInt16)0x0010)
#define COL_GREEN_2B        ((sal_uInt16)0x0020)
#define COL_BLUE_1B         ((sal_uInt16)0x0100)
#define COL_BLUE_2B         ((sal_uInt16)0x0200)

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, Color& rColor )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "Color::>> - Solar-Version not set on rIStream" );

    sal_uInt16      nColorName;
    sal_uInt16      nRed;
    sal_uInt16      nGreen;
    sal_uInt16      nBlue;

    rIStream >> nColorName;

    if ( nColorName & COL_NAME_USER )
    {
        if ( rIStream.GetCompressMode() == COMPRESSMODE_FULL )
        {
            unsigned char   cAry[6];
            sal_uInt16          i = 0;

            nRed    = 0;
            nGreen  = 0;
            nBlue   = 0;

            if ( nColorName & COL_RED_2B )
                i += 2;
            else if ( nColorName & COL_RED_1B )
                i++;
            if ( nColorName & COL_GREEN_2B )
                i += 2;
            else if ( nColorName & COL_GREEN_1B )
                i++;
            if ( nColorName & COL_BLUE_2B )
                i += 2;
            else if ( nColorName & COL_BLUE_1B )
                i++;

            rIStream.Read( cAry, i );
            i = 0;

            if ( nColorName & COL_RED_2B )
            {
                nRed = cAry[i];
                nRed <<= 8;
                i++;
                nRed |= cAry[i];
                i++;
            }
            else if ( nColorName & COL_RED_1B )
            {
                nRed = cAry[i];
                nRed <<= 8;
                i++;
            }
            if ( nColorName & COL_GREEN_2B )
            {
                nGreen = cAry[i];
                nGreen <<= 8;
                i++;
                nGreen |= cAry[i];
                i++;
            }
            else if ( nColorName & COL_GREEN_1B )
            {
                nGreen = cAry[i];
                nGreen <<= 8;
                i++;
            }
            if ( nColorName & COL_BLUE_2B )
            {
                nBlue = cAry[i];
                nBlue <<= 8;
                i++;
                nBlue |= cAry[i];
                i++;
            }
            else if ( nColorName & COL_BLUE_1B )
            {
                nBlue = cAry[i];
                nBlue <<= 8;
                i++;
            }
        }
        else
        {
            rIStream >> nRed;
            rIStream >> nGreen;
            rIStream >> nBlue;
        }

        rColor.mnColor = RGB_COLORDATA( nRed>>8, nGreen>>8, nBlue>>8 );
    }
    else
    {
        static ColorData aColAry[] =
        {
            COL_BLACK,                          // COL_BLACK
            COL_BLUE,                           // COL_BLUE
            COL_GREEN,                          // COL_GREEN
            COL_CYAN,                           // COL_CYAN
            COL_RED,                            // COL_RED
            COL_MAGENTA,                        // COL_MAGENTA
            COL_BROWN,                          // COL_BROWN
            COL_GRAY,                           // COL_GRAY
            COL_LIGHTGRAY,                      // COL_LIGHTGRAY
            COL_LIGHTBLUE,                      // COL_LIGHTBLUE
            COL_LIGHTGREEN,                     // COL_LIGHTGREEN
            COL_LIGHTCYAN,                      // COL_LIGHTCYAN
            COL_LIGHTRED,                       // COL_LIGHTRED
            COL_LIGHTMAGENTA,                   // COL_LIGHTMAGENTA
            COL_YELLOW,                         // COL_YELLOW
            COL_WHITE,                          // COL_WHITE
            COL_WHITE,                          // COL_MENUBAR
            COL_BLACK,                          // COL_MENUBARTEXT
            COL_WHITE,                          // COL_POPUPMENU
            COL_BLACK,                          // COL_POPUPMENUTEXT
            COL_BLACK,                          // COL_WINDOWTEXT
            COL_WHITE,                          // COL_WINDOWWORKSPACE
            COL_BLACK,                          // COL_HIGHLIGHT
            COL_WHITE,                          // COL_HIGHLIGHTTEXT
            COL_BLACK,                          // COL_3DTEXT
            COL_LIGHTGRAY,                      // COL_3DFACE
            COL_WHITE,                          // COL_3DLIGHT
            COL_GRAY,                           // COL_3DSHADOW
            COL_LIGHTGRAY,                      // COL_SCROLLBAR
            COL_WHITE,                          // COL_FIELD
            COL_BLACK                           // COL_FIELDTEXT
        };

        if ( nColorName < (sizeof( aColAry )/sizeof(ColorData)) )
            rColor.mnColor = aColAry[nColorName];
        else
            rColor.mnColor = COL_BLACK;
    }

    return rIStream;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const Color& rColor )
{
    DBG_ASSERTWARNING( rOStream.GetVersion(), "Color::<< - Solar-Version not set on rOStream" );

    sal_uInt16 nColorName   = COL_NAME_USER;
    sal_uInt16 nRed         = rColor.GetRed();
    sal_uInt16 nGreen       = rColor.GetGreen();
    sal_uInt16 nBlue        = rColor.GetBlue();
    nRed    = (nRed<<8) + nRed;
    nGreen  = (nGreen<<8) + nGreen;
    nBlue   = (nBlue<<8) + nBlue;

    if ( rOStream.GetCompressMode() == COMPRESSMODE_FULL )
    {
        unsigned char   cAry[6];
        sal_uInt16          i = 0;

        if ( nRed & 0x00FF )
        {
            nColorName |= COL_RED_2B;
            cAry[i] = (unsigned char)(nRed & 0xFF);
            i++;
            cAry[i] = (unsigned char)((nRed >> 8) & 0xFF);
            i++;
        }
        else if ( nRed & 0xFF00 )
        {
            nColorName |= COL_RED_1B;
            cAry[i] = (unsigned char)((nRed >> 8) & 0xFF);
            i++;
        }
        if ( nGreen & 0x00FF )
        {
            nColorName |= COL_GREEN_2B;
            cAry[i] = (unsigned char)(nGreen & 0xFF);
            i++;
            cAry[i] = (unsigned char)((nGreen >> 8) & 0xFF);
            i++;
        }
        else if ( nGreen & 0xFF00 )
        {
            nColorName |= COL_GREEN_1B;
            cAry[i] = (unsigned char)((nGreen >> 8) & 0xFF);
            i++;
        }
        if ( nBlue & 0x00FF )
        {
            nColorName |= COL_BLUE_2B;
            cAry[i] = (unsigned char)(nBlue & 0xFF);
            i++;
            cAry[i] = (unsigned char)((nBlue >> 8) & 0xFF);
            i++;
        }
        else if ( nBlue & 0xFF00 )
        {
            nColorName |= COL_BLUE_1B;
            cAry[i] = (unsigned char)((nBlue >> 8) & 0xFF);
            i++;
        }

        rOStream << nColorName;
        rOStream.Write( cAry, i );
    }
    else
    {
        rOStream << nColorName;
        rOStream << nRed;
        rOStream << nGreen;
        rOStream << nBlue;
    }

    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
