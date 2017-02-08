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

#include <unistd.h>
#include <cstdio>
#include <cstring>

#include <bmp.hxx>

#include <X11_selection.hxx>
#include <unx/x11/xlimits.hxx>

#include <sal/macros.h>
#include <tools/stream.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>

using namespace x11;

/*
 *  helper functions
 */

inline void writeLE( sal_uInt16 nNumber, sal_uInt8* pBuffer )
{
    pBuffer[ 0 ] = (nNumber & 0xff);
    pBuffer[ 1 ] = ((nNumber>>8)&0xff);
}

inline void writeLE( sal_uInt32 nNumber, sal_uInt8* pBuffer )
{
    pBuffer[ 0 ] = (nNumber & 0xff);
    pBuffer[ 1 ] = ((nNumber>>8)&0xff);
    pBuffer[ 2 ] = ((nNumber>>16)&0xff);
    pBuffer[ 3 ] = ((nNumber>>24)&0xff);
}

inline sal_uInt16 readLE16( const sal_uInt8* pBuffer )
{
    //This is untainted data which comes from a controlled source
    //so, using a byte-swapping pattern which coverity doesn't
    //detect as such
    //http://security.coverity.com/blog/2014/Apr/on-detecting-heartbleed-with-static-analysis.html
    sal_uInt16 v = pBuffer[1]; v <<= 8;
    v |= pBuffer[0];
    return v;
}

inline sal_uInt32 readLE32( const sal_uInt8* pBuffer )
{
    //This is untainted data which comes from a controlled source
    //so, using a byte-swapping pattern which coverity doesn't
    //detect as such
    //http://security.coverity.com/blog/2014/Apr/on-detecting-heartbleed-with-static-analysis.html
    sal_uInt32 v = pBuffer[3]; v <<= 8;
    v |= pBuffer[2]; v <<= 8;
    v |= pBuffer[1]; v <<= 8;
    v |= pBuffer[0];
    return v;
}

/*
 * scanline helpers
 */

inline void X11_writeScanlinePixel( unsigned long nColor, sal_uInt8* pScanline, int depth, int x )
{
    switch( depth )
    {
        case 1:
            pScanline[ x/8 ] &= ~(1 << (x&7));
            pScanline[ x/8 ] |= ((nColor & 1) << (x&7));
            break;
        case 4:
            pScanline[ x/2 ] &= ((x&1) ? 0x0f : 0xf0);
            pScanline[ x/2 ] |= ((x&1) ? (nColor & 0x0f) : ((nColor & 0x0f) << 4));
            break;
        default:
        case 8:
            pScanline[ x ] = (nColor & 0xff);
            break;
    }
}

