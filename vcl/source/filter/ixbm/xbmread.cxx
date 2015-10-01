/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#define _XBMPRIVATE
#include <ctype.h>
#include <comphelper/string.hxx>
#include "xbmread.hxx"

XBMReader::XBMReader( SvStream& rStm ) :
            rIStm           ( rStm ),
            pAcc1           ( NULL ),
            nLastPos        ( rStm.Tell() ),
            nWidth          ( 0 ),
            nHeight         ( 0 ),
            bStatus         ( true )
{
    pHexTable = new short[ 256 ];
    maUpperName = "SVIXBM";
    InitTable();
}

XBMReader::~XBMReader()
{
    delete[] pHexTable;

    if( pAcc1 )
        Bitmap::ReleaseAccess( pAcc1 );
}

void XBMReader::InitTable()
{
    memset( pHexTable, 0, sizeof( short ) * 256 );

    pHexTable[(int)'0'] = 0;
    pHexTable[(int)'1'] = 1;
    pHexTable[(int)'2'] = 2;
    pHexTable[(int)'3'] = 3;
    pHexTable[(int)'4'] = 4;
    pHexTable[(int)'5'] = 5;
    pHexTable[(int)'6'] = 6;
    pHexTable[(int)'7'] = 7;
    pHexTable[(int)'8'] = 8;
    pHexTable[(int)'9'] = 9;
    pHexTable[(int)'A'] = 10;
    pHexTable[(int)'B'] = 11;
    pHexTable[(int)'C'] = 12;
    pHexTable[(int)'D'] = 13;
    pHexTable[(int)'E'] = 14;
    pHexTable[(int)'F'] = 15;
    pHexTable[(int)'X'] = 0;
    pHexTable[(int)'a'] = 10;
    pHexTable[(int)'b'] = 11;
    pHexTable[(int)'c'] = 12;
    pHexTable[(int)'d'] = 13;
    pHexTable[(int)'e'] = 14;
    pHexTable[(int)'f'] = 15;
    pHexTable[(int)'x'] = 0;
    pHexTable[(int)' '] = -1;
    pHexTable[(int)','] = -1;
    pHexTable[(int)'}'] = -1;
    pHexTable[(int)'\n'] = -1;
    pHexTable[(int)'\t'] = -1;
    pHexTable[(int)'\0'] = -1;
}

OString XBMReader::FindTokenLine( SvStream* pInStm, const char* pTok1,
                                 const char* pTok2, const char* pTok3 )
{
    OString aRet;
    sal_Int32 nPos1, nPos2, nPos3;

    bStatus = false;

    do
    {
        if( !pInStm->ReadLine( aRet ) )
            break;

        if( pTok1 )
        {
            if( ( nPos1 = aRet.indexOf( pTok1 ) ) != -1 )
            {
                bStatus = true;

                if( pTok2 )
                {
                    bStatus = false;

                    if( ( ( nPos2 = aRet.indexOf( pTok2 ) ) != -1 ) &&
                         ( nPos2 > nPos1 ) )
                    {
                        bStatus = true;

                        if( pTok3 )
                        {
                            bStatus = false;

                            if( ( ( nPos3 = aRet.indexOf( pTok3 ) ) != -1 ) && ( nPos3 > nPos2 ) )
                                bStatus = true;
                        }
                    }
                }
            }
        }
    }
    while( !bStatus );

    return aRet;
}

