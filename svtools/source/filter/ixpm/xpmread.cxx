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
#include "precompiled_svtools.hxx"

#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include "rgbtable.hxx"
#define _XPMPRIVATE
#include "xpmread.hxx"

// -------------
// - XPMReader -
// -------------

XPMReader::XPMReader( SvStream& rStm ) :
            mrIStm          ( rStm ),
            mpAcc           ( NULL ),
            mpMaskAcc       ( NULL ),
            mnLastPos       ( rStm.Tell() ),
            mnWidth         ( 0 ),
            mnHeight        ( 0 ),
            mnColors        ( 0 ),
            mnCpp           ( 0 ),
            mbTransparent   ( sal_False ),
            mbStatus        ( sal_True ),
            mnStatus        ( 0 ),
            mnIdentifier    ( XPMIDENTIFIER ),
            mcThisByte      ( 0 ),
            mnTempAvail     ( 0 ),
            mpFastColorTable( NULL ),
            mpColMap        ( NULL )
{

}

// ------------------------------------------------------------------------

XPMReader::~XPMReader()
{
    if( mpAcc )
        maBmp.ReleaseAccess( mpAcc );
}

// ------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ("",off)
#endif

ReadState XPMReader::ReadXPM( Graphic& rGraphic )
{
    ReadState   eReadState;
    sal_uInt8       cDummy;

    // sehen, ob wir _alles_ lesen koennen
    mrIStm.Seek( STREAM_SEEK_TO_END );
    mrIStm >> cDummy;

    // falls wir nicht alles lesen koennen
    // kehren wir zurueck und warten auf neue Daten
    if ( mrIStm.GetError() != ERRCODE_IO_PENDING )
    {
        mrIStm.Seek( mnLastPos );
        mbStatus = sal_True;

        if ( mbStatus )
        {
            mpStringBuf = new sal_uInt8 [ XPMSTRINGBUF ];
            mpTempBuf = new sal_uInt8 [ XPMTEMPBUFSIZE ];

            if ( ( mbStatus = ImplGetString() ) == sal_True )
            {
                mnIdentifier = XPMVALUES;           // Bitmap informationen einholen
                mnWidth = ImplGetULONG( 0 );
                mnHeight = ImplGetULONG( 1 );
                mnColors = ImplGetULONG( 2 );
                mnCpp = ImplGetULONG( 3 );
            }
            if ( mnColors > ( SAL_MAX_UINT32 / ( 4 + mnCpp ) ) )
                mbStatus = sal_False;
            if ( ( mnWidth * mnCpp ) >= XPMSTRINGBUF )
                mbStatus = sal_False;
            if ( mbStatus && mnWidth && mnHeight && mnColors && mnCpp )
            {
                mnIdentifier = XPMCOLORS;

                // mpColMap beinhaltet fuer jede vorhandene
                // Farbe:   ( mnCpp )Byte(s)-> ASCII Eintrag der der Farbe zugeordnet ist
                //              1    Byte   -> 0xff wenn Farbe transparent ist
                //              3    Bytes  -> RGB Wert der Farbe
                mpColMap = new sal_uInt8[ mnColors * ( 4 + mnCpp ) ];
                if ( mpColMap )
                {
                    for ( sal_uLong i = 0; i < mnColors; i++ )
                    {
                        if ( ImplGetColor( i ) == sal_False )
                        {
                            mbStatus = sal_False;
                            break;
                        }
                    }
                }
                else
                    mbStatus = sal_False;

                if ( mbStatus )
                {
                    // bei mehr als 256 Farben wird eine 24 Bit Grafik erstellt
                    sal_uInt16  nBits = 1;
                    if ( mnColors > 256 )
                        nBits = 24;
                    else if ( mnColors > 16 )
                        nBits = 8;
                    else if ( mnColors > 2 )
                        nBits = 4;
                    else
                        nBits = 1;

                    maBmp = Bitmap( Size( mnWidth, mnHeight ), nBits );
                    mpAcc = maBmp.AcquireWriteAccess();

                    // mbTransparent ist sal_True wenn mindestens eine Farbe Transparent ist
                    if ( mbTransparent )
                    {
                        maMaskBmp = Bitmap( Size( mnWidth, mnHeight ), 1 );
                        if ( ( mpMaskAcc = maMaskBmp.AcquireWriteAccess() ) == NULL )
                            mbStatus =  sal_False;
                    }
                    if( mpAcc && mbStatus )
                    {
                        sal_uLong   i;
                        if ( mnColors <=256 )   // palette is only needed by using less than 257
                        {                       // colors

                            sal_uInt8*  pPtr = &mpColMap[mnCpp];

                            for ( i = 0; i < mnColors; i++ )
                            {
                                mpAcc->SetPaletteColor( (sal_uInt8)i, Color( pPtr[1], pPtr[2], pPtr[3] ) );
                                pPtr += ( mnCpp + 4 );
                            }
                            // using 2 charakters per pixel and less than 257 Colors we speed up
                            if ( mnCpp == 2 )   // by using a 64kb indexing table
                            {
                                mpFastColorTable = new sal_uInt8[ 256 * 256 ];
                                for ( pPtr = mpColMap, i = 0; i < mnColors; i++, pPtr += mnCpp + 4 )
                                {
                                    sal_uLong   j =  pPtr[ 0 ] << 8;
                                            j += pPtr[ 1 ];
                                    mpFastColorTable[ j ] = (sal_uInt8)i;
                                }
                            }
                        }
                        // now we get the bitmap data
                        mnIdentifier = XPMPIXELS;
                        for ( i = 0; i < mnHeight; i++ )
                        {
                            if ( ImplGetScanLine( i ) == sal_False )
                            {
                                mbStatus = sal_False;
                                break;
                            }
                        }
                        mnIdentifier = XPMEXTENSIONS;
                    }
                }
            }

            delete[] mpFastColorTable;
            delete[] mpColMap;
            delete[] mpStringBuf;
            delete[] mpTempBuf;

        }
        if( mbStatus )
        {
            if ( mpMaskAcc )
            {
                maMaskBmp.ReleaseAccess ( mpMaskAcc), mpMaskAcc = NULL;
                maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
                rGraphic = Graphic( BitmapEx( maBmp, maMaskBmp ) );
            }
            else
            {
                maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
                rGraphic = maBmp;
            }
            eReadState = XPMREAD_OK;
        }
        else
        {
            if ( mpMaskAcc ) maMaskBmp.ReleaseAccess ( mpMaskAcc), mpMaskAcc = NULL;
            if ( mpAcc ) maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
            eReadState = XPMREAD_ERROR;
        }
    }
    else
    {
        mrIStm.ResetError();
        eReadState = XPMREAD_NEED_MORE;
    }
    return eReadState;
}