static sal_uInt8* X11_getPaletteBmpFromImage(
                                             Display* pDisplay,
                                             XImage* pImage,
                                             Colormap aColormap,
                                             sal_Int32& rOutSize
                                             )
{
    sal_uInt32 nColors = 0;

    rOutSize = 0;

    sal_uInt8* pBuffer = nullptr;
    sal_uInt32 nHeaderSize, nScanlineSize;
    sal_uInt16 nBitCount;
    // determine header and scanline size
    switch( pImage->depth )
    {
        case 1:
            nHeaderSize = 64;
            nScanlineSize = (pImage->width+31)/32;
            nBitCount = 1;
            break;
        case 4:
            nHeaderSize = 72;
            nScanlineSize = (pImage->width+1)/2;
            nBitCount = 4;
            break;
        default:
        case 8:
            nHeaderSize = 1084;
            nScanlineSize = pImage->width;
            nBitCount = 8;
            break;
    }
    // adjust scan lines to begin on %4 boundaries
    if( nScanlineSize & 3 )
    {
        nScanlineSize &= 0xfffffffc;
        nScanlineSize += 4;
    }

    // allocate buffer to hold header and scanlines, initialize to zero
    rOutSize = nHeaderSize + nScanlineSize*pImage->height;
    pBuffer = static_cast<sal_uInt8*>(rtl_allocateZeroMemory( rOutSize ));
    for( int y = 0; y < pImage->height; y++ )
    {
        sal_uInt8* pScanline = pBuffer + nHeaderSize + (pImage->height-1-y)*nScanlineSize;
        for( int x = 0; x < pImage->width; x++ )
        {
            unsigned long nPixel = XGetPixel( pImage, x, y );
            if( nPixel >= nColors )
                nColors = nPixel+1;
            X11_writeScanlinePixel( nPixel, pScanline, pImage->depth, x );
        }
    }

    // fill in header fields
    pBuffer[ 0 ] = 'B';
    pBuffer[ 1 ] = 'M';

    writeLE( nHeaderSize, pBuffer+10 );
    writeLE( (sal_uInt32)40, pBuffer+14 );
    writeLE( (sal_uInt32)pImage->width, pBuffer+18 );
    writeLE( (sal_uInt32)pImage->height, pBuffer+22 );
    writeLE( (sal_uInt16)1, pBuffer+26 );
    writeLE( nBitCount, pBuffer+28 );
    writeLE( (sal_uInt32)(DisplayWidth(pDisplay,DefaultScreen(pDisplay))*1000/DisplayWidthMM(pDisplay,DefaultScreen(pDisplay))), pBuffer+38);
    writeLE( (sal_uInt32)(DisplayHeight(pDisplay,DefaultScreen(pDisplay))*1000/DisplayHeightMM(pDisplay,DefaultScreen(pDisplay))), pBuffer+42);
    writeLE( nColors, pBuffer+46 );
    writeLE( nColors, pBuffer+50 );

    XColor aColors[256];
    if( nColors > (1U << nBitCount) ) // paranoia
        nColors = (1U << nBitCount);
    for( unsigned long nPixel = 0; nPixel < nColors; nPixel++ )
    {
        aColors[nPixel].flags = DoRed | DoGreen | DoBlue;
        aColors[nPixel].pixel = nPixel;
    }
    XQueryColors( pDisplay, aColormap, aColors, nColors );
    for( sal_uInt32 i = 0; i < nColors; i++ )
    {
        pBuffer[ 54 + i*4 ] = (sal_uInt8)(aColors[i].blue >> 8);
        pBuffer[ 55 + i*4 ] = (sal_uInt8)(aColors[i].green >> 8);
        pBuffer[ 56 + i*4 ] = (sal_uInt8)(aColors[i].red >> 8);
    }

    // done

    return pBuffer;
}

inline unsigned long doRightShift( unsigned long nValue, int nShift )
{
    return (nShift > 0) ? (nValue >> nShift) : (nValue << (-nShift));
}

inline unsigned long doLeftShift( unsigned long nValue, int nShift )
{
    return (nShift > 0) ? (nValue << nShift) : (nValue >> (-nShift));
}

static void getShift( unsigned long nMask, int& rShift, int& rSigBits, int& rShift2 )
{
    unsigned long nUseMask = nMask;
    rShift = 0;
    while( nMask & 0xffffff00 )
    {
        rShift++;
        nMask >>= 1;
    }
    if( rShift == 0 )
        while( ! (nMask & 0x00000080) )
        {
            rShift--;
            nMask <<= 1;
        }

    int nRotate = sizeof(unsigned long)*8 - rShift;
    rSigBits = 0;
    nMask = doRightShift( nUseMask, rShift) ;
    while( nRotate-- )
    {
        if( nMask & 1 )
            rSigBits++;
        nMask >>= 1;
    }

    rShift2 = 0;
    if( rSigBits < 8 )
        rShift2 = 8-rSigBits;
}

