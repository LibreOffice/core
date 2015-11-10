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

#include <cstddef>
#include <limits>

#include <basebmp/scanlineformats.hxx>
#include <basebmp/color.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <tools/color.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/salbtype.hxx>

#include "bmpfast.hxx"
#include "quartz/salbmp.h"
#include "quartz/utils.h"

#ifdef MACOSX
#include "osx/saldata.hxx"
#else
#include "saldatabasic.hxx"
#endif

static const unsigned long k16BitRedColorMask   = 0x00007c00;
static const unsigned long k16BitGreenColorMask = 0x000003e0;
static const unsigned long k16BitBlueColorMask  = 0x0000001f;

static const unsigned long k32BitRedColorMask   = 0x00ff0000;
static const unsigned long k32BitGreenColorMask = 0x0000ff00;
static const unsigned long k32BitBlueColorMask  = 0x000000ff;

#if defined IOS && defined DBG_UTIL

#include <MobileCoreServices/UTCoreTypes.h>
#include <ImageIO/ImageIO.h>

static void writeImageToFile(CGImageRef image, const char *baseName)
{
    static bool bDoIt = getenv("DBG_WRITE_CGIMAGES");
    if (!bDoIt)
        return;

    static int counter = 0;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *path = [NSString stringWithFormat:@"%@/%s.%d.png", documentsDirectory, baseName, counter++];
    CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:path];
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
    CGImageDestinationAddImage(destination, image, nil);

    if (!CGImageDestinationFinalize(destination))
    {
        NSLog(@"Failed to write image to %@", path);
    }
    else
    {
        SAL_DEBUG("--- saved image " << baseName << " to " << [path UTF8String]);
    }

    CFRelease(destination);
}

#define DBG_WRITE_IMAGE(image, name) writeImageToFile(image, name)

#else

#define DBG_WRITE_IMAGE(image, name) /* empty */

#endif

static bool isValidBitCount( sal_uInt16 nBitCount )
{
    return (nBitCount == 1) || (nBitCount == 4) || (nBitCount == 8) ||
        (nBitCount == 16) || (nBitCount == 24) || (nBitCount == 32);
}

QuartzSalBitmap::QuartzSalBitmap()
  : mxGraphicContext( nullptr )
  , mxCachedImage( nullptr )
  , mnBits(0)
  , mnWidth(0)
  , mnHeight(0)
  , mnBytesPerRow(0)
{
}

QuartzSalBitmap::~QuartzSalBitmap()
{
    Destroy();
}

bool QuartzSalBitmap::Create( CGLayerRef xLayer, int nBitmapBits,
    int nX, int nY, int nWidth, int nHeight )
{
    DBG_ASSERT( xLayer, "QuartzSalBitmap::Create() from non-layered context" );

    // sanitize input parameters
    if( nX < 0 )
        nWidth += nX, nX = 0;

    if( nY < 0 )
        nHeight += nY, nY = 0;

    const CGSize aLayerSize = CGLayerGetSize( xLayer );
    SAL_INFO("vcl.cg", "CGLayerGetSize(" << xLayer << ") = " << aLayerSize );
    if( nWidth >= (int)aLayerSize.width - nX )
        nWidth = (int)aLayerSize.width - nX;

    if( nHeight >= (int)aLayerSize.height - nY )
        nHeight = (int)aLayerSize.height - nY;

    if( (nWidth < 0) || (nHeight < 0) )
        nWidth = nHeight = 0;

    // initialize properties
    mnWidth  = nWidth;
    mnHeight = nHeight;
    mnBits   = nBitmapBits ? nBitmapBits : 32;

    // initialize drawing context
    CreateContext();

    // copy layer content into the bitmap buffer
    const CGPoint aSrcPoint = { static_cast<CGFloat>(-nX), static_cast<CGFloat>(-nY) };
    if(mxGraphicContext) // remove warning
    {
        SAL_INFO("vcl.cg", "CGContextDrawLayerAtPoint(" << mxGraphicContext << "," << aSrcPoint << "," << xLayer << ")" );
        CGContextDrawLayerAtPoint( mxGraphicContext, aSrcPoint, xLayer );
    }
    return true;
}

bool QuartzSalBitmap::Create( const Size& rSize, sal_uInt16 nBits, const BitmapPalette& rBitmapPalette )
{
    if( !isValidBitCount( nBits ) )
        return false;

    maPalette = rBitmapPalette;
    mnBits = nBits;
    mnWidth = rSize.Width();
    mnHeight = rSize.Height();
    return AllocateUserData();
}

bool QuartzSalBitmap::Create( const SalBitmap& rSalBmp )
{
    return Create( rSalBmp, rSalBmp.GetBitCount() );
}

bool QuartzSalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    return Create( rSalBmp, pGraphics ? pGraphics->GetBitCount() : rSalBmp.GetBitCount() );
}

