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


#include "generic/glyphcache.hxx"
#include <string.h>


RawBitmap::RawBitmap()
: mnAllocated(0)
{}


RawBitmap::~RawBitmap()
{}


// used by 90 and 270 degree rotations on 8 bit deep bitmaps
static void ImplRotate8_90( unsigned char* p1, const unsigned char* p2,
    int xmax, int ymax, int dx, int dy, int nPad )
{
    for( int y = ymax; --y >= 0; p2 += dy )
    {
        for( int x = xmax; --x >= 0; p2 += dx )
            *(p1++) = *p2;
        for( int i = nPad; --i >= 0; )
            *(p1++) = 0;
    }
}


// used by inplace 180 degree rotation on 8 bit deep bitmaps
static void ImplRotate8_180( unsigned char* p1, int xmax, int ymax, int nPad )
{
    unsigned char* p2 = p1 + ymax * (xmax + nPad);
    for( int y = ymax/2; --y >= 0; )
    {
        p2 -= nPad;
        for( int x = xmax; --x >= 0; )
        {
            unsigned char cTmp = *(--p2);
            *p2 = *p1;
            *(p1++) = cTmp;
        }
        p1 += nPad;
    }

    // reverse middle line
    p2 -= nPad;
    while( p1 < p2 )
    {
        unsigned char cTmp = *(--p2);
        *p2 = *p1;
        *(p1++) = cTmp;
    }
}


// used by 90 or 270 degree rotations on 1 bit deep bitmaps
static void ImplRotate1_90( unsigned char* p1, const unsigned char* p2,
    int xmax, int ymax, int dx, int nShift, int nDeltaShift, int nPad )
{
    for( int y = ymax; --y >= 0; )
    {
        unsigned nTemp = 1;
        const unsigned char* p20 = p2;
        for( int x = xmax; --x >= 0; p2 += dx )
        {
            // build bitwise and store when byte finished
           nTemp += nTemp + ((*p2 >> nShift) & 1);
            if( nTemp >= 0x100U )
            {
                *(p1++) = (unsigned char)nTemp;
                nTemp = 1;
            }
        }
        p2 = p20;

        // store left aligned remainder if needed
        if( nTemp > 1 )
        {
            for(; nTemp < 0x100U; nTemp += nTemp ) ;
            *(p1++) = (unsigned char)nTemp;
        }
        // pad scanline with zeroes
        for( int i = nPad; --i >= 0;)
            *(p1++) = 0;

        // increase/decrease shift, but keep bound inside 0 to 7
        nShift += nDeltaShift;
        if( nShift != (nShift & 7) )
            p2 -= nDeltaShift;
        nShift &= 7;
    }
}


// used by 180 degrees rotations on 1 bit deep bitmaps
static void ImplRotate1_180( unsigned char* p1, const unsigned char* p2,
    int xmax, int ymax, int nPad )
{
    --p2;
    for( int y = ymax; --y >= 0; )
    {
        p2 -= nPad;

        unsigned nTemp = 1;
        unsigned nInp = (0x100 + *p2) >> (-xmax & 7);
        for( int x = xmax; --x >= 0; )
        {
            // build bitwise and store when byte finished
            nTemp += nTemp + (nInp & 1);
            if( nTemp >= 0x100 )
            {
                *(p1++) = (unsigned char)nTemp;
                nTemp = 1;
            }
            // update input byte if needed (and available)
            if( (nInp >>= 1) <= 1 && ((y != 0) || (x != 0)) )
                nInp = 0x100 + *(--p2);
        }

        // store left aligned remainder if needed
        if( nTemp > 1 )
        {
            for(; nTemp < 0x100; nTemp += nTemp ) ;
            *(p1++) = (unsigned char)nTemp;
        }
        // scanline pad is already clean
        p1 += nPad;
    }
}


bool RawBitmap::Rotate( int nAngle )
{
    sal_uLong nNewScanlineSize = 0;
    sal_uLong nNewHeight = 0;
    sal_uLong nNewWidth = 0;

    // do inplace rotation or prepare double buffered rotation
    switch( nAngle )
    {
        case 0:     // nothing to do
        case 3600:
            return true;
        default:    // non rectangular angles not allowed
            return false;
        case 1800:  // rotate by 180 degrees
            mnXOffset = -(mnXOffset + mnWidth);
            mnYOffset = -(mnYOffset + mnHeight);
            if( mnBitCount == 8 )
            {
                ImplRotate8_180( mpBits.get(), mnWidth, mnHeight, mnScanlineSize-mnWidth );
                return true;
            }
            nNewWidth        = mnWidth;
            nNewHeight       = mnHeight;
            nNewScanlineSize = mnScanlineSize;
            break;
        case +900:  // left by 90 degrees
        case -900:
        case 2700:  // right by 90 degrees
            nNewWidth        = mnHeight;
            nNewHeight       = mnWidth;
            if( mnBitCount==1 )
                nNewScanlineSize = (nNewWidth + 7) / 8;
            else
                nNewScanlineSize = (nNewWidth + 3) & -4;
            break;
    }

    unsigned int nBufSize = nNewHeight * nNewScanlineSize;
    unsigned char* pBuf = new unsigned char[ nBufSize ];
    if( !pBuf )
        return false;

    memset( pBuf, 0, nBufSize );
    int i;

    // dispatch non-inplace rotations
    switch( nAngle )
    {
        case 1800:  // rotate by 180 degrees
            // we know we only need to deal with 1 bit depth
            ImplRotate1_180( pBuf, mpBits.get() + mnHeight * mnScanlineSize,
                mnWidth, mnHeight, mnScanlineSize - (mnWidth + 7) / 8 );
            break;
        case +900:  // rotate left by 90 degrees
            i = mnXOffset;
            mnXOffset = mnYOffset;
            mnYOffset = -nNewHeight - i;
            if( mnBitCount == 8 )
                ImplRotate8_90( pBuf, mpBits.get() + mnWidth - 1,
                    nNewWidth, nNewHeight, +mnScanlineSize, -1-mnHeight*mnScanlineSize,
                    nNewScanlineSize - nNewWidth );
            else
                ImplRotate1_90( pBuf, mpBits.get() + (mnWidth - 1) / 8,
                    nNewWidth, nNewHeight, +mnScanlineSize,
                    (-mnWidth & 7), +1, nNewScanlineSize - (nNewWidth + 7) / 8 );
            break;
        case 2700:  // rotate right by 90 degrees
        case -900:
            i = mnXOffset;
            mnXOffset = -(nNewWidth + mnYOffset);
            mnYOffset = i;
            if( mnBitCount == 8 )
                ImplRotate8_90( pBuf, mpBits.get() + mnScanlineSize * (mnHeight-1),
                    nNewWidth, nNewHeight, -mnScanlineSize, +1+mnHeight*mnScanlineSize,
                    nNewScanlineSize - nNewWidth );
            else
                ImplRotate1_90( pBuf, mpBits.get() + mnScanlineSize * (mnHeight-1),
                    nNewWidth, nNewHeight, -mnScanlineSize,
                    +7, -1, nNewScanlineSize - (nNewWidth + 7) / 8 );
            break;
    }

    mnWidth        = nNewWidth;
    mnHeight       = nNewHeight;
    mnScanlineSize = nNewScanlineSize;

    if( nBufSize < mnAllocated )
    {
        memcpy( mpBits.get(), pBuf, nBufSize );
        delete[] pBuf;
    }
    else
    {
        mpBits.reset(pBuf);
        mnAllocated = nBufSize;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