static sal_uInt8* X11_getTCBmpFromImage(
                                             Display* pDisplay,
                                             XImage* pImage,
                                             sal_Int32& rOutSize,
                                             int nScreenNo
                                             )
{
    // get masks from visual info (guesswork)
    XVisualInfo aVInfo;
    if( ! XMatchVisualInfo( pDisplay, nScreenNo, pImage->depth, TrueColor, &aVInfo ) )
        return nullptr;

    rOutSize = 0;

    sal_uInt8* pBuffer = nullptr;
    sal_uInt32 nHeaderSize = 60;
    sal_uInt32 nScanlineSize = pImage->width*3;

    // adjust scan lines to begin on %4 boundaries
    if( nScanlineSize & 3 )
    {
        nScanlineSize &= 0xfffffffc;
        nScanlineSize += 4;
    }
    int nRedShift, nRedSig, nRedShift2 = 0;
    getShift( aVInfo.red_mask, nRedShift, nRedSig, nRedShift2 );
    int nGreenShift, nGreenSig, nGreenShift2 = 0;
    getShift( aVInfo.green_mask, nGreenShift, nGreenSig, nGreenShift2 );
    int nBlueShift, nBlueSig, nBlueShift2 = 0;
    getShift( aVInfo.blue_mask, nBlueShift, nBlueSig, nBlueShift2 );

    // allocate buffer to hold header and scanlines, initialize to zero
    rOutSize = nHeaderSize + nScanlineSize*pImage->height;
    pBuffer = static_cast<sal_uInt8*>(rtl_allocateZeroMemory( rOutSize ));
    for( int y = 0; y < pImage->height; y++ )
    {
        sal_uInt8* pScanline = pBuffer + nHeaderSize + (pImage->height-1-y)*nScanlineSize;
        for( int x = 0; x < pImage->width; x++ )
        {
            unsigned long nPixel = XGetPixel( pImage, x, y );

            sal_uInt8 nValue = (sal_uInt8)doRightShift( nPixel&aVInfo.blue_mask, nBlueShift);
            if( nBlueShift2 )
                nValue |= (nValue >> nBlueShift2 );
            *pScanline++ = nValue;

            nValue = (sal_uInt8)doRightShift( nPixel&aVInfo.green_mask, nGreenShift);
            if( nGreenShift2 )
                nValue |= (nValue >> nGreenShift2 );
            *pScanline++ = nValue;

            nValue = (sal_uInt8)doRightShift( nPixel&aVInfo.red_mask, nRedShift);
            if( nRedShift2 )
                nValue |= (nValue >> nRedShift2 );
            *pScanline++ = nValue;
        }
    }

    // fill in header fields
    pBuffer[  0 ] = 'B';
    pBuffer[  1 ] = 'M';

    writeLE( nHeaderSize, pBuffer+10 );
    writeLE( (sal_uInt32)40, pBuffer+14 );
    writeLE( (sal_uInt32)pImage->width, pBuffer+18 );
    writeLE( (sal_uInt32)pImage->height, pBuffer+22 );
    writeLE( (sal_uInt16)1, pBuffer+26 );
    writeLE( (sal_uInt16)24, pBuffer+28 );
    writeLE( (sal_uInt32)(DisplayWidth(pDisplay,DefaultScreen(pDisplay))*1000/DisplayWidthMM(pDisplay,DefaultScreen(pDisplay))), pBuffer+38);
    writeLE( (sal_uInt32)(DisplayHeight(pDisplay,DefaultScreen(pDisplay))*1000/DisplayHeightMM(pDisplay,DefaultScreen(pDisplay))), pBuffer+42);

    // done

    return pBuffer;
}

sal_uInt8* x11::X11_getBmpFromPixmap(
                                Display* pDisplay,
                                Drawable aDrawable,
                                Colormap aColormap,
                                sal_Int32& rOutSize
                                )
{
    // get geometry of drawable
    ::Window aRoot;
    int x,y;
    unsigned int w, h, bw, d;
    XGetGeometry( pDisplay, aDrawable, &aRoot, &x, &y, &w, &h, &bw, &d );

    // find which screen we are on
    int nScreenNo = ScreenCount( pDisplay );
    while( nScreenNo-- )
    {
        if( RootWindow( pDisplay, nScreenNo ) == aRoot )
            break;
    }
    if( nScreenNo < 0 )
        return nullptr;

    if( aColormap == None )
        aColormap = DefaultColormap( pDisplay, nScreenNo );

    // get the image
    XImage* pImage = XGetImage( pDisplay, aDrawable, 0, 0, w, h, AllPlanes, ZPixmap );
    if( ! pImage )
        return nullptr;

    sal_uInt8* pBmp = d <= 8 ?
        X11_getPaletteBmpFromImage( pDisplay, pImage, aColormap, rOutSize ) :
        X11_getTCBmpFromImage( pDisplay, pImage, rOutSize, nScreenNo );
    XDestroyImage( pImage );

    return pBmp;
}

