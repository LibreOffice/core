/*************************************************************************
 *
 *  $RCSfile: ieps.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2000-11-10 08:34:35 $
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

#include <stdio.h>

//
#include <vcl/sv.h>
#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.h>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/fltcall.hxx>
#include <tools/urlobj.hxx>

// we will parse only 4kb for graphic information
#define PS_BUF_SIZE 0x1000

/*************************************************************************
|*
|*    ImpSearchEntry()
|*
|*    Beschreibung      Prueft ob im Speicherbereich pSource der nComp Bytes
|*                      gross ist eine Zeichenkette(pDest) mit der länge nSize
|*                      liegt. Geprueft wird NON-CASE-SENSITIVE und der Rueck-
|*                      gabewert ist die Adresse an der die Zeichekette gefunden
|*                      wurde oder NULL
|*
|*    Ersterstellung    SJ 04.03.98 ( und das an meinem Geburtstag )
|*    Letzte Aenderung  SJ 04.03.98
|*
*************************************************************************/

static BYTE* ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize )
{
    while ( nComp-- >= nSize )
    {
        ULONG i;
        for ( i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
                break;
        }
        if ( i == nSize )
            return pSource;
        pSource++;
    }
    return NULL;
}

//--------------------------------------------------------------------------
// SecurityCount is the buffersize of the buffer in which we will parse for a number
static long ImplGetNumber( BYTE **pBuf, int& nSecurityCount )
{
    BOOL    bValid = TRUE;
    BOOL    bNegative = FALSE;
    long    nRetValue = 0;
    while ( ( --nSecurityCount ) && ( **pBuf == ' ' ) || ( **pBuf == 0x9 ) ) (*pBuf)++;
    BYTE nByte = **pBuf;
    while ( nSecurityCount && ( nByte != ' ' ) && ( nByte != 0x9 ) && ( nByte != 0xd ) && ( nByte != 0xa ) )
    {
        switch ( nByte )
        {
            case '.' :
                // we'll only use the integer format
                bValid = FALSE;
                break;
            case '-' :
                bNegative = TRUE;
                break;
            default :
                if ( ( nByte < '0' ) || ( nByte > '9' ) )
                    nSecurityCount = 1;         // error parsing the bounding box values
                else if ( bValid )
                {
                    nRetValue *= 10;
                    nRetValue += nByte - '0';
                }
                break;
        }
        nSecurityCount--;
        nByte = *(++(*pBuf));
    }
    if ( bNegative )
        nRetValue = -nRetValue;
    return nRetValue;
}

//--------------------------------------------------------------------------

static int ImplGetLen( BYTE* pBuf, int nMax )
{
    int nLen = 0;
    while( nLen != nMax )
    {
        BYTE nDat = *pBuf++;
        if ( nDat == 0x0a || nDat == 0x25 )
            break;
        nLen++;
    }
    return nLen;
}

