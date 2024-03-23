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

#include <filter/XpmReader.hxx>

#include <vcl/graph.hxx>
#include <tools/stream.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <graphic/GraphicReader.hxx>

#include "rgbtable.hxx"

#include <cstring>
#include <array>
#include <map>

#define XPMTEMPBUFSIZE      0x00008000
#define XPMSTRINGBUF        0x00008000

#define XPMIDENTIFIER       0x00000001          // mnIdentifier includes one of the six phases
#define XPMDEFINITION       0x00000002          // the XPM format consists of
#define XPMVALUES           0x00000003
#define XPMCOLORS           0x00000004
#define XPMPIXELS           0x00000005
#define XPMEXTENSIONS       0x00000006
#define XPMENDEXT           0x00000007

#define XPMREMARK           0x00000001          // defines used by mnStatus
#define XPMDOUBLE           0x00000002
#define XPMSTRING           0x00000004
#define XPMFINISHED         0x00000008

namespace {

enum ReadState
{
    XPMREAD_OK,
    XPMREAD_ERROR,
    XPMREAD_NEED_MORE
};

}

class BitmapWriteAccess;
class Graphic;

namespace
{

class XPMReader : public GraphicReader
{
private:
    SvStream& mrStream;
    Bitmap maBitmap;
    BitmapScopedWriteAccess mpWriterAccess;
    Bitmap maMaskBitmap;
    BitmapScopedWriteAccess mpMaskWriterAccess;
    sal_uInt64 mnLastPos;

    tools::Long mnWidth = 0;
    tools::Long mnHeight = 0;
    sal_uLong mnColors = 0;
    sal_uInt32 mnCpp = 0; // characters per pix
    bool mbTransparent = false;
    bool mbStatus  = true;
    sal_uLong mnStatus = 0;
    sal_uLong mnIdentifier = XPMIDENTIFIER;
    sal_uInt8 mcThisByte = 0;
    sal_uInt8 mcLastByte = 0;
    sal_uLong mnTempAvail = 0;
    sal_uInt8* mpTempBuf = nullptr;
    sal_uInt8* mpTempPtr = nullptr;
    // each key is ( mnCpp )Byte(s)-> ASCII entry assigned to the colour
    // each colordata is
    // 1 Byte   -> 0xFF if colour is transparent
    // 3 Bytes  -> RGB value of the colour
    typedef std::array<sal_uInt8, 4> ColorData;
    typedef std::map<OString, ColorData> ColorMap;
    ColorMap maColMap;
    sal_uLong mnStringSize = 0;
    sal_uInt8* mpStringBuf = nullptr;
    sal_uLong mnParaSize = 0;
    sal_uInt8* mpPara = nullptr;

    bool                    ImplGetString();
    bool                    ImplGetColor();
    bool                    ImplGetScanLine( sal_uLong );
    bool                    ImplGetColSub(ColorData &rDest);
    bool                    ImplGetColKey( sal_uInt8 );
    void                    ImplGetRGBHex(ColorData &rDest, sal_uLong);
    bool                    ImplGetPara( sal_uLong numb );
    static bool             ImplCompare(sal_uInt8 const *, sal_uInt8 const *, sal_uLong);
    sal_uLong               ImplGetULONG( sal_uLong nPara );

public:
    explicit XPMReader(SvStream& rStream);

    ReadState ReadXPM(Graphic& rGraphic);
};

}

XPMReader::XPMReader(SvStream& rStream)
    : mrStream(rStream)
    , mnLastPos(rStream.Tell())
{
}