#ifdef _MSC_VER
#pragma optimize ("",on)
#endif

// ------------------------------------------------------------------------
// ImplGetColor ermittelt saemtliche Farbwerte,
// die Rueckgabe ist sal_True wenn saemtliche Farben zugeordnet werden konnten

sal_Bool XPMReader::ImplGetColor( sal_uLong nNumb )
{
    sal_uInt8*  pString = mpStringBuf;
    sal_uInt8*  pPtr =  ( mpColMap + nNumb * ( 4 + mnCpp ) );
    sal_Bool    bStatus = ImplGetString();

    if ( bStatus )
    {
        for ( sal_uLong i = 0; i < mnCpp; i++ )
            *pPtr++ = *pString++;
        bStatus = ImplGetColSub ( pPtr );
    }
    return bStatus;
}

// ------------------------------------------------------------------------
// ImpGetScanLine liest den String mpBufSize aus und schreibt die Pixel in die
// Bitmap. Der Parameter nY gibt die horizontale Position an.

sal_Bool XPMReader::ImplGetScanLine( sal_uLong nY )
{
    sal_Bool    bStatus = ImplGetString();
    sal_uInt8*  pString = mpStringBuf;
    sal_uInt8*  pColor;
    BitmapColor     aWhite;
    BitmapColor     aBlack;

    if ( bStatus )
    {
        if ( mpMaskAcc )
        {
            aWhite = mpMaskAcc->GetBestMatchingColor( Color( COL_WHITE ) );
            aBlack = mpMaskAcc->GetBestMatchingColor( Color( COL_BLACK ) );
        }
        if ( mnStringSize != ( mnWidth * mnCpp ))
            bStatus = sal_False;
        else
        {
            sal_uLong i, j;
            if ( mpFastColorTable )
            {
                for ( i = 0; i < mnWidth; i++ )
                {
                    j = (*pString++) << 8;
                    j += *pString++;
                    sal_uInt8 k = (sal_uInt8)mpFastColorTable[ j ];
                    mpAcc->SetPixel( nY, i, BitmapColor( (sal_uInt8)k ) );

                    if ( mpMaskAcc )
                        mpMaskAcc->SetPixel( nY, i,
                            ( mpColMap[ k * (mnCpp + 4) + mnCpp] ) ? aWhite : aBlack );
                }
            }
            else for ( i = 0; i < mnWidth; i++ )
            {
                pColor = mpColMap;
                for ( j = 0; j < mnColors; j++ )
                {
                    if ( ImplCompare( pString, pColor, mnCpp, XPMCASESENSITIVE ) == sal_True )
                    {
                        if ( mnColors > 256 )
                            mpAcc->SetPixel( nY, i, Color ( pColor[3], pColor[4], pColor[5] ) );
                        else
                            mpAcc->SetPixel( nY, i, BitmapColor( (sal_uInt8) j ) );

                        if ( mpMaskAcc )
                            mpMaskAcc->SetPixel( nY, i, (
                                pColor[ mnCpp ] ) ? aWhite : aBlack );

                        break;
                    }
                    pColor += ( mnCpp + 4 );
                }
                pString += mnCpp;
            }

        }
    }
    return bStatus;
}

