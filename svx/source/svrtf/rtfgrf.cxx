/*************************************************************************
 *
 *  $RCSfile: rtfgrf.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-09-25 18:57:21 $
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

#pragma hdrstop

#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _RTFKEYWD_HXX
#include <svtools/rtfkeywd.hxx>
#endif
#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif

#include "impgrf.hxx"
#include "svxrtf.hxx"


#ifdef PRODUCT
#undef DEBUG_JP
#endif

#ifdef DEBUG_JP

#include <tools/fsys.hxx>

class GrfWindow : public WorkWindow
{
    Graphic aGrf;
public:
    GrfWindow( const Graphic& rGrf );
    virtual void    Paint( const Rectangle& rRect );
};

GrfWindow::GrfWindow( const Graphic& rGrf )
    : WorkWindow( GetpApp()->GetAppWindow() ),
    aGrf( rGrf )
{
    SetPosSizePixel( Point( 100, 0 ), Size( 300, 300 ));
    Show();
    Invalidate();
    Update();
}

void GrfWindow::Paint( const Rectangle& )
{
    aGrf.Draw( this, Point(0,0), GetSizePixel() );
}
#endif

static BYTE __FAR_DATA aPal1[ 2 * 4 ] = {
        0x00, 0x00, 0x00, 0x00,             // Schwarz
        0xFF, 0xFF, 0xFF, 0x00              // Weiss
};

static BYTE __FAR_DATA aPal4[ 16 * 4 ] = {
        0x00, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x00, 0x00,
        0x00, 0x80, 0x00, 0x00,
        0x80, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x00,
        0x80, 0x00, 0x80, 0x00,
        0x00, 0x80, 0x80, 0x00,
        0x80, 0x80, 0x80, 0x00,
        0xC0, 0xC0, 0xC0, 0x00,
        0xFF, 0x00, 0x00, 0x00,
        0x00, 0xFF, 0x00, 0x00,
        0xFF, 0xFF, 0x00, 0x00,
        0x00, 0x00, 0xFF, 0x00,
        0xFF, 0x00, 0xFF, 0x00,
        0x00, 0xFF, 0xFF, 0x00,
        0xFF, 0xFF, 0xFF, 0x00
};

static BYTE __FAR_DATA aPal8[ 256 * 4 ] =
{
0x00, 0x00, 0x00, 0x00,   0x80, 0x00, 0x00, 0x00,   0x00, 0x92, 0x00, 0x00,
0x80, 0x92, 0x00, 0x00,   0x00, 0x00, 0xAA, 0x00,   0x80, 0x00, 0xAA, 0x00,
0x00, 0x92, 0xAA, 0x00,   0xC1, 0xC1, 0xC1, 0x00,   0xC9, 0xC9, 0xC9, 0x00,
0xAA, 0xDB, 0xFF, 0x00,   0x00, 0x49, 0xAA, 0x00,   0x00, 0x49, 0xFF, 0x00,
0x00, 0x6D, 0x00, 0x00,   0x00, 0x6D, 0x55, 0x00,   0x00, 0x6D, 0xAA, 0x00,
0x00, 0x6D, 0xFF, 0x00,   0x00, 0x24, 0x00, 0x00,   0x00, 0x92, 0x55, 0x00,
0x00, 0x24, 0xAA, 0x00,   0x00, 0x92, 0xFF, 0x00,   0x00, 0xB6, 0x00, 0x00,
0x00, 0xB6, 0x55, 0x00,   0x00, 0xB6, 0xAA, 0x00,   0x00, 0xB6, 0xFF, 0x00,
0x00, 0xDB, 0x00, 0x00,   0x00, 0xDB, 0x55, 0x00,   0x00, 0xDB, 0xAA, 0x00,
0x00, 0xDB, 0xFF, 0x00,   0xFF, 0xDB, 0xAA, 0x00,   0x00, 0xFF, 0x55, 0x00,
0x00, 0xFF, 0xAA, 0x00,   0xFF, 0xFF, 0xAA, 0x00,   0x2B, 0x00, 0x00, 0x00,
0x2B, 0x00, 0x55, 0x00,   0x2B, 0x00, 0xAA, 0x00,   0x2B, 0x00, 0xFF, 0x00,
0x2B, 0x24, 0x00, 0x00,   0x2B, 0x24, 0x55, 0x00,   0x2B, 0x24, 0xAA, 0x00,
0x2B, 0x24, 0xFF, 0x00,   0x2B, 0x49, 0x00, 0x00,   0x2B, 0x49, 0x55, 0x00,
0x2B, 0x49, 0xAA, 0x00,   0x2B, 0x49, 0xFF, 0x00,   0x2B, 0x6D, 0x00, 0x00,
0x2B, 0x6D, 0x55, 0x00,   0x2B, 0x6D, 0xAA, 0x00,   0x2B, 0x6D, 0xFF, 0x00,
0x2B, 0x92, 0x00, 0x00,   0x2B, 0x92, 0x55, 0x00,   0x2B, 0x92, 0xAA, 0x00,
0x2B, 0x92, 0xFF, 0x00,   0x2B, 0xB6, 0x00, 0x00,   0x2B, 0xB6, 0x55, 0x00,
0x2B, 0xB6, 0xAA, 0x00,   0x2B, 0xB6, 0xFF, 0x00,   0x2B, 0xDB, 0x00, 0x00,
0x2B, 0xDB, 0x55, 0x00,   0x2B, 0xDB, 0xAA, 0x00,   0x2B, 0xDB, 0xFF, 0x00,
0x2B, 0xFF, 0x00, 0x00,   0x2B, 0xFF, 0x55, 0x00,   0x2B, 0xFF, 0xAA, 0x00,
0x2B, 0xFF, 0xFF, 0x00,   0x55, 0x00, 0x00, 0x00,   0x55, 0x00, 0x55, 0x00,
0x55, 0x00, 0xAA, 0x00,   0x55, 0x00, 0xFF, 0x00,   0x55, 0x24, 0x00, 0x00,
0x55, 0x24, 0x55, 0x00,   0x55, 0x24, 0xAA, 0x00,   0x55, 0x24, 0xFF, 0x00,
0x55, 0x49, 0x00, 0x00,   0x55, 0x49, 0x55, 0x00,   0x55, 0x49, 0xAA, 0x00,
0x55, 0x49, 0xFF, 0x00,   0x55, 0x6D, 0x00, 0x00,   0x55, 0x6D, 0x55, 0x00,
0x55, 0x6D, 0xAA, 0x00,   0x55, 0x6D, 0xFF, 0x00,   0x55, 0x92, 0x00, 0x00,
0x55, 0x92, 0x55, 0x00,   0x55, 0x92, 0xAA, 0x00,   0x55, 0x92, 0xFF, 0x00,
0x55, 0xB6, 0x00, 0x00,   0x55, 0xB6, 0x55, 0x00,   0x55, 0xB6, 0xAA, 0x00,
0x55, 0xB6, 0xFF, 0x00,   0x55, 0xDB, 0x00, 0x00,   0x55, 0xDB, 0x55, 0x00,
0x55, 0xDB, 0xAA, 0x00,   0x55, 0xDB, 0xFF, 0x00,   0x55, 0xFF, 0x00, 0x00,
0x55, 0xFF, 0x55, 0x00,   0x55, 0xFF, 0xAA, 0x00,   0x55, 0xFF, 0xFF, 0x00,
0x00, 0x00, 0x55, 0x00,   0x80, 0x00, 0x55, 0x00,   0x00, 0x24, 0x55, 0x00,
0x80, 0x00, 0xFF, 0x00,   0x80, 0x24, 0x00, 0x00,   0x80, 0x24, 0x55, 0x00,
0x80, 0x24, 0xAA, 0x00,   0x80, 0x24, 0xFF, 0x00,   0x80, 0x49, 0x00, 0x00,
0x80, 0x49, 0x55, 0x00,   0x80, 0x49, 0xAA, 0x00,   0x80, 0x49, 0xFF, 0x00,
0x80, 0x6D, 0x00, 0x00,   0x80, 0x6D, 0x55, 0x00,   0x80, 0x6D, 0xAA, 0x00,
0x80, 0x6D, 0xFF, 0x00,   0x08, 0x08, 0x08, 0x00,   0x0F, 0x0F, 0x0F, 0x00,
0x17, 0x17, 0x17, 0x00,   0x1F, 0x1F, 0x1F, 0x00,   0x27, 0x27, 0x27, 0x00,
0x2E, 0x2E, 0x2E, 0x00,   0x36, 0x36, 0x36, 0x00,   0x3E, 0x3E, 0x3E, 0x00,
0x46, 0x46, 0x46, 0x00,   0x4D, 0x4D, 0x4D, 0x00,   0x55, 0x55, 0x55, 0x00,
0x5D, 0x5D, 0x5D, 0x00,   0x64, 0x64, 0x64, 0x00,   0x6C, 0x6C, 0x6C, 0x00,
0x74, 0x74, 0x74, 0x00,   0x7C, 0x7C, 0x7C, 0x00,   0xFF, 0xDB, 0x00, 0x00,
0x8B, 0x8B, 0x8B, 0x00,   0x93, 0x93, 0x93, 0x00,   0x9B, 0x9B, 0x9B, 0x00,
0xFF, 0xB6, 0xFF, 0x00,   0xAA, 0xAA, 0xAA, 0x00,   0xB2, 0xB2, 0xB2, 0x00,
0xB9, 0xB9, 0xB9, 0x00,   0x00, 0x24, 0xFF, 0x00,   0x00, 0x49, 0x00, 0x00,
0xD1, 0xD1, 0xD1, 0x00,   0xD8, 0xD8, 0xD8, 0x00,   0xE0, 0xE0, 0xE0, 0x00,
0xE8, 0xE8, 0xE8, 0x00,   0xF0, 0xF0, 0xF0, 0x00,   0xFF, 0xB6, 0xAA, 0x00,
0xFF, 0xDB, 0xFF, 0x00,   0x80, 0x92, 0x55, 0x00,   0x80, 0x92, 0xAA, 0x00,
0x80, 0x92, 0xFF, 0x00,   0x80, 0xB6, 0x00, 0x00,   0x80, 0xB6, 0x55, 0x00,
0x80, 0xB6, 0xAA, 0x00,   0x80, 0xB6, 0xFF, 0x00,   0x80, 0xDB, 0x00, 0x00,
0x80, 0xDB, 0x55, 0x00,   0x80, 0xDB, 0xAA, 0x00,   0x80, 0xDB, 0xFF, 0x00,
0x80, 0xFF, 0x00, 0x00,   0x80, 0xFF, 0x55, 0x00,   0x80, 0xFF, 0xAA, 0x00,
0x80, 0xFF, 0xFF, 0x00,   0xAA, 0x00, 0x00, 0x00,   0xAA, 0x00, 0x55, 0x00,
0xAA, 0x00, 0xAA, 0x00,   0xAA, 0x00, 0xFF, 0x00,   0xAA, 0x24, 0x00, 0x00,
0xAA, 0x24, 0x55, 0x00,   0xAA, 0x24, 0xAA, 0x00,   0xAA, 0x24, 0xFF, 0x00,
0xAA, 0x49, 0x00, 0x00,   0xAA, 0x49, 0x55, 0x00,   0xAA, 0x49, 0xAA, 0x00,
0xAA, 0x49, 0xFF, 0x00,   0xAA, 0x6D, 0x00, 0x00,   0xAA, 0x6D, 0x55, 0x00,
0xAA, 0x6D, 0xAA, 0x00,   0xAA, 0x6D, 0xFF, 0x00,   0xAA, 0x92, 0x00, 0x00,
0xAA, 0x92, 0x55, 0x00,   0xAA, 0x92, 0xAA, 0x00,   0xAA, 0x92, 0xFF, 0x00,
0xAA, 0xB6, 0x00, 0x00,   0xAA, 0xB6, 0x55, 0x00,   0xAA, 0xB6, 0xAA, 0x00,
0xAA, 0xB6, 0xFF, 0x00,   0xAA, 0xDB, 0x00, 0x00,   0xAA, 0xDB, 0x55, 0x00,
0xAA, 0xDB, 0xAA, 0x00,   0x00, 0x49, 0x55, 0x00,   0xAA, 0xFF, 0x00, 0x00,
0xAA, 0xFF, 0x55, 0x00,   0xAA, 0xFF, 0xAA, 0x00,   0xAA, 0xFF, 0xFF, 0x00,
0xD5, 0x00, 0x00, 0x00,   0xD5, 0x00, 0x55, 0x00,   0xD5, 0x00, 0xAA, 0x00,
0xD5, 0x00, 0xFF, 0x00,   0xD5, 0x24, 0x00, 0x00,   0xD5, 0x24, 0x55, 0x00,
0xD5, 0x24, 0xAA, 0x00,   0xD5, 0x24, 0xFF, 0x00,   0xD5, 0x49, 0x00, 0x00,
0xD5, 0x49, 0x55, 0x00,   0xD5, 0x49, 0xAA, 0x00,   0xD5, 0x49, 0xFF, 0x00,
0xD5, 0x6D, 0x00, 0x00,   0xD5, 0x6D, 0x55, 0x00,   0xD5, 0x6D, 0xAA, 0x00,
0xD5, 0x6D, 0xFF, 0x00,   0xD5, 0x92, 0x00, 0x00,   0xD5, 0x92, 0x55, 0x00,
0xD5, 0x92, 0xAA, 0x00,   0xD5, 0x92, 0xFF, 0x00,   0xD5, 0xB6, 0x00, 0x00,
0xD5, 0xB6, 0x55, 0x00,   0xD5, 0xB6, 0xAA, 0x00,   0xD5, 0xB6, 0xFF, 0x00,
0xD5, 0xDB, 0x00, 0x00,   0xD5, 0xDB, 0x55, 0x00,   0xD5, 0xDB, 0xAA, 0x00,
0xD5, 0xDB, 0xFF, 0x00,   0xD5, 0xFF, 0x00, 0x00,   0xD5, 0xFF, 0x55, 0x00,
0xD5, 0xFF, 0xAA, 0x00,   0xD5, 0xFF, 0xFF, 0x00,   0xFF, 0xDB, 0x55, 0x00,
0xFF, 0x00, 0x55, 0x00,   0xFF, 0x00, 0xAA, 0x00,   0xFF, 0xFF, 0x55, 0x00,
0xFF, 0x24, 0x00, 0x00,   0xFF, 0x24, 0x55, 0x00,   0xFF, 0x24, 0xAA, 0x00,
0xFF, 0x24, 0xFF, 0x00,   0xFF, 0x49, 0x00, 0x00,   0xFF, 0x49, 0x55, 0x00,
0xFF, 0x49, 0xAA, 0x00,   0xFF, 0x49, 0xFF, 0x00,   0xFF, 0x6D, 0x00, 0x00,
0xFF, 0x6D, 0x55, 0x00,   0xFF, 0x6D, 0xAA, 0x00,   0xFF, 0x6D, 0xFF, 0x00,
0xFF, 0x92, 0x00, 0x00,   0xFF, 0x92, 0x55, 0x00,   0xFF, 0x92, 0xAA, 0x00,
0xFF, 0x92, 0xFF, 0x00,   0xFF, 0xB6, 0x00, 0x00,   0xFF, 0xB6, 0x55, 0x00,
0xF7, 0xF7, 0xF7, 0x00,   0xA2, 0xA2, 0xA2, 0x00,   0x83, 0x83, 0x83, 0x00,
0xFF, 0x00, 0x00, 0x00,   0x00, 0xFF, 0x00, 0x00,   0xFF, 0xFF, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00,   0xFF, 0x00, 0xFF, 0x00,   0x00, 0xFF, 0xFF, 0x00,
0xFF, 0xFF, 0xFF, 0x00
};


/*  */


