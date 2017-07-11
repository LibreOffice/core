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
#include <vcl/bitmapaccess.hxx>

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

    bool                ImplReadBody(BitmapWriteAccess * pAcc);
    bool                ImplReadHeader();
    sal_uInt8           ImplGetByte();

public:
    explicit RASReader(SvStream &rRAS);
    bool                ReadRAS(Graphic & rGraphic);
};

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

    bool bPalette(false);
    BitmapPalette aPalette;

    bool bOk = true;
    if ( mnDstBitsPerPix <= 8 )     // pallets pictures
    {
        if ( mnColorMapType == RAS_COLOR_RAW_MAP )      // RAW color map is skipped
        {
            sal_uLong nCurPos = m_rRAS.Tell();
            bOk = checkSeek(m_rRAS, nCurPos + mnColorMapSize);
        }
        else if ( mnColorMapType == RAS_COLOR_RGB_MAP ) // we can read out the RGB
        {
            mnDstColors = (sal_uInt16)( mnColorMapSize / 3 );

            if ( ( 1 << mnDstBitsPerPix ) < mnDstColors )
                return false;

            if ( ( mnDstColors >= 2 ) && ( ( mnColorMapSize % 3 ) == 0 ) )
            {
                aPalette.SetEntryCount(mnDstColors);
                sal_uInt16  i;
                sal_uInt8   nRed[256], nGreen[256], nBlue[256];
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS.ReadUChar( nRed[ i ] );
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS.ReadUChar( nGreen[ i ] );
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS.ReadUChar( nBlue[ i ] );
                for ( i = 0; i < mnDstColors; i++ )
                {
                    aPalette[i] = BitmapColor(nRed[ i ], nGreen[ i ], nBlue[ i ]);
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
            aPalette.SetEntryCount(mnDstColors);
            for ( sal_uInt16 i = 0; i < mnDstColors; i++ )
            {
                sal_uLong nCount = 255 - ( 255 * i / ( mnDstColors - 1 ) );
                aPalette[i] = BitmapColor((sal_uInt8)nCount, (sal_uInt8)nCount, (sal_uInt8)nCount);
            }
            bPalette = true;
        }
    }
    else
    {
        if ( mnColorMapType != RAS_COLOR_NO_MAP )   // when graphic has more than 256 colors and a color map we skip
        {                                           // the colormap
            sal_uLong nCurPos = m_rRAS.Tell();
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
    if (m_rRAS.remainingSize() * nMaxCompression < static_cast<sal_uInt64>(mnHeight) * mnWidth * mnDepth / 8)
    {
        return false;
    }

    Bitmap aBmp(Size(mnWidth, mnHeight), mnDstBitsPerPix);
    Bitmap::ScopedWriteAccess pAcc(aBmp);
    if (pAcc == nullptr)
        return false;

    if (bPalette)
    {
        pAcc->SetPalette(aPalette);
    }


    // read in the bitmap data
    mbStatus = ImplReadBody(pAcc.get());

    if ( mbStatus )
        rGraphic = aBmp;

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
            mnDstBitsPerPix = (sal_uInt16)mnDepth;
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

bool RASReader::ImplReadBody(BitmapWriteAccess * pAcc)
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
                    pAcc->SetPixelIndex( y, x,
                        sal::static_int_cast< sal_uInt8 >(
                            nDat >> ( ( x & 7 ) ^ 7 )) );
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
                    pAcc->SetPixelIndex( y, x, nDat );
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
                            pAcc->SetPixel ( y, x, BitmapColor( nRed, nGreen, nBlue ) );
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
                            pAcc->SetPixel ( y, x, BitmapColor( nRed, nGreen, nBlue ) );
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

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
iraGraphicImport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* )
{
    RASReader aRASReader(rStream);

    return aRASReader.ReadRAS(rGraphic );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