// ------------------------------------------------------------------------
// versucht aus mpStringBuf einen Farbwert zu uebermitteln
// wurde eine Farbe gefunden wird an pDest[1]..pDest[2] der RGB wert geschrieben
// pDest[0] enthaelt 0xff wenn die Farbe transparent ist sonst 0

sal_Bool XPMReader::ImplGetColSub( sal_uInt8* pDest )
{
    unsigned char cTransparent[] = "None";

    sal_Bool bColStatus = sal_False;

    if ( ImplGetColKey( 'c' ) || ImplGetColKey( 'm' ) || ImplGetColKey( 'g' ) )
    {
        // hexentry for RGB or HSV color ?
        if ( *mpPara == '#' )
        {
                *pDest++ = 0;
                bColStatus = sal_True;
                switch ( mnParaSize )
                {
                    case 25 :
                        ImplGetRGBHex ( pDest, 6 );
                        break;
                    case 13 :
                        ImplGetRGBHex ( pDest, 2 );
                        break;
                    case  7 :
                        ImplGetRGBHex ( pDest, 0 );
                        break;
                    default:
                        bColStatus = sal_False;
                        break;
                }
        }
        // maybe pixel is transparent
        else if ( ImplCompare( &cTransparent[0], mpPara, 4 ))
        {
            *pDest++ = 0xff;
            bColStatus = sal_True;
            mbTransparent = sal_True;
        }
        // last we will try to get the colorname
        else if ( mnParaSize > 2 )  // name must enlarge the minimum size
        {
            sal_uLong i = 0;
            while ( sal_True )
            {
                if ( pRGBTable[ i ].name == NULL )
                    break;
                if ( pRGBTable[ i ].name[ mnParaSize ] == 0 )
                {
                    if ( ImplCompare ( (unsigned char*)pRGBTable[ i ].name,
                            mpPara, mnParaSize, XPMCASENONSENSITIVE ) )
                    {
                        bColStatus = sal_True;
                        *pDest++ = 0;
                        *pDest++ = pRGBTable[ i ].red;
                        *pDest++ = pRGBTable[ i ].green;
                        *pDest++ = pRGBTable[ i ].blue;
                    }
                }
                i++;
            }
        }
    }
    return bColStatus;
}

// ------------------------------------------------------------------------
// ImplGetColKey durchsuch den String mpStringBuf nach einem Parameter 'nKey'
// und gibt einen sal_Bool zurueck. ( wenn sal_True werden mpPara und mnParaSize gesetzt )