ReadState XPMReader::ReadXPM(Graphic& rGraphic)
{
    ReadState   eReadState;
    sal_uInt8       cDummy;

    // check if we can real ALL
    mrStream.Seek( STREAM_SEEK_TO_END );
    mrStream.ReadUChar( cDummy );

    // if we could not read all
    // return and wait for new data
    if (mrStream.GetError() != ERRCODE_IO_PENDING)
    {
        mrStream.Seek( mnLastPos );
        mbStatus = true;

        if ( mbStatus )
        {
            mpStringBuf = new sal_uInt8 [ XPMSTRINGBUF ];
            mpTempBuf = new sal_uInt8 [ XPMTEMPBUFSIZE ];

            mbStatus = ImplGetString();
            if ( mbStatus )
            {
                mnIdentifier = XPMVALUES;           // fetch Bitmap information
                mnWidth = ImplGetULONG( 0 );
                mnHeight = ImplGetULONG( 1 );
                mnColors = ImplGetULONG( 2 );
                mnCpp = ImplGetULONG( 3 );
            }
            if ( mnColors > ( SAL_MAX_UINT32 / ( 4 + mnCpp ) ) )
                mbStatus = false;
            if ( ( mnWidth * mnCpp ) >= XPMSTRINGBUF )
                mbStatus = false;
            //xpms are a minimum of one character (one byte) per pixel, so if the file isn't
            //even that long, it's not all there
            if (mrStream.remainingSize() + mnTempAvail < static_cast<sal_uInt64>(mnWidth) * mnHeight)
                mbStatus = false;
            if ( mbStatus && mnWidth && mnHeight && mnColors && mnCpp )
            {
                mnIdentifier = XPMCOLORS;

                for (sal_uLong i = 0; i < mnColors; ++i)
                {
                    if (!ImplGetColor())
                    {
                        mbStatus = false;
                        break;
                    }
                }

                if ( mbStatus )
                {
                    // create a 24bit graphic when more as 256 colours present
                    auto ePixelFormat = vcl::PixelFormat::INVALID;
                    if ( mnColors > 256 )
                        ePixelFormat = vcl::PixelFormat::N24_BPP;
                    else
                        ePixelFormat = vcl::PixelFormat::N8_BPP;

                    maBitmap = Bitmap(Size(mnWidth, mnHeight), ePixelFormat);
                    mpWriterAccess = maBitmap;

                    // mbTransparent is TRUE if at least one colour is transparent
                    if ( mbTransparent )
                    {
                        maMaskBitmap = Bitmap(Size(mnWidth, mnHeight), vcl::PixelFormat::N8_BPP, &Bitmap::GetGreyPalette(256));
                        mpMaskWriterAccess = maMaskBitmap;
                        if (!mpMaskWriterAccess)
                            mbStatus = false;
                    }
                    if (mpWriterAccess && mbStatus)
                    {
                        if (mnColors <= 256)  // palette is only needed by using less than 257
                        {                     // colors
                            sal_uInt8 i = 0;
                            for (auto& elem : maColMap)
                            {
                                mpWriterAccess->SetPaletteColor(i, Color(elem.second[1], elem.second[2], elem.second[3]));
                                //reuse map entry, overwrite color with palette index
                                elem.second[1] = i;
                                i++;
                            }
                        }

                        // now we get the bitmap data
                        mnIdentifier = XPMPIXELS;
                        for (tools::Long i = 0; i < mnHeight; ++i)
                        {
                            if ( !ImplGetScanLine( i ) )
                            {
                                mbStatus = false;
                                break;
                            }
                        }
                        mnIdentifier = XPMEXTENSIONS;
                    }
                }
            }

            delete[] mpStringBuf;
            delete[] mpTempBuf;

        }
        if( mbStatus )
        {
            mpWriterAccess.reset();
            if (mpMaskWriterAccess)
            {
                mpMaskWriterAccess.reset();
                rGraphic = Graphic(BitmapEx(maBitmap, maMaskBitmap));
            }
            else
            {
                rGraphic = BitmapEx(maBitmap);
            }
            eReadState = XPMREAD_OK;
        }
        else
        {
            mpMaskWriterAccess.reset();
            mpWriterAccess.reset();

            eReadState = XPMREAD_ERROR;
        }
    }
    else
    {
        mrStream.ResetError();
        eReadState = XPMREAD_NEED_MORE;
    }
    return eReadState;
}

// ImplGetColor returns various colour values,
// returns TRUE if various colours could be assigned
bool XPMReader::ImplGetColor()
{
    sal_uInt8*  pString = mpStringBuf;
    if (!ImplGetString())
        return false;

    if (mnStringSize < mnCpp)
        return false;

    OString aKey(reinterpret_cast<char*>(pString), mnCpp);
    ColorData aValue{0};
    bool bStatus = ImplGetColSub(aValue);
    if (bStatus)
    {
        maColMap[aKey] = aValue;
    }
    return bStatus;
}

