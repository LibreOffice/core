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

#include <sal/config.h>
#include <o3tl/safeint.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/graph.hxx>
#include <vcl/BitmapTools.hxx>
#include <tools/stream.hxx>
#include <filter/PbmReader.hxx>

//============================ PBMReader ==================================

namespace {

class PBMReader {

private:

    SvStream&           mrPBM;              // the PBM file to read

    bool            mbStatus;
    bool            mbRemark;           // sal_False if the stream is in a comment
    bool            mbRaw;              // RAW/ASCII MODE
    sal_uInt8           mnMode;             // 0->PBM, 1->PGM, 2->PPM
    std::unique_ptr<vcl::bitmap::RawBitmap> mpRawBmp;
    std::vector<Color>  mvPalette;
    sal_Int32       mnWidth, mnHeight;  // dimensions in pixel
    sal_uInt16           mnCol;
    sal_uInt64           mnMaxVal;           // max value in the <missing comment>
    bool            ImplReadBody();
    bool            ImplReadHeader();

public:
    explicit PBMReader(SvStream & rPBM);
    bool                ReadPBM(Graphic & rGraphic );
};

}

//=================== Methods of PBMReader ==============================

PBMReader::PBMReader(SvStream & rPBM)
    : mrPBM(rPBM)
    , mbStatus(true)
    , mbRemark(false)
    , mbRaw(true)
    , mnMode(0)
    , mnWidth(0)
    , mnHeight(0)
    , mnCol(0)
    , mnMaxVal(0)
{
}

bool PBMReader::ReadPBM(Graphic & rGraphic )
{
    if ( mrPBM.GetError() )
        return false;

    mrPBM.SetEndian( SvStreamEndian::LITTLE );

    // read header:

    mbStatus = ImplReadHeader();
    if ( !mbStatus )
        return false;

    if ( ( mnMaxVal == 0 ) || ( mnWidth <= 0 ) || ( mnHeight <= 0 ) )
        return false;

    sal_uInt32 nPixelsRequired;
    if (o3tl::checked_multiply<sal_uInt32>(mnWidth, mnHeight, nPixelsRequired))
        return false;
    const auto nRemainingSize = mrPBM.remainingSize();

    // 0->PBM, 1->PGM, 2->PPM
    switch ( mnMode )
    {
        case 0:
        {
            if (nRemainingSize < nPixelsRequired / 8)
                return false;

            mpRawBmp.reset( new vcl::bitmap::RawBitmap( Size( mnWidth, mnHeight ), 24 ) );
            mvPalette.resize( 2 );
            mvPalette[0] = Color( 0xff, 0xff, 0xff );
            mvPalette[1] = Color( 0x00, 0x00, 0x00 );
            break;
        }
        case 1 :
            if (nRemainingSize < nPixelsRequired)
                return false;

            mpRawBmp.reset( new vcl::bitmap::RawBitmap( Size( mnWidth, mnHeight ), 24 ) );
            mnCol = static_cast<sal_uInt16>(mnMaxVal) + 1;
            if ( mnCol > 256 )
                mnCol = 256;

            mvPalette.resize( 256 );
            for ( sal_uInt16 i = 0; i < mnCol; i++ )
            {
                sal_uInt16 nCount = 255 * i / mnCol;
                mvPalette[i] = Color( static_cast<sal_uInt8>(nCount), static_cast<sal_uInt8>(nCount), static_cast<sal_uInt8>(nCount) );
            }
            break;
        case 2 :
            if (nRemainingSize / 3 < nPixelsRequired)
                return false;

            mpRawBmp.reset( new vcl::bitmap::RawBitmap( Size( mnWidth, mnHeight ), 24 ) );
            break;
    }

    // read bitmap data
    mbStatus = ImplReadBody();

    if ( mbStatus )
        rGraphic = vcl::bitmap::CreateFromData(std::move(*mpRawBmp));

    return mbStatus;
}

bool PBMReader::ImplReadHeader()
{
    sal_uInt8   nID[ 2 ];
    sal_uInt8   nDat;
    sal_uInt8   nMax, nCount = 0;
    bool    bFinished = false;

    mrPBM.ReadUChar( nID[ 0 ] ).ReadUChar( nID[ 1 ] );
    if ( nID[ 0 ] != 'P' )
        return false;
    mnMaxVal = mnWidth = mnHeight = 0;
    switch ( nID[ 1 ] )
    {
        case '1' :
            mbRaw = false;
            [[fallthrough]];
        case '4' :
            mnMode = 0;
            nMax = 2;               // number of parameters in Header
            mnMaxVal = 1;
            break;
        case '2' :
            mbRaw = false;
            [[fallthrough]];
        case '5' :
            mnMode = 1;
            nMax = 3;
            break;
        case '3' :
            mbRaw = false;
            [[fallthrough]];
        case '6' :
            mnMode = 2;
            nMax = 3;
            break;
        default:
            return false;
    }
    while ( !bFinished )
    {
        if (!mrPBM.good())
            return false;

        mrPBM.ReadUChar( nDat );

        if ( nDat == '#' )
        {
            mbRemark = true;
            continue;
        }
        else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
        {
            mbRemark = false;
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
                    bFinished = true;
            }
            else if ( ( nCount == 2 ) && mnMaxVal )
            {
                bFinished = true;
            }
            continue;
        }
        if ( ( nDat >= '0' ) && ( nDat <= '9' ) )
        {
            nDat -= '0';
            if ( nCount == 0 )
            {
                if (mnWidth > SAL_MAX_INT32 / 10)
                {
                    return false;
                }
                mnWidth *= 10;
                if (nDat > SAL_MAX_INT32 - mnWidth)
                {
                    return false;
                }
                mnWidth += nDat;
            }
            else if ( nCount == 1 )
            {
                if (mnHeight > SAL_MAX_INT32 / 10)
                {
                    return false;
                }
                mnHeight *= 10;
                if (nDat > SAL_MAX_INT32 - mnHeight)
                {
                    return false;
                }
                mnHeight += nDat;
            }
            else if ( nCount == 2 )
            {
                if (mnMaxVal > std::numeric_limits<sal_uInt64>::max() / 10)
                {
                    return false;
                }
                mnMaxVal *= 10;
                if (nDat > std::numeric_limits<sal_uInt64>::max() - mnMaxVal)
                {
                    return false;
                }
                mnMaxVal += nDat;
            }
        }
        else
            return false;
    }
    return mbStatus;
}