inline long SwapLong( long n )
{
#ifndef __LITTLEENDIAN
    return SWAPLONG( n );
#else
    return n;
#endif
}

inline short SwapShort( short n )
{
#ifndef __LITTLEENDIAN
    return SWAPSHORT( n );
#else
    return n;
#endif
}


static void WriteBMPHeader( SvStream& rStream,
                            const SvxRTFPictureType& rPicType )
{
    ULONG n4Width = rPicType.nWidth;
    ULONG n4Height = rPicType.nHeight;
    USHORT n4ColBits = rPicType.nBitsPerPixel;

    USHORT nColors = (1 << n4ColBits);  // Anzahl der Farben ( 1, 16, 256 )
    USHORT nWdtOut = rPicType.nWidthBytes;
    if( !nWdtOut )
        nWdtOut = (USHORT)((( n4Width * n4ColBits + 31 ) / 32 ) * 4 );

    long nOffset = 14 + 40;     // BMP_FILE_HD_SIZ + sizeof(*pBmpInfo);
    if( 256 >= nColors )
        nOffset += nColors * 4;
    long nSize = nOffset + nWdtOut * n4Height;
    rStream << "BM"                     // = "BM"
            << SwapLong(nSize)          // Filesize in Bytes
            << SwapShort(0)             // Reserviert
            << SwapShort(0)             // Reserviert
            << SwapLong(nOffset);       // Offset?

    rStream << SwapLong(40)             // sizeof( BmpInfo )
            << SwapLong(n4Width)
            << SwapLong(n4Height)
            << (USHORT)1
            << n4ColBits
            << SwapLong(0)
            << SwapLong(0)
            << SwapLong( rPicType.nGoalWidth
                        ? rPicType.nGoalWidth * 1000L / 254L
                        : 0 )         // DPI in Pixel per Meter
            << SwapLong( rPicType.nGoalHeight
                        ? rPicType.nGoalHeight * 1000L / 254L      // dito
                        : 0 )
            << SwapLong(0)
            << SwapLong(0);


    switch( rPicType.nBitsPerPixel )
    {
    case 1:     rStream.Write( aPal1, sizeof( aPal1 )); break;
    case 4:     rStream.Write( aPal4, sizeof( aPal4 )); break;
    case 8:     rStream.Write( aPal8, sizeof( aPal8 )); break;
    }
}