// ImpGetScanLine reads the string mpBufSize and writes the pixel in the
// Bitmap. Parameter nY is the horizontal position.
bool XPMReader::ImplGetScanLine( sal_uLong nY )
{
    bool    bStatus = ImplGetString();
    sal_uInt8*  pString = mpStringBuf;
    BitmapColor     aWhite;
    BitmapColor     aBlack;

    if ( bStatus )
    {
        if (mpMaskWriterAccess)
        {
            aWhite = mpMaskWriterAccess->GetBestMatchingColor( COL_WHITE );
            aBlack = mpMaskWriterAccess->GetBestMatchingColor( COL_BLACK );
        }
        if (mnStringSize != (sal_uLong(mnWidth) * mnCpp))
            bStatus = false;
        else
        {
            Scanline pScanline = mpWriterAccess->GetScanline(nY);
            Scanline pMaskScanline = mpMaskWriterAccess ? mpMaskWriterAccess->GetScanline(nY) : nullptr;
            for (tools::Long i = 0; i < mnWidth; ++i)
            {
                OString aKey(reinterpret_cast<char*>(pString), mnCpp);
                auto it = maColMap.find(aKey);
                if (it != maColMap.end())
                {
                    if (mnColors > 256)
                        mpWriterAccess->SetPixelOnData(pScanline, i, Color(it->second[1], it->second[2], it->second[3]));
                    else
                        mpWriterAccess->SetPixelOnData(pScanline, i, BitmapColor(it->second[1]));
                    if (pMaskScanline)
                        mpMaskWriterAccess->SetPixelOnData(pMaskScanline, i, it->second[0] ? aWhite : aBlack);
                }
                pString += mnCpp;
            }
        }
    }
    return bStatus;
}

// tries to determine a colour value from mpStringBuf
// if a colour was found the RGB value is written a pDest[1]..pDest[2]
// pDest[0] contains 0xFF if the colour is transparent otherwise 0

bool XPMReader::ImplGetColSub(ColorData& rDest)
{
    unsigned char cTransparent[] = "None";

    bool bColStatus = false;

    if ( ImplGetColKey( 'c' ) || ImplGetColKey( 'm' ) || ImplGetColKey( 'g' ) )
    {
        // hexentry for RGB or HSV color ?
        if (*mpPara == '#')
        {
            rDest[0] = 0;
            bColStatus = true;
            switch ( mnParaSize )
            {
                case 25 :
                    ImplGetRGBHex(rDest, 6);
                    break;
                case 13 :
                    ImplGetRGBHex(rDest, 2);
                    break;
                case  7 :
                    ImplGetRGBHex(rDest, 0);
                    break;
                default:
                    bColStatus = false;
                    break;
            }
        }
        // maybe pixel is transparent
        else if ( ImplCompare( &cTransparent[0], mpPara, 4 ))
        {
            rDest[0] = 0xff;
            bColStatus = true;
            mbTransparent = true;
        }
        // last we will try to get the colorname
        else if ( mnParaSize > 2 )  // name must enlarge the minimum size
        {
            sal_uLong i = 0;
            while ( true )
            {
                if ( pRGBTable[ i ].name == nullptr )
                    break;
                if ( std::strlen(pRGBTable[i].name) > mnParaSize &&
                        pRGBTable[ i ].name[ mnParaSize ] == 0 )
                {
                    if ( ImplCompare ( reinterpret_cast<unsigned char const *>(pRGBTable[ i ].name),
                            mpPara, mnParaSize ) )
                    {
                        bColStatus = true;
                        rDest[0] = 0;
                        rDest[1] = pRGBTable[i].red;
                        rDest[2] = pRGBTable[i].green;
                        rDest[3] = pRGBTable[i].blue;
                        break;
                    }
                }
                i++;
            }
        }
    }
    return bColStatus;
}

// ImplGetColKey searches string mpStringBuf for a parameter 'nKey'
// and returns a boolean. (if TRUE mpPara and mnParaSize will be set)

bool XPMReader::ImplGetColKey( sal_uInt8 nKey )
{
    sal_uInt8 nTemp, nPrev = ' ';

    if (mnStringSize < mnCpp + 1)
        return false;

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
        mpPara++;
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
    return mnParaSize != 0;
}

// ImplGetRGBHex translates the ASCII-Hexadecimalvalue belonging to mpPara
// in a RGB value and writes this to rDest
// below formats should be contained in mpPara:
// if nAdd = 0 : '#12ab12'                    -> RGB = 0x12, 0xab, 0x12
//           2 : '#1234abcd1234'                  "      "     "     "
//           6 : '#12345678abcdefab12345678'      "      "     "     "

void XPMReader::ImplGetRGBHex(ColorData& rDest, sal_uLong nAdd)
{
    sal_uInt8*  pPtr = mpPara+1;

    for (sal_uLong i = 1; i < 4; ++i)
    {
        sal_uInt8 nHex = (*pPtr++) - '0';
        if ( nHex > 9 )
            nHex = ((nHex - 'A' + '0') & 7) + 10;

        sal_uInt8 nTemp = (*pPtr++) - '0';
        if ( nTemp > 9 )
            nTemp = ((nTemp - 'A' + '0') & 7) + 10;
        nHex = ( nHex << 4 ) + nTemp;

        pPtr += nAdd;
        rDest[i] = nHex;
    }
}