bool QuartzSalBitmap::Create( const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount )
{
    const QuartzSalBitmap& rSourceBitmap = static_cast<const QuartzSalBitmap&>(rSalBmp);

    if( isValidBitCount( nNewBitCount ) &&  rSourceBitmap.maUserBuffer.get() )
    {
        mnBits = nNewBitCount;
        mnWidth = rSourceBitmap.mnWidth;
        mnHeight = rSourceBitmap.mnHeight;
        maPalette = rSourceBitmap.maPalette;

        if( AllocateUserData() )
        {
            ConvertBitmapData( mnWidth, mnHeight, mnBits, mnBytesPerRow, maPalette,
                               maUserBuffer.get(), rSourceBitmap.mnBits,
                               rSourceBitmap.mnBytesPerRow, rSourceBitmap.maPalette,
                               rSourceBitmap.maUserBuffer.get() );
            return true;
        }
    }
    return false;
}

bool QuartzSalBitmap::Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& /*xBitmapCanvas*/,
                              Size& /*rSize*/, bool /*bMask*/ )
{
    return false;
}

void QuartzSalBitmap::Destroy()
{
    DestroyContext();
    maUserBuffer.reset();
}

void QuartzSalBitmap::DestroyContext()
{
    if( mxCachedImage )
    {
        SAL_INFO("vcl.cg", "CGImageRelease(" << mxCachedImage << ")" );
        CGImageRelease( mxCachedImage );
        mxCachedImage = nullptr;
    }

    if( mxGraphicContext )
    {
        SAL_INFO("vcl.cg", "CGContextRelease(" << mxGraphicContext << ")" );
        CGContextRelease( mxGraphicContext );
        mxGraphicContext = nullptr;
        maContextBuffer.reset();
    }
}

bool QuartzSalBitmap::CreateContext()
{
    DestroyContext();

    // prepare graphics context
    // convert image from user input if available
    const bool bSkipConversion = !maUserBuffer;
    if( bSkipConversion )
        AllocateUserData();

    // default to RGBA color space
    CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
    CGBitmapInfo aCGBmpInfo = kCGImageAlphaNoneSkipFirst;

    // convert data into something accepted by CGBitmapContextCreate()
    size_t bitsPerComponent = (mnBits == 16) ? 5 : 8;
    sal_uInt32 nContextBytesPerRow = mnBytesPerRow;
    if( (mnBits == 16) || (mnBits == 32) )
    {
        // no conversion needed for truecolor
        maContextBuffer = maUserBuffer;
    }
    else if( mnBits == 8 && maPalette.IsGreyPalette() )
    {
        // no conversion needed for grayscale
        maContextBuffer = maUserBuffer;
        aCGColorSpace = GetSalData()->mxGraySpace;
        aCGBmpInfo = kCGImageAlphaNone;
        bitsPerComponent = mnBits;
    }
    // TODO: is special handling for 1bit input buffers worth it?
    else
    {
        // convert user data to 32 bit
        nContextBytesPerRow = mnWidth << 2;
        try
        {
            maContextBuffer.reset( new sal_uInt8[ mnHeight * nContextBytesPerRow ] );
#ifdef DBG_UTIL
            for (size_t i = 0; i < mnHeight * nContextBytesPerRow; i++)
                maContextBuffer.get()[i] = (i & 0xFF);
#endif

            if( !bSkipConversion )
            {
                ConvertBitmapData( mnWidth, mnHeight,
                                   32, nContextBytesPerRow, maPalette, maContextBuffer.get(),
                                   mnBits, mnBytesPerRow, maPalette, maUserBuffer.get() );
            }
        }
        catch( const std::bad_alloc& )
        {
            mxGraphicContext = nullptr;
        }
    }

    if( maContextBuffer.get() )
    {
        mxGraphicContext = CGBitmapContextCreate( maContextBuffer.get(), mnWidth, mnHeight,
                                                  bitsPerComponent, nContextBytesPerRow,
                                                  aCGColorSpace, aCGBmpInfo );
        SAL_INFO("vcl.cg", "CGBitmapContextCreate(" << mnWidth << "x" << mnHeight << "x" << bitsPerComponent << ") = " << mxGraphicContext );
    }

    if( !mxGraphicContext )
        maContextBuffer.reset();

    return mxGraphicContext != nullptr;
}

bool QuartzSalBitmap::AllocateUserData()
{
    Destroy();

    if( mnWidth && mnHeight )
    {
        mnBytesPerRow =  0;

        switch( mnBits )
        {
        case 1:     mnBytesPerRow = (mnWidth + 7) >> 3; break;
        case 4:     mnBytesPerRow = (mnWidth + 1) >> 1; break;
        case 8:     mnBytesPerRow = mnWidth; break;
        case 16:    mnBytesPerRow = mnWidth << 1; break;
        case 24:    mnBytesPerRow = (mnWidth << 1) + mnWidth; break;
        case 32:    mnBytesPerRow = mnWidth << 2; break;
        default:
            OSL_FAIL("vcl::QuartzSalBitmap::AllocateUserData(), illegal bitcount!");
        }
    }

    bool alloc = false;
    if (mnBytesPerRow != 0 &&
        mnBytesPerRow <= std::numeric_limits<sal_uInt32>::max() / mnHeight)
    {
        try
        {
            maUserBuffer.reset( new sal_uInt8[mnBytesPerRow * mnHeight] );
            alloc = true;
        }
        catch (std::bad_alloc &) {}
    }
    if (!alloc)
    {
        SAL_WARN( "vcl.quartz", "bad alloc " << mnBytesPerRow << "x" << mnHeight);
        maUserBuffer.reset( static_cast<sal_uInt8*>(nullptr) );
        mnBytesPerRow = 0;
    }
#ifdef DBG_UTIL
    else
    {
        for (size_t i = 0; i < mnBytesPerRow * mnHeight; i++)
        {
            maUserBuffer.get()[i] = (i & 0xFF);
        }
    }
#endif

    return maUserBuffer.get() != nullptr;
}

namespace {

class ImplPixelFormat
{
public:
    static ImplPixelFormat* GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette );

    virtual void StartLine( sal_uInt8* pLine ) = 0;
    virtual void SkipPixel( sal_uInt32 nPixel ) = 0;
    virtual ColorData ReadPixel() = 0;
    virtual void WritePixel( ColorData nColor ) = 0;
    virtual ~ImplPixelFormat() { }
};

class ImplPixelFormat32 : public ImplPixelFormat
// currently ARGB-format for 32bit depth
{
    sal_uInt8* pData;
public:
    virtual void StartLine( sal_uInt8* pLine ) override { pData = pLine; }
    virtual void SkipPixel( sal_uInt32 nPixel ) override
    {
        pData += nPixel << 2;
    }
    virtual ColorData ReadPixel() override
    {
        const ColorData c = RGB_COLORDATA( pData[1], pData[2], pData[3] );
        pData += 4;
        return c;
    }
    virtual void WritePixel( ColorData nColor ) override
    {
        *pData++ = 0;
        *pData++ = COLORDATA_RED( nColor );
        *pData++ = COLORDATA_GREEN( nColor );
        *pData++ = COLORDATA_BLUE( nColor );
    }
};

class ImplPixelFormat24 : public ImplPixelFormat
// currently BGR-format for 24bit depth
{
    sal_uInt8* pData;
public:
    virtual void StartLine( sal_uInt8* pLine ) override { pData = pLine; }
    virtual void SkipPixel( sal_uInt32 nPixel ) override
    {
        pData += (nPixel << 1) + nPixel;
    }
    virtual ColorData ReadPixel() override
    {
        const ColorData c = RGB_COLORDATA( pData[2], pData[1], pData[0] );
        pData += 3;
        return c;
    }
    virtual void WritePixel( ColorData nColor ) override
    {
        *pData++ = COLORDATA_BLUE( nColor );
        *pData++ = COLORDATA_GREEN( nColor );
        *pData++ = COLORDATA_RED( nColor );
    }
};

class ImplPixelFormat16 : public ImplPixelFormat
// currently R5G6B5-format for 16bit depth
{
    sal_uInt16* pData;
public:

    virtual void StartLine( sal_uInt8* pLine ) override
    {
        pData = reinterpret_cast<sal_uInt16*>(pLine);
    }
    virtual void SkipPixel( sal_uInt32 nPixel ) override
    {
        pData += nPixel;
    }
    virtual ColorData ReadPixel() override
    {
        const ColorData c = RGB_COLORDATA( (*pData & 0xf800) >> 8, (*pData & 0x07e0) >> 3 , (*pData & 0x001f) << 3 );
        pData++;
        return c;
    }
    virtual void WritePixel( ColorData nColor ) override
    {
        *pData++ =  ((COLORDATA_RED( nColor ) & 0xf8 ) << 8 ) |
                    ((COLORDATA_GREEN( nColor ) & 0xfc ) << 3 ) |
                    ((COLORDATA_BLUE( nColor ) & 0xf8 ) >> 3 );
    }
};

class ImplPixelFormat8 : public ImplPixelFormat
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;

public:
    explicit ImplPixelFormat8( const BitmapPalette& rPalette )
        : mrPalette( rPalette )
        {
        }
    virtual void StartLine( sal_uInt8* pLine ) override { pData = pLine; }
    virtual void SkipPixel( sal_uInt32 nPixel ) override
        {
            pData += nPixel;
        }
    virtual ColorData ReadPixel() override
        {
            return mrPalette[ *pData++ ].operator Color().GetColor();
        }
    virtual void WritePixel( ColorData nColor ) override
        {
            const BitmapColor aColor( COLORDATA_RED( nColor ),
                                      COLORDATA_GREEN( nColor ),
                                      COLORDATA_BLUE( nColor ) );
            *pData++ = static_cast< sal_uInt8 >( mrPalette.GetBestIndex( aColor ) );
        }
};

