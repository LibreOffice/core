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

#include <vcl/bitmapaccess.hxx>
#include <vcl/graphic.hxx>
#include "rgbtable.hxx"
#define _XPMPRIVATE
#include "xpmread.hxx"
#include <cstring>

XPMReader::XPMReader(SvStream& rStm)
    : mrIStm(rStm)
    , mpAcc(nullptr)
    , mpMaskAcc(nullptr)
    , mnLastPos(rStm.Tell())
    , mnWidth(0)
    , mnHeight(0)
    , mnColors(0)
    , mnCpp(0)
    , mbTransparent(false)
    , mbStatus(true)
    , mnStatus( 0 )
    , mnIdentifier(XPMIDENTIFIER)
    , mcThisByte(0)
    , mcLastByte(0)
    , mnTempAvail(0)
    , mpTempBuf(nullptr)
    , mpTempPtr(nullptr)
    , mpFastColorTable(nullptr)
    , mpColMap(nullptr)
    , mnStringSize(0)
    , mpStringBuf(nullptr)
    , mnParaSize(0)
    , mpPara(nullptr)
{
}

XPMReader::~XPMReader()
{
    if( mpAcc )
        Bitmap::ReleaseAccess( mpAcc );
}

ReadState XPMReader::ReadXPM( Graphic& rGraphic )
{
    ReadState   eReadState;
    sal_uInt8       cDummy;

    // check if we can real ALL
    mrIStm.Seek( STREAM_SEEK_TO_END );
    mrIStm.ReadUChar( cDummy );

    // if we could not read all
    // return and wait for new data
    if ( mrIStm.GetError() != ERRCODE_IO_PENDING )
    {
        mrIStm.Seek( mnLastPos );
        mbStatus = true;

        if ( mbStatus )
        {
            mpStringBuf = new sal_uInt8 [ XPMSTRINGBUF ];
            mpTempBuf = new sal_uInt8 [ XPMTEMPBUFSIZE ];

            if ( ( mbStatus = ImplGetString() ) )
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
            if ( mbStatus && mnWidth && mnHeight && mnColors && mnCpp )
            {
                mnIdentifier = XPMCOLORS;

                // mpColMap constitutes for all available
                // colour:   ( mnCpp )Byte(s)-> ASCII entry assigned to the colour
                //              1    Byte   -> 0xFF if colour is transparent
                //              3    Bytes  -> RGB value of the colour
                mpColMap = new sal_uInt8[ mnColors * ( 4 + mnCpp ) ];
                for ( sal_uLong i = 0; i < mnColors; i++ )
                {
                    if ( !ImplGetColor( i ) )
                    {
                        mbStatus = false;
                        break;
                    }
                }

                if ( mbStatus )
                {
                    // create a 24bit graphic when more as 256 colours present
                    sal_uInt16  nBits = 1;
                    if ( mnColors > 256 )
                        nBits = 24;
                    else if ( mnColors > 16 )
                        nBits = 8;
                    else if ( mnColors > 2 )
                        nBits = 4;
                    else
                        nBits = 1;

                    maBmp = Bitmap( Size( mnWidth, mnHeight ), nBits );
                    mpAcc = maBmp.AcquireWriteAccess();

                    // mbTransparent is TRUE if at least one colour is transparent
                    if ( mbTransparent )
                    {
                        maMaskBmp = Bitmap( Size( mnWidth, mnHeight ), 1 );
                        if ( ( mpMaskAcc = maMaskBmp.AcquireWriteAccess() ) == nullptr )
                            mbStatus = false;
                    }
                    if( mpAcc && mbStatus )
                    {
                        sal_uLong   i;
                        if ( mnColors <=256 )   // palette is only needed by using less than 257
                        {                       // colors

                            sal_uInt8*  pPtr = &mpColMap[mnCpp];

                            for ( i = 0; i < mnColors; i++ )
                            {
                                mpAcc->SetPaletteColor( (sal_uInt8)i, Color( pPtr[1], pPtr[2], pPtr[3] ) );
                                pPtr += ( mnCpp + 4 );
                            }
                            // using 2 charakters per pixel and less than 257 Colors we speed up
                            if ( mnCpp == 2 )   // by using a 64kb indexing table
                            {
                                const size_t nSize = 256 * 256;
                                mpFastColorTable = new sal_uInt8[nSize];
                                memset(mpFastColorTable, 0, nSize);
                                for ( pPtr = mpColMap, i = 0; i < mnColors; i++, pPtr += mnCpp + 4 )
                                {
                                    sal_uLong   j =  pPtr[ 0 ] << 8;
                                            j += pPtr[ 1 ];
                                    mpFastColorTable[ j ] = (sal_uInt8)i;
                                }
                            }
                        }
                        // now we get the bitmap data
                        mnIdentifier = XPMPIXELS;
                        for ( i = 0; i < mnHeight; i++ )
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

            delete[] mpFastColorTable;
            delete[] mpColMap;
            delete[] mpStringBuf;
            delete[] mpTempBuf;

        }
        if( mbStatus )
        {
            if ( mpMaskAcc )
            {
                Bitmap::ReleaseAccess ( mpMaskAcc), mpMaskAcc = nullptr;
                Bitmap::ReleaseAccess( mpAcc ), mpAcc = nullptr;
                rGraphic = Graphic( BitmapEx( maBmp, maMaskBmp ) );
            }
            else
            {
                Bitmap::ReleaseAccess( mpAcc ), mpAcc = nullptr;
                rGraphic = maBmp;
            }
            eReadState = XPMREAD_OK;
        }
        else
        {
            if ( mpMaskAcc ) Bitmap::ReleaseAccess ( mpMaskAcc), mpMaskAcc = nullptr;
            if ( mpAcc ) Bitmap::ReleaseAccess( mpAcc ), mpAcc = nullptr;
            eReadState = XPMREAD_ERROR;
        }
    }
    else
    {
        mrIStm.ResetError();
        eReadState = XPMREAD_NEED_MORE;
    }
    return eReadState;
}

// ImplGetColor returns various colour values,
// returns TRUE if various colours could be assigned

bool XPMReader::ImplGetColor( sal_uLong nNumb )
{
    sal_uInt8*  pString = mpStringBuf;
    sal_uInt8*  pPtr =  ( mpColMap + nNumb * ( 4 + mnCpp ) );
    bool    bStatus = ImplGetString();

    if ( bStatus )
    {
        for ( sal_uLong i = 0; i < mnCpp; i++ )
            *pPtr++ = *pString++;
        bStatus = ImplGetColSub ( pPtr );
    }
    return bStatus;
}

// ImpGetScanLine reads the string mpBufSize and writes the pixel in the
// Bitmap. Parameter nY is the horizontal position.

bool XPMReader::ImplGetScanLine( sal_uLong nY )
{
    bool    bStatus = ImplGetString();
    sal_uInt8*  pString = mpStringBuf;
    sal_uInt8*  pColor;
    BitmapColor     aWhite;
    BitmapColor     aBlack;

    if ( bStatus )
    {
        if ( mpMaskAcc )
        {
            aWhite = mpMaskAcc->GetBestMatchingColor( Color( COL_WHITE ) );
            aBlack = mpMaskAcc->GetBestMatchingColor( Color( COL_BLACK ) );
        }
        if ( mnStringSize != ( mnWidth * mnCpp ))
            bStatus = false;
        else
        {
            sal_uLong i, j;
            if ( mpFastColorTable )
            {
                for ( i = 0; i < mnWidth; i++ )
                {
                    j = (*pString++) << 8;
                    j += *pString++;
                    sal_uInt8 k = (sal_uInt8)mpFastColorTable[ j ];
                    mpAcc->SetPixel( nY, i, BitmapColor( (sal_uInt8)k ) );

                    if ( mpMaskAcc )
                        mpMaskAcc->SetPixel( nY, i,
                            ( mpColMap[ k * (mnCpp + 4) + mnCpp] ) ? aWhite : aBlack );
                }
            }
            else for ( i = 0; i < mnWidth; i++ )
            {
                pColor = mpColMap;
                for ( j = 0; j < mnColors; j++ )
                {
                    if ( ImplCompare( pString, pColor, mnCpp, XPMCASESENSITIVE ) )
                    {
                        if ( mnColors > 256 )
                            mpAcc->SetPixel( nY, i, Color ( pColor[3], pColor[4], pColor[5] ) );
                        else
                            mpAcc->SetPixel( nY, i, BitmapColor( (sal_uInt8) j ) );

                        if ( mpMaskAcc )
                            mpMaskAcc->SetPixel( nY, i, (
                                pColor[ mnCpp ] ) ? aWhite : aBlack );

                        break;
                    }
                    pColor += ( mnCpp + 4 );
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

bool XPMReader::ImplGetColSub( sal_uInt8* pDest )
{
    unsigned char cTransparent[] = "None";

    bool bColStatus = false;

    if ( ImplGetColKey( 'c' ) || ImplGetColKey( 'm' ) || ImplGetColKey( 'g' ) )
    {
        // hexentry for RGB or HSV color ?
        if ( *mpPara == '#' )
        {
                *pDest++ = 0;
                bColStatus = true;
                switch ( mnParaSize )
                {
                    case 25 :
                        ImplGetRGBHex ( pDest, 6 );
                        break;
                    case 13 :
                        ImplGetRGBHex ( pDest, 2 );
                        break;
                    case  7 :
                        ImplGetRGBHex ( pDest, 0 );
                        break;
                    default:
                        bColStatus = false;
                        break;
                }
        }
        // maybe pixel is transparent
        else if ( ImplCompare( &cTransparent[0], mpPara, 4 ))
        {
            *pDest++ = 0xff;
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
                        *pDest++ = 0;
                        *pDest++ = pRGBTable[ i ].red;
                        *pDest++ = pRGBTable[ i ].green;
                        *pDest++ = pRGBTable[ i ].blue;
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
// in a RGB value and writes this to pDest
// below formats should be contained in mpPara:
// if nAdd = 0 : '#12ab12'                    -> RGB = 0x12, 0xab, 0x12
//           2 : '#1234abcd1234'                  "      "     "     "
//           6 : '#12345678abcdefab12345678'      "      "     "     "

void XPMReader::ImplGetRGBHex( sal_uInt8* pDest,sal_uLong  nAdd )
{
    sal_uInt8*  pPtr = mpPara+1;
    sal_uInt8   nHex, nTemp;

    for ( sal_uLong i = 0; i < 3; i++ )
    {
        nHex = (*pPtr++) - '0';
        if ( nHex > 9 )
            nHex = ((nHex - 'A' + '0') & 7) + 10;

        nTemp = (*pPtr++) - '0';
        if ( nTemp > 9 )
            nTemp = ((nTemp - 'A' + '0') & 7) + 10;
        nHex = ( nHex << 4 ) + nTemp;

        pPtr += nAdd;
        *pDest++ = (sal_uInt8)nHex;
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

bool XPMReader::ImplCompare( sal_uInt8 const * pSource, sal_uInt8 const * pDest, sal_uLong nSize, sal_uLong nMode )
{
    bool bRet = true;

    if ( nMode == XPMCASENONSENSITIVE )
    {
        for ( sal_uLong i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
            {
                bRet = false;
                break;
            }
        }
    }
    else
    {
        for ( sal_uLong i = 0; i < nSize; i++ )
        {
            if ( pSource[i] != pDest[i] )
            {
                bRet = false;
                break;
            }
        }
    }
    return bRet;
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
    return ( ( nCount == nNumb ) && ( mpPara ) );
}

// The next string is read and stored in mpStringBuf (terminated with 0);
// mnStringSize contains the size of the string read.
// Comments like '//' and '/*....*/' are skipped.

bool XPMReader::ImplGetString()
{
    sal_uInt8       sID[] = "/* XPM */";
    sal_uInt8*      pString = mpStringBuf;

    mnStringSize = 0;
    mpStringBuf[0] = 0;

    while( mbStatus && ( mnStatus != XPMFINISHED ) )
    {
        if ( mnTempAvail == 0 )
        {
            mnTempAvail = mrIStm.Read( mpTempBuf, XPMTEMPBUFSIZE );
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

// - ImportXPM -

VCL_DLLPUBLIC bool ImportXPM( SvStream& rStm, Graphic& rGraphic )
{
    XPMReader*  pXPMReader = static_cast<XPMReader*>(rGraphic.GetContext());
    ReadState   eReadState;
    bool        bRet = true;

    if( !pXPMReader )
        pXPMReader = new XPMReader( rStm );

    rGraphic.SetContext( nullptr );
    eReadState = pXPMReader->ReadXPM( rGraphic );

    if( eReadState == XPMREAD_ERROR )
    {
        bRet = false;
        delete pXPMReader;
    }
    else if( eReadState == XPMREAD_OK )
        delete pXPMReader;
    else
        rGraphic.SetContext( pXPMReader );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