void x11::X11_freeBmp( sal_uInt8* pBmp )
{
    rtl_freeMemory( pBmp );
}

/*
 *  PixmapHolder
 */

PixmapHolder::PixmapHolder( Display* pDisplay )
    : m_pDisplay(pDisplay)
    , m_aColormap(None)
    , m_aPixmap(None)
    , m_aBitmap(None)
    , m_nRedShift(0)
    , m_nRedShift2(0)
    , m_nGreenShift(0)
    , m_nGreenShift2(0)
    , m_nBlueShift(0)
    , m_nBlueShift2(0)
    , m_nBlueShift2Mask(0)
    , m_nRedShift2Mask(0)
    , m_nGreenShift2Mask(0)
{
    /*  try to get a 24 bit true color visual, if that fails,
     *  revert to default visual
     */
    if( ! XMatchVisualInfo( m_pDisplay, DefaultScreen( m_pDisplay ), 24, TrueColor, &m_aInfo ) )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "PixmapHolder reverting to default visual\n" );
#endif
        Visual* pVisual     = DefaultVisual( m_pDisplay, DefaultScreen( m_pDisplay ) );
        m_aInfo.screen      = DefaultScreen( m_pDisplay );
        m_aInfo.visual      = pVisual;
        m_aInfo.visualid    = pVisual->visualid;
        m_aInfo.c_class     = pVisual->c_class;
        m_aInfo.red_mask    = pVisual->red_mask;
        m_aInfo.green_mask  = pVisual->green_mask;
        m_aInfo.blue_mask   = pVisual->blue_mask;
        m_aInfo.depth       = DefaultDepth( m_pDisplay, m_aInfo.screen );
    }
    m_aColormap         = DefaultColormap( m_pDisplay, m_aInfo.screen );
#if OSL_DEBUG_LEVEL > 1
    static const char* pClasses[] =
        { "StaticGray", "GrayScale", "StaticColor", "PseudoColor", "TrueColor", "DirectColor" };
    fprintf( stderr, "PixmapHolder visual: id = 0x%lx, class = %s (%d), depth=%d; color map = 0x%lx\n",
             m_aInfo.visualid,
             (m_aInfo.c_class >= 0 && unsigned(m_aInfo.c_class) < SAL_N_ELEMENTS(pClasses)) ? pClasses[m_aInfo.c_class] : "<unknown>",
             m_aInfo.c_class,
             m_aInfo.depth,
             m_aColormap  );
#endif
    if( m_aInfo.c_class == TrueColor )
    {
        int nRedSig, nGreenSig, nBlueSig;
        getShift( m_aInfo.red_mask, m_nRedShift, nRedSig, m_nRedShift2 );
        getShift( m_aInfo.green_mask, m_nGreenShift, nGreenSig, m_nGreenShift2 );
        getShift( m_aInfo.blue_mask, m_nBlueShift, nBlueSig, m_nBlueShift2 );

        m_nBlueShift2Mask = m_nBlueShift2 ? ~((unsigned long)((1<<m_nBlueShift2)-1)) : ~0L;
        m_nGreenShift2Mask = m_nGreenShift2 ? ~((unsigned long)((1<<m_nGreenShift2)-1)) : ~0L;
        m_nRedShift2Mask = m_nRedShift2 ? ~((unsigned long)((1<<m_nRedShift2)-1)) : ~0L;
    }
}

PixmapHolder::~PixmapHolder()
{
    if( m_aPixmap != None )
        XFreePixmap( m_pDisplay, m_aPixmap );
    if( m_aBitmap != None )
        XFreePixmap( m_pDisplay, m_aBitmap );
}

unsigned long PixmapHolder::getTCPixel( sal_uInt8 r, sal_uInt8 g, sal_uInt8 b ) const
{
    unsigned long nPixel = 0;
    unsigned long nValue = (unsigned long)b;
    nValue &= m_nBlueShift2Mask;
    nPixel |= doLeftShift( nValue, m_nBlueShift );

    nValue = (unsigned long)g;
    nValue &= m_nGreenShift2Mask;
    nPixel |= doLeftShift( nValue, m_nGreenShift );

    nValue = (unsigned long)r;
    nValue &= m_nRedShift2Mask;
    nPixel |= doLeftShift( nValue, m_nRedShift );

    return nPixel;
}