class ImplPixelFormat4 : public ImplPixelFormat
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;
    sal_uInt32 mnShift;

public:
    explicit ImplPixelFormat4( const BitmapPalette& rPalette )
        : mrPalette( rPalette )
        {
        }
    virtual void SkipPixel( sal_uInt32 nPixel ) override
        {
            mnX += nPixel;
            if( (nPixel & 1) )
            {
                mnShift ^= 4;
            }
        }
    virtual void StartLine( sal_uInt8* pLine ) override
        {
            pData = pLine;
            mnX = 0;
            mnShift = 4;
        }
    virtual ColorData ReadPixel() override
        {
            const BitmapColor& rColor = mrPalette[( pData[mnX >> 1] >> mnShift) & 0x0f];
            mnX++;
            mnShift ^= 4;
            return rColor.operator Color().GetColor();
        }
    virtual void WritePixel( ColorData nColor ) override
        {
            const BitmapColor aColor( COLORDATA_RED( nColor ),
                                      COLORDATA_GREEN( nColor ),
                                      COLORDATA_BLUE( nColor ) );
            pData[mnX>>1] &= (0xf0 >> mnShift);
            pData[mnX>>1] |= (static_cast< sal_uInt8 >( mrPalette.GetBestIndex( aColor ) ) & 0x0f);
            mnX++;
            mnShift ^= 4;
        }
};

class ImplPixelFormat1 : public ImplPixelFormat
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;

public:
    explicit ImplPixelFormat1( const BitmapPalette& rPalette )
        : mrPalette( rPalette )
        {
        }
    virtual void SkipPixel( sal_uInt32 nPixel ) override
        {
            mnX += nPixel;
        }
    virtual void StartLine( sal_uInt8* pLine ) override
        {
            pData = pLine;
            mnX = 0;
        }
    virtual ColorData ReadPixel() override
        {
            const BitmapColor& rColor = mrPalette[ (pData[mnX >> 3 ] >> ( 7 - ( mnX & 7 ) )) & 1];
            mnX++;
            return rColor.operator Color().GetColor();
        }
    virtual void WritePixel( ColorData nColor ) override
        {
            const BitmapColor aColor( COLORDATA_RED( nColor ),
                                      COLORDATA_GREEN( nColor ),
                                      COLORDATA_BLUE( nColor ) );
            if( mrPalette.GetBestIndex( aColor ) & 1 )
            {
                pData[ mnX >> 3 ] |= 1 << ( 7 - ( mnX & 7 ) );
            }
            else
            {
                pData[ mnX >> 3 ] &= ~( 1 << ( 7 - ( mnX & 7 ) ) );
            }
            mnX++;
        }
};

ImplPixelFormat* ImplPixelFormat::GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette )
{
    switch( nBits )
    {
    case 1: return new ImplPixelFormat1( rPalette );
    case 4: return new ImplPixelFormat4( rPalette );
    case 8: return new ImplPixelFormat8( rPalette );
    case 16: return new ImplPixelFormat16;
    case 24: return new ImplPixelFormat24;
    case 32: return new ImplPixelFormat32;
    default:
        assert(false);
        return nullptr;
    }

    return nullptr;
}

} // namespace

void QuartzSalBitmap::ConvertBitmapData( sal_uInt32 nWidth, sal_uInt32 nHeight,
                                         sal_uInt16 nDestBits, sal_uInt32 nDestBytesPerRow,
                                         const BitmapPalette& rDestPalette, sal_uInt8* pDestData,
                                         sal_uInt16 nSrcBits, sal_uInt32 nSrcBytesPerRow,
                                         const BitmapPalette& rSrcPalette, sal_uInt8* pSrcData )

{
    if( (nDestBytesPerRow == nSrcBytesPerRow) &&
        (nDestBits == nSrcBits) && ((nSrcBits != 8) || (rDestPalette.operator==( rSrcPalette ))) )
    {
        // simple case, same format, so just copy
        memcpy( pDestData, pSrcData, nHeight * nDestBytesPerRow );
        return;
    }

    // try accelerated conversion if possible
    // TODO: are other truecolor conversions except BGR->ARGB worth it?
    bool bConverted = false;
    if( (nSrcBits == 24) && (nDestBits == 32) )
    {
        // TODO: extend bmpfast.cxx with a method that can be directly used here
        BitmapBuffer aSrcBuf;
        aSrcBuf.mnFormat = BMP_FORMAT_24BIT_TC_BGR;
        aSrcBuf.mpBits = pSrcData;
        aSrcBuf.mnBitCount = nSrcBits;
        aSrcBuf.mnScanlineSize = nSrcBytesPerRow;
        BitmapBuffer aDstBuf;
        aDstBuf.mnFormat = BMP_FORMAT_32BIT_TC_ARGB;
        aDstBuf.mpBits = pDestData;
        aSrcBuf.mnBitCount = nDestBits;
        aDstBuf.mnScanlineSize = nDestBytesPerRow;

        aSrcBuf.mnWidth = aDstBuf.mnWidth = nWidth;
        aSrcBuf.mnHeight = aDstBuf.mnHeight = nHeight;

        SalTwoRect aTwoRects(0, 0, mnWidth, mnHeight, 0, 0, mnWidth, mnHeight);
        bConverted = ::ImplFastBitmapConversion( aDstBuf, aSrcBuf, aTwoRects );
    }

    if( !bConverted )
    {
        // TODO: this implementation is for clarity, not for speed

        ImplPixelFormat* pD = ImplPixelFormat::GetFormat( nDestBits, rDestPalette );
        ImplPixelFormat* pS = ImplPixelFormat::GetFormat( nSrcBits, rSrcPalette );

        if( pD && pS )
        {
            sal_uInt32 nY = nHeight;
            while( nY-- )
            {
                pD->StartLine( pDestData );
                pS->StartLine( pSrcData );

                sal_uInt32 nX = nWidth;
                while( nX-- )
                {
                    pD->WritePixel( pS->ReadPixel() );
                }
                pSrcData += nSrcBytesPerRow;
                pDestData += nDestBytesPerRow;
            }
        }
        delete pS;
        delete pD;
    }
}