//================== GraphicImport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#else
extern "C" BOOL GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#endif
{
    if ( rStream.GetError() )
        return FALSE;

    Graphic aGraphic;
    BOOL    bRetValue = FALSE;
    BOOL    bHasPreview = FALSE;
    UINT32  nSignature, nPSStreamPos, nPSSize, nSize, nPos;
    UINT32  nOrigPos = nPSStreamPos = rStream.Tell();
    USHORT  nOldFormat = rStream.GetNumberFormatInt();
    rStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStream >> nSignature;
    if ( nSignature == 0xc6d3d0c5 )
    {
        rStream >> nPSStreamPos >> nPSSize >> nPos >> nSize;

        // first we try to get the metafile grafix

        if ( nSize )
        {
            if ( nPos != 0 )
            {
                rStream.Seek( nOrigPos + nPos );
                if ( GraphicConverter::Import( rStream, aGraphic, CVT_WMF ) == ERRCODE_NONE )
                    bHasPreview = bRetValue = TRUE;
            }
        }
        else
        {
            rStream >> nPos >> nSize;

            // else we have to get the tiff grafix

            if ( ( nPos != 0 ) && ( nSize != 0 ) )
            {
                rStream.Seek( nOrigPos + nPos );
                if ( GraphicConverter::Import( rStream, aGraphic, CVT_TIF ) == ERRCODE_NONE )
                {
                    VirtualDevice   aVDev;
                    GDIMetaFile     aMtf;
                    Bitmap          aBmp( aGraphic.GetBitmap() );
                    Size            aSize = aBmp.GetPrefSize();

                    if( !aSize.Width() || !aSize.Height() )
                        aSize = Application::GetDefaultDevice()->PixelToLogic( aBmp.GetSizePixel(), MAP_100TH_MM );
                    else
                        aSize = Application::GetDefaultDevice()->LogicToLogic( aSize, aBmp.GetPrefMapMode(), MAP_100TH_MM );

                    aVDev.EnableOutput( FALSE );
                    aMtf.Record( &aVDev );
                    aVDev.DrawBitmap( Point(), aSize, aGraphic.GetBitmap() );
                    aMtf.Stop();
                    aMtf.WindStart();
                    aMtf.SetPrefMapMode( MAP_100TH_MM );
                    aMtf.SetPrefSize( aSize );
                    aGraphic = aMtf;
                    rStream.Seek( nOrigPos + nPos );
                    bHasPreview = bRetValue = TRUE;
                }
            }
        }
    }
    else
    {
        nPSStreamPos = nOrigPos;            // no preview available _>so we must get the size manually
        nPSSize = rStream.Seek( STREAM_SEEK_TO_END ) - nOrigPos;
    }
    rStream.Seek( nPSStreamPos );
    BYTE* pBuf = new BYTE[ PS_BUF_SIZE ];
    if ( pBuf )
    {
        rStream.Read( pBuf, 22 );   // check PostScript header
        if ( ImplSearchEntry( pBuf, (BYTE*)"%!PS-Adobe", 10, 10 ) &&
            ImplSearchEntry( &pBuf[15], (BYTE*)"EPS", 3, 3 ) )
        {
            ULONG nBufStartPos = rStream.Tell();
            ULONG nBytesRead = rStream.Read( pBuf, PS_BUF_SIZE );
            if ( nBytesRead > 64 )      // assuming a eps file is greater than 64 bytes
            {
                int nSecurityCount = 32;
                if ( !bHasPreview )     // if there is no tiff/wmf preview, we will parse for an preview in the eps prolog
                {
                    BYTE* pDest = ImplSearchEntry( pBuf, (BYTE*)"%%BeginPreview:", nBytesRead - 32, 15 );
                    if ( pDest  )
                    {
                        pDest += 15;
                        long nWidth = ImplGetNumber( &pDest, nSecurityCount );
                        long nHeight = ImplGetNumber( &pDest, nSecurityCount );
                        long nBitDepth = ImplGetNumber( &pDest, nSecurityCount );
                        long nScanLines = ImplGetNumber( &pDest, nSecurityCount );
                        pDest = ImplSearchEntry( pDest, (BYTE*)"%", 16, 1 );        // go to the first Scanline
                        if ( nSecurityCount && pDest && nWidth && nHeight && ( ( nBitDepth == 1 ) || ( nBitDepth == 8 ) ) && nScanLines )
                        {
                            rStream.Seek( nBufStartPos + ( pDest - pBuf ) );

                            Bitmap aBitmap( Size( nWidth, nHeight ), 1 );
                            BitmapWriteAccess* pAcc = aBitmap.AcquireWriteAccess();
                            if ( pAcc )
                            {
                                int  nBitsLeft;
                                BOOL bIsValid = TRUE;
                                BYTE nDat;
                                char nByte;
                                for ( long y = 0; bIsValid && ( y < nHeight ); y++ )
                                {
                                    nBitsLeft = 0;
                                    for ( long x = 0; x < nWidth; x++ )
                                    {
                                        if ( --nBitsLeft < 0 )
                                        {
                                            while ( bIsValid && ( nBitsLeft != 7 ) )
                                            {
                                                rStream >> nByte;
                                                switch ( nByte )
                                                {
                                                    case 0x0a :
                                                        if ( --nScanLines < 0 )
                                                            bIsValid = FALSE;
                                                    case 0x09 :
                                                    case 0x0d :
                                                    case 0x20 :
                                                    case 0x25 :
                                                    break;
                                                    default:
                                                    {
                                                        if ( nByte >= '0' )
                                                        {
                                                            if ( nByte > '9' )
                                                            {
                                                                nByte &=~0x20;  // case none sensitive for hexadezimal values
                                                                nByte -= ( 'A' - 10 );
                                                                if ( nByte > 15 )
                                                                    bIsValid = FALSE;
                                                            }
                                                            else
                                                                nByte -= '0';
                                                            nBitsLeft += 4;
                                                            nDat <<= 4;
                                                            nDat |= ( nByte ^ 0xf ); // in epsi a zero bit represents white color
                                                        }
                                                        else
                                                            bIsValid = FALSE;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                        if ( nBitDepth == 1 )
                                            pAcc->SetPixel( y, x, ( nDat >> nBitsLeft ) & 1 );
                                        else
                                        {
                                            pAcc->SetPixel( y, x, ( nDat ) ? 1 : 0 );   // nBitDepth == 8
                                            nBitsLeft = 0;
                                        }
                                    }
                                }
                                if ( bIsValid )
                                {
                                    VirtualDevice   aVDev;
                                    GDIMetaFile     aMtf;
                                    Size            aSize;
                                    aVDev.EnableOutput( FALSE );
                                    aMtf.Record( &aVDev );
                                    aSize = aBitmap.GetPrefSize();
                                    if( !aSize.Width() || !aSize.Height() )
                                        aSize = Application::GetDefaultDevice()->PixelToLogic( aBitmap.GetSizePixel(), MAP_100TH_MM );
                                    else
                                        aSize = Application::GetDefaultDevice()->LogicToLogic( aSize, aBitmap.GetPrefMapMode(), MAP_100TH_MM );
                                    aVDev.DrawBitmap( Point(), aSize, aBitmap );
                                    aMtf.Stop();
                                    aMtf.WindStart();
                                    aMtf.SetPrefMapMode( MAP_100TH_MM );
                                    aMtf.SetPrefSize( aSize );
                                    aGraphic = aMtf;
                                    bHasPreview = bRetValue = TRUE;
                                }
                                aBitmap.ReleaseAccess( pAcc );
                            }
                        }
                    }
                }

                BYTE* pDest = ImplSearchEntry( pBuf, (BYTE*)"%%BoundingBox:", nBytesRead, 14 );
                if ( pDest )
                {
                    nSecurityCount = 100;
                    long nNumb[4];
                    nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
                    pDest += 14;
                    for ( int i = 0; ( i < 4 ) && nSecurityCount; i++ )
                    {
                        nNumb[ i ] = ImplGetNumber( &pDest, nSecurityCount );
                    }
                    if ( nSecurityCount)
                    {
                        BYTE* pLinkBuf = new BYTE[ nPSSize ];
                        if ( pLinkBuf )
                        {
                            rStream.Seek( nPSStreamPos );
                            if ( rStream.Read( pLinkBuf, nPSSize ) == nPSSize )
                            {
                                GfxLink     aGfxLink( pLinkBuf, nPSSize, GFX_LINK_TYPE_EPS_BUFFER, TRUE ) ;
                                GDIMetaFile aMtf;

                                long nWidth =  nNumb[2] - nNumb[0] + 1;
                                long nHeight = nNumb[3] - nNumb[1] + 1;

                                if( !bHasPreview )      // if there is no preview -> make a red box
                                {
                                    VirtualDevice   aVDev;
                                    GDIMetaFile     aMtf2;
                                    Font            aFont;

                                    aVDev.EnableOutput( FALSE );
                                    aMtf2.Record( &aVDev );
                                    aVDev.SetLineColor( Color( COL_RED ) );
                                    aVDev.SetFillColor();

                                    aFont.SetColor( COL_LIGHTRED );
                                    aFont.SetSize( Size( 0, nHeight / 5 ) );

                                    aVDev.Push( PUSH_FONT );
                                    aVDev.SetFont( aFont );

                                    Rectangle aRect( Point( 0, 0 ), Size( nWidth, nHeight ) );
                                    aVDev.DrawRect( aRect );

                                    String aString;
                                    int nLen;
                                    pDest = ImplSearchEntry( pBuf, (BYTE*)"%%Title:", nBytesRead - 32, 8 );
                                    if ( pDest )
                                    {
                                        pDest += 8;
                                        if ( *pDest == ' ' )
                                            pDest++;
                                        nLen = ImplGetLen( pBuf, 32 );
                                        pDest[ nLen ] = 0;
                                        if ( strcmp( (const char*)pDest, "none" ) != 0 )
                                        {
                                            aString.AppendAscii( (char*)pDest );
                                        }
                                    }
                                    pDest = ImplSearchEntry( pBuf, (BYTE*)"%%Creator:", nBytesRead - 32, 10 );
                                    if ( pDest )
                                    {
                                        pDest += 10;
                                        if ( *pDest == ' ' )
                                            pDest++;
                                        nLen = ImplGetLen( pDest, 32 );
                                        pDest[ nLen ] = 0;
                                        aString.AppendAscii( "\n" );
                                        aString.AppendAscii( (char*)pDest );
                                    }
                                    pDest = ImplSearchEntry( pBuf, (BYTE*)"%%CreationDate:", nBytesRead - 32, 15 );
                                    if ( pDest )
                                    {
                                        pDest += 15;
                                        if ( *pDest == ' ' )
                                            pDest++;
                                        nLen = ImplGetLen( pDest, 32 );
                                        pDest[ nLen ] = 0;
                                        if ( strcmp( (const char*)pDest, "none" ) != 0 )
                                        {
                                            aString.AppendAscii( "\n" );
                                            aString.AppendAscii( (char*)pDest );
                                        }
                                    }
                                    pDest = ImplSearchEntry( pBuf, (BYTE*)"%%LanguageLevel:", nBytesRead - 4, 16 );
                                    if ( pDest )
                                    {
                                        pDest += 16;
                                        int nCount = 4;
                                        long nNumber = ImplGetNumber( &pDest, nCount );
                                        if ( nCount && ( (UINT32)nNumber < 10 ) )
                                        {
                                            aString.AppendAscii( "\nLevel:" );
                                            aString.Append( UniString::CreateFromInt32( nNumber ) );
                                        }
                                    }
                                    aVDev.DrawText( Rectangle( aRect ), aString, TEXT_DRAW_CLIP | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
                                    aVDev.Pop();
                                    aMtf2.Stop();
                                    aMtf2.WindStart();
                                    aMtf2.SetPrefMapMode( MAP_POINT );
                                    aMtf2.SetPrefSize( Size( nWidth, nHeight ) );
                                    aGraphic = aMtf2;
                                }

                                aMtf.AddAction( (MetaAction*)( new MetaEPSAction( Point(), Size( nWidth, nHeight ),
                                                                                  aGfxLink, aGraphic.GetGDIMetaFile() ) ) );
                                aMtf.WindStart();
                                aMtf.SetPrefMapMode( MAP_POINT );
                                aMtf.SetPrefSize( Size( nWidth, nHeight ) );
                                rGraphic = aMtf;
                                bRetValue = TRUE;
                            }
                            else
                                delete pLinkBuf;
                        }
                    }
                }
            }
        }
        delete pBuf;
    }
    rStream.SetNumberFormatInt(nOldFormat);
    rStream.Seek( nOrigPos );
    return ( bRetValue );
}

//================== ein bischen Muell fuer Windows ==========================

#pragma hdrstop

#ifdef WIN

static HINSTANCE hDLLInst = 0;      // HANDLE der DLL

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif

