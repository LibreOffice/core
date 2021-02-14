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


#include <vcl/graph.hxx>
#include <vcl/BitmapTools.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <filter/RasReader.hxx>

class FilterConfigItem;

#define RAS_TYPE_OLD            0x00000000      // supported formats by this filter
#define RAS_TYPE_STANDARD       0x00000001
#define RAS_TYPE_BYTE_ENCODED   0x00000002
#define RAS_TYPE_RGB_FORMAT     0x00000003

#define RAS_COLOR_NO_MAP        0x00000000
#define RAS_COLOR_RGB_MAP       0x00000001
#define RAS_COLOR_RAW_MAP       0x00000002

#define SUNRASTER_MAGICNUMBER   0x59a66a95

//============================ RASReader ==================================

namespace {

class RASReader {

private:

    SvStream&           m_rRAS;                 // the RAS file to be read in

    bool                mbStatus;
    sal_Int32           mnWidth, mnHeight;      // image dimensions in pixels
    sal_uInt16          mnDstBitsPerPix;
    sal_uInt16          mnDstColors;
    sal_Int32           mnDepth, mnImageDatSize, mnType;
    sal_Int32           mnColorMapType, mnColorMapSize;
    sal_uInt8           mnRepCount, mnRepVal;   // RLE Decoding

    bool                ImplReadBody(vcl::bitmap::RawBitmap&, std::vector<Color> const & rvPalette);
    bool                ImplReadHeader();
    sal_uInt8           ImplGetByte();

public:
    explicit RASReader(SvStream &rRAS);
    bool                ReadRAS(Graphic & rGraphic);
};

}

//=================== Methods of RASReader ==============================

RASReader::RASReader(SvStream &rRAS)
    : m_rRAS(rRAS)
    , mbStatus(true)
    , mnWidth(0)
    , mnHeight(0)
    , mnDstBitsPerPix(0)
    , mnDstColors(0)
    , mnDepth(0)
    , mnImageDatSize(0)
    , mnType(0)
    , mnColorMapType(0)
    , mnColorMapSize(0)
    , mnRepCount(0)
    , mnRepVal(0)
{
}

bool RASReader::ReadRAS(Graphic & rGraphic)
{
    sal_uInt32 nMagicNumber;

    if ( m_rRAS.GetError() )
        return false;

    m_rRAS.SetEndian( SvStreamEndian::BIG );
    m_rRAS.ReadUInt32( nMagicNumber );
    if ( nMagicNumber != SUNRASTER_MAGICNUMBER )
        return false;

    // Kopf einlesen:

    mbStatus = ImplReadHeader();
    if ( !mbStatus )
        return false;

    std::vector<Color> aPalette;
    bool bOk = true;

    if ( mnDstBitsPerPix <= 8 )     // pallets pictures
    {
        bool bPalette(false);

        if ( mnColorMapType == RAS_COLOR_RAW_MAP )      // RAW color map is skipped
        {
            sal_uInt64 nCurPos = m_rRAS.Tell();
            bOk = checkSeek(m_rRAS, nCurPos + mnColorMapSize);
        }
        else if ( mnColorMapType == RAS_COLOR_RGB_MAP ) // we can read out the RGB
        {
            mnDstColors = static_cast<sal_uInt16>( mnColorMapSize / 3 );

            if ( ( 1 << mnDstBitsPerPix ) < mnDstColors )
                return false;

            if ( ( mnDstColors >= 2 ) && ( ( mnColorMapSize % 3 ) == 0 ) )
            {
                aPalette.resize(mnDstColors);
                sal_uInt16  i;
                sal_uInt8   nRed[256], nGreen[256], nBlue[256];
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS.ReadUChar( nRed[ i ] );
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS.ReadUChar( nGreen[ i ] );
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS.ReadUChar( nBlue[ i ] );
                for ( i = 0; i < mnDstColors; i++ )
                {
                    aPalette[i] = Color(nRed[ i ], nGreen[ i ], nBlue[ i ]);
                }
                bPalette = true;
            }
            else
                return false;

        }
        else if ( mnColorMapType != RAS_COLOR_NO_MAP )  // everything else is not standard
            return false;

        if (!bPalette)
        {
            mnDstColors = 1 << mnDstBitsPerPix;
            aPalette.resize(mnDstColors);
            for ( sal_uInt16 i = 0; i < mnDstColors; i++ )
            {
                sal_uInt8 nCount = 255 - ( 255 * i / ( mnDstColors - 1 ) );
                aPalette[i] = Color(nCount, nCount, nCount);
            }
        }
    }
    else
    {
        if ( mnColorMapType != RAS_COLOR_NO_MAP )   // when graphic has more than 256 colors and a color map we skip
        {                                           // the colormap
            sal_uInt64 nCurPos = m_rRAS.Tell();
            bOk = checkSeek(m_rRAS, nCurPos + mnColorMapSize);
        }
    }

    if (!bOk)
        return false;

    //The RLE packets are typically three bytes in size:
    //The first byte is a Flag Value indicating the type of RLE packet.
    //The second byte is the Run Count.
    //The third byte is the Run Value.
    //
    //for the sake of simplicity we'll assume that RAS_TYPE_BYTE_ENCODED can
    //describe data 255 times larger than the data stored
    size_t nMaxCompression = mnType != RAS_TYPE_BYTE_ENCODED ? 1 : 255;
    sal_Int32 nBitSize;
    if (o3tl::checked_multiply<sal_Int32>(mnWidth, mnHeight, nBitSize) || o3tl::checked_multiply<sal_Int32>(nBitSize, mnDepth, nBitSize))
        return false;
    if (m_rRAS.remainingSize() * nMaxCompression < static_cast<sal_uInt32>(nBitSize) / 8)
        return false;

    vcl::bitmap::RawBitmap aBmp(Size(mnWidth, mnHeight), 24);

    // read in the bitmap data
    mbStatus = ImplReadBody(aBmp, aPalette);

    if ( mbStatus )
        rGraphic = vcl::bitmap::CreateFromData(std::move(aBmp));

    return mbStatus;
}

bool RASReader::ImplReadHeader()
{
    m_rRAS.ReadInt32(mnWidth).ReadInt32(mnHeight).ReadInt32(mnDepth).ReadInt32(mnImageDatSize).ReadInt32(mnType).ReadInt32(mnColorMapType).ReadInt32(mnColorMapSize);

    if (!m_rRAS.good() || mnWidth <= 0 || mnHeight <= 0 || mnImageDatSize <= 0)
        mbStatus = false;

    switch ( mnDepth )
    {
        case 24 :
        case  8 :
        case  1 :
            mnDstBitsPerPix = static_cast<sal_uInt16>(mnDepth);
            break;
        case 32 :
            mnDstBitsPerPix = 24;
            break;

        default :
            mbStatus = false;
    }

    switch ( mnType )
    {
        case RAS_TYPE_OLD :
        case RAS_TYPE_STANDARD :
        case RAS_TYPE_RGB_FORMAT :
        case RAS_TYPE_BYTE_ENCODED :            // this type will be supported later
            break;

        default:
            mbStatus = false;
    }
    return mbStatus;
}

namespace
{
    const Color& SanitizePaletteIndex(std::vector<Color> const & rvPalette, sal_uInt8 nIndex)
    {
        if (nIndex >= rvPalette.size())
        {
            auto nSanitizedIndex = nIndex % rvPalette.size();
            SAL_WARN_IF(nIndex != nSanitizedIndex, "filter.ras", "invalid colormap index: "
                        << static_cast<unsigned int>(nIndex) << ", colormap len is: "
                        << rvPalette.size());
            nIndex = nSanitizedIndex;
        }
        return rvPalette[nIndex];
    }
}