Size QuartzSalBitmap::GetSize() const
{
    return Size( mnWidth, mnHeight );
}

sal_uInt16 QuartzSalBitmap::GetBitCount() const
{
    return mnBits;
}

static struct pal_entry
{
    sal_uInt8 mnRed;
    sal_uInt8 mnGreen;
    sal_uInt8 mnBlue;
}
const aImplSalSysPalEntryAry[ 16 ] =
{
{    0,    0,    0 },
{    0,    0, 0x80 },
{    0, 0x80,    0 },
{    0, 0x80, 0x80 },
{ 0x80,    0,    0 },
{ 0x80,    0, 0x80 },
{ 0x80, 0x80,    0 },
{ 0x80, 0x80, 0x80 },
{ 0xC0, 0xC0, 0xC0 },
{    0,    0, 0xFF },
{    0, 0xFF,    0 },
{    0, 0xFF, 0xFF },
{ 0xFF,    0,    0 },
{ 0xFF,    0, 0xFF },
{ 0xFF, 0xFF,    0 },
{ 0xFF, 0xFF, 0xFF }
};

const BitmapPalette& GetDefaultPalette( int mnBits, bool bMonochrome )
{
    if( bMonochrome )
        return Bitmap::GetGreyPalette( 1U << mnBits );

    // at this point we should provide some kind of default palette
    // since all other platforms do so, too.
    static bool bDefPalInit = false;
    static BitmapPalette aDefPalette256;
    static BitmapPalette aDefPalette16;
    static BitmapPalette aDefPalette2;
    if( ! bDefPalInit )
    {
        bDefPalInit = true;
        aDefPalette256.SetEntryCount( 256 );
        aDefPalette16.SetEntryCount( 16 );
        aDefPalette2.SetEntryCount( 2 );

        // Standard colors
        unsigned int i;
        for( i = 0; i < 16; i++ )
        {
            aDefPalette16[i] =
            aDefPalette256[i] = BitmapColor( aImplSalSysPalEntryAry[i].mnRed,
                                             aImplSalSysPalEntryAry[i].mnGreen,
                                             aImplSalSysPalEntryAry[i].mnBlue );
        }

        aDefPalette2[0] = BitmapColor( 0, 0, 0 );
        aDefPalette2[1] = BitmapColor( 0xff, 0xff, 0xff );

        // own palette (6/6/6)
        const int DITHER_PAL_STEPS = 6;
        const sal_uInt8 DITHER_PAL_DELTA = 51;
        int nB, nG, nR;
        sal_uInt8 nRed, nGreen, nBlue;
        for( nB=0, nBlue=0; nB < DITHER_PAL_STEPS; nB++, nBlue += DITHER_PAL_DELTA )
        {
            for( nG=0, nGreen=0; nG < DITHER_PAL_STEPS; nG++, nGreen += DITHER_PAL_DELTA )
            {
                for( nR=0, nRed=0; nR < DITHER_PAL_STEPS; nR++, nRed += DITHER_PAL_DELTA )
                {
                    aDefPalette256[ i ] = BitmapColor( nRed, nGreen, nBlue );
                    i++;
                }
            }
        }
    }

    // now fill in appropriate palette
    switch( mnBits )
    {
    case 1: return aDefPalette2;
    case 4: return aDefPalette16;
    case 8: return aDefPalette256;
    default: break;
    }

    const static BitmapPalette aEmptyPalette;
    return aEmptyPalette;
}