long XBMReader::ParseDefine( const sal_Char* pDefine )
{
    long    nRet = 0;
    char*   pTmp = const_cast<char*>(pDefine);
    unsigned char   cTmp;

    // move to end
    pTmp += ( strlen( pDefine ) - 1 );
    cTmp = *pTmp--;

    // search last digit
    while( pHexTable[ cTmp ] == -1 )
        cTmp = *pTmp--;

    // move before number
    while( pHexTable[ cTmp ] != -1 )
        cTmp = *pTmp--;

    // move to start of number
    pTmp += 2;

    // read Hex
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
    // read decimal
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

bool XBMReader::ParseData( SvStream* pInStm, const OString& aLastLine, XBMFormat eFormat )
{
    OString    aLine;
    long            nRow = 0;
    long            nCol = 0;
    long            nBits = ( eFormat == XBM10 ) ? 16 : 8;
    long            nBit;
    sal_uInt16          nValue;
    sal_uInt16          nDigits;
    bool            bFirstLine = true;

    while( nRow < nHeight )
    {
        if( bFirstLine )
        {
            sal_Int32 nPos;

            // delete opening curly bracket
            if( (nPos = ( aLine = aLastLine ).indexOf('{') ) != -1 )
                aLine = aLine.copy(nPos + 1);

            bFirstLine = false;
        }
        else if( !pInStm->ReadLine( aLine ) )
            break;

        if (!aLine.isEmpty())
        {
            const sal_Int32 nCount = comphelper::string::getTokenCount(aLine, ',');

            for( sal_Int32 i = 0; ( i < nCount ) && ( nRow < nHeight ); ++i )
            {
                const OString aToken(aLine.getToken(i, ','));
                const sal_Int32 nLen = aToken.getLength();
                bool bProcessed = false;

                nBit = nDigits = nValue = 0;

                for (sal_Int32 n = 0; n < nLen; ++n)
                {
                    const unsigned char cChar = aToken[n];
                    const short         nTable = pHexTable[ cChar ];

                    if( isxdigit( cChar ) || !nTable )
                    {
                        nValue = ( nValue << 4 ) + nTable;
                        nDigits++;
                        bProcessed = true;
                    }
                    else if( ( nTable < 0 ) && nDigits )
                    {
                        bProcessed = true;
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

    return true;
}

ReadState XBMReader::ReadXBM( Graphic& rGraphic )
{
    ReadState   eReadState;
    sal_uInt8       cDummy;

    // check if we can read ALL
    rIStm.Seek( STREAM_SEEK_TO_END );
    rIStm.ReadUChar( cDummy );

    // if we cannot read all
    // we return and wait for new data
    if ( rIStm.GetError() != ERRCODE_IO_PENDING )
    {
        rIStm.Seek( nLastPos );
        bStatus = false;
        OString aLine = FindTokenLine( &rIStm, "#define", "_width" );

        if ( bStatus )
        {
            int nValue;
            if ( ( nValue = (int) ParseDefine( aLine.getStr() ) ) > 0 )
            {
                nWidth = nValue;
                aLine = FindTokenLine( &rIStm, "#define", "_height" );

                // if height was not received, we search again
                // from start of the file
                if ( !bStatus )
                {
                    rIStm.Seek( nLastPos );
                    aLine = FindTokenLine( &rIStm, "#define", "_height" );
                }
            }
            else
                bStatus = false;

            if ( bStatus )
            {
                if ( ( nValue = (int) ParseDefine( aLine.getStr() ) ) > 0 )
                {
                    nHeight = nValue;
                    aLine = FindTokenLine( &rIStm, "static", "_bits" );

                    if ( bStatus )
                    {
                        XBMFormat eFormat = XBM10;

                        if (aLine.indexOf("short") != -1)
                            eFormat = XBM10;
                        else if (aLine.indexOf("char") != -1)
                            eFormat = XBM11;
                        else
                            bStatus = false;

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
                                bStatus = false;
                        }
                    }
                }
            }
        }

        if (bStatus && pAcc1)
        {
            Bitmap aBlackBmp( Size( pAcc1->Width(), pAcc1->Height() ), 1 );

            Bitmap::ReleaseAccess( pAcc1 ), pAcc1 = NULL;
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

VCL_DLLPUBLIC bool ImportXBM( SvStream& rStm, Graphic& rGraphic )
{
    XBMReader*  pXBMReader = static_cast<XBMReader*>( rGraphic.GetContext() );
    ReadState   eReadState;
    bool        bRet = true;

    if( !pXBMReader )
        pXBMReader = new XBMReader( rStm );

    rGraphic.SetContext( NULL );
    eReadState = pXBMReader->ReadXBM( rGraphic );

    if( eReadState == XBMREAD_ERROR )
    {
        bRet = false;
        delete pXBMReader;
    }
    else if( eReadState == XBMREAD_OK )
        delete pXBMReader;
    else
        rGraphic.SetContext( pXBMReader );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