/*  */

        // wandel die ASCII-HexCodes in binaere Zeichen um. Werden
        // ungueltige Daten gefunden (Zeichen ausser 0-9|a-f|A-F, so
        // wird USHRT_MAX returnt, ansonsten die Anzahl der umgewandelten Ze.
xub_StrLen SvxRTFParser::HexToBin( String& rToken )
{
    // dann mache aus den Hex-Werten mal "Binare Daten"
    // (missbrauche den String als temp Buffer)
    if( rToken.Len() & 1 )      // ungerade Anzahl, mit 0 auffuellen
        rToken += '0';

    xub_StrLen n, nLen;
    sal_Unicode nVal;
    BOOL bValidData = TRUE;
    const sal_Unicode* pStr = rToken.GetBufferAccess();
    sal_Char* pData = (sal_Char*)pStr;
    for( n = 0, nLen = rToken.Len(); n < nLen; ++n, ++pStr )
    {
        if( ((nVal = *pStr) >= '0') && ( nVal <= '9') )
            nVal -= '0';
        else if( (nVal >= 'A') && (nVal <= 'F') )
            nVal -= 'A' - 10;
        else if( (nVal >= 'a') && (nVal <= 'f') )
            nVal -= 'a' - 10;
        else
        {
            DBG_ASSERT( !this, "ungueltiger Hex-Wert" );
            bValidData = FALSE;
            break;
        }

        if( n & 1 )
            *(pData++) |= nVal & 0x0f;
        else
            *(pData) = ( nVal << 4 ) & 0xf0;
    }
    // the len div 2, because 2 character are one byte
    return bValidData ? nLen / 2  : STRING_NOTFOUND;
}