sal_Bool XPMReader::ImplGetColKey( sal_uInt8 nKey )
{
    sal_uInt8 nTemp, nPrev = ' ';

    mpPara = mpStringBuf + mnCpp + 1;
    mnParaSize = 0;

    while ( *mpPara != 0 )
    {
        if ( *mpPara == nKey )
        {
            nTemp = *( mpPara + 1 );
            if ( nTemp == ' ' || nTemp == 0x09 )
            {
                if ( nPrev == ' ' || nPrev == 0x09 )
                    break;
            }
        }
        nPrev = *mpPara;
        mpPara++;
    }
    if ( *mpPara )
    {
        mpPara++;
        while ( (*mpPara == ' ') || (*mpPara == 0x09) )
        {
            mpPara++;
        }
        if ( *mpPara != 0 )
        {
            while ( *(mpPara+mnParaSize) != ' ' && *(mpPara+mnParaSize) != 0x09 &&
                        *(mpPara+mnParaSize) != 0 )
            {
                mnParaSize++;
            }
        }
    }
    return ( mnParaSize ) ? sal_True : sal_False;
}

// ------------------------------------------------------------------------
// ImplGetRGBHex uebersetzt den ASCII-Hexadezimalwert der sich bei mpPara befindet
// in einen RGB wert und schreibt diesen nach pDest
// folgende Formate muessen sich bei mpPara befinden:
// wenn nAdd = 0 : '#12ab12'                    -> RGB = 0x12, 0xab, 0x12
//             2 : '#1234abcd1234'                  "      "     "     "
//             6 : '#12345678abcdefab12345678'      "      "     "     "


void XPMReader::ImplGetRGBHex( sal_uInt8* pDest,sal_uLong  nAdd )
{
    sal_uInt8*  pPtr = mpPara+1;
    sal_uInt8   nHex, nTemp;

    for ( sal_uLong i = 0; i < 3; i++ )
    {
        nHex = (*pPtr++) - '0';
        if ( nHex > 9 )
            nHex = ((nHex - 'A' + '0') & 7) + 10;

        nTemp = (*pPtr++) - '0';
        if ( nTemp > 9 )
            nTemp = ((nTemp - 'A' + '0') & 7) + 10;
        nHex = ( nHex << 4 ) + nTemp;

        pPtr += nAdd;
        *pDest++ = (sal_uInt8)nHex;
    }
}

// ------------------------------------------------------------------------
// ImplGetUlong gibt den wert einer bis zu 6stelligen ASCII-Dezimalzahl zurueck.

sal_uLong XPMReader::ImplGetULONG( sal_uLong nPara )
{
    if ( ImplGetPara ( nPara ) )
    {
        sal_uLong nRetValue = 0;
        sal_uInt8* pPtr = mpPara;

        if ( ( mnParaSize > 6 ) || ( mnParaSize == 0 ) ) return 0;
        for ( sal_uLong i = 0; i < mnParaSize; i++ )
        {
            sal_uInt8 j = (*pPtr++) - 48;
            if ( j > 9 ) return 0;              // ascii is invalid
            nRetValue*=10;
            nRetValue+=j;
        }
        return nRetValue;
    }
    else return 0;
}

// ------------------------------------------------------------------------

sal_Bool XPMReader::ImplCompare( sal_uInt8* pSource, sal_uInt8* pDest, sal_uLong nSize, sal_uLong nMode )
{
    sal_Bool bRet = sal_True;

    if ( nMode == XPMCASENONSENSITIVE )
    {
        for ( sal_uLong i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
            {
                bRet = sal_False;
                break;
            }
        }
    }
    else
    {
        for ( sal_uLong i = 0; i < nSize; i++ )
        {
            if ( pSource[i] != pDest[i] )
            {
                bRet = sal_False;
                break;
            }
        }
    }
    return bRet;
}

// ------------------------------------------------------------------------
// ImplGetPara versucht den nNumb  ( 0...x ) Parameter aus mpStringBuf zu ermitteln.
// Ein Parameter ist durch Spaces oder Tabs von den anderen getrennt.
// Konnte der Parameter gefunden werden ist der Rueckgabewert sal_True und mpPara + mnParaSize
// werden gesetzt.

sal_Bool XPMReader::ImplGetPara ( sal_uLong nNumb )
{
    sal_uInt8   nByte;
    sal_uLong   pSize = 0;
    sal_uInt8*  pPtr = mpStringBuf;
    sal_uLong   nCount = 0;

    if ( ( *pPtr != ' ' ) && ( *pPtr != 0x09 ) )
    {
        mpPara = pPtr;
        mnParaSize = 0;
        nCount = 0;
    }
    else
    {
        mpPara = NULL;
        nCount = 0xffffffff;
    }

    while ( pSize < mnStringSize )
    {
        nByte = *pPtr;

        if ( mpPara )
        {
            if ( ( nByte == ' ' ) || ( nByte == 0x09 ) )
            {
                if ( nCount == nNumb )
                    break;
                else
                    mpPara = NULL;
            }
            else
                mnParaSize++;
        }
        else
        {
            if ( ( nByte != ' ' ) && ( nByte != 0x09 ) )
            {
                mpPara = pPtr;
                mnParaSize = 1;
                nCount++;
            }
        }
        pSize++;
        pPtr++;
    }
    return ( ( nCount == nNumb ) && ( mpPara ) ) ? sal_True : sal_False;
}