bool PBMReader::ImplReadBody()
{
    sal_uInt8   nDat = 0, nCount;
    sal_uInt64   nGrey, nRGB[3];
    sal_Int32 nWidth = 0;
    sal_Int32 nHeight = 0;

    if ( mbRaw )
    {
        signed char nShift = 0;
        switch ( mnMode )
        {

            // PBM
            case 0 :
                while ( nHeight != mnHeight )
                {
                    if (!mrPBM.good())
                        return false;

                    if ( --nShift < 0 )
                    {
                        mrPBM.ReadUChar( nDat );
                        nShift = 7;
                    }
                    mpRawBmp->SetPixel( nHeight, nWidth, mvPalette[(nDat >> nShift) & 0x01] );
                    if ( ++nWidth == mnWidth )
                    {
                        nShift = 0;
                        nWidth = 0;
                        nHeight++;
                    }
                }
                break;

            // PGM
            case 1 :
                while ( nHeight != mnHeight )
                {
                    if (!mrPBM.good())
                        return false;

                    mrPBM.ReadUChar( nDat );
                    mpRawBmp->SetPixel( nHeight, nWidth++, mvPalette[nDat]);

                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        nHeight++;
                    }
                }
                break;

            // PPM
            case 2 :
                while ( nHeight != mnHeight )
                {
                    if (!mrPBM.good())
                        return false;

                    sal_uInt8   nR, nG, nB;
                    sal_uInt8   nRed, nGreen, nBlue;
                    mrPBM.ReadUChar( nR ).ReadUChar( nG ).ReadUChar( nB );
                    nRed = 255 * nR / mnMaxVal;
                    nGreen = 255 * nG / mnMaxVal;
                    nBlue = 255 * nB / mnMaxVal;
                    mpRawBmp->SetPixel( nHeight, nWidth++, Color( nRed, nGreen, nBlue ) );
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        nHeight++;
                    }
                }
                break;
        }
    }
    else
    {
        bool bPara = false;
        bool bFinished = false;

        switch  ( mnMode )
        {
        // PBM
        case 0 :
            while ( !bFinished )
            {
                if (!mrPBM.good())
                    return false;

                mrPBM.ReadUChar( nDat );

                if ( nDat == '#' )
                {
                    mbRemark = true;
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = false;
                    continue;
                }
                if ( mbRemark || nDat == 0x20 || nDat == 0x09 )
                    continue;

                if ( nDat == '0' || nDat == '1' )
                {
                    mpRawBmp->SetPixel( nHeight, nWidth, mvPalette[static_cast<sal_uInt8>(nDat - '0')] );
                    nWidth++;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = true;
                    }
                }
                else
                    return false;
            }
            break;

        // PGM
        case 1 :

            bPara = false;
            nCount = 0;
            nGrey = 0;

            while ( !bFinished )
            {
                if ( nCount )
                {
                    nCount--;
                    if ( nGrey <= mnMaxVal )
                        nGrey = 255 * nGrey / mnMaxVal;
                    mpRawBmp->SetPixel( nHeight, nWidth++, mvPalette[static_cast<sal_uInt8>(nGrey)] );
                    nGrey = 0;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = true;
                    }
                    continue;
                }

                if (!mrPBM.good())
                    return false;

                mrPBM.ReadUChar( nDat );

                if ( nDat == '#' )
                {
                    mbRemark = true;
                    if ( bPara )
                    {
                        bPara = false;
                        nCount++;
                    }
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = false;
                    if ( bPara )
                    {
                        bPara = false;
                        nCount++;
                    }
                    continue;
                }

                if ( nDat == 0x20 || nDat == 0x09 )
                {
                    if ( bPara )
                    {
                        bPara = false;
                        nCount++;
                    }
                    continue;
                }
                if ( nDat >= '0' && nDat <= '9' )
                {
                    bPara = true;
                    nGrey *= 10;
                    nGrey += nDat-'0';
                    continue;
                }
                else
                    return false;
            }
            break;


        // PPM
        case 2 :

            bPara = false;
            nCount = 0;
            nRGB[ 0 ] = nRGB[ 1 ] = nRGB[ 2 ] = 0;

            while ( !bFinished )
            {
                if ( nCount == 3 )
                {
                    nCount = 0;
                    mpRawBmp->SetPixel( nHeight, nWidth++, Color( static_cast< sal_uInt8 >( ( nRGB[ 0 ] * 255 ) / mnMaxVal ),
                                                                     static_cast< sal_uInt8 >( ( nRGB[ 1 ] * 255 ) / mnMaxVal ),
                                                                     static_cast< sal_uInt8 >( ( nRGB[ 2 ] * 255 ) / mnMaxVal ) ) );
                    nRGB[ 0 ] = nRGB[ 1 ] = nRGB[ 2 ] = 0;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = true;
                    }
                    continue;
                }

                if (!mrPBM.good())
                    return false;

                mrPBM.ReadUChar( nDat );

                if ( nDat == '#' )
                {
                    mbRemark = true;
                    if ( bPara )
                    {
                        bPara = false;
                        nCount++;
                    }
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = false;
                    if ( bPara )
                    {
                        bPara = false;
                        nCount++;
                    }
                    continue;
                }

                if ( nDat == 0x20 || nDat == 0x09 )
                {
                    if ( bPara )
                    {
                        bPara = false;
                        nCount++;
                    }
                    continue;
                }
                if ( nDat >= '0' && nDat <= '9' )
                {
                    bPara = true;
                    nRGB[ nCount ] *= 10;
                    nRGB[ nCount ] += nDat-'0';
                    continue;
                }
                else
                    return false;
            }
            break;
        }
    }
    return mbStatus;
}

//================== GraphicImport - the exported function ================

bool ImportPbmGraphic( SvStream & rStream, Graphic & rGraphic)
{
    PBMReader aPBMReader(rStream);

    return aPBMReader.ReadPBM(rGraphic );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