// ImplGetUlong returns the value of a up to 6-digit long ASCII-decimal number.

sal_uLong XPMReader::ImplGetULONG( sal_uLong nPara )
{
    if ( ImplGetPara ( nPara ) )
    {
        sal_uLong nRetValue = 0;
        sal_uInt8* pPtr = mpPara;

        if ( ( mnParaSize > 6 ) || ( mnParaSize == 0 ) ) return 0;
        for ( sal_uLong i = 0; i < mnParaSize; i++ )
        {
            sal_uInt8 j = (*pPtr++) - 48;
            if ( j > 9 ) return 0;              // ascii is invalid
            nRetValue*=10;
            nRetValue+=j;
        }
        return nRetValue;
    }
    else return 0;
}

bool XPMReader::ImplCompare(sal_uInt8 const * pSource, sal_uInt8 const * pDest, sal_uLong nSize)
{
    for (sal_uLong i = 0; i < nSize; ++i)
    {
        if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
        {
            return false;
        }
    }
    return true;
}

// ImplGetPara tries to retrieve nNumb (0...x) parameters from mpStringBuf.
// Parameters are separated by spaces or tabs.
// If a parameter was found then the return value is TRUE and mpPara + mnParaSize
// are set.

bool XPMReader::ImplGetPara ( sal_uLong nNumb )
{
    sal_uInt8   nByte;
    sal_uLong   nSize = 0;
    sal_uInt8*  pPtr = mpStringBuf;
    sal_uLong   nCount = 0;

    if ( ( *pPtr != ' ' ) && ( *pPtr != 0x09 ) )
    {
        mpPara = pPtr;
        mnParaSize = 0;
        nCount = 0;
    }
    else
    {
        mpPara = nullptr;
        nCount = 0xffffffff;
    }

    while ( nSize < mnStringSize )
    {
        nByte = *pPtr;

        if ( mpPara )
        {
            if ( ( nByte == ' ' ) || ( nByte == 0x09 ) )
            {
                if ( nCount == nNumb )
                    break;
                else
                    mpPara = nullptr;
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
        nSize++;
        pPtr++;
    }
    return ( ( nCount == nNumb ) && mpPara );
}

// The next string is read and stored in mpStringBuf (terminated with 0);
// mnStringSize contains the size of the string read.
// Comments like '//' and '/*...*/' are skipped.

bool XPMReader::ImplGetString()
{
    sal_uInt8 const sID[] = "/* XPM */";
    sal_uInt8*      pString = mpStringBuf;

    mnStringSize = 0;
    mpStringBuf[0] = 0;

    while( mbStatus && ( mnStatus != XPMFINISHED ) )
    {
        if ( mnTempAvail == 0 )
        {
            mnTempAvail = mrStream.ReadBytes( mpTempBuf, XPMTEMPBUFSIZE );
            if ( mnTempAvail == 0 )
                break;

            mpTempPtr = mpTempBuf;

            if ( mnIdentifier == XPMIDENTIFIER )
            {
                if ( mnTempAvail <= 50 )
                {
                    mbStatus = false;   // file is too short to be a correct XPM format
                    break;
                }
                for ( int i = 0; i < 9; i++ )   // searching for "/* XPM */"
                    if ( *mpTempPtr++ != sID[i] )
                    {
                        mbStatus = false;
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
            if ( mnStringSize >= ( XPMSTRINGBUF - 1 ) )
            {
                mbStatus = false;
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


VCL_DLLPUBLIC bool ImportXPM( SvStream& rStm, Graphic& rGraphic )
{
    std::shared_ptr<GraphicReader> pContext = rGraphic.GetReaderContext();
    rGraphic.SetReaderContext(nullptr);
    XPMReader* pXPMReader = dynamic_cast<XPMReader*>( pContext.get() );
    if (!pXPMReader)
    {
        pContext = std::make_shared<XPMReader>( rStm );
        pXPMReader = static_cast<XPMReader*>( pContext.get() );
    }

    bool bRet = true;

    ReadState eReadState = pXPMReader->ReadXPM( rGraphic );

    if( eReadState == XPMREAD_ERROR )
    {
        bRet = false;
    }
    else if( eReadState == XPMREAD_NEED_MORE )
        rGraphic.SetReaderContext( pContext );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