void PixmapHolder::setBitmapDataPalette( const sal_uInt8* pData, XImage* pImage )
{
    // setup palette
    XColor aPalette[256];

    sal_uInt32 nColors = readLE32( pData+32 );
    sal_uInt32 nWidth   = readLE32( pData+4 );
    sal_uInt32 nHeight  = readLE32( pData+8 );
    sal_uInt16 nDepth = readLE16( pData+14 );

    for( sal_uInt32 i = 0 ; i < nColors; i++ )
    {
        if( m_aInfo.c_class != TrueColor )
        {
            //This is untainted data which comes from a controlled source
            //so, using a byte-swapping pattern which coverity doesn't
            //detect as such
            //http://security.coverity.com/blog/2014/Apr/on-detecting-heartbleed-with-static-analysis.html
            aPalette[i].red = ((unsigned short)pData[42 + i*4]);
            aPalette[i].red <<= 8;
            aPalette[i].red |= ((unsigned short)pData[42 + i*4]);

            aPalette[i].green = ((unsigned short)pData[41 + i*4]);
            aPalette[i].green <<= 8;
            aPalette[i].green |= ((unsigned short)pData[41 + i*4]);

            aPalette[i].blue = ((unsigned short)pData[40 + i*4]);
            aPalette[i].blue <<= 8;
            aPalette[i].blue |= ((unsigned short)pData[40 + i*4]);
            XAllocColor( m_pDisplay, m_aColormap, aPalette+i );
        }
        else
            aPalette[i].pixel = getTCPixel( pData[42+i*4], pData[41+i*4], pData[40+i*4] );
    }
    const sal_uInt8* pBMData = pData + readLE32( pData ) + 4*nColors;

    sal_uInt32 nScanlineSize = 0;
    switch( nDepth )
    {
        case 1:
            nScanlineSize = (nWidth+31)/32;
            break;
        case 4:
            nScanlineSize = (nWidth+1)/2;
            break;
        case 8:
            nScanlineSize = nWidth;
            break;
    }
    // adjust scan lines to begin on %4 boundaries
    if( nScanlineSize & 3 )
    {
        nScanlineSize &= 0xfffffffc;
        nScanlineSize += 4;
    }

    // allocate buffer to hold header and scanlines, initialize to zero
    for( unsigned int y = 0; y < nHeight; y++ )
    {
        const sal_uInt8* pScanline = pBMData + (nHeight-1-y)*nScanlineSize;
        for( unsigned int x = 0; x < nWidth; x++ )
        {
            int nCol = 0;
            switch( nDepth )
            {
                case 1: nCol = (pScanline[ x/8 ] & (0x80 >> (x&7))) != 0 ? 0 : 1; break;
                case 4:
                    if( x & 1 )
                        nCol = (int)(pScanline[ x/2 ] >> 4);
                    else
                        nCol = (int)(pScanline[ x/2 ] & 0x0f);
                    break;
                case 8: nCol = (int)pScanline[x];
            }
            XPutPixel( pImage, x, y, aPalette[nCol].pixel );
        }
    }
}