BitmapBuffer* QuartzSalBitmap::AcquireBuffer( BitmapAccessMode /*nMode*/ )
{
    if( !maUserBuffer.get() )
//  || maContextBuffer.get() && (maUserBuffer.get() != maContextBuffer.get()) )
    {
        // fprintf(stderr,"ASB::Acq(%dx%d,d=%d)\n",mnWidth,mnHeight,mnBits);
        // TODO: AllocateUserData();
        return nullptr;
    }

    BitmapBuffer* pBuffer = new BitmapBuffer;
    pBuffer->mnWidth = mnWidth;
    pBuffer->mnHeight = mnHeight;
    pBuffer->maPalette = maPalette;
    pBuffer->mnScanlineSize = mnBytesPerRow;
    pBuffer->mpBits = maUserBuffer.get();
    pBuffer->mnBitCount = mnBits;
    switch( mnBits )
    {
        case 1:
            pBuffer->mnFormat = BMP_FORMAT_1BIT_MSB_PAL;
            break;
        case 4:
            pBuffer->mnFormat = BMP_FORMAT_4BIT_MSN_PAL;
            break;
        case 8:
            pBuffer->mnFormat = BMP_FORMAT_8BIT_PAL;
            break;
        case 16:
        {
            pBuffer->mnFormat = BMP_FORMAT_16BIT_TC_MSB_MASK;
            ColorMaskElement aRedMask(k16BitRedColorMask);
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(k16BitGreenColorMask);
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(k16BitBlueColorMask);
            aBlueMask.CalcMaskShift();
            pBuffer->maColorMask  = ColorMask(aRedMask, aGreenMask, aBlueMask);
            break;
        }
        case 24:
            pBuffer->mnFormat = BMP_FORMAT_24BIT_TC_BGR;
            break;
        case 32:
        {
            pBuffer->mnFormat = BMP_FORMAT_32BIT_TC_ARGB;
            ColorMaskElement aRedMask(k32BitRedColorMask);
            aRedMask.CalcMaskShift();
            ColorMaskElement aGreenMask(k32BitGreenColorMask);
            aGreenMask.CalcMaskShift();
            ColorMaskElement aBlueMask(k32BitBlueColorMask);
            aBlueMask.CalcMaskShift();
            pBuffer->maColorMask  = ColorMask(aRedMask, aGreenMask, aBlueMask);
            break;
        }
    }
    pBuffer->mnFormat |= BMP_FORMAT_BOTTOM_UP;

    // some BitmapBuffer users depend on a complete palette
    if( (mnBits <= 8) && !maPalette )
        pBuffer->maPalette = GetDefaultPalette( mnBits, true );

    return pBuffer;
}

void QuartzSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
    // invalidate graphic context if we have different data
    if( nMode == BITMAP_WRITE_ACCESS )
    {
        maPalette = pBuffer->maPalette;
        if( mxGraphicContext )
        {
            DestroyContext();
        }
    }

    delete pBuffer;
}

CGImageRef QuartzSalBitmap::CreateCroppedImage( int nX, int nY, int nNewWidth, int nNewHeight ) const
{
    if( !mxCachedImage )
    {
        if( !mxGraphicContext )
        {
            if( !const_cast<QuartzSalBitmap*>(this)->CreateContext() )
            {
                return nullptr;
            }
        }
        mxCachedImage = CGBitmapContextCreateImage( mxGraphicContext );
        SAL_INFO("vcl.cg", "CGBitmapContextCreateImage(" << mxGraphicContext << ") = " << mxCachedImage );
    }

    CGImageRef xCroppedImage = nullptr;
    // short circuit if there is nothing to crop
    if( !nX && !nY && (mnWidth == nNewWidth) && (mnHeight == nNewHeight) )
    {
          xCroppedImage = mxCachedImage;
          SAL_INFO("vcl.cg", "CFRetain(" << xCroppedImage << ")" );
          CFRetain( xCroppedImage );
    }
    else
    {
        nY = mnHeight - (nY + nNewHeight); // adjust for y-mirrored context
        const CGRect aCropRect = { { static_cast<CGFloat>(nX), static_cast<CGFloat>(nY) }, { static_cast<CGFloat>(nNewWidth), static_cast<CGFloat>(nNewHeight) } };
        xCroppedImage = CGImageCreateWithImageInRect( mxCachedImage, aCropRect );
        SAL_INFO("vcl.cg", "CGImageCreateWithImageInRect(" << mxCachedImage << "," << aCropRect << ") = " << xCroppedImage );
    }

    return xCroppedImage;
}

static void CFRTLFree(void* /*info*/, const void* data, size_t /*size*/)
{
    rtl_freeMemory( const_cast<void*>(data) );
}

