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
#include <o3tl/string_view.hxx>
#include <rtl/character.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <graphic/GraphicReader.hxx>

#include "xbmread.hxx"

namespace {

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
    SvStream& mrStream;
    Bitmap maBitmap;
    BitmapScopedWriteAccess mpWriteAccess;
    std::array<short, 256> mpHexTable = { 0 };
    BitmapColor maWhite;
    BitmapColor maBlack;
    tools::Long mnLastPos = 0;
    tools::Long nWidth = 0;
    tools::Long nHeight = 0;
    bool bStatus = true;

    void InitTable();
    OString FindTokenLine(SvStream* pInStm, const char* pTok1, const char* pTok2);
    int ParseDefine(const char* pDefine);
    void ParseData(SvStream* pInStm, const OString& aLastLine, XBMFormat eFormat);

public:

    explicit XBMReader(SvStream& rStream);

    ReadState ReadXBM(Graphic& rGraphic);
};

}

XBMReader::XBMReader(SvStream& rStream)
    : mrStream(rStream)
    , mnLastPos(rStream.Tell())
{
    maUpperName = "SVIXBM";
    InitTable();
}

void XBMReader::InitTable()
{
    mpHexTable[int('0')] = 0;
    mpHexTable[int('1')] = 1;
    mpHexTable[int('2')] = 2;
    mpHexTable[int('3')] = 3;
    mpHexTable[int('4')] = 4;
    mpHexTable[int('5')] = 5;
    mpHexTable[int('6')] = 6;
    mpHexTable[int('7')] = 7;
    mpHexTable[int('8')] = 8;
    mpHexTable[int('9')] = 9;
    mpHexTable[int('A')] = 10;
    mpHexTable[int('B')] = 11;
    mpHexTable[int('C')] = 12;
    mpHexTable[int('D')] = 13;
    mpHexTable[int('E')] = 14;
    mpHexTable[int('F')] = 15;
    mpHexTable[int('X')] = 0;
    mpHexTable[int('a')] = 10;
    mpHexTable[int('b')] = 11;
    mpHexTable[int('c')] = 12;
    mpHexTable[int('d')] = 13;
    mpHexTable[int('e')] = 14;
    mpHexTable[int('f')] = 15;
    mpHexTable[int('x')] = 0;
    mpHexTable[int(' ')] = -1;
    mpHexTable[int(',')] = -1;
    mpHexTable[int('}')] = -1;
    mpHexTable[int('\n')] = -1;
    mpHexTable[int('\t')] = -1;
    mpHexTable[int('\0')] = -1;
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

                    nPos2 = aRet.indexOf( pTok2 );
                    if( ( nPos2 != -1 ) && ( nPos2 > nPos1 ) )
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

int XBMReader::ParseDefine( const char* pDefine )
{
    sal_Int32 nRet = 0;
    const char* pTmp = pDefine;
    unsigned char   cTmp;

    // move to end
    pTmp += ( strlen( pDefine ) - 1 );
    cTmp = *pTmp--;

    // search last digit
    while (mpHexTable[ cTmp ] == -1 && pTmp >= pDefine)
        cTmp = *pTmp--;

    // move before number
    while (mpHexTable[ cTmp ] != -1 && pTmp >= pDefine)
        cTmp = *pTmp--;

    // move to start of number
    pTmp += 2;

    // read Hex
    if( ( pTmp[0] == '0' ) && ( ( pTmp[1] == 'X' ) || ( pTmp[1] == 'x' ) ) )
    {
        pTmp += 2;
        nRet = o3tl::toInt32(std::string_view(pTmp), 16);
    }
    else // read decimal
    {
        nRet = o3tl::toInt32(std::string_view(pTmp));
    }

    return nRet;
}

void XBMReader::ParseData( SvStream* pInStm, const OString& aLastLine, XBMFormat eFormat )
{
    OString    aLine;
    tools::Long            nRow = 0;
    tools::Long            nCol = 0;
    tools::Long            nBits = ( eFormat == XBM10 ) ? 16 : 8;
    tools::Long            nBit;
    sal_uInt16          nValue;
    sal_uInt16          nDigits;
    bool            bFirstLine = true;

    while( nRow < nHeight )
    {
        if( bFirstLine )
        {
            sal_Int32 nPos;

            // delete opening curly bracket
            aLine = aLastLine;
            nPos = aLine.indexOf('{');
            if( nPos != -1 )
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

                    const short nTable = mpHexTable[cChar];

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
                    Scanline pScanline = mpWriteAccess->GetScanline(nRow);
                    while( ( nCol < nWidth ) && ( nBit < nBits ) )
                        mpWriteAccess->SetPixelOnData(pScanline, nCol++, ( nValue & ( 1 << nBit++ ) ) ? maBlack : maWhite);

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
    mrStream.Seek( STREAM_SEEK_TO_END );
    mrStream.ReadUChar( cDummy );

    // if we cannot read all
    // we return and wait for new data
    if (mrStream.GetError() != ERRCODE_IO_PENDING )
    {
        mrStream.Seek(mnLastPos);
        bStatus = false;
        OString aLine = FindTokenLine(&mrStream, "#define", "_width");

        if ( bStatus )
        {
            int nValue;
            if ( ( nValue = ParseDefine( aLine.getStr() ) ) > 0 )
            {
                nWidth = nValue;
                aLine = FindTokenLine(&mrStream, "#define", "_height");

                // if height was not received, we search again
                // from start of the file
                if ( !bStatus )
                {
                    mrStream.Seek(mnLastPos);
                    aLine = FindTokenLine(&mrStream, "#define", "_height");
                }
            }
            else
                bStatus = false;

            if ( bStatus )
            {
                if ( ( nValue = ParseDefine( aLine.getStr() ) ) > 0 )
                {
                    nHeight = nValue;
                    aLine = FindTokenLine(&mrStream, "static", "_bits");

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
                        if (mrStream.remainingSize() < (static_cast<sal_uInt64>(nWidth) * nHeight) / 8)
                            bStatus = false;

                        if ( bStatus && nWidth && nHeight )
                        {
                            maBitmap = Bitmap(Size(nWidth, nHeight), vcl::PixelFormat::N8_BPP, &Bitmap::GetGreyPalette(256));
                            mpWriteAccess = maBitmap;

                            if (mpWriteAccess)
                            {
                                maWhite = mpWriteAccess->GetBestMatchingColor(COL_WHITE);
                                maBlack = mpWriteAccess->GetBestMatchingColor(COL_BLACK);
                                ParseData(&mrStream, aLine, eFormat);
                            }
                            else
                                bStatus = false;
                        }
                    }
                }
            }
        }

        if (bStatus && mpWriteAccess)
        {
            Bitmap aBlackBmp(Size(mpWriteAccess->Width(), mpWriteAccess->Height()), vcl::PixelFormat::N8_BPP, &Bitmap::GetGreyPalette(256));

            mpWriteAccess.reset();
            aBlackBmp.Erase( COL_BLACK );
            rGraphic = BitmapEx(aBlackBmp, maBitmap);
            eReadState = XBMREAD_OK;
        }
        else
            eReadState = XBMREAD_ERROR;
    }
    else
    {
        mrStream.ResetError();
        eReadState = XBMREAD_NEED_MORE;
    }

    return eReadState;
}

VCL_DLLPUBLIC bool ImportXBM( SvStream& rStm, Graphic& rGraphic )
{
    std::shared_ptr<GraphicReader> pContext = rGraphic.GetReaderContext();
    rGraphic.SetReaderContext(nullptr);
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
        rGraphic.SetReaderContext( pContext );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