BOOL SvxRTFParser::ReadBmpData( Graphic& rGrf, SvxRTFPictureType& rPicType )
{
    // die alten Daten loeschen
    rGrf.Clear();
    ULONG nBmpSize = 0;

    rtl_TextEncoding eOldEnc = GetSrcEncoding();
    SetSrcEncoding( RTL_TEXTENCODING_MS_1252 );

    String sFilter;
    SvCacheStream* pTmpFile = 0;

    int nToken, bValidBmp = TRUE, bFirstTextToken = TRUE;
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!

    while( nOpenBrakets && IsParserWorking() && bValidBmp )
    {
        nToken = GetNextToken();
        USHORT nVal = USHORT( nTokenValue );
        switch( nToken )
        {
        case '}':       --nOpenBrakets; break;
        case '{':
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;
                }
                ++nOpenBrakets;
            }
            break;

        case RTF_WBITMAP:
            rPicType.eStyle = SvxRTFPictureType::RTF_BITMAP;
            rPicType.nType = nVal;
            pTmpFile = new SvCacheStream;
            break;

        case RTF_WMETAFILE:
            rPicType.eStyle = SvxRTFPictureType::WIN_METAFILE;
            rPicType.nType = nVal;
            // WinMetaFile ueber Grafik-Filter einlesen
            pTmpFile = new SvCacheStream;
            sFilter.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "WMF" ));
            break;

        case RTF_MACPICT:
            {
                rPicType.eStyle = SvxRTFPictureType::MAC_QUICKDRAW;
                // Mac-Pict bekommt einen leeren Header voran
                pTmpFile = new SvCacheStream;
                ByteString aStr;
                aStr.Fill( 512, '\0' );
                pTmpFile->Write( aStr.GetBuffer(), aStr.Len() );
                sFilter.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "PCT" ));
            }
            break;

        case RTF_OSMETAFILE:
            rPicType.eStyle = SvxRTFPictureType::OS2_METAFILE;
            rPicType.nType = nVal;
            pTmpFile = new SvCacheStream;
            break;

        case RTF_DIBITMAP:
            rPicType.eStyle = SvxRTFPictureType::RTF_DI_BMP;
            rPicType.nType = nVal;
            pTmpFile = new SvCacheStream;
            break;

        case RTF_PICW:              rPicType.nWidth = nVal; break;
        case RTF_PICH:              rPicType.nHeight = nVal; break;
        case RTF_WBMBITSPIXEL:      rPicType.nBitsPerPixel = nVal; break;
        case RTF_WBMPLANES:         rPicType.nPlanes = nVal; break;
        case RTF_WBMWIDTHBYTES:     rPicType.nWidthBytes = nVal; break;
        case RTF_PICWGOAL:          rPicType.nGoalWidth = nVal; break;
        case RTF_PICHGOAL:          rPicType.nGoalHeight = nVal; break;
        case RTF_BIN:               rPicType.nMode = SvxRTFPictureType::BINARY_MODE;
                                    rPicType.uPicLen = nVal;
                                    break;
        case RTF_PICSCALEX:         rPicType.nScalX = nVal; break;
        case RTF_PICSCALEY:         rPicType.nScalY = nVal; break;
        case RTF_PICSCALED:         break;

        case RTF_PICCROPT:          rPicType.nCropT = (short)nTokenValue; break;
        case RTF_PICCROPB:          rPicType.nCropB = (short)nTokenValue; break;
        case RTF_PICCROPL:          rPicType.nCropL = (short)nTokenValue; break;
        case RTF_PICCROPR:          rPicType.nCropR = (short)nTokenValue; break;

        case RTF_TEXTTOKEN:
            // JP 26.06.98: Bug #51719# - nur TextToken auf 1. Ebene
            //              auswerten. Alle anderen sind irgendwelche
            //              nicht auszuwertende Daten
            if( 1 != nOpenBrakets )
                break;

            if( bFirstTextToken )
            {
                switch( rPicType.eStyle )
                {
                case SvxRTFPictureType::RTF_BITMAP:
                    // erstmal die Header und Info-Struktur schreiben
                    if( pTmpFile )
                        ::WriteBMPHeader( *pTmpFile, rPicType );
                    break;
                }
                bFirstTextToken = FALSE;
            }

            if( pTmpFile && SvxRTFPictureType::HEX_MODE == rPicType.nMode )
            {
                xub_StrLen nTokenLen = HexToBin( aToken );
                if( STRING_NOTFOUND == nTokenLen )
                    bValidBmp = FALSE;
                else
                {
                    pTmpFile->Write( (sal_Char*)aToken.GetBuffer(),
                                        nTokenLen );
                    bValidBmp = 0 == pTmpFile->GetError();
                }
            }
            break;
        }
    }

    if( pTmpFile )
    {
        if( bValidBmp )
        {
            GraphicFilter* pGF = ::GetGrfFilter();
            USHORT nImportFilter = GRFILTER_FORMAT_DONTKNOW;

            if( sFilter.Len() )
            {
                for( USHORT n = pGF->GetImportFormatCount(); n; )
                {
                    String sTmp( pGF->GetImportFormatShortName( --n ) );
                    if( sTmp == sFilter )
                    {
                        nImportFilter = n;
                        break;
                    }
                }
            }

            String sTmpStr;
            pTmpFile->Seek( STREAM_SEEK_TO_BEGIN );
            bValidBmp = 0 == pGF->ImportGraphic( rGrf, sTmpStr, *pTmpFile,
                                                nImportFilter );
/*          if( bValidBmp )
            {
                MapMode aMapMode;
                Size aSz;

                if( rPicType.nGoalWidth && rPicType.nGoalHeight )
                {
                    aMapMode.SetMapUnit( MAP_TWIP );
                    aSz.Width() = rPicType.nGoalWidth;
                    aSz.Height() = rPicType.nGoalHeight;
                }
                else
                {
                    aMapMode.SetMapUnit( MAP_100TH_MM );
                    aSz.Width() = rPicType.nWidth;
                    aSz.Height() = rPicType.nHeight;
                }
                switch( rPicType.eStyle )
                {
                case SvxRTFPictureType::WIN_METAFILE:
                    {
                        GDIMetaFile aMTF( rGrf.GetGDIMetaFile() );
                        aMTF.SetPrefMapMode( aMapMode );
                        aMTF.SetPrefSize( aSz );
                        Fraction aScale( 1, 1 );
                        aMTF.Scale( aScale, aScale );
                        rGrf = Graphic( aMTF );
                    }
                    break;
                case SvxRTFPictureType::RTF_BITMAP:
                    {
                        Bitmap aBmp( rGrf.GetBitmap() );
                        aBmp.SetPrefMapMode( aMapMode );
                        aBmp.SetPrefSize( aSz );
                        rGrf = Graphic( aBmp );
                    }
                    break;
                }
            }
*/
        }
        delete pTmpFile;
    }

    if( !bValidBmp )
    {
        rGrf.Clear();
        if( '}' != nToken )
            SkipGroup();
    }
#ifdef DEBUG_JP
    else
        new GrfWindow( rGrf );
#endif

    SetSrcEncoding( eOldEnc );

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
    return bValidBmp;
}

    // alle Werte auf default; wird nach einlesen der Bitmap aufgerufen !
void SvxRTFPictureType::ResetValues()
{   // setze alle Werte RTF-Defaults
    eStyle = RTF_BITMAP;
    nMode = HEX_MODE;
    nType = nGoalWidth = nGoalHeight = 0;
    nWidth = nHeight = uPicLen = nWidthBytes = 0;
    nBitsPerPixel = nPlanes = 1;
    nScalX = nScalY = 100;      // Skalierung in Prozent
    nCropT = nCropB = nCropL = nCropR = 0;
}


