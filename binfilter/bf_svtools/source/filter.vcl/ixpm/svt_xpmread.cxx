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


#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include "rgbtable.hxx"
#define _XPMPRIVATE
#include "xpmread.hxx"

namespace binfilter
{

// -------------
// - XPMReader -
// -------------

XPMReader::XPMReader( SvStream& rStm ) :
            mrIStm			( rStm ),
            mpAcc			( NULL ),
            mpMaskAcc		( NULL ),
            mnLastPos		( rStm.Tell() ),
            mnWidth			( 0 ),
            mnHeight		( 0 ),
            mnColors		( 0 ),
            mnCpp			( 0 ),
            mbTransparent	( FALSE ),
            mbStatus		( TRUE ),
            mnStatus		( 0 ),
            mnIdentifier	( XPMIDENTIFIER ),
            mcThisByte		( 0 ),
            mnTempAvail		( 0 ),
            mpFastColorTable( NULL ),
            mpColMap		( NULL )
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
    ReadState	eReadState;
    BYTE		cDummy;

    // sehen, ob wir _alles_ lesen koennen
    mrIStm.Seek( STREAM_SEEK_TO_END );
    mrIStm >> cDummy;

    // falls wir nicht alles lesen koennen
    // kehren wir zurueck und warten auf neue Daten
    if ( mrIStm.GetError() != ERRCODE_IO_PENDING )
    {
        mrIStm.Seek( mnLastPos );
        mbStatus = TRUE;

        if ( mbStatus )
        {
            mpStringBuf = new BYTE [ XPMSTRINGBUF ];
            mpTempBuf = new BYTE [ XPMTEMPBUFSIZE ];

            if ( ( mbStatus = ImplGetString() ) == TRUE )
            {
                mnIdentifier = XPMVALUES;			// Bitmap informationen einholen
                mnWidth = ImplGetULONG( 0 );
                mnHeight = ImplGetULONG( 1 );
                mnColors = ImplGetULONG( 2 );
                mnCpp = ImplGetULONG( 3 );
            }
            if ( mbStatus && mnWidth && mnHeight && mnColors && mnCpp )
            {
                mnIdentifier = XPMCOLORS;

                // mpColMap beinhaltet fuer jede vorhandene
                // Farbe:	( mnCpp )Byte(s)-> ASCII Eintrag der der Farbe zugeordnet ist
                //			    1    Byte	-> 0xff wenn Farbe transparent ist
                //				3    Bytes  -> RGB Wert der Farbe
                mpColMap = new BYTE[ mnColors * ( 4 + mnCpp ) ];

                for ( ULONG i = 0; i < mnColors; i++ )
                {
                    if ( ImplGetColor( i ) == FALSE )
                    {
                        mbStatus = FALSE;
                        break;
                    }
                }
                if ( mbStatus )
                {
                    // bei mehr als 256 Farben wird eine 24 Bit Grafik erstellt
                    sal_uInt16  nBits = 1;
                    if ( mnColors > 256 )
                        nBits = 24;
                    else if ( mnColors > 16 )
                        nBits = 8;
                    else if ( mnColors > 4 )
                        nBits = 4;
                    else if ( mnColors > 2 )
                        nBits = 2;
                    else
                        nBits = 1;

                    maBmp = Bitmap( Size( mnWidth, mnHeight ), nBits );
                    mpAcc = maBmp.AcquireWriteAccess();

                    // mbTransparent ist TRUE wenn mindestens eine Farbe Transparent ist
                    if ( mbTransparent )
                    {
                        maMaskBmp = Bitmap( Size( mnWidth, mnHeight ), 1 );
                        if ( ( mpMaskAcc = maMaskBmp.AcquireWriteAccess() ) == NULL )
                            mbStatus =	FALSE;
                    }
                    if( mpAcc && mbStatus )
                    {
                        ULONG	i;
                        if ( mnColors <=256 )	// palette is only needed by using less than 257
                        {						// colors

                            BYTE*	pPtr = &mpColMap[mnCpp];

                            for ( i = 0; i < mnColors; i++ )
                            {
                                mpAcc->SetPaletteColor( (BYTE)i, Color( pPtr[1], pPtr[2], pPtr[3] ) );
                                pPtr += ( mnCpp + 4 );
                            }
                            // using 2 charakters per pixel and less than 257 Colors we speed up
                            if ( mnCpp == 2 )	// by using a 64kb indexing table
                            {
                                mpFastColorTable = new BYTE[ 256 * 256 ];
                                for ( pPtr = mpColMap, i = 0; i < mnColors; i++, pPtr += mnCpp + 4 )
                                {
                                    ULONG	j =  pPtr[ 0 ] << 8;
                                            j += pPtr[ 1 ];
                                    mpFastColorTable[ j ] = (BYTE)i;
                                }
                            }
                        }
                        // now we get the bitmap data
                        mnIdentifier = XPMPIXELS;
                        for ( i = 0; i < mnHeight; i++ )
                        {
                            if ( ImplGetScanLine( i ) == FALSE )
                            {
                                mbStatus = FALSE;
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
// die Rueckgabe ist TRUE wenn saemtliche Farben zugeordnet werden konnten

BOOL XPMReader::ImplGetColor( ULONG nNumb )
{
    BYTE*	pString = mpStringBuf;
    BYTE*	pPtr =  ( mpColMap + nNumb * ( 4 + mnCpp ) );
    BOOL	bStatus = ImplGetString();

    if ( bStatus )
    {
        for ( ULONG i = 0; i < mnCpp; i++ )
            *pPtr++ = *pString++;
        bStatus = ImplGetColSub ( pPtr );
    }
    return bStatus;
}

// ------------------------------------------------------------------------
// ImpGetScanLine liest den String mpBufSize aus und schreibt die Pixel in die
// Bitmap. Der Parameter nY gibt die horizontale Position an.

BOOL XPMReader::ImplGetScanLine( ULONG nY )
{
    BOOL	bStatus = ImplGetString();
    BYTE*	pString = mpStringBuf;
    BYTE*	pColor;
    BitmapColor		aWhite;
    BitmapColor		aBlack;

    if ( bStatus )
    {
        if ( mpMaskAcc )
        {
            aWhite = mpMaskAcc->GetBestMatchingColor( Color( COL_WHITE ) );
            aBlack = mpMaskAcc->GetBestMatchingColor( Color( COL_BLACK ) );
        }
        if ( mnStringSize != ( mnWidth * mnCpp ))
            bStatus = FALSE;
        else
        {
            ULONG i, j;
            if ( mpFastColorTable )
            {
                for ( i = 0; i < mnWidth; i++ )
                {
                    j = (*pString++) << 8;
                    j += *pString++;
                    BYTE k = (BYTE)mpFastColorTable[ j ];
                    mpAcc->SetPixel( nY, i, BitmapColor( (BYTE)k ) );

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
                    if ( ImplCompare( pString, pColor, mnCpp, XPMCASESENSITIVE ) == TRUE )
                    {
                        if ( mnColors > 256 )
                            mpAcc->SetPixel( nY, i, Color ( pColor[3], pColor[4], pColor[5] ) );
                        else
                            mpAcc->SetPixel( nY, i, BitmapColor( (BYTE) j ) );

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

BOOL XPMReader::ImplGetColSub( BYTE* pDest )
{
    unsigned char cTransparent[] = "None";

    BOOL bColStatus = FALSE;

    if ( ImplGetColKey( 'c' ) || ImplGetColKey( 'm' ) || ImplGetColKey( 'g' ) )
    {
        // hexentry for RGB or HSV color ?
        if ( *mpPara == '#' )
        {
                *pDest++ = 0;
                bColStatus = TRUE;
                switch ( mnParaSize	)
                {
                    case 25	:
                        ImplGetRGBHex ( pDest, 6 );
                        break;
                    case 13 :
                        ImplGetRGBHex ( pDest, 2 );
                        break;
                    case  7 :
                        ImplGetRGBHex ( pDest, 0 );
                        break;
                    default:
                        bColStatus = FALSE;
                        break;
                }
        }
        // maybe pixel is transparent
        else if ( ImplCompare( &cTransparent[0], mpPara, 4 ))
        {
            *pDest++ = 0xff;
            bColStatus = TRUE;
            mbTransparent = TRUE;
        }
        // last we will try to get the colorname
        else if ( mnParaSize > 2 )	// name must enlarge the minimum size
        {
            ULONG i = 0;
            while ( TRUE )
            {
                if ( pRGBTable[ i ].name == NULL )
                    break;
                if ( pRGBTable[ i ].name[ mnParaSize ] == 0 )
                {
                    if ( ImplCompare ( (unsigned char*)pRGBTable[ i ].name,
                            mpPara, mnParaSize, XPMCASENONSENSITIVE ) )
                    {
                        bColStatus = TRUE;
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
// und gibt einen BOOL zurueck. ( wenn TRUE werden mpPara und mnParaSize gesetzt )

BOOL XPMReader::ImplGetColKey( BYTE nKey )
{
    BYTE nTemp, nPrev = ' ';

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
    return ( mnParaSize ) ? TRUE : FALSE;
}

// ------------------------------------------------------------------------
// ImplGetRGBHex uebersetzt den ASCII-Hexadezimalwert der sich bei mpPara befindet
// in einen RGB wert und schreibt diesen nach pDest
// folgende Formate muessen sich bei mpPara befinden:
// wenn nAdd = 0 : '#12ab12'					-> RGB = 0x12, 0xab, 0x12
//			   2 : '#1234abcd1234'					"	   "     "	   "
//			   6 : '#12345678abcdefab12345678'		"	   "	 "	   "


void XPMReader::ImplGetRGBHex( BYTE* pDest,ULONG  nAdd )
{
    BYTE*	pPtr = mpPara+1;
    BYTE 	nHex, nTemp;

    for ( ULONG i = 0; i < 3; i++ )
    {
        nHex = (*pPtr++) - '0';
        if ( nHex > 9 )
            nHex = ((nHex - 'A' + '0') & 7) + 10;

        nTemp = (*pPtr++) - '0';
        if ( nTemp > 9 )
            nTemp = ((nTemp - 'A' + '0') & 7) + 10;
        nHex = ( nHex << 4 ) + nTemp;

        pPtr += nAdd;
        *pDest++ = (BYTE)nHex;
    }
}

// ------------------------------------------------------------------------
// ImplGetUlong gibt den wert einer bis zu 6stelligen ASCII-Dezimalzahl zurueck.

ULONG XPMReader::ImplGetULONG( ULONG nPara )
{
    if ( ImplGetPara ( nPara ) )
    {
        ULONG nRetValue = 0;
        BYTE* pPtr = mpPara;

        if ( ( mnParaSize > 6 ) || ( mnParaSize == 0 ) ) return 0;
        for ( ULONG i = 0; i < mnParaSize; i++ )
        {
            BYTE j = (*pPtr++) - 48;
            if ( j > 9 ) return 0;				// ascii is invalid
            nRetValue*=10;
            nRetValue+=j;
        }
        return nRetValue;
    }
    else return 0;
}

// ------------------------------------------------------------------------

BOOL XPMReader::ImplCompare( BYTE* pSource, BYTE* pDest, ULONG nSize, ULONG nMode )
{
    BOOL bRet = TRUE;

    if ( nMode == XPMCASENONSENSITIVE )
    {
        for ( ULONG i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
            {
                bRet = FALSE;
                break;
            }
        }
    }
    else
    {
        for ( ULONG i = 0; i < nSize; i++ )
        {
            if ( pSource[i] != pDest[i] )
            {
                bRet = FALSE;
                break;
            }
        }
    }
    return bRet;
}

// ------------------------------------------------------------------------
// ImplGetPara versucht den nNumb  ( 0...x ) Parameter aus mpStringBuf zu ermitteln.
// Ein Parameter ist durch Spaces oder Tabs von den anderen getrennt.
// Konnte der Parameter gefunden werden ist der Rueckgabewert TRUE und mpPara + mnParaSize
// werden gesetzt.

BOOL XPMReader::ImplGetPara ( ULONG nNumb )
{
    BYTE 	nByte;
    ULONG	pSize = 0;
    BYTE*	pPtr = mpStringBuf;
    ULONG	nCount = 0;

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
    return ( ( nCount == nNumb ) && ( mpPara ) ) ? TRUE : FALSE;
}

// ------------------------------------------------------------------------
// Der naechste String wird ausgelesen und in mpStringBuf (mit 0 abgeschlossen) abgelegt;
// mnStringSize enthaelt die Groesse des gelesenen Strings.
// Bemerkungen wie '//' und '/*.....*/' werden uebersprungen.

BOOL XPMReader::ImplGetString( void )
{
    BYTE		sID[] = "/* XPM */";
    BYTE*		pString = mpStringBuf;

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
                    mbStatus = FALSE;	// file is too short to be a correct XPM format
                    break;
                }
                for ( int i = 0; i < 9; i++ )	// searching for "/* XPM */"
                    if ( *mpTempPtr++ != sID[i] )
                    {
                        mbStatus = FALSE;
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
        if ( mnStatus & XPMSTRING )				// characters in string
        {
            if ( mcThisByte == '"' )
            {
                mnStatus &=~XPMSTRING;			// end of parameter by eol
                break;
            }
            if ( mnStringSize >= XPMSTRINGBUF )
            {
                mbStatus = FALSE;
                break;
            }
            *pString++ = mcThisByte;
            pString[0] = 0;
            mnStringSize++;
            continue;
        }
        else
        {											// characters beside string
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

BOOL ImportXPM( SvStream& rStm, Graphic& rGraphic )
{
    XPMReader*	pXPMReader = (XPMReader*) rGraphic.GetContext();
    ReadState	eReadState;
    BOOL		bRet = TRUE;

    if( !pXPMReader )
        pXPMReader = new XPMReader( rStm );

    rGraphic.SetContext( NULL );
    eReadState = pXPMReader->ReadXPM( rGraphic );

    if( eReadState == XPMREAD_ERROR )
    {
        bRet = FALSE;
        delete pXPMReader;
    }
    else if( eReadState == XPMREAD_OK )
        delete pXPMReader;
    else
        rGraphic.SetContext( pXPMReader );

    return bRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
