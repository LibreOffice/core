/*************************************************************************
 *
 *  $RCSfile: xbmread.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:59 $
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

#define XBMMINREAD 512

#define _XBMPRIVATE
#include <ctype.h>
#include "xbmread.hxx"

// -------------
// - XBMReader -
// -------------

XBMReader::XBMReader( SvStream& rStm, void* pCallData ) :
            rIStm           ( rStm ),
            nWidth          ( 0 ),
            nHeight         ( 0 ),
            nLastPos        ( rStm.Tell() ),
            pAcc1           ( NULL ),
            bStatus         ( TRUE )
{
    pHexTable = new short[ 256 ];
    maUpperName = String::CreateFromAscii( "SVIXBM", 6 );
    InitTable();
}

// ------------------------------------------------------------------------

XBMReader::~XBMReader()
{
    delete[] pHexTable;

    if( pAcc1 )
        aBmp1.ReleaseAccess( pAcc1 );
}

// ------------------------------------------------------------------------

void XBMReader::InitTable()
{
    memset( pHexTable, 0, sizeof( short ) );

    pHexTable['0'] = 0;
    pHexTable['1'] = 1;
    pHexTable['2'] = 2;
    pHexTable['3'] = 3;
    pHexTable['4'] = 4;
    pHexTable['5'] = 5;
    pHexTable['6'] = 6;
    pHexTable['7'] = 7;
    pHexTable['8'] = 8;
    pHexTable['9'] = 9;
    pHexTable['A'] = 10;
    pHexTable['B'] = 11;
    pHexTable['C'] = 12;
    pHexTable['D'] = 13;
    pHexTable['E'] = 14;
    pHexTable['F'] = 15;
    pHexTable['X'] = 0;
    pHexTable['a'] = 10;
    pHexTable['b'] = 11;
    pHexTable['c'] = 12;
    pHexTable['d'] = 13;
    pHexTable['e'] = 14;
    pHexTable['f'] = 15;
    pHexTable['x'] = 0;
    pHexTable[' '] =     -1;
    pHexTable[','] = -1;
    pHexTable['}'] = -1;
    pHexTable['\n'] = -1;
    pHexTable['\t'] = -1;
    pHexTable['\0'] = -1;
}

// ------------------------------------------------------------------------

ByteString XBMReader::FindTokenLine( SvStream* pInStm, const char* pTok1,
                                 const char* pTok2, const char* pTok3 )
{
    ByteString  aRet;
    long        nPos1;
    long        nPos2;
    long        nPos3;

    bStatus = FALSE;

    do
    {
        if( !pInStm->ReadLine( aRet ) )
            break;

        if( pTok1 )
        {
            if( ( nPos1 = aRet.Search( pTok1 ) ) != STRING_NOTFOUND )
            {
                bStatus = TRUE;

                if( pTok2 )
                {
                    bStatus = FALSE;

                    if( ( ( nPos2 = aRet.Search( pTok2 ) ) != STRING_NOTFOUND ) &&
                         ( nPos2 > nPos1 ) )
                    {
                        bStatus = TRUE;

                        if( pTok3 )
                        {
                            bStatus = FALSE;

                            if( ( ( nPos3 = aRet.Search( pTok3 ) ) != STRING_NOTFOUND ) && ( nPos3 > nPos2 ) )
                                bStatus = TRUE;
                        }
                    }
                }
            }
        }
    }
    while( !bStatus );

    return aRet;
}

// ------------------------------------------------------------------------

long XBMReader::ParseDefine( const sal_Char* pDefine )
{
    long    nRet = 0;
    char*   pTmp = (char*) pDefine;
    char    cTmp;

    // bis zum Ende gehen
    pTmp += ( strlen( pDefine ) - 1 );
    cTmp = *pTmp--;

    // letzte Ziffer suchen
    while( pHexTable[ cTmp ] == -1 )
        cTmp = *pTmp--;

    // bis vor die Zahl laufen
    while( pHexTable[ cTmp ] != -1 )
        cTmp = *pTmp--;

    // auf Anfang der Zahl gehen
    pTmp += 2;

    // Hex lesen
    if( ( pTmp[0] == '0' ) && ( ( pTmp[1] == 'X' ) || ( pTmp[1] == 'x' ) ) )
    {
        pTmp += 2;
        cTmp = *pTmp++;

        while ( pHexTable[ cTmp ] != -1 )
        {
            nRet = ( nRet << 4 ) + pHexTable[ cTmp ];
            cTmp = *pTmp++;
        }
    }
    // Dezimal lesen
    else
    {
        cTmp = *pTmp++;
        while( ( cTmp >= '0' ) && ( cTmp <= '9' ) )
        {
            nRet = nRet * 10 + ( cTmp - '0' );
            cTmp = *pTmp++;
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------

BOOL XBMReader::ParseData( SvStream* pInStm, const ByteString& aLastLine, XBMFormat eFormat )
{
    ByteString      aLine;
    long            nRow = 0;
    long            nCol = 0;
    long            nBits = ( eFormat == XBM10 ) ? 16 : 8;
    long            nBit;
    USHORT          nValue;
    USHORT          nDigits;
    BOOL            bFirstLine = TRUE;

    while( nRow < nHeight )
    {
        if( bFirstLine )
        {
            long nPos;

            // einfuehrende geschweifte Klammer loeschen
            if( (nPos = ( aLine = aLastLine ).Search( '{' ) ) != STRING_NOTFOUND )
                aLine.Erase( 0, nPos + 1 );

            bFirstLine = FALSE;
        }
        else if( !pInStm->ReadLine( aLine ) )
            break;

        if( aLine.Len() )
        {
            const USHORT nCount = aLine.GetTokenCount( ',' );

            for( USHORT i = 0; ( i < nCount ) && ( nRow < nHeight ); i++ )
            {
                const ByteString    aToken( aLine.GetToken( i, ',' ) );
                const ULONG         nLen = aToken.Len();
                BOOL                bProcessed = FALSE;

                nBit = nDigits = nValue = 0;

                for( ULONG n = 0UL; n < nLen; n++ )
                {
                    const unsigned char cChar = aToken.GetChar( n );
                    const short         nTable = pHexTable[ cChar ];

                    if( isxdigit( cChar ) || !nTable )
                    {
                        nValue = ( nValue << 4 ) + nTable;
                        nDigits++;
                        bProcessed = TRUE;
                    }
                    else if( ( nTable < 0 ) && nDigits )
                    {
                        bProcessed = TRUE;
                        break;
                    }
                }

                if( bProcessed )
                {
                    while( ( nCol < nWidth ) && ( nBit < nBits ) )
                        pAcc1->SetPixel( nRow, nCol++, ( nValue & ( 1 << nBit++ ) ) ? aBlack : aWhite );

                    if( nCol == nWidth )
                        nCol = 0, nRow++;
                }
            }
        }
    }

    return TRUE;
}

// ------------------------------------------------------------------------

ReadState XBMReader::ReadXBM( Graphic& rGraphic )
{
    ReadState   eReadState;
    BYTE        cDummy;

    // sehen, ob wir _alles_ lesen koennen
    rIStm.Seek( STREAM_SEEK_TO_END );
    rIStm >> cDummy;

    // falls wir nicht alles lesen koennen
    // kehren wir zurueck und warten auf neue Daten
    if ( rIStm.GetError() != ERRCODE_IO_PENDING )
    {
        ByteString  aLine;
        int         nValue;

        rIStm.Seek( nLastPos );
        bStatus = FALSE;
        aLine = FindTokenLine( &rIStm, "#define", "_width" );

        if ( bStatus )
        {
            if ( ( nValue = (int) ParseDefine( aLine.GetBuffer() ) ) > 0 )
            {
                nWidth = nValue;
                aLine = FindTokenLine( &rIStm, "#define", "_height" );

                // Falls die Hoehe nicht folgt, suchen wir noch
                // einmal vom Anfang der Datei an
                if ( !bStatus )
                {
                    rIStm.Seek( nLastPos );
                    aLine = FindTokenLine( &rIStm, "#define", "_height" );
                }
            }
            else
                bStatus = FALSE;

            if ( bStatus )
            {
                if ( ( nValue = (int) ParseDefine( aLine.GetBuffer() ) ) > 0 )
                {
                    nHeight = nValue;
                    aLine = FindTokenLine( &rIStm, "static", "_bits" );

                    if ( bStatus )
                    {
                        XBMFormat eFormat;

                        if ( aLine.Search( "short" ) != STRING_NOTFOUND )
                            eFormat = XBM10;
                        else if ( aLine.Search( "char" ) != STRING_NOTFOUND )
                            eFormat = XBM11;
                        else
                            bStatus = FALSE;

                        if ( bStatus && nWidth && nHeight )
                        {
                            aBmp1 = Bitmap( Size( nWidth, nHeight ), 1 );
                            pAcc1 = aBmp1.AcquireWriteAccess();

                            if( pAcc1 )
                            {
                                aWhite = pAcc1->GetBestMatchingColor( Color( COL_WHITE ) );
                                aBlack = pAcc1->GetBestMatchingColor( Color( COL_BLACK ) );
                                bStatus = ParseData( &rIStm, aLine, eFormat );
                            }
                            else
                                bStatus = FALSE;
                        }
                    }
                }
            }
        }

        if( bStatus )
        {
            Bitmap aBlackBmp( Size( pAcc1->Width(), pAcc1->Height() ), 1 );

            aBmp1.ReleaseAccess( pAcc1 ), pAcc1 = NULL;
            aBlackBmp.Erase( Color( COL_BLACK ) );
            rGraphic = BitmapEx( aBlackBmp, aBmp1 );
            eReadState = XBMREAD_OK;
        }
        else
            eReadState = XBMREAD_ERROR;
    }
    else
    {
        rIStm.ResetError();
        eReadState = XBMREAD_NEED_MORE;
    }

    return eReadState;
}

// -------------
// - ImportXBM -
// -------------

BOOL ImportXBM( SvStream& rStm, Graphic& rGraphic, void* pCallerData )
{
    XBMReader*  pXBMReader = (XBMReader*) rGraphic.GetContext();
    ReadState   eReadState;
    BOOL        bRet = TRUE;

    if( !pXBMReader )
        pXBMReader = new XBMReader( rStm, pCallerData );

    rGraphic.SetContext( NULL );
    eReadState = pXBMReader->ReadXBM( rGraphic );

    if( eReadState == XBMREAD_ERROR )
    {
        bRet = FALSE;
        delete pXBMReader;
    }
    else if( eReadState == XBMREAD_OK )
        delete pXBMReader;
    else
        rGraphic.SetContext( pXBMReader );

    return bRet;
}
