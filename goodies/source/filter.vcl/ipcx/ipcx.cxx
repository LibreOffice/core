/*************************************************************************
 *
 *  $RCSfile: ipcx.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-08 15:47:25 $
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

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/fltcall.hxx>

//============================ PCXReader ==================================

class PCXReader {

private:

    PFilterCallback pCallback;
    void * pCallerData;

    SvStream*           pPCX;               // Die einzulesende PCX-Datei

    Bitmap              aBmp;
    BitmapWriteAccess*  pAcc;
    BYTE                nVersion;           // PCX-Version
    BYTE                nEncoding;          // Art der Komprimierung
    ULONG               nBitsPerPlanePix;   // Bits Pro Ebene pro Pixel
    ULONG               nPlanes;            // Anzahl Ebenen
    ULONG               nBytesPerPlaneLin;  // Bytes in einer Ebenen pro Zeile
    USHORT              nPaletteInfo;

    ULONG               nWidth, nHeight;    // Bildausmass in Pixeln
    USHORT              nResX, nResY;       // Aufloesung in Pixel pro Inch oder 0,0
    USHORT              nDestBitsPerPixel;  // Bits pro Pixel der Zielbitmap 1,4,8 oder 24
    BYTE*               pPalette;           //
    BOOL                nStatus;            // status nun nicht mehr am stream abfragen ( SJ )


    BOOL                Callback( USHORT nPercent );
    void                ImplReadBody();
    void                ImplReadPalette( ULONG nCol );
    void                ImplReadHeader();

public:
                        PCXReader();
                        ~PCXReader();
    BOOL                ReadPCX( SvStream & rPCX, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata );
                        // Liesst aus dem Stream eine PCX-Datei und fuellt das GDIMetaFile
};

//=================== Methoden von PCXReader ==============================

PCXReader::PCXReader() :
    pAcc        ( NULL )
{
    pPalette = new BYTE[ 768 ];
}

PCXReader::~PCXReader()
{
    delete[] pPalette;
}

BOOL PCXReader::Callback( USHORT nPercent )
{
    if (pCallback!=NULL) {
        if (((*pCallback)(pCallerData,nPercent))==TRUE) {
            nStatus = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL PCXReader::ReadPCX( SvStream & rPCX, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata)
{
    if ( rPCX.GetError() )
        return FALSE;

    ULONG*  pDummy = new ULONG; delete pDummy; // damit unter OS/2
                                               // das richtige (Tools-)new
                                               // verwendet wird, da es sonst
                                               // in dieser DLL nur Vector-news
                                               // gibt;

    pCallback = pcallback;
    pCallerData = pcallerdata;

    pPCX = &rPCX;
    pPCX->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);

    // Kopf einlesen:

    nStatus = TRUE;

    ImplReadHeader();

    // BMP-Header und ggf. (eventuell zunaechst ungueltige) Farbpalette schreiben:
    if ( nStatus )
    {
        aBmp = Bitmap( Size( nWidth, nHeight ), nDestBitsPerPixel );
        if ( ( pAcc = aBmp.AcquireWriteAccess() ) == FALSE )
            return FALSE;

        if ( nDestBitsPerPixel <= 8 )
        {
            USHORT nColors = 1 << nDestBitsPerPixel;
            BYTE* pPal = pPalette;
            pAcc->SetPaletteEntryCount( nColors );
            for ( USHORT i = 0; i < nColors; i++, pPal += 3 )
            {
                pAcc->SetPaletteColor( i, BitmapColor ( pPal[ 0 ], pPal[ 1 ], pPal[ 2 ] ) );
            }
        }
        // Bitmap-Daten einlesen
        ImplReadBody();

        // Wenn erweiterte Farbpalette am Ende von PCX, dann diese einlesen, und nochmals
        // in Palette schreiben:
        if ( nDestBitsPerPixel == 8 && nStatus )
        {
            BYTE* pPal = pPalette;
            pPCX->SeekRel(1);
            ImplReadPalette(256);
            pAcc->SetPaletteEntryCount( 256 );
            for ( USHORT i = 0; i < 256; i++, pPal += 3 )
            {
                pAcc->SetPaletteColor( i, BitmapColor ( pPal[ 0 ], pPal[ 1 ], pPal[ 2 ] ) );
            }
        }
    /*
        // Aufloesung einstellen:
        if (nResX!=0 && nResY!=0) {
            MapMode aMapMode(MAP_INCH,Point(0,0),Fraction(1,nResX),Fraction(1,nResY));
            rBitmap.SetPrefMapMode(aMapMode);
            rBitmap.SetPrefSize(Size(nWidth,nHeight));
        }
    */  if ( nStatus && pAcc )
        {
            aBmp.ReleaseAccess( pAcc ), pAcc = NULL;
            rGraphic = aBmp;
            return TRUE;
        }
    }
    return FALSE;
}

