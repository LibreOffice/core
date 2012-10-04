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
#include <vcl/bmpacc.hxx>

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

    SvStream&           m_rRAS;                 // Die einzulesende RAS-Datei

    sal_Bool                mbStatus;
    Bitmap              maBmp;
    BitmapWriteAccess*  mpAcc;
    sal_uInt32          mnWidth, mnHeight;      // Bildausmass in Pixeln
    sal_uInt16              mnDstBitsPerPix;
    sal_uInt16              mnDstColors;
    sal_uInt32          mnDepth, mnImageDatSize, mnType;
    sal_uInt32          mnColorMapType, mnColorMapSize;
    sal_uInt8               mnRepCount, mnRepVal;   // RLE Decoding
    sal_Bool                mbPalette;

    sal_Bool                ImplReadBody();
    sal_Bool                ImplReadHeader();
    sal_uInt8               ImplGetByte();

public:
                        RASReader(SvStream &rRAS);
                        ~RASReader();
    sal_Bool                ReadRAS(Graphic & rGraphic);
};

//=================== Methoden von RASReader ==============================

RASReader::RASReader(SvStream &rRAS)
    : m_rRAS(rRAS)
    , mbStatus(sal_True)
    , mpAcc(NULL)
    , mnRepCount(0)
    , mbPalette(sal_False)
{
}

RASReader::~RASReader()
{
}

//----------------------------------------------------------------------------

sal_Bool RASReader::ReadRAS(Graphic & rGraphic)
{
    sal_uInt32 nMagicNumber;

    if ( m_rRAS.GetError() )
        return sal_False;

    m_rRAS.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    m_rRAS >> nMagicNumber;
    if ( nMagicNumber != SUNRASTER_MAGICNUMBER )
        return sal_False;

    // Kopf einlesen:

    if ( ( mbStatus = ImplReadHeader() ) == sal_False )
        return sal_False;

    maBmp = Bitmap( Size( mnWidth, mnHeight ), mnDstBitsPerPix );
    if ( ( mpAcc = maBmp.AcquireWriteAccess() ) == sal_False )
        return sal_False;

    if ( mnDstBitsPerPix <= 8 )     // paletten bildchen
    {
        if ( mnColorMapType == RAS_COLOR_RAW_MAP )      // RAW Colormap wird geskipped
        {
            sal_uLong nCurPos = m_rRAS.Tell();
            m_rRAS.Seek( nCurPos + mnColorMapSize );
        }
        else if ( mnColorMapType == RAS_COLOR_RGB_MAP ) // RGB koennen wir auslesen
        {
            mnDstColors = (sal_uInt16)( mnColorMapSize / 3 );

            if ( ( 1 << mnDstBitsPerPix ) < mnDstColors )
                return sal_False;

            if ( ( mnDstColors >= 2 ) && ( ( mnColorMapSize % 3 ) == 0 ) )
            {
                mpAcc->SetPaletteEntryCount( mnDstColors );
                sal_uInt16  i;
                sal_uInt8   nRed[256], nGreen[256], nBlue[256];
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS >> nRed[ i ];
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS >> nGreen[ i ];
                for ( i = 0; i < mnDstColors; i++ ) m_rRAS >> nBlue[ i ];
                for ( i = 0; i < mnDstColors; i++ )
                {
                    mpAcc->SetPaletteColor( i, BitmapColor( nRed[ i ], nGreen[ i ], nBlue[ i ] ) );
                }
                mbPalette = sal_True;
            }
            else
                return sal_False;

        }
        else if ( mnColorMapType != RAS_COLOR_NO_MAP )  // alles andere ist kein standard
            return sal_False;

        if ( !mbPalette )
        {
            mnDstColors = 1 << mnDstBitsPerPix;
            mpAcc->SetPaletteEntryCount( mnDstColors );
            for ( sal_uInt16 i = 0; i < mnDstColors; i++ )
            {
                sal_uLong nCount = 255 - ( 255 * i / ( mnDstColors - 1 ) );
                mpAcc->SetPaletteColor( i, BitmapColor( (sal_uInt8)nCount, (sal_uInt8)nCount, (sal_uInt8)nCount ) );
            }
        }
    }
    else
    {
        if ( mnColorMapType != RAS_COLOR_NO_MAP )   // when graphic has more then 256 colors and a color map we skip
        {                                           // the colormap
            sal_uLong nCurPos = m_rRAS.Tell();
            m_rRAS.Seek( nCurPos + mnColorMapSize );
        }
    }

    // Bitmap-Daten einlesen
    mbStatus = ImplReadBody();

    if ( mpAcc )
    {
        maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
    }
    if ( mbStatus )
        rGraphic = maBmp;

    return mbStatus;
}