void PixmapHolder::setBitmapDataTCDither( const sal_uInt8* pData, XImage* pImage )
{
    XColor aPalette[216];

    int nNonAllocs = 0;

    for( int r = 0; r < 6; r++ )
    {
        for( int g = 0; g < 6; g++ )
        {
            for( int b = 0; b < 6; b++ )
            {
                int i = r*36+g*6+b;
                aPalette[i].red     = r == 5 ? 0xffff : r*10922;
                aPalette[i].green   = g == 5 ? 0xffff : g*10922;
                aPalette[i].blue    = b == 5 ? 0xffff : b*10922;
                aPalette[i].pixel   = 0;
                if( ! XAllocColor( m_pDisplay, m_aColormap, aPalette+i ) )
                    nNonAllocs++;
            }
        }
    }

    if( nNonAllocs )
    {
        XColor aRealPalette[256];
        int nColors = 1 << m_aInfo.depth;
        int i;
        for( i = 0; i < nColors; i++ )
            aRealPalette[i].pixel = (unsigned long)i;
        XQueryColors( m_pDisplay, m_aColormap, aRealPalette, nColors );
        for( i = 0; i < nColors; i++ )
        {
            sal_uInt8 nIndex =
                36*(sal_uInt8)(aRealPalette[i].red/10923) +
                6*(sal_uInt8)(aRealPalette[i].green/10923) +
                (sal_uInt8)(aRealPalette[i].blue/10923);
            if( aPalette[nIndex].pixel == 0 )
                aPalette[nIndex] = aRealPalette[i];
        }
    }

    sal_uInt32 nWidth   = readLE32( pData+4 );
    sal_uInt32 nHeight  = readLE32( pData+8 );

    const sal_uInt8* pBMData = pData + readLE32( pData );
    sal_uInt32 nScanlineSize = nWidth*3;
    // adjust scan lines to begin on %4 boundaries
    if( nScanlineSize & 3 )
    {
        nScanlineSize &= 0xfffffffc;
        nScanlineSize += 4;
    }

    for( int y = 0; y < (int)nHeight; y++ )
    {
        const sal_uInt8* pScanline = pBMData + (nHeight-1-(sal_uInt32)y)*nScanlineSize;
        for( int x = 0; x < (int)nWidth; x++ )
        {
            sal_uInt8 b = pScanline[3*x];
            sal_uInt8 g = pScanline[3*x+1];
            sal_uInt8 r = pScanline[3*x+2];
            sal_uInt8 i = 36*(r/43) + 6*(g/43) + (b/43);

            XPutPixel( pImage, x, y, aPalette[ i ].pixel );
        }
    }
}

void PixmapHolder::setBitmapDataTC( const sal_uInt8* pData, XImage* pImage )
{
    sal_uInt32 nWidth   = readLE32( pData+4 );
    sal_uInt32 nHeight  = readLE32( pData+8 );

    if (!nWidth || !nHeight)
        return;

    const sal_uInt8* pBMData = pData + readLE32( pData );
    sal_uInt32 nScanlineSize = nWidth*3;
    // adjust scan lines to begin on %4 boundaries
    if( nScanlineSize & 3 )
    {
        nScanlineSize &= 0xfffffffc;
        nScanlineSize += 4;
    }

    for( int y = 0; y < (int)nHeight; y++ )
    {
        const sal_uInt8* pScanline = pBMData + (nHeight-1-(sal_uInt32)y)*nScanlineSize;
        for( int x = 0; x < (int)nWidth; x++ )
        {
            unsigned long nPixel = getTCPixel( pScanline[3*x+2], pScanline[3*x+1], pScanline[3*x] );
            XPutPixel( pImage, x, y, nPixel );
        }
    }
}

bool PixmapHolder::needsConversion( const sal_uInt8* pData )
{
    if( pData[0] != 'B' || pData[1] != 'M' )
        return true;

    pData = pData+14;
    sal_uInt32 nDepth = readLE32( pData+14 );
    if(  nDepth == 24 )
    {
        if( m_aInfo.c_class != TrueColor )
            return true;
    }
    else if( nDepth != (sal_uInt32)m_aInfo.depth )
    {
        if( m_aInfo.c_class != TrueColor )
            return true;
    }

    return false;
}

