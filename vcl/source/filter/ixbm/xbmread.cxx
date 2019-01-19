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

#include <memory>
#include <sal/config.h>
#include <tools/stream.hxx>

#include <rtl/character.hxx>
#include <bitmapwriteaccess.hxx>

#include "xbmread.hxx"

enum XBMFormat
{
    XBM10,
    XBM11
};

enum ReadState
{
    XBMREAD_OK,
    XBMREAD_ERROR,
    XBMREAD_NEED_MORE
};

class XBMReader : public GraphicReader
{
    SvStream&           rIStm;
    Bitmap              aBmp1;
    BitmapScopedWriteAccess pAcc1;
    std::unique_ptr<short[]>
                        pHexTable;
    BitmapColor         aWhite;
    BitmapColor         aBlack;
    long const          nLastPos;
    long                nWidth;
    long                nHeight;
    bool                bStatus;

    void            InitTable();
    OString         FindTokenLine( SvStream* pInStm, const char* pTok1, const char* pTok2 );
    int             ParseDefine( const sal_Char* pDefine );
    void            ParseData( SvStream* pInStm, const OString& aLastLine, XBMFormat eFormat );

public:

    explicit        XBMReader( SvStream& rStm );

    ReadState       ReadXBM( Graphic& rGraphic );
};

XBMReader::XBMReader( SvStream& rStm ) :
            rIStm           ( rStm ),
            nLastPos        ( rStm.Tell() ),
            nWidth          ( 0 ),
            nHeight         ( 0 ),
            bStatus         ( true )
{
    pHexTable.reset( new short[ 256 ] );
    maUpperName = "SVIXBM";
    InitTable();
}

void XBMReader::InitTable()
{
    memset( pHexTable.get(), 0, sizeof( short ) * 256 );

    pHexTable[int('0')] = 0;
    pHexTable[int('1')] = 1;
    pHexTable[int('2')] = 2;
    pHexTable[int('3')] = 3;
    pHexTable[int('4')] = 4;
    pHexTable[int('5')] = 5;
    pHexTable[int('6')] = 6;
    pHexTable[int('7')] = 7;
    pHexTable[int('8')] = 8;
    pHexTable[int('9')] = 9;
    pHexTable[int('A')] = 10;
    pHexTable[int('B')] = 11;
    pHexTable[int('C')] = 12;
    pHexTable[int('D')] = 13;
    pHexTable[int('E')] = 14;
    pHexTable[int('F')] = 15;
    pHexTable[int('X')] = 0;
    pHexTable[int('a')] = 10;
    pHexTable[int('b')] = 11;
    pHexTable[int('c')] = 12;
    pHexTable[int('d')] = 13;
    pHexTable[int('e')] = 14;
    pHexTable[int('f')] = 15;
    pHexTable[int('x')] = 0;
    pHexTable[int(' ')] = -1;
    pHexTable[int(',')] = -1;
    pHexTable[int('}')] = -1;
    pHexTable[int('\n')] = -1;
    pHexTable[int('\t')] = -1;
    pHexTable[int('\0')] = -1;
}

OString XBMReader::FindTokenLine( SvStream* pInStm, const char* pTok1,
                                 const char* pTok2 )
{
    OString aRet;
    sal_Int32 nPos1, nPos2;

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
                    }
                }
            }
        }
    }
    while( !bStatus );

    return aRet;
}

int XBMReader::ParseDefine( const sal_Char* pDefine )
{
    sal_Int32 nRet = 0;
    const char* pTmp = pDefine;
    unsigned char   cTmp;

    // move to end
    pTmp += ( strlen( pDefine ) - 1 );
    cTmp = *pTmp--;

    // search last digit
    while (pHexTable[ cTmp ] == -1 && pTmp >= pDefine)
        cTmp = *pTmp--;

    // move before number
    while (pHexTable[ cTmp ] != -1 && pTmp >= pDefine)
        cTmp = *pTmp--;

    // move to start of number
    pTmp += 2;

    // read Hex
    if( ( pTmp[0] == '0' ) && ( ( pTmp[1] == 'X' ) || ( pTmp[1] == 'x' ) ) )
    {
        pTmp += 2;
        nRet = OString(pTmp, strlen(pTmp)).toInt32(16);
    }
    else // read decimal
    {
        nRet = OString(pTmp, strlen(pTmp)).toInt32();
    }

    return nRet;
}

void XBMReader::ParseData( SvStream* pInStm, const OString& aLastLine, XBMFormat eFormat )
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
            sal_Int32 nIndex = 0;
            const sal_Int32 nLen {aLine.getLength()};
            while (nRow<nHeight && nIndex<nLen)
            {
                bool bProcessed = false;

                nBit = nDigits = nValue = 0;

                while (nIndex<nLen)
                {
                    const unsigned char cChar = aLine[nIndex];

                    ++nIndex;
                    if (cChar==',') // sequence completed, ',' already skipped for next loop
                        break;

                    const short         nTable = pHexTable[ cChar ];

                    if( rtl::isAsciiHexDigit( cChar ) || !nTable )
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
                    Scanline pScanline = pAcc1->GetScanline(nRow);
                    while( ( nCol < nWidth ) && ( nBit < nBits ) )
                        pAcc1->SetPixelOnData(pScanline, nCol++, ( nValue & ( 1 << nBit++ ) ) ? aBlack : aWhite);

                    if( nCol == nWidth )
                    {
                        nCol = 0;
                        nRow++;
                    }
                }
            }
        }
    }
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
            if ( ( nValue = ParseDefine( aLine.getStr() ) ) > 0 )
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
                if ( ( nValue = ParseDefine( aLine.getStr() ) ) > 0 )
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

                        //xbms are a minimum of one character per 8 pixels, so if the file isn't
                        //even that long, it's not all there
                        if (rIStm.remainingSize() < (static_cast<sal_uInt64>(nWidth) * nHeight) / 8)
                            bStatus = false;

                        if ( bStatus && nWidth && nHeight )
                        {
                            aBmp1 = Bitmap( Size( nWidth, nHeight ), 1 );
                            pAcc1 = BitmapScopedWriteAccess(aBmp1);

                            if( pAcc1 )
                            {
                                aWhite = pAcc1->GetBestMatchingColor( COL_WHITE );
                                aBlack = pAcc1->GetBestMatchingColor( COL_BLACK );
                                ParseData( &rIStm, aLine, eFormat );
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

            pAcc1.reset();
            aBlackBmp.Erase( COL_BLACK );
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
    std::shared_ptr<GraphicReader> pContext = rGraphic.GetContext();
    rGraphic.SetContext(nullptr);
    XBMReader* pXBMReader = dynamic_cast<XBMReader*>( pContext.get() );
    if (!pXBMReader)
    {
        pContext = std::make_shared<XBMReader>( rStm );
        pXBMReader = static_cast<XBMReader*>( pContext.get() );
    }

    bool bRet = true;

    ReadState eReadState = pXBMReader->ReadXBM( rGraphic );

    if( eReadState == XBMREAD_ERROR )
    {
        bRet = false;
    }
    else if( eReadState == XBMREAD_NEED_MORE )
        rGraphic.SetContext( pContext );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
