/*************************************************************************
 *
 *  $RCSfile: ipbm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-08 15:46:25 $
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

//============================ PBMReader ==================================

class PBMReader {

private:

    PFilterCallback pCallback;
    void * pCallerData;

    SvStream*           mpPBM;          // Die einzulesende PBM-Datei

    BOOL                mbStatus;
    BOOL                mbRemark;       // FALSE wenn sich stream in einem Kommentar befindet
    BOOL                mbRaw;          // RAW/ASCII MODE
    ULONG               mnMode;         // 0->PBM, 1->PGM, 2->PPM
    Bitmap              maBmp;
    BitmapWriteAccess*  mpAcc;
    ULONG               mnWidth, mnHeight;  // Bildausmass in Pixeln
    ULONG               mnCol;
    ULONG               mnMaxVal;           // maximaler wert in den
    BOOL                ImplCallback( USHORT nPercent );
    BOOL                ImplReadBody();
    BOOL                ImplReadHeader();

public:
                        PBMReader();
                        ~PBMReader();
    BOOL                ReadPBM( SvStream & rPBM, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata );
};

//=================== Methoden von PBMReader ==============================

PBMReader::PBMReader() :
    mpAcc       ( NULL ),
    mbStatus    ( TRUE ),
    mbRemark    ( FALSE ),
    mbRaw       ( TRUE )
{
}

PBMReader::~PBMReader()
{
}

BOOL PBMReader::ImplCallback( USHORT nPercent )
{
    if ( pCallback != NULL )
    {
        if ( ( (*pCallback)( pCallerData, nPercent ) ) == TRUE )
        {
            mpPBM->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return TRUE;
        }
    }
    return FALSE;
}

BOOL PBMReader::ReadPBM( SvStream & rPBM, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata)
{
    USHORT i;

    if ( rPBM.GetError() ) return FALSE;

    pCallback = pcallback;
    pCallerData = pcallerdata;

    mpPBM = &rPBM;
    mpPBM->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // Kopf einlesen:

    if ( ( mbStatus = ImplReadHeader() ) == FALSE )
        return FALSE;

    if ( mnWidth == 0 || mnHeight == 0 )
        return FALSE;

    // 0->PBM, 1->PGM, 2->PPM
    switch ( mnMode )
    {
        case 0 :
            maBmp = Bitmap( Size( mnWidth, mnHeight ), 1 );
            if ( ( mpAcc = maBmp.AcquireWriteAccess() ) == FALSE )
                return FALSE;
            mpAcc->SetPaletteEntryCount( 2 );
            mpAcc->SetPaletteColor( 0, BitmapColor( 0xff, 0xff, 0xff ) );
            mpAcc->SetPaletteColor( 1, BitmapColor( 0x00, 0x00, 0x00 ) );
            break;

        case 1 :
            if ( mnMaxVal <= 1 )
                maBmp = Bitmap( Size( mnWidth, mnHeight ), 1);
            else if ( mnMaxVal <= 15 )
                maBmp = Bitmap( Size( mnWidth, mnHeight ), 4);
            else
                maBmp = Bitmap( Size( mnWidth, mnHeight ), 8);

            if ( ( mpAcc = maBmp.AcquireWriteAccess() ) == FALSE )
                return FALSE;
            mnCol = (USHORT)mnMaxVal + 1;
            if ( mnCol > 256 )
                mnCol = 256;

            mpAcc->SetPaletteEntryCount( 256 );
            for ( i = 0; i < mnCol; i++ )
            {
                ULONG nCount = 255 * i / mnCol;
                mpAcc->SetPaletteColor( i, BitmapColor( (BYTE)nCount, (BYTE)nCount, (BYTE)nCount ) );
            }
            break;
        case 2 :
            maBmp = Bitmap( Size( mnWidth, mnHeight ), 24 );
            if ( ( mpAcc = maBmp.AcquireWriteAccess() ) == FALSE )
                return FALSE;
            break;
    }

    // Bitmap-Daten einlesen
    mbStatus = ImplReadBody();

    if ( mpAcc )
    {
        maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
    }
    if ( mbStatus )
        rGraphic = maBmp;

    return mbStatus;
}

BOOL PBMReader::ImplReadHeader()
{
    BYTE    nID[ 2 ];
    BYTE    nDat;
    BYTE    nMax, nCount = 0;
    BOOL    bFinished = FALSE;

    *mpPBM >> nID[ 0 ] >> nID[ 1 ];
    if ( nID[ 0 ] != 'P' )
        return FALSE;
    switch ( nID[ 1 ] )
    {
        case '1' :
            mbRaw = FALSE;
        case '4' :
            mnMode = 0;
            nMax = 2;               // number of parameters in Header
            break;
        case '2' :
            mbRaw = FALSE;
        case '5' :
            mnMode = 1;
            nMax = 3;
            break;
        case '3' :
            mbRaw = FALSE;
        case '6' :
            mnMode = 2;
            nMax = 3;
            break;
        default:
            return FALSE;
    }

    mnMaxVal = mnWidth = mnHeight = 0;

    while ( bFinished == FALSE )
    {
        if ( mpPBM->GetError() )
            return FALSE;

        *mpPBM >> nDat;

        if ( nDat == '#' )
        {
            mbRemark = TRUE;
            continue;
        }
        else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
        {
            mbRemark = FALSE;
            nDat = 0x20;
        }
        if ( mbRemark )
            continue;

        if ( ( nDat == 0x20 ) || ( nDat == 0x09 ) )
        {
            if ( ( nCount == 0 ) && mnWidth )
                nCount++;
            else if ( ( nCount == 1 ) && mnHeight )
            {
                if ( ++nCount == nMax )
                    bFinished = TRUE;
            }
            else if ( ( nCount == 2 ) && mnMaxVal )
            {
                bFinished = TRUE;
            }
            continue;
        }
        if ( ( nDat >= '0' ) && ( nDat <= '9' ) )
        {
            nDat -= '0';
            if ( nCount == 0 )
            {
                mnWidth *= 10;
                mnWidth += nDat;
            }
            else if ( nCount == 1 )
            {
                mnHeight *= 10;
                mnHeight += nDat;
            }
            else if ( nCount == 2 )
            {
                mnMaxVal *= 10;
                mnMaxVal += nDat;
            }
        }
        else
            return FALSE;
    }
    return mbStatus;
}

BOOL PBMReader::ImplReadBody()
{
    BOOL    bPara, bFinished = FALSE;
    BYTE    nDat, nCount;
    ULONG   nGrey, nRGB[3];
    ULONG   nWidth = 0;
    ULONG   nHeight = 0;
    char    nShift = 0;

    if ( mbRaw )
    {
        switch ( mnMode )
        {

            // PBM
            case 0 :
                while ( nHeight != mnHeight )
                {
                    if ( mpPBM->IsEof() || mpPBM->GetError() )
                        return FALSE;

                    if ( --nShift < 0 )
                    {
                        *mpPBM >> nDat;
                        nShift = 7;
                    }
                    mpAcc->SetPixel( nHeight, nWidth, nDat >> nShift );
                    if ( ++nWidth == mnWidth )
                    {
                        nShift = 0;
                        nWidth = 0;
                        nHeight++;
                        ImplCallback( (USHORT)( ( 100 * nHeight ) / mnHeight ) );   // processing output in percent
                    }
                }
                break;

            // PGM
            case 1 :
                while ( nHeight != mnHeight )
                {
                    if ( mpPBM->IsEof() || mpPBM->GetError() )
                        return FALSE;

                    *mpPBM >> nDat;
                    mpAcc->SetPixel( nHeight, nWidth++, nDat);

                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        nHeight++;
                        ImplCallback( (USHORT)( ( 100 * nHeight ) / mnHeight ) );   // processing output in percent
                    }
                }
                break;

            // PPM
            case 2 :
                while ( nHeight != mnHeight )
                {
                    if ( mpPBM->IsEof() || mpPBM->GetError() )
                        return FALSE;

                    BYTE    nR, nG, nB;
                    ULONG   nRed, nGreen, nBlue;
                    *mpPBM >> nR >> nG >> nB;
                    nRed = 255 * nR / mnMaxVal;
                    nGreen = 255 * nG / mnMaxVal;
                    nBlue = 255 * nB / mnMaxVal;
                    mpAcc->SetPixel( nHeight, nWidth++, BitmapColor( (BYTE)nRed, (BYTE)nGreen, (BYTE)nBlue ) );
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        nHeight++;
                        ImplCallback( (USHORT) ( ( 100 * nHeight ) / mnHeight ) );  // processing output in percent
                    }
                }
                break;
        }
    }
    else switch  ( mnMode )
    {
        // PBM
        case 0 :
            while ( bFinished == FALSE )
            {
                if ( mpPBM->IsEof() || mpPBM->GetError() )
                    return FALSE;

                *mpPBM >> nDat;

                if ( nDat == '#' )
                {
                    mbRemark = TRUE;
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = FALSE;
                    continue;
                }
                if ( mbRemark || nDat == 0x20 || nDat == 0x09 )
                    continue;

                if ( nDat == '0' || nDat == '1' )
                {
                    mpAcc->SetPixel( nHeight, nWidth, (BYTE)nDat-'0' );
                    nWidth++;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = TRUE;
                        ImplCallback( (USHORT) ( ( 100 * nHeight ) / mnHeight ) );  // processing output in percent
                    }
                }
                else
                    return FALSE;
            }
            break;

        // PGM
        case 1 :

            bPara = FALSE;
            nCount = 0;
            nGrey = 0;

            while ( bFinished == FALSE )
            {
                if ( nCount )
                {
                    nCount--;
                    if ( nGrey <= mnMaxVal )
                        nGrey = 255 * nGrey / mnMaxVal;
                        mpAcc->SetPixel( nHeight, nWidth++, (BYTE)nGrey );
                    nGrey = 0;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = TRUE;
                        ImplCallback( (USHORT) ( ( 100 * nHeight ) / mnHeight ) );  // processing output in percent
                    }
                    continue;
                }

                if ( mpPBM->IsEof() || mpPBM->GetError() )
                    return FALSE;

                *mpPBM >> nDat;

                if ( nDat == '#' )
                {
                    mbRemark = TRUE;
                    if ( bPara )
                    {
                        bPara = FALSE;
                        nCount++;
                    }
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = FALSE;
                    if ( bPara )
                    {
                        bPara = FALSE;
                        nCount++;
                    }
                    continue;
                }

                if ( nDat == 0x20 || nDat == 0x09 )
                {
                    if ( bPara )
                    {
                        bPara = FALSE;
                        nCount++;
                    }
                    continue;
                }
                if ( nDat >= '0' && nDat <= '9' )
                {
                    bPara = TRUE;
                    nGrey *= 10;
                    nGrey += nDat-'0';
                    continue;
                }
                else
                    return FALSE;
            }
            break;



        // PPM
        case 2 :

            bPara = FALSE;
            nCount = 0;
            nRGB[ 0 ] = nRGB[ 1 ] = nRGB[ 2 ] = 0;

            while ( bFinished == FALSE )
            {
                if ( nCount == 3 )
                {
                    nCount = 0;
                    mpAcc->SetPixel( nHeight, nWidth++, BitmapColor( (BYTE)nRGB[ 0 ], (BYTE)nRGB[ 1 ], (BYTE)nRGB[ 2 ] ) );
                    nCount = 0;
                    nRGB[ 0 ] = nRGB[ 1 ] = nRGB[ 2 ] = 0;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = TRUE;
                        ImplCallback( (USHORT) ( ( 100 * nHeight ) / mnHeight ) );  // processing output in percent
                    }
                    continue;
                }

                if ( mpPBM->IsEof() || mpPBM->GetError() )
                    return FALSE;

                *mpPBM >> nDat;

                if ( nDat == '#' )
                {
                    mbRemark = TRUE;
                    if ( bPara )
                    {
                        bPara = FALSE;
                        nCount++;
                    }
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = FALSE;
                    if ( bPara )
                    {
                        bPara = FALSE;
                        nCount++;
                    }
                    continue;
                }

                if ( nDat == 0x20 || nDat == 0x09 )
                {
                    if ( bPara )
                    {
                        bPara = FALSE;
                        nCount++;
                    }
                    continue;
                }
                if ( nDat >= '0' && nDat <= '9' )
                {
                    bPara = TRUE;
                    nRGB[ nCount ] *= 10;
                    nRGB[ nCount ] += nDat-'0';
                    continue;
                }
                else
                    return FALSE;
            }
            break;
    }
    return mbStatus;
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
    PBMReader aPBMReader;

    return aPBMReader.ReadPBM( rStream, rGraphic, pCallback, pCallerData );
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