void PCXReader::ImplReadHeader()
{
    BYTE nbyte;
    USHORT nushort;
    USHORT nMinX,nMinY,nMaxX,nMaxY;

    *pPCX >> nbyte >> nVersion >> nEncoding;
    if ( nbyte!=0x0a || (nVersion != 0 && nVersion != 2 && nVersion != 3 && nVersion != 5) || nEncoding > 1 )
    {
        nStatus = FALSE;
        return;
    }

    *pPCX >> nbyte; nBitsPerPlanePix = (ULONG)nbyte;
    *pPCX >> nMinX >> nMinY >> nMaxX >> nMaxY;
    nWidth = nMaxX-nMinX+1;
    nHeight = nMaxY-nMinY+1;

    *pPCX >> nResX;
    *pPCX >> nResY;
    if ( nResX >= nWidth || nResY >= nHeight || ( nResX != nResY ) )
        nResX = nResY = 0;

    ImplReadPalette( 16 );

    pPCX->SeekRel( 1 );
    *pPCX >> nbyte;   nPlanes = (ULONG)nbyte;
    *pPCX >> nushort; nBytesPerPlaneLin = (ULONG)nushort;
    *pPCX >> nPaletteInfo;

    pPCX->SeekRel( 58 );

    nDestBitsPerPixel = (USHORT)( nBitsPerPlanePix * nPlanes );
    if (nDestBitsPerPixel == 2 || nDestBitsPerPixel == 3) nDestBitsPerPixel = 4;

    if ( ( nDestBitsPerPixel != 1 && nDestBitsPerPixel != 4 && nDestBitsPerPixel != 8 && nDestBitsPerPixel != 24 )
        || nPlanes > 4 || nBytesPerPlaneLin < ( ( nWidth * nBitsPerPlanePix+7 ) >> 3 ) )
    {
        nStatus = FALSE;
        return;
    }

    // Wenn das Bild nur 2 Farben hat, ist die Palette zumeist ungueltig, und es handelt sich
    // immer (?) um ein schwarz-weiss-Bild:
    if ( nPlanes == 1 && nBitsPerPlanePix == 1 )
    {
        pPalette[ 0 ] = pPalette[ 1 ] = pPalette[ 2 ] = 0x00;
        pPalette[ 3 ] = pPalette[ 4 ] = pPalette[ 5 ] = 0xff;
    }
}

