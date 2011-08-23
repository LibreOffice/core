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


#define XBMMINREAD 512

#define _XBMPRIVATE
#include <ctype.h>
#include "xbmread.hxx"

// -------------
// - XBMReader -
// -------------

namespace binfilter
{

XBMReader::XBMReader( SvStream& rStm ) :
            rIStm			( rStm ),
            pAcc1			( NULL ),
            nLastPos		( rStm.Tell() ),
            nWidth			( 0 ),
            nHeight			( 0 ),
            bStatus			( TRUE )
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
    pHexTable[' '] =	 -1;
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
    ByteString	aRet;
    long		nPos1;
    long		nPos2;
    long		nPos3;

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
    long	nRet = 0;
    char*	pTmp = (char*) pDefine;
    int		cTmp;

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
    ByteString		aLine;
    long			nRow = 0;
    long			nCol = 0;
    long			nBits = ( eFormat == XBM10 ) ? 16 : 8;
    long			nBit;
    USHORT			nValue;
    USHORT			nDigits;
    BOOL			bFirstLine = TRUE;

    while( nRow < nHeight )
    {
        if( bFirstLine )
        {
            xub_StrLen nPos;

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
                const ByteString	aToken( aLine.GetToken( i, ',' ) );
                const xub_StrLen nLen = aToken.Len();
                BOOL				bProcessed = FALSE;

                nBit = nDigits = nValue = 0;

                for( xub_StrLen n = 0UL; n < nLen; n++ )
                {
                    const unsigned char cChar = aToken.GetChar( n );
                    const short			nTable = pHexTable[ cChar ];

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
    ReadState	eReadState;
    BYTE		cDummy;

    // sehen, ob wir _alles_ lesen koennen
    rIStm.Seek( STREAM_SEEK_TO_END );
    rIStm >> cDummy;

    // falls wir nicht alles lesen koennen
    // kehren wir zurueck und warten auf neue Daten
    if ( rIStm.GetError() != ERRCODE_IO_PENDING )
    {
        ByteString	aLine;
        int			nValue;

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
                        XBMFormat eFormat = XBM10;

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

BOOL ImportXBM( SvStream& rStm, Graphic& rGraphic )
{
    XBMReader*	pXBMReader = (XBMReader*) rGraphic.GetContext();
    ReadState	eReadState;
    BOOL		bRet = TRUE;

    if( !pXBMReader )
        pXBMReader = new XBMReader( rStm );

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
}