CGImageRef QuartzSalBitmap::CreateWithMask( const QuartzSalBitmap& rMask,
    int nX, int nY, int nWidth, int nHeight ) const
{
    CGImageRef xImage( CreateCroppedImage( nX, nY, nWidth, nHeight ) );
    if( !xImage )
        return nullptr;

    CGImageRef xMask = rMask.CreateCroppedImage( nX, nY, nWidth, nHeight );
    if( !xMask )
        return xImage;

    // CGImageCreateWithMask() only likes masks or greyscale images => convert if needed
    // TODO: isolate in an extra method?
    DBG_WRITE_IMAGE(xMask, "xMask");
    if( !CGImageIsMask(xMask) || rMask.GetBitCount() != 8)//(CGImageGetColorSpace(xMask) != GetSalData()->mxGraySpace) )
    {
        const CGRect xImageRect=CGRectMake( 0, 0, nWidth, nHeight );//the rect has no offset

        // create the alpha mask image fitting our image
        // TODO: is caching the full mask or the subimage mask worth it?
        int nMaskBytesPerRow = ((nWidth + 3) & ~3);
        void* pMaskMem = rtl_allocateMemory( nMaskBytesPerRow * nHeight );
        CGContextRef xMaskContext = CGBitmapContextCreate( pMaskMem,
            nWidth, nHeight, 8, nMaskBytesPerRow, GetSalData()->mxGraySpace, kCGImageAlphaNone );
        SAL_INFO("vcl.cg", "CGBitmapContextCreate(" << nWidth << "x" << nHeight << "x8," << nMaskBytesPerRow << ") = " << xMaskContext );
        SAL_INFO("vcl.cg", "CGContextDrawImage(" << xMaskContext << "," << xImageRect << "," << xMask << ")" );
        CGContextDrawImage( xMaskContext, xImageRect, xMask );
        SAL_INFO("vcl.cg", "CFRelease(" << xMask << ")" );
        CFRelease( xMask );
        CGDataProviderRef xDataProvider( CGDataProviderCreateWithData( nullptr,
        pMaskMem, nHeight * nMaskBytesPerRow, &CFRTLFree ) );

        static const CGFloat* pDecode = nullptr;
        xMask = CGImageMaskCreate( nWidth, nHeight, 8, 8, nMaskBytesPerRow, xDataProvider, pDecode, false );
        SAL_INFO("vcl.cg", "CGImageMaskCreate(" << nWidth << "," << nHeight << ",8,8) = " << xMask );
        CFRelease( xDataProvider );
        SAL_INFO("vcl.cg", "CFRelease(" << xMaskContext << ")" );
        CFRelease( xMaskContext );
    }

    if( !xMask )
        return xImage;

    // combine image and alpha mask
    CGImageRef xMaskedImage = CGImageCreateWithMask( xImage, xMask );
    SAL_INFO("vcl.cg", "CGImageCreateWithMask(" << xImage << "," << xMask << ") = " << xMaskedImage );
    DBG_WRITE_IMAGE(xImage, "xImage");
    DBG_WRITE_IMAGE(xMaskedImage, "xMaskedImage");
    SAL_INFO("vcl.cg", "CFRelease(" << xMask << ")" );
    CFRelease( xMask );
    SAL_INFO("vcl.cg", "CFRelease(" << xImage << ")" );
    CFRelease( xImage );
    return xMaskedImage;
}

/** creates an image from the given rectangle, replacing all black pixels
    with nMaskColor and make all other full transparent */
CGImageRef QuartzSalBitmap::CreateColorMask( int nX, int nY, int nWidth,
                                             int nHeight, SalColor nMaskColor ) const
{
    CGImageRef xMask = nullptr;
    if( maUserBuffer.get() && (nX + nWidth <= mnWidth) && (nY + nHeight <= mnHeight) )
    {
        const sal_uInt32 nDestBytesPerRow = nWidth << 2;
        sal_uInt32* pMaskBuffer = static_cast<sal_uInt32*>( rtl_allocateMemory( nHeight * nDestBytesPerRow ) );
        sal_uInt32* pDest = pMaskBuffer;

        ImplPixelFormat* pSourcePixels = ImplPixelFormat::GetFormat( mnBits, maPalette );

        if( pMaskBuffer && pSourcePixels )
        {
            sal_uInt32 nColor;
            reinterpret_cast<sal_uInt8*>(&nColor)[0] = 0xff;
            reinterpret_cast<sal_uInt8*>(&nColor)[1] = SALCOLOR_RED( nMaskColor );
            reinterpret_cast<sal_uInt8*>(&nColor)[2] = SALCOLOR_GREEN( nMaskColor );
            reinterpret_cast<sal_uInt8*>(&nColor)[3] = SALCOLOR_BLUE( nMaskColor );

            sal_uInt8* pSource = maUserBuffer.get();
            if( nY )
                pSource += nY * mnBytesPerRow;

            int y = nHeight;
            while( y-- )
            {
                pSourcePixels->StartLine( pSource );
                pSourcePixels->SkipPixel(nX);
                sal_uInt32 x = nWidth;
                while( x-- )
                {
                    *pDest++ = ( pSourcePixels->ReadPixel() == 0 ) ? nColor : 0;
                }
                pSource += mnBytesPerRow;
            }

            CGDataProviderRef xDataProvider( CGDataProviderCreateWithData(nullptr, pMaskBuffer, nHeight * nDestBytesPerRow, &CFRTLFree) );
            xMask = CGImageCreate(nWidth, nHeight, 8, 32, nDestBytesPerRow, GetSalData()->mxRGBSpace, kCGImageAlphaPremultipliedFirst, xDataProvider, nullptr, true, kCGRenderingIntentDefault);
            SAL_INFO("vcl.cg", "CGImageCreate(" << nWidth << "x" << nHeight << "x8) = " << xMask );
            CFRelease(xDataProvider);
        }
        else
        {
            free(pMaskBuffer);
        }

        delete pSourcePixels;
    }
    return xMask;
}