// ------------------------------------------------------------------------
// Der naechste String wird ausgelesen und in mpStringBuf (mit 0 abgeschlossen) abgelegt;
// mnStringSize enthaelt die Groesse des gelesenen Strings.
// Bemerkungen wie '//' und '/*.....*/' werden uebersprungen.

sal_Bool XPMReader::ImplGetString( void )
{
    sal_uInt8       sID[] = "/* XPM */";
    sal_uInt8*      pString = mpStringBuf;

    mnStringSize = 0;
    mpStringBuf[0] = 0;

    while( mbStatus && ( mnStatus != XPMFINISHED ) )
    {
        if ( mnTempAvail == 0 )
        {
            mnTempAvail = mrIStm.Read( mpTempBuf, XPMTEMPBUFSIZE );
            if ( mnTempAvail == 0 )
                break;

            mpTempPtr = mpTempBuf;

            if ( mnIdentifier == XPMIDENTIFIER )
            {
                if ( mnTempAvail <= 50 )
                {
                    mbStatus = sal_False;   // file is too short to be a correct XPM format
                    break;
                }
                for ( int i = 0; i < 9; i++ )   // searching for "/* XPM */"
                    if ( *mpTempPtr++ != sID[i] )
                    {
                        mbStatus = sal_False;
                        break;
                    }
                mnTempAvail-=9;
                mnIdentifier++;
            }
        }
        mcLastByte = mcThisByte;
        mcThisByte = *mpTempPtr++;
        mnTempAvail--;

        if ( mnStatus & XPMDOUBLE )
        {
            if ( mcThisByte == 0x0a )
                mnStatus &=~XPMDOUBLE;
            continue;
        }
        if ( mnStatus & XPMREMARK )
        {
            if ( ( mcThisByte == '/' )  && ( mcLastByte == '*' ) )
                mnStatus &=~XPMREMARK;
            continue;
        }
        if ( mnStatus & XPMSTRING )             // characters in string
        {
            if ( mcThisByte == '"' )
            {
                mnStatus &=~XPMSTRING;          // end of parameter by eol
                break;
            }
            if ( mnStringSize >= ( XPMSTRINGBUF - 1 ) )
            {
                mbStatus = sal_False;
                break;
            }
            *pString++ = mcThisByte;
            pString[0] = 0;
            mnStringSize++;
            continue;
        }
        else
        {                                           // characters beside string
            switch ( mcThisByte )
            {
                case '*' :
                    if ( mcLastByte == '/' ) mnStatus |= XPMREMARK;
                    break;
                case '/' :
                    if ( mcLastByte == '/' ) mnStatus |= XPMDOUBLE;
                    break;
                case '"' : mnStatus |= XPMSTRING;
                    break;
                case '{' :
                    if ( mnIdentifier == XPMDEFINITION )
                        mnIdentifier++;
                    break;
                case '}' :
                    if ( mnIdentifier == XPMENDEXT )
                        mnStatus = XPMFINISHED;
                    break;
            }
        }
    }
    return mbStatus;
}

// -------------
// - ImportXPM -
// -------------

sal_Bool ImportXPM( SvStream& rStm, Graphic& rGraphic )
{
    XPMReader*  pXPMReader = (XPMReader*) rGraphic.GetContext();
    ReadState   eReadState;
    sal_Bool        bRet = sal_True;

    if( !pXPMReader )
        pXPMReader = new XPMReader( rStm );

    rGraphic.SetContext( NULL );
    eReadState = pXPMReader->ReadXPM( rGraphic );

    if( eReadState == XPMREAD_ERROR )
    {
        bRet = sal_False;
        delete pXPMReader;
    }
    else if( eReadState == XPMREAD_OK )
        delete pXPMReader;
    else
        rGraphic.SetContext( pXPMReader );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