Pixmap PixmapHolder::setBitmapData( const sal_uInt8* pData )
{
    if( pData[0] != 'B' || pData[1] != 'M' )
        return None;

    pData = pData+14;

    // reject compressed data
    if( readLE32( pData + 16 ) != 0 )
        return None;

    sal_uInt32 nWidth   = readLE32( pData+4 );
    sal_uInt32 nHeight  = readLE32( pData+8 );

    if( m_aPixmap != None )
    {
        XFreePixmap( m_pDisplay, m_aPixmap );
        m_aPixmap = None;
    }
    if( m_aBitmap != None )
    {
        XFreePixmap( m_pDisplay, m_aBitmap );
        m_aBitmap = None;
    }

    m_aPixmap = limitXCreatePixmap( m_pDisplay,
                               RootWindow( m_pDisplay, m_aInfo.screen ),
                               nWidth, nHeight, m_aInfo.depth );

    if( m_aPixmap != None )
    {
        XImage aImage;
        aImage.width            = (int)nWidth;
        aImage.height           = (int)nHeight;
        aImage.xoffset          = 0;
        aImage.format           = ZPixmap;
        aImage.data             = nullptr;
        aImage.byte_order       = ImageByteOrder( m_pDisplay );
        aImage.bitmap_unit      = BitmapUnit( m_pDisplay );
        aImage.bitmap_bit_order = BitmapBitOrder( m_pDisplay );
        aImage.bitmap_pad       = BitmapPad( m_pDisplay );
        aImage.depth            = m_aInfo.depth;
        aImage.red_mask         = m_aInfo.red_mask;
        aImage.green_mask       = m_aInfo.green_mask;
        aImage.blue_mask        = m_aInfo.blue_mask;
        aImage.bytes_per_line   = 0; // filled in by XInitImage
        if( m_aInfo.depth <= 8 )
            aImage.bits_per_pixel = m_aInfo.depth;
        else
            aImage.bits_per_pixel = 8*((m_aInfo.depth+7)/8);
        aImage.obdata           = nullptr;

        XInitImage( &aImage );
        aImage.data = static_cast<char*>(rtl_allocateMemory( nHeight*aImage.bytes_per_line ));

        if( readLE32( pData+14 ) == 24 )
        {
            if( m_aInfo.c_class == TrueColor )
                setBitmapDataTC( pData, &aImage );
            else
                setBitmapDataTCDither( pData, &aImage );
        }
        else
            setBitmapDataPalette( pData, &aImage );

        // put the image
        XPutImage( m_pDisplay,
                   m_aPixmap,
                   DefaultGC( m_pDisplay, m_aInfo.screen ),
                   &aImage,
                   0, 0,
                   0, 0,
                   nWidth, nHeight );

        // clean up
        rtl_freeMemory( aImage.data );

        // prepare bitmap (mask)
        m_aBitmap = limitXCreatePixmap( m_pDisplay,
                                   RootWindow( m_pDisplay, m_aInfo.screen ),
                                   nWidth, nHeight, 1 );
        XGCValues aVal;
        aVal.function = GXcopy;
        aVal.foreground = 0xffffffff;
        GC aGC = XCreateGC( m_pDisplay, m_aBitmap, GCFunction | GCForeground, &aVal );
        XFillRectangle( m_pDisplay, m_aBitmap, aGC, 0, 0, nWidth, nHeight );
        XFreeGC( m_pDisplay, aGC );
    }

    return m_aPixmap;
}

css::uno::Sequence<sal_Int8> x11::convertBitmapDepth(
    css::uno::Sequence<sal_Int8> const & data, int depth)
{
    if (depth < 4) {
        depth = 1;
    } else if (depth < 8) {
        depth = 4;
    } else if (depth > 8 && depth < 24) {
        depth = 24;
    }
    SolarMutexGuard g;
    SvMemoryStream in(
        const_cast<sal_Int8 *>(data.getConstArray()), data.getLength(),
        StreamMode::READ);
    Bitmap bm;
    ReadDIB(bm, in, true);
    if (bm.GetBitCount() == 24 && depth <= 8) {
        bm.Dither(BmpDitherFlags::Floyd);
    }
    if (bm.GetBitCount() != depth) {
        switch (depth) {
        case 1:
            bm.Convert(BmpConversion::N1BitThreshold);
            break;
        case 4:
            bm.ReduceColors(1<<4);
            break;
        case 8:
            bm.ReduceColors(1<<8);
            break;
        case 24:
            bm.Convert(BmpConversion::N24Bit);
            break;
        }
    }
    SvMemoryStream out;
    WriteDIB(bm, out, false, true);
    return css::uno::Sequence<sal_Int8>(
        static_cast<sal_Int8 const *>(out.GetData()), out.GetEndOfData());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