//----------------------------------------------------------------------------

sal_Bool RASReader::ImplReadHeader()
{
    m_rRAS >> mnWidth >> mnHeight >> mnDepth >> mnImageDatSize >>
        mnType >> mnColorMapType >> mnColorMapSize;

    if ( mnWidth == 0 || mnHeight == 0 )
        mbStatus = sal_False;

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
            mbStatus = sal_False;
    }

    switch ( mnType )
    {
        case RAS_TYPE_OLD :
        case RAS_TYPE_STANDARD :
        case RAS_TYPE_RGB_FORMAT :
        case RAS_TYPE_BYTE_ENCODED :            // this type will be supported later
            break;

        default:
            mbStatus = sal_False;
    }
    return mbStatus;
}

//----------------------------------------------------------------------------

sal_Bool RASReader::ImplReadBody()
{
    sal_uLong   x, y;
    sal_uInt8   nDat = 0;
    sal_uInt8    nRed, nGreen, nBlue;
    switch ( mnDstBitsPerPix )
    {
        case 1 :
            for ( y = 0; y < mnHeight; y++ )
            {
                for ( x = 0; x < mnWidth; x++ )
                {
                    if (!(x & 7))
                        nDat = ImplGetByte();
                    mpAcc->SetPixel (
                        y, x,
                        sal::static_int_cast< sal_uInt8 >(
                            nDat >> ( ( x & 7 ) ^ 7 )) );
                }
                if (!( ( x - 1 ) & 0x8 ) ) ImplGetByte();       // WORD ALIGNMENT ???
            }
            break;

        case 8 :
            for ( y = 0; y < mnHeight; y++ )
            {
                for ( x = 0; x < mnWidth; x++ )
                {
                    nDat = ImplGetByte();
                    mpAcc->SetPixel ( y, x, nDat );
                }
                if ( x & 1 ) ImplGetByte();                     // WORD ALIGNMENT ???
            }
            break;

        case 24 :
            switch ( mnDepth )
            {

                case 24 :
                    for ( y = 0; y < mnHeight; y++ )
                    {
                        for ( x = 0; x < mnWidth; x++ )
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
                            mpAcc->SetPixel ( y, x, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                        if ( x & 1 ) ImplGetByte();                     // WORD ALIGNMENT ???
                    }
                    break;

                case 32 :
                    for ( y = 0; y < mnHeight; y++ )
                    {
                        for ( x = 0; x < mnWidth; x++ )
                        {
                            nDat = ImplGetByte();               // pad byte > nil
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
                            mpAcc->SetPixel ( y, x, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                    }
                    break;
            }
            break;

        default:
            mbStatus = sal_False;
            break;
    }
    return mbStatus;
}

//----------------------------------------------------------------------------

sal_uInt8 RASReader::ImplGetByte()
{
    sal_uInt8 nRetVal;
    if ( mnType != RAS_TYPE_BYTE_ENCODED )
    {
        m_rRAS >> nRetVal;
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
            m_rRAS >> nRetVal;
            if ( nRetVal != 0x80 )
                return nRetVal;
            m_rRAS >> nRetVal;
            if ( nRetVal == 0 )
                return 0x80;
            mnRepCount = nRetVal    ;
            m_rRAS >> mnRepVal;
            return mnRepVal;
        }
    }
}

//================== GraphicImport - die exportierte Funktion ================

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI
GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, sal_Bool)
{
    RASReader aRASReader(rStream);

    return aRASReader.ReadRAS(rGraphic );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