bool RASReader::ImplReadBody(vcl::bitmap::RawBitmap& rBitmap, std::vector<Color> const & rvPalette)
{
    sal_Int32 x, y;
    sal_uInt8    nRed, nGreen, nBlue;
    switch ( mnDstBitsPerPix )
    {
        case 1 :
        {
            sal_uInt8 nDat = 0;
            for (y = 0; y < mnHeight && mbStatus; ++y)
            {
                for (x = 0; x < mnWidth && mbStatus; ++x)
                {
                    if (!(x & 7))
                    {
                        nDat = ImplGetByte();
                        if (!m_rRAS.good())
                            mbStatus = false;
                    }
                    rBitmap.SetPixel(y, x, SanitizePaletteIndex(rvPalette,
                        sal::static_int_cast< sal_uInt8 >(
                            nDat >> ( ( x & 7 ) ^ 7 ))));
                }
                if (!( ( x - 1 ) & 0x8 ) )
                {
                    ImplGetByte();       // WORD ALIGNMENT ???
                    if (!m_rRAS.good())
                        mbStatus = false;
                }
            }
            break;
        }

        case 8 :
            for (y = 0; y < mnHeight && mbStatus; ++y)
            {
                for (x = 0; x < mnWidth && mbStatus; ++x)
                {
                    sal_uInt8 nDat = ImplGetByte();
                    rBitmap.SetPixel(y, x, SanitizePaletteIndex(rvPalette, nDat));
                    if (!m_rRAS.good())
                        mbStatus = false;
                }
                if ( x & 1 )
                {
                    ImplGetByte();                     // WORD ALIGNMENT ???
                    if (!m_rRAS.good())
                        mbStatus = false;
                }
            }
            break;

        case 24 :
            switch ( mnDepth )
            {

                case 24 :
                    for (y = 0; y < mnHeight && mbStatus; ++y)
                    {
                        for (x = 0; x < mnWidth && mbStatus; ++x)
                        {
                            if ( mnType == RAS_TYPE_RGB_FORMAT )
                            {
                                nRed = ImplGetByte();
                                nGreen = ImplGetByte();
                                nBlue = ImplGetByte();
                            }
                            else
                            {
                                nBlue = ImplGetByte();
                                nGreen = ImplGetByte();
                                nRed = ImplGetByte();
                            }
                            rBitmap.SetPixel(y, x, Color(nRed, nGreen, nBlue));
                            if (!m_rRAS.good())
                                mbStatus = false;
                        }
                        if ( x & 1 )
                        {
                            ImplGetByte();                     // WORD ALIGNMENT ???
                            if (!m_rRAS.good())
                                mbStatus = false;
                        }
                    }
                    break;

                case 32 :
                    for (y = 0; y < mnHeight && mbStatus; ++y)
                    {
                        for (x = 0; x < mnWidth && mbStatus; ++x)
                        {
                            ImplGetByte();               // pad byte > nil
                            if ( mnType == RAS_TYPE_RGB_FORMAT )
                            {
                                nRed = ImplGetByte();
                                nGreen = ImplGetByte();
                                nBlue = ImplGetByte();
                            }
                            else
                            {
                                nBlue = ImplGetByte();
                                nGreen = ImplGetByte();
                                nRed = ImplGetByte();
                            }
                            rBitmap.SetPixel(y, x, Color(nRed, nGreen, nBlue));
                            if (!m_rRAS.good())
                                mbStatus = false;
                        }
                    }
                    break;
            }
            break;

        default:
            mbStatus = false;
            break;
    }
    return mbStatus;
}

sal_uInt8 RASReader::ImplGetByte()
{
    sal_uInt8 nRetVal;
    if ( mnType != RAS_TYPE_BYTE_ENCODED )
    {
        m_rRAS.ReadUChar( nRetVal );
        return nRetVal;
    }
    else
    {
        if ( mnRepCount )
        {
            mnRepCount--;
            return mnRepVal;
        }
        else
        {
            m_rRAS.ReadUChar( nRetVal );
            if ( nRetVal != 0x80 )
                return nRetVal;
            m_rRAS.ReadUChar( nRetVal );
            if ( nRetVal == 0 )
                return 0x80;
            mnRepCount = nRetVal    ;
            m_rRAS.ReadUChar( mnRepVal );
            return mnRepVal;
        }
    }
}

//================== GraphicImport - the exported function ================

bool ImportRasGraphic( SvStream & rStream, Graphic & rGraphic)
{
    bool bRet = false;

    try
    {
        RASReader aRASReader(rStream);
        bRet = aRASReader.ReadRAS(rGraphic );
    }
    catch (...)
    {
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