void PCXReader::ImplReadBody()
{
    BYTE    *pPlane[ 4 ], * pDest, * pSource1, * pSource2, * pSource3, *pSource4;
    ULONG   i, nx, ny, np, nCount, nUsedLineSize, nLineSize, nPercent, nLastPercent;
    BYTE    nDat, nCol;

    nUsedLineSize = (ULONG)( ( ( nWidth * (ULONG)nDestBitsPerPixel ) + 7 ) >> 3 );
    nLineSize = ( nUsedLineSize + 3 ) & 0xfffc;

    for( np = 0; np < nPlanes; np++ )
        pPlane[ np ] = new BYTE[ nBytesPerPlaneLin ];

    nCount = 0;
    for ( ny = 0; ny < nHeight; ny++ )
    {
        nPercent = ny * 60 / nHeight + 10;
        if ( ny == 0 || nLastPercent + 4 <= nPercent )
        {
            nLastPercent = nPercent;
            if ( Callback( (USHORT)nPercent ) == TRUE )
                break;
        }
        for ( np = 0; np < nPlanes; np++)
        {
            if ( nEncoding == 0)
                pPCX->Read( (void *)pPlane[ np ], nBytesPerPlaneLin );
            else
            {
                pDest = pPlane[ np ];
                nx = nBytesPerPlaneLin;
                while ( nCount > 0 && nx > 0)
                {
                    *(pDest++) = nDat;
                    nx--;
                    nCount--;
                }
                while ( nx > 0 )
                {
                    *pPCX >> nDat;
                    if ( ( nDat & 0xc0 ) == 0xc0 )
                    {
                        nCount =( (ULONG)nDat ) & 0x003f;
                        *pPCX >> nDat;
                        if ( nCount < nx )
                        {
                            nx -= nCount;
                            while ( nCount > 0)
                            {
                                *(pDest++) = nDat;
                                nCount--;
                            }
                        }
                        else
                        {
                            nCount -= nx;
                            do
                            {
                                *(pDest++) = nDat;
                                nx--;
                            }
                            while ( nx > 0 );
                            break;
                        }
                    }
                    else
                    {
                        *(pDest++) = nDat;
                        nx--;
                    }
                }
            }
        }
        pSource1 = pPlane[ 0 ];
        pSource2 = pPlane[ 1 ];
        pSource3 = pPlane[ 2 ];
        pSource4 = pPlane[ 3 ];
        switch ( nBitsPerPlanePix + ( nPlanes << 8 ) )
        {
            // 2 colors
            case 0x101 :
                for ( i = 0; i < nWidth; i++ )
                {
                    ULONG nShift = ( i & 7 ) ^ 7;
                    if ( nShift == 0 )
                        pAcc->SetPixel( ny, i, ( *pSource1++ & 1 ) );
                    else
                        pAcc->SetPixel( ny, i, ( *pSource1 >> nShift ) & 1 );
                }
                break;
            // 4 colors
            case 0x102 :
                for ( i = 0; i < nWidth; i++ )
                {
                    switch( i & 3 )
                    {
                        case 0 :
                            nCol = *pSource1 >> 6;
                            break;
                        case 1 :
                            nCol = ( *pSource1 >> 4 ) & 0x03 ;
                            break;
                        case 2 :
                            nCol = ( *pSource1 >> 2 ) & 0x03;
                            break;
                        case 3 :
                            nCol = ( *pSource1++ ) & 0x03;
                            break;
                    }
                    pAcc->SetPixel( ny, i, nCol );
                }
                break;
            // 256 colors
            case 0x108 :
                for ( i = 0; i < nWidth; i++ )
                {
                    pAcc->SetPixel( ny, i, *pSource1++ );
                }
                break;
            // 8 colors
            case 0x301 :
                for ( i = 0; i < nWidth; i++ )
                {
                    ULONG nShift = ( i & 7 ) ^ 7;
                    if ( nShift == 0 )
                    {
                        nCol = ( *pSource1++ & 1) + ( ( *pSource2++ << 1 ) & 2 ) + ( ( *pSource3++ << 2 ) & 4 );
                        pAcc->SetPixel( ny, i, nCol );
                    }
                    else
                    {
                        nCol = ( ( *pSource1 >> nShift ) & 1)  + ( ( ( *pSource2 >> nShift ) << 1 ) & 2 ) +
                            ( ( ( *pSource3 >> nShift ) << 2 ) & 4 );
                        pAcc->SetPixel( ny, i, nCol );
                    }
                }
                break;
            // 16 colors
            case 0x401 :
                for ( i = 0; i < nWidth; i++ )
                {
                    ULONG nShift = ( i & 7 ) ^ 7;
                    if ( nShift == 0 )
                    {
                        nCol = ( *pSource1++ & 1) + ( ( *pSource2++ << 1 ) & 2 ) + ( ( *pSource3++ << 2 ) & 4 ) +
                            ( ( *pSource4++ << 3 ) & 8 );
                        pAcc->SetPixel( ny, i, nCol );
                    }
                    else
                    {
                        nCol = ( ( *pSource1 >> nShift ) & 1)  + ( ( ( *pSource2 >> nShift ) << 1 ) & 2 ) +
                            ( ( ( *pSource3 >> nShift ) << 2 ) & 4 ) + ( ( ( *pSource4 >> nShift ) << 3 ) & 8 );
                        pAcc->SetPixel( ny, i, nCol );
                    }
                }
                break;
            // 16m colors
            case 0x308 :
                for ( i = 0; i < nWidth; i++ )
                {
                    pAcc->SetPixel( ny, i, Color( *pSource1++, *pSource2++, *pSource3++ ) );

                }
                break;
            default :
                nStatus = FALSE;
                break;
        }
    }
    for ( np = 0; np < nPlanes; np++ )
        delete[] pPlane[ np ];
}

void PCXReader::ImplReadPalette( ULONG nCol )
{
    BYTE    r, g, b;
    BYTE*   pPtr = pPalette;
    for ( ULONG i = 0; i < nCol; i++ )
    {
        *pPCX >> r >> g >> b;
        *pPtr++ = r;
        *pPtr++ = g;
        *pPtr++ = b;
    }
}

//================== GraphicImport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicImport(SvStream & rStream, Graphic & rGraphic,
                            PFilterCallback pCallback, void * pCallerData,
                                FilterConfigItem*, BOOL)
#else
extern "C" BOOL GraphicImport(SvStream & rStream, Graphic & rGraphic,
                            PFilterCallback pCallback, void * pCallerData,
                                FilterConfigItem*, BOOL)
#endif
{
    PCXReader aPCXReader;
    BOOL nRetValue = aPCXReader.ReadPCX( rStream, rGraphic, pCallback, pCallerData );
    if ( nRetValue == FALSE )
        rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
    return nRetValue;
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

