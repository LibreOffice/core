/*************************************************************************
 *
 *  $RCSfile: xpmread.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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

#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#include "rgbtable.hxx"
#define _XPMPRIVATE
#include "xpmread.hxx"

// -------------
// - XPMReader -
// -------------

XPMReader::XPMReader( SvStream& rStm, void* pCallData ) :
            mrIStm          ( rStm ),
            mnIdentifier    ( XPMIDENTIFIER ),
            mnTempAvail     ( 0 ),
            mnStatus        ( 0 ),
            mbTransparent   ( FALSE ),
            mnCpp           ( 0 ),
            mnWidth         ( 0 ),
            mnHeight        ( 0 ),
            mnColors        ( 0 ),
            mcThisByte      ( 0 ),
            mnLastPos       ( rStm.Tell() ),
            mpFastColorTable( NULL ),
            mpColMap        ( NULL ),
            mpAcc           ( NULL ),
            mpMaskAcc       ( NULL ),
            mbStatus        ( TRUE )
{

}

// ------------------------------------------------------------------------

XPMReader::~XPMReader()
{
    if( mpAcc )
        maBmp.ReleaseAccess( mpAcc );
}

// ------------------------------------------------------------------------

#ifdef WNT
#pragma optimize ("",off)
#endif

ReadState XPMReader::ReadXPM( Graphic& rGraphic )
{
    ReadState   eReadState;
    BYTE        cDummy;

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
                mnIdentifier = XPMVALUES;           // Bitmap informationen einholen
                mnWidth = ImplGetULONG( 0 );
                mnHeight = ImplGetULONG( 1 );
                mnColors = ImplGetULONG( 2 );
                mnCpp = ImplGetULONG( 3 );
            }
            if ( mbStatus && mnWidth && mnHeight && mnColors && mnCpp )
            {
                mnIdentifier = XPMCOLORS;

                // mpColMap beinhaltet für jede vorhandene
                // Farbe:   ( mnCpp )Byte(s)-> ASCII Eintrag der der Farbe zugeordnet ist
                //              1    Byte   -> 0xff wenn Farbe transparent ist
                //              3    Bytes  -> RGB Wert der Farbe
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
                    maBmp = Bitmap( Size( mnWidth, mnHeight ), ( mnColors > 256 ) ? 24 : 8 );
                    mpAcc = maBmp.AcquireWriteAccess();

                    // mbTransparent ist TRUE wenn mindestens eine Farbe Transparent ist
                    if ( mbTransparent )
                    {
                        maMaskBmp = Bitmap( Size( mnWidth, mnHeight ), 1 );
                        if ( ( mpMaskAcc = maMaskBmp.AcquireWriteAccess() ) == NULL )
                            mbStatus =  FALSE;
                    }
                    if( mpAcc && mbStatus )
                    {
                        ULONG   i;
                        if ( mnColors <=256 )   // palette is only needed by using less than 257
                        {                       // colors

                            BYTE*   pPtr = &mpColMap[mnCpp];

                            for ( i = 0; i < mnColors; i++ )
                            {
                                mpAcc->SetPaletteColor( (BYTE)i, Color( pPtr[1], pPtr[2], pPtr[3] ) );
                                pPtr += ( mnCpp + 4 );
                            }
                            // using 2 charakters per pixel and less than 257 Colors we speed up
                            if ( mnCpp == 2 )   // by using a 64kb indexing table
                            {
                                mpFastColorTable = new BYTE[ 256 * 256 ];
                                for ( pPtr = mpColMap, i = 0; i < mnColors; i++, pPtr += mnCpp + 4 )
                                {
                                    ULONG   j =  pPtr[ 0 ] << 8;
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

#ifdef WNT
#pragma optimize ("",on)
#endif

// ------------------------------------------------------------------------
// ImplGetColor ermittelt sämtliche Farbwerte,
// die Rückgabe ist TRUE wenn sämtliche Farben zugeordnet werden konnten

BOOL XPMReader::ImplGetColor( ULONG nNumb )
{
    BYTE*   pString = mpStringBuf;
    BYTE*   pPtr =  ( mpColMap + nNumb * ( 4 + mnCpp ) );
    BOOL    bStatus = ImplGetString();

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
    BOOL    bStatus = ImplGetString();
    BYTE*   pString = mpStringBuf;
    BYTE*   pColor;
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
// versucht aus mpStringBuf einen Farbwert zu übermitteln
// wurde eine Farbe gefunden wird an pDest[1]..pDest[2] der RGB wert geschrieben
// pDest[0] enthält 0xff wenn die Farbe transparent ist sonst 0

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
        else if ( mnParaSize > 2 )  // name must enlarge the minimum size
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
// und gibt einen BOOL zurück. ( wenn TRUE werden mpPara und mnParaSize gesetzt )

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
        *mpPara++;
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
// ImplGetRGBHex übersetzt den ASCII-Hexadezimalwert der sich bei mpPara befindet
// in einen RGB wert und schreibt diesen nach pDest
// folgende Formate müssen sich bei mpPara befinden:
// wenn nAdd = 0 : '#12ab12'                    -> RGB = 0x12, 0xab, 0x12
//             2 : '#1234abcd1234'                  "      "     "     "
//             6 : '#12345678abcdefab12345678'      "      "     "     "


void XPMReader::ImplGetRGBHex( BYTE* pDest,ULONG  nAdd )
{
    BYTE*   pPtr = mpPara+1;
    BYTE    nHex, nTemp;

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
// ImplGetUlong gibt den wert einer bis zu 6stelligen ASCII-Dezimalzahl zurück.

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
            if ( j > 9 ) return 0;              // ascii is invalid
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
// Konnte der Parameter gefunden werden ist der Rückgabewert TRUE und mpPara + mnParaSize
// werden gesetzt.

BOOL XPMReader::ImplGetPara ( ULONG nNumb )
{
    BYTE    nByte;
    ULONG   pSize = 0;
    BYTE*   pPtr = mpStringBuf;
    ULONG   nCount = 0;

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
// Der nächste String wird ausgelesen und in mpStringBuf (mit 0 abgeschloßen) abgelegt;
// mnStringSize enthält die Größe des gelesenen Strings.
// Bemerkungen wie '//' und '/*.....*/' werden übersprungen.

BOOL XPMReader::ImplGetString( void )
{
    BYTE        sID[] = "/* XPM */";
    BYTE*       pString = mpStringBuf;

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
                    mbStatus = FALSE;   // file is too short to be a correct XPM format
                    break;
                }
                for ( int i = 0; i < 9; i++ )   // searching for "/* XPM */"
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
        if ( mnStatus & XPMSTRING )             // characters in string
        {
            if ( mcThisByte == '"' )
            {
                mnStatus &=~XPMSTRING;          // end of parameter by eol
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

BOOL ImportXPM( SvStream& rStm, Graphic& rGraphic, void* pCallerData )
{
    XPMReader*  pXPMReader = (XPMReader*) rGraphic.GetContext();
    ReadState   eReadState;
    BOOL        bRet = TRUE;

    if( !pXPMReader )
        pXPMReader = new XPMReader( rStm, pCallerData );

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
