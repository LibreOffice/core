/*************************************************************************
 *
 *  $RCSfile: ipcd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:15 $
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

#include <tools/new.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/config.hxx>
#include <vcl/svapp.hxx>
#include <svtools/fltcall.hxx>
#include <svtools/solar.hrc>
#include "strings.hrc"
#include "dlgipcd.hrc"
#include "dlgipcd.hxx"

//============================ PCDReader ==================================

// Diese Aufloesungen sind in einer PCD-Datei enthalten:
enum PCDResolution {
    PCDRES_BASE16,  //  192 x  128
    PCDRES_BASE4,   //  384 x  256
    PCDRES_BASE,    //  768 x  512
    // Die folgenden sind komprimiert und koennen
    // von uns NICHT gelesen werden:
    PCDRES_4BASE,   // 1536 x 1024
    PCDRES_16BASE   // 3072 x 3072
};


// Schluesselworte in der INI-Datei:
#define PCDINI_RES_KEY     "PCD-IMPORT-RESOLUTION"
#define PCDINI_RES_BASE16  "BASE/16"
#define PCDINI_RES_BASE4   "BASE/4"
#define PCDINI_RES_BASE    "BASE"


class PCDReader {

private:

    BOOL bStatus;

    PFilterCallback     pCallback;
    void*               pCallerData;
    ULONG               nLastPercent;

    SvStream*           pPCD;
    BitmapWriteAccess*  mpAcc;

    BYTE                nOrientation;   // Ausrichtung des Bildes in der PCD-Datei:
                                        // 0 - Turmspitze zeigt nach oben
                                        // 1 - Turmspitze zeigt nach rechts
                                        // 2 - Turmspitze zeigt nach unten
                                        // 3 - Turmspitze zeigt nach links

    PCDResolution       eResolution;    // Welche Aufloesung wir haben wollen

    ULONG               nWidth;         // Breite des PCD-Bildes
    ULONG               nHeight;        // Hoehe des PCD-Bildes
    ULONG               nImagePos;      // Position des Bildes in der PCD-Datei

    // Temporare BLue-Green-Red-Bitmap
    ULONG               nBMPWidth;
    ULONG               nBMPHeight;

    void    MayCallback(ULONG nPercent);

    void    CheckPCDImagePacFile();
        // Prueft, ob es eine Photo-CD-Datei mit 'Image Pac' ist.

    void    ReadOrientation();
        // Liest die Ausrichtung und setzt nOrientation

    void    ReadImage(ULONG nMinPercent, ULONG nMaxPercent);

public:

    PCDReader() {}
    ~PCDReader() {}

    BOOL ReadPCD(SvStream & rPCD, Graphic & rGraphic,
                  PFilterCallback pcallback, void * pcallerdata,
                  Config * pConfig);
};

//=================== Methoden von PCDReader ==============================

BOOL PCDReader::ReadPCD(SvStream & rPCD, Graphic & rGraphic,
                        PFilterCallback pcallback, void * pcallerdata,
                        Config * pConfig)
{
    Bitmap       aBmp;

    bStatus      = TRUE;
    pCallback    = pcallback;
    pCallerData  = pcallerdata;
    nLastPercent = 0;
    pPCD         = &rPCD;

    MayCallback( 0 );

    // Ist es eine PCD-Datei mit Bild ? ( setzt bStatus == FALSE, wenn nicht ):
    CheckPCDImagePacFile();

    // Orientierung des Bildes einlesen:
    ReadOrientation();

    // Welche Aufloesung wollen wir ?:
    if ( pConfig == NULL )
        eResolution = PCDRES_BASE;
    else
    {
        String aStr;

        pConfig->Update();
        aStr = UniString( pConfig->ReadKey( PCDINI_RES_KEY, PCDINI_RES_BASE ), RTL_TEXTENCODING_UTF8 );
        aStr.ToUpperAscii();
        if ( aStr.CompareToAscii( PCDINI_RES_BASE16 ) == COMPARE_EQUAL )
            eResolution = PCDRES_BASE16;
        else if ( aStr.CompareToAscii( PCDINI_RES_BASE4 ) == COMPARE_EQUAL )
            eResolution = PCDRES_BASE4;
        else
            eResolution = PCDRES_BASE;
    }

    // Groesse und Position (Position in PCD-Datei) des Bildes bestimmen:
    switch (eResolution)
    {
        case PCDRES_BASE16 :
            nWidth = 192;
            nHeight = 128;
            nImagePos = 8192;
            break;

        case PCDRES_BASE4 :
            nWidth = 384;
            nHeight = 256;
            nImagePos = 47104;
            break;

        case PCDRES_BASE :
            nWidth = 768;
            nHeight = 512;
            nImagePos = 196608;
            break;

        default:
            bStatus = FALSE;
    }
    if ( bStatus )
    {
        if ( ( nOrientation & 0x01 ) == 0 )
        {
            nBMPWidth = nWidth;
            nBMPHeight = nHeight;
        }
        else
        {
            nBMPWidth = nHeight;
            nBMPHeight = nWidth;
        }
        aBmp = Bitmap( Size( nBMPWidth, nBMPHeight ), 24 );
        if ( ( mpAcc = aBmp.AcquireWriteAccess() ) == FALSE )
            return FALSE;

        ReadImage( 5 ,65 );

        aBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
        rGraphic = aBmp;
    }
    return bStatus;
}

// -------------------------------------------------------------------------------------------

void PCDReader::MayCallback(ULONG nPercent)
{
    if ( nPercent >= nLastPercent + 3 )
    {
        nLastPercent=nPercent;
        if ( pCallback != NULL && nPercent <= 100 && bStatus == TRUE )
        {
            if ( ( (*pCallback)( pCallerData, (USHORT)nPercent ) ) == TRUE )
                bStatus = FALSE;
        }
    }
}

// -------------------------------------------------------------------------------------------

void PCDReader::CheckPCDImagePacFile()
{
    char Buf[ 8 ];

    pPCD->Seek( 2048 );
    pPCD->Read( Buf, 7 );
    Buf[ 7 ] = 0;
    if ( ByteString( Buf ).CompareTo( "PCD_IPI" ) != COMPARE_EQUAL )
        bStatus = FALSE;
}

// -------------------------------------------------------------------------------------------

void PCDReader::ReadOrientation()
{
    if ( bStatus == FALSE )
        return;
    pPCD->Seek( 194635 );
    *pPCD >> nOrientation;
    nOrientation &= 0x03;
}

// -------------------------------------------------------------------------------------------

void PCDReader::ReadImage(ULONG nMinPercent, ULONG nMaxPercent)
{
    ULONG  nx,ny,nW2,nH2,nYPair,ndy,nXPair;
    long   nL,nCb,nCr,nRed,nGreen,nBlue;
    BYTE * pt;
    BYTE * pL0; // Luminanz fuer jeden Pixel der 1. Zeile des aktuellen Zeilen-Paars
    BYTE * pL1; // Luminanz fuer jeden Pixel der 2. Zeile des aktuellen Zeilen-Paars
    BYTE * pCb; // Blau-Chrominanz fuer je 2x2 Pixel des aktuellen Zeilen-Paars
    BYTE * pCr; // Rot-Chrominanz fuer je 2x2 Pixel des aktuellen Zeilen-Paars
    BYTE * pL0N, * pL1N, * pCbN, * pCrN; // wie oben, nur fuer das naechste Zeilen-Paar

    if ( bStatus == FALSE )
        return;

    nW2=nWidth>>1;
    nH2=nHeight>>1;

    pL0 =(BYTE*)SvMemAlloc(nWidth,MEM_NOCALLNEWHDL);
    pL1 =(BYTE*)SvMemAlloc(nWidth,MEM_NOCALLNEWHDL);
    pCb =(BYTE*)SvMemAlloc(nW2+1 ,MEM_NOCALLNEWHDL);
    pCr =(BYTE*)SvMemAlloc(nW2+1 ,MEM_NOCALLNEWHDL);
    pL0N=(BYTE*)SvMemAlloc(nWidth,MEM_NOCALLNEWHDL);
    pL1N=(BYTE*)SvMemAlloc(nWidth,MEM_NOCALLNEWHDL);
    pCbN=(BYTE*)SvMemAlloc(nW2+1 ,MEM_NOCALLNEWHDL);
    pCrN=(BYTE*)SvMemAlloc(nW2+1 ,MEM_NOCALLNEWHDL);

    if ( pL0 == NULL || pL1 == NULL || pCb == NULL || pCr == NULL ||
        pL0N == NULL || pL1N == NULL || pCbN == NULL || pCrN == NULL)
    {
        if (pL0 !=NULL) SvMemFree((void*)pL0 );
        if (pL1 !=NULL) SvMemFree((void*)pL1 );
        if (pCb !=NULL) SvMemFree((void*)pCb );
        if (pCr !=NULL) SvMemFree((void*)pCr );
        if (pL0N!=NULL) SvMemFree((void*)pL0N);
        if (pL1N!=NULL) SvMemFree((void*)pL1N);
        if (pCbN!=NULL) SvMemFree((void*)pCbN);
        if (pCrN!=NULL) SvMemFree((void*)pCrN);
        bStatus = FALSE;
        return;
    }

    pPCD->Seek( nImagePos );

    // naechstes Zeilen-Paar := erstes Zeile-Paar:
    pPCD->Read( pL0N, nWidth );
    pPCD->Read( pL1N, nWidth );
    pPCD->Read( pCbN, nW2 );
    pPCD->Read( pCrN, nW2 );
    pCbN[ nW2 ] = pCbN[ nW2 - 1 ];
    pCrN[ nW2 ] = pCrN[ nW2 - 1 ];

    for ( nYPair = 0; nYPair < nH2; nYPair++ )
    {
        // aktuelles Zeilen-Paar := naechstes Zeilen-Paar
        pt=pL0; pL0=pL0N; pL0N=pt;
        pt=pL1; pL1=pL1N; pL1N=pt;
        pt=pCb; pCb=pCbN; pCbN=pt;
        pt=pCr; pCr=pCrN; pCrN=pt;

        // naechstes Zeilen-Paar holen:
        if ( nYPair < nH2 - 1 )
        {
            pPCD->Read( pL0N, nWidth );
            pPCD->Read( pL1N, nWidth );
            pPCD->Read( pCbN, nW2 );
            pPCD->Read( pCrN, nW2 );
            pCbN[nW2]=pCbN[ nW2 - 1 ];
            pCrN[nW2]=pCrN[ nW2 - 1 ];
        }
        else
        {
            for ( nXPair = 0; nXPair < nW2; nXPair++ )
            {
                pCbN[ nXPair ] = pCb[ nXPair ];
                pCrN[ nXPair ] = pCr[ nXPair ];
            }
        }

        // Schleife uber die beiden Zeilen des Zeilen-Paars:
        for ( ndy = 0; ndy < 2; ndy++ )
        {
            ny = ( nYPair << 1 ) + ndy;

            // Schleife ueber X:
            for ( nx = 0; nx < nWidth; nx++ )
            {
                // nL,nCb,nCr fuer den Pixel nx,ny holen/berechenen:
                nXPair = nx >> 1;
                if ( ndy == 0 )
                {
                    nL = (long)pL0[ nx ];
                    if (( nx & 1 ) == 0 )
                    {
                        nCb = (long)pCb[ nXPair ];
                        nCr = (long)pCr[ nXPair ];
                    }
                    else
                    {
                        nCb = ( ( (long)pCb[ nXPair ] ) + ( (long)pCb[ nXPair + 1 ] ) ) >> 1;
                        nCr = ( ( (long)pCr[ nXPair ] ) + ( (long)pCr[ nXPair + 1 ] ) ) >> 1;
                    }
                }
                else {
                    nL = pL1[ nx ];
                    if ( ( nx & 1 ) == 0 )
                    {
                        nCb = ( ( (long)pCb[ nXPair ] ) + ( (long)pCbN[ nXPair ] ) ) >> 1;
                        nCr = ( ( (long)pCr[ nXPair ] ) + ( (long)pCrN[ nXPair ] ) ) >> 1;
                    }
                    else
                    {
                        nCb = ( ( (long)pCb[ nXPair ] ) + ( (long)pCb[ nXPair + 1 ] ) +
                               ( (long)pCbN[ nXPair ] ) + ( (long)pCbN[ nXPair + 1 ] ) ) >> 2;
                        nCr = ( ( (long)pCr[ nXPair ] ) + ( (long)pCr[ nXPair + 1] ) +
                               ( (long)pCrN[ nXPair ] ) + ( (long)pCrN[ nXPair + 1 ] ) ) >> 2;
                    }
                }
                // Umwandlung von nL,nCb,nCr in nRed,nGreen,nBlue:
                nL *= 89024L;
                nCb -= 156;
                nCr -= 137;
                nRed = ( nL + nCr * 119374L + 0x8000 ) >> 16;
                if ( nRed < 0 )
                    nRed = 0;
                if ( nRed > 255)
                    nRed = 255;
                nGreen = ( nL - nCb * 28198L - nCr * 60761L + 0x8000 ) >> 16;
                if ( nGreen < 0 )
                    nGreen = 0;
                if ( nGreen > 255 )
                    nGreen = 255;
                nBlue = ( nL + nCb * 145352L + 0x8000 ) >> 16;
                if ( nBlue < 0 )
                    nBlue = 0;
                if ( nBlue > 255 )
                    nBlue = 255;

                // Farbwert in pBMPMap eintragen:
                if ( nOrientation < 2 )
                {
                    if ( nOrientation == 0 )
                        mpAcc->SetPixel( ny, nx, BitmapColor( (BYTE)nRed, (BYTE)nGreen, (BYTE)nBlue ) );
                    else
                        mpAcc->SetPixel( nWidth - 1 - nx, ny, BitmapColor( (BYTE)nRed, (BYTE)nGreen, (BYTE)nBlue ) );
                }
                else
                {
                    if ( nOrientation == 2 )
                        mpAcc->SetPixel( nHeight - 1 - ny, ( nWidth - 1 - nx ), BitmapColor( (BYTE)nRed, (BYTE)nGreen, (BYTE)nBlue ) );
                    else
                        mpAcc->SetPixel( nx, ( nHeight - 1 - ny ), BitmapColor( (BYTE)nRed, (BYTE)nGreen, (BYTE)nBlue ) );
                }
            }
        }

        if ( pPCD->GetError() )
            bStatus = FALSE;
        MayCallback( nMinPercent + ( nMaxPercent - nMinPercent ) * nYPair / nH2 );
        if ( bStatus == FALSE )
            break;
    }
    SvMemFree((void*)pL0 );
    SvMemFree((void*)pL1 );
    SvMemFree((void*)pCb );
    SvMemFree((void*)pCr );
    SvMemFree((void*)pL0N);
    SvMemFree((void*)pL1N);
    SvMemFree((void*)pCbN);
    SvMemFree((void*)pCrN);
}

//================== GraphicImport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config * pOptionsConfig, BOOL)
#else
extern "C" BOOL GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config * pOptionsConfig, BOOL)
#endif
{
    PCDReader aPCDReader;
    return aPCDReader.ReadPCD( rStream, rGraphic, pCallback, pCallerData, pOptionsConfig );
}

//================== GraphicDialog - die exportierte Funktion ================

extern "C" BOOL SAL_CALL DoImportDialog( FltCallDialogParameter& rPara )
{
    BOOL    bRet = FALSE;

    if ( rPara.pWindow && rPara.pCfg )
    {
        ByteString  aResMgrName( "icd" );
        ResMgr* pResMgr;

        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational(). GetLanguage() );

        rPara.pResMgr = pResMgr;
        bRet = ( DlgIPCD( rPara ).Execute() == RET_OK );

        delete pResMgr;
    }

    return bRet;
}

//============================= fuer Windows ==================================

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


