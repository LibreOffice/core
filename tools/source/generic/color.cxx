/*************************************************************************
 *
 *  $RCSfile: color.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdlib.h>

#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <color.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <stream.hxx>
#endif

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

UINT8 Color::GetColorError( const Color& rCompareColor ) const
{
    const long nErrAbs = labs( (long) rCompareColor.GetRed() - GetRed() ) +
                         labs( (long) rCompareColor.GetGreen() - GetGreen() ) +
                         labs( (long) rCompareColor.GetBlue() - GetBlue() );

    return (UINT8) _FRound( nErrAbs * 0.3333333333 );
}

// -----------------------------------------------------------------------

void Color::IncreaseLuminance( UINT8 cLumInc )
{
    SetRed( (UINT8) VOS_BOUND( (long) COLORDATA_RED( mnColor ) + cLumInc, 0L, 255L ) );
    SetGreen( (UINT8) VOS_BOUND( (long) COLORDATA_GREEN( mnColor ) + cLumInc, 0L, 255L ) );
    SetBlue( (UINT8) VOS_BOUND( (long) COLORDATA_BLUE( mnColor ) + cLumInc, 0L, 255L ) );
}

// -----------------------------------------------------------------------

void Color::DecreaseLuminance( UINT8 cLumDec )
{
    SetRed( (UINT8) VOS_BOUND( (long) COLORDATA_RED( mnColor ) - cLumDec, 0L, 255L ) );
    SetGreen( (UINT8) VOS_BOUND( (long) COLORDATA_GREEN( mnColor ) - cLumDec, 0L, 255L ) );
    SetBlue( (UINT8) VOS_BOUND( (long) COLORDATA_BLUE( mnColor ) - cLumDec, 0L, 255L ) );
}

// -----------------------------------------------------------------------

void Color::IncreaseContrast( UINT8 cContInc )
{
    if( cContInc)
    {
        const double fM = 128.0 / ( 128.0 - 0.4985 * cContInc );
        const double fOff = 128.0 - fM * 128.0;

        SetRed( (UINT8) VOS_BOUND( _FRound( COLORDATA_RED( mnColor ) * fM + fOff ), 0L, 255L ) );
        SetGreen( (UINT8) VOS_BOUND( _FRound( COLORDATA_GREEN( mnColor ) * fM + fOff ), 0L, 255L ) );
        SetBlue( (UINT8) VOS_BOUND( _FRound( COLORDATA_BLUE( mnColor ) * fM + fOff ), 0L, 255L ) );
    }
}

// -----------------------------------------------------------------------

void Color::DecreaseContrast( UINT8 cContDec )
{
    if( cContDec )
    {
        const double fM = ( 128.0 - 0.4985 * cContDec ) / 128.0;
        const double fOff = 128.0 - fM * 128.0;

        SetRed( (UINT8) VOS_BOUND( _FRound( COLORDATA_RED( mnColor ) * fM + fOff ), 0L, 255L ) );
        SetGreen( (UINT8) VOS_BOUND( _FRound( COLORDATA_GREEN( mnColor ) * fM + fOff ), 0L, 255L ) );
        SetBlue( (UINT8) VOS_BOUND( _FRound( COLORDATA_BLUE( mnColor ) * fM + fOff ), 0L, 255L ) );
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

SvStream& Color::Read( SvStream& rIStm, BOOL bNewFormat )
{
    if ( bNewFormat )
        rIStm >> mnColor;
    else
        rIStm >> *this;

    return rIStm;
}

// -----------------------------------------------------------------------

SvStream& Color::Write( SvStream& rOStm, BOOL bNewFormat )
{
    if ( bNewFormat )
        rOStm << mnColor;
    else
        rOStm << *this;

    return rOStm;
}

// -----------------------------------------------------------------------

#define COL_NAME_USER       ((USHORT)0x8000)
#define COL_RED_1B          ((USHORT)0x0001)
#define COL_RED_2B          ((USHORT)0x0002)
#define COL_GREEN_1B        ((USHORT)0x0010)
#define COL_GREEN_2B        ((USHORT)0x0020)
#define COL_BLUE_1B         ((USHORT)0x0100)
#define COL_BLUE_2B         ((USHORT)0x0200)

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, Color& rColor )
{
    DBG_ASSERTWARNING( rIStream.GetVersion(), "Color::>> - Solar-Version not set on rIStream" );

    USHORT      nColorName;
    USHORT      nRed;
    USHORT      nGreen;
    USHORT      nBlue;

    rIStream >> nColorName;

    if ( nColorName & COL_NAME_USER )
    {
        if ( rIStream.GetCompressMode() == COMPRESSMODE_FULL )
        {
            unsigned char   cAry[6];
            USHORT          i = 0;

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

    USHORT nColorName   = COL_NAME_USER;
    USHORT nRed         = rColor.GetRed();
    USHORT nGreen       = rColor.GetGreen();
    USHORT nBlue        = rColor.GetBlue();
    nRed    = (nRed<<8) + nRed;
    nGreen  = (nGreen<<8) + nGreen;
    nBlue   = (nBlue<<8) + nBlue;

    if ( rOStream.GetCompressMode() == COMPRESSMODE_FULL )
    {
        unsigned char   cAry[6];
        USHORT          i = 0;

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