/** QuartzSalBitmap::GetSystemData Get platform native image data from existing image
 *
 *  @param rData struct BitmapSystemData, defined in vcl/inc/bitmap.hxx
 *  @return true if successful
**/
bool QuartzSalBitmap::GetSystemData( BitmapSystemData& rData )
{
    bool bRet = false;

    if( !mxGraphicContext )
        CreateContext();

    if ( mxGraphicContext )
    {
        bRet = true;

        if ((CGBitmapContextGetBitsPerPixel(mxGraphicContext) == 32) &&
            (CGBitmapContextGetBitmapInfo(mxGraphicContext) & kCGBitmapByteOrderMask) != kCGBitmapByteOrder32Host)
        {
            /**
             * We need to hack things because VCL does not use kCGBitmapByteOrder32Host, while Cairo requires it.
             */
            OSL_TRACE("QuartzSalBitmap::%s(): kCGBitmapByteOrder32Host not found => inserting it.",__func__);

            CGImageRef xImage = CGBitmapContextCreateImage (mxGraphicContext);
            SAL_INFO("vcl.cg", "CGBitmapContextCreateImage(" << mxGraphicContext << ") = " << xImage );

            // re-create the context with single change: include kCGBitmapByteOrder32Host flag.
            CGContextRef mxGraphicContextNew = CGBitmapContextCreate( CGBitmapContextGetData(mxGraphicContext),
                                                                      CGBitmapContextGetWidth(mxGraphicContext),
                                                                      CGBitmapContextGetHeight(mxGraphicContext),
                                                                      CGBitmapContextGetBitsPerComponent(mxGraphicContext),
                                                                      CGBitmapContextGetBytesPerRow(mxGraphicContext),
                                                                      CGBitmapContextGetColorSpace(mxGraphicContext),
                                                                      CGBitmapContextGetBitmapInfo(mxGraphicContext) | kCGBitmapByteOrder32Host);
            SAL_INFO("vcl.cg", "CGBitmapContextCreate(" << CGBitmapContextGetWidth(mxGraphicContext) << "x" << CGBitmapContextGetHeight(mxGraphicContext) << "x" << CGBitmapContextGetBitsPerComponent(mxGraphicContext) << ") = " << mxGraphicContextNew );

            SAL_INFO("vcl.cg", "CFRelease(" << mxGraphicContext << ")" );
            CFRelease(mxGraphicContext);

            // Needs to be flipped
            SAL_INFO("vcl.cg", "CGContextSaveGState(" << mxGraphicContextNew << ")" );
            CGContextSaveGState( mxGraphicContextNew );
            SAL_INFO("vcl.cg", "CGContextTranslateCTM(" << mxGraphicContextNew << ",0," << CGBitmapContextGetHeight(mxGraphicContextNew) << ")" );
            CGContextTranslateCTM (mxGraphicContextNew, 0, CGBitmapContextGetHeight(mxGraphicContextNew));
            SAL_INFO("vcl.cg", "CGContextScaleCTM(" << mxGraphicContextNew << ",1,-1)" );
            CGContextScaleCTM (mxGraphicContextNew, 1.0, -1.0);

            SAL_INFO("vcl.cg", "CGContextDrawImage(" << mxGraphicContextNew << "," << CGRectMake(0, 0, CGImageGetWidth(xImage), CGImageGetHeight(xImage)) << "," << xImage << ")" );
            CGContextDrawImage(mxGraphicContextNew, CGRectMake( 0, 0, CGImageGetWidth(xImage), CGImageGetHeight(xImage)), xImage);

            // Flip back
            SAL_INFO("vcl.cg", "CGContextRestoreGState(" << mxGraphicContextNew << ")" );
            CGContextRestoreGState( mxGraphicContextNew );

            SAL_INFO("vcl.cg", "CGImageRelease(" << xImage << ")" );
            CGImageRelease( xImage );
            mxGraphicContext = mxGraphicContextNew;
        }

        rData.rImageContext = static_cast<void *>(mxGraphicContext);
        rData.mnWidth = mnWidth;
        rData.mnHeight = mnHeight;
    }

    return bRet;
}

bool QuartzSalBitmap::Scale( const double& /*rScaleX*/, const double& /*rScaleY*/, BmpScaleFlag /*nScaleFlag*/ )
{
    return false;
}

bool QuartzSalBitmap::Replace( const Color& /*rSearchColor*/, const Color& /*rReplaceColor*/, sal_uLong /*nTol*/ )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
