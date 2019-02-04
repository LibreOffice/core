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
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <cstddef>
#include <limits>

#include <o3tl/make_shared.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <tools/color.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/salbtype.hxx>

#include <bmpfast.hxx>
#include <quartz/salbmp.h>
#include <quartz/utils.h>

#ifdef MACOSX
#include <osx/saldata.hxx>
#else
#include "saldatabasic.hxx"
#endif

static const unsigned long k16BitRedColorMask   = 0x00007c00;
static const unsigned long k16BitGreenColorMask = 0x000003e0;
static const unsigned long k16BitBlueColorMask  = 0x0000001f;

static const unsigned long k32BitRedColorMask   = 0x00ff0000;
static const unsigned long k32BitGreenColorMask = 0x0000ff00;
static const unsigned long k32BitBlueColorMask  = 0x000000ff;

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
    int nX, int nY, int nWidth, int nHeight, bool bFlipped )
{
    SAL_WARN_IF( !xLayer, "vcl", "QuartzSalBitmap::Create() from non-layered context" );

    // sanitize input parameters
    if( nX < 0 ) {
        nWidth += nX;
        nX = 0;
    }

    if( nY < 0 ) {
        nHeight += nY;
        nY = 0;
    }

    const CGSize aLayerSize = CGLayerGetSize( xLayer );
    SAL_INFO("vcl.cg", "CGLayerGetSize(" << xLayer << ") = " << aLayerSize );
    if( nWidth >= static_cast<int>(aLayerSize.width) - nX )
        nWidth = static_cast<int>(aLayerSize.width) - nX;

    if( nHeight >= static_cast<int>(aLayerSize.height) - nY )
        nHeight = static_cast<int>(aLayerSize.height) - nY;

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
        if( bFlipped )
        {
            SAL_INFO( "vcl.cg", "CGContextTranslateCTM(" << mxGraphicContext << ",0," << mnHeight << ")" );
            CGContextTranslateCTM( mxGraphicContext, 0, +mnHeight );
            SAL_INFO( "vcl.cg", "CGContextScaleCTM(" << mxGraphicContext << ",+1,-1)" );
            CGContextScaleCTM( mxGraphicContext, +1, -1 );
        }

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

    if (isValidBitCount(nNewBitCount) && rSourceBitmap.m_pUserBuffer.get())
    {
        mnBits = nNewBitCount;
        mnWidth = rSourceBitmap.mnWidth;
        mnHeight = rSourceBitmap.mnHeight;
        maPalette = rSourceBitmap.maPalette;

        if( AllocateUserData() )
        {
            ConvertBitmapData( mnWidth, mnHeight, mnBits, mnBytesPerRow, maPalette,
                               m_pUserBuffer.get(), rSourceBitmap.mnBits,
                               rSourceBitmap.mnBytesPerRow, rSourceBitmap.maPalette,
                               rSourceBitmap.m_pUserBuffer.get() );
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
    m_pUserBuffer.reset();
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
        m_pContextBuffer.reset();
    }
}

bool QuartzSalBitmap::CreateContext()
{
    DestroyContext();

    // prepare graphics context
    // convert image from user input if available
    const bool bSkipConversion = !m_pUserBuffer;
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
        m_pContextBuffer = m_pUserBuffer;
    }
    else if( mnBits == 8 && maPalette.IsGreyPalette() )
    {
        // no conversion needed for grayscale
        m_pContextBuffer = m_pUserBuffer;
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
            m_pContextBuffer = o3tl::make_shared_array<sal_uInt8>(mnHeight * nContextBytesPerRow);

            if( !bSkipConversion )
            {
                ConvertBitmapData( mnWidth, mnHeight,
                                   32, nContextBytesPerRow, maPalette, m_pContextBuffer.get(),
                                   mnBits, mnBytesPerRow, maPalette, m_pUserBuffer.get() );
            }
        }
        catch( const std::bad_alloc& )
        {
            mxGraphicContext = nullptr;
        }
    }

    if (m_pContextBuffer.get())
    {
        mxGraphicContext = CGBitmapContextCreate( m_pContextBuffer.get(), mnWidth, mnHeight,
                                                  bitsPerComponent, nContextBytesPerRow,
                                                  aCGColorSpace, aCGBmpInfo );
        SAL_INFO("vcl.cg", "CGBitmapContextCreate(" << mnWidth << "x" << mnHeight << "x" << bitsPerComponent << ") = " << mxGraphicContext );
    }

    if( !mxGraphicContext )
        m_pContextBuffer.reset();

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
            m_pUserBuffer = o3tl::make_shared_array<sal_uInt8>(mnBytesPerRow * mnHeight);
            alloc = true;
        }
        catch (std::bad_alloc &) {}
    }
    if (!alloc)
    {
        SAL_WARN( "vcl.quartz", "bad_alloc: " << mnWidth << "x" << mnHeight << " (" << mnBytesPerRow * mnHeight << " bytes)");
        m_pUserBuffer.reset( static_cast<sal_uInt8*>(nullptr) );
        mnBytesPerRow = 0;
    }

    return m_pUserBuffer.get() != nullptr;
}

namespace {

class ImplPixelFormat
{
public:
    static std::unique_ptr<ImplPixelFormat> GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette );

    virtual void StartLine( sal_uInt8* pLine ) = 0;
    virtual void SkipPixel( sal_uInt32 nPixel ) = 0;
    virtual Color ReadPixel() = 0;
    virtual void WritePixel( Color nColor ) = 0;
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
    virtual Color ReadPixel() override
    {
        const Color c = Color( pData[1], pData[2], pData[3] );
        pData += 4;
        return c;
    }
    virtual void WritePixel( Color nColor ) override
    {
        *pData++ = 0;
        *pData++ = nColor.GetRed();
        *pData++ = nColor.GetGreen();
        *pData++ = nColor.GetBlue();
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
    virtual Color ReadPixel() override
    {
        const Color c = Color( pData[2], pData[1], pData[0] );
        pData += 3;
        return c;
    }
    virtual void WritePixel( Color nColor ) override
    {
        *pData++ = nColor.GetBlue();
        *pData++ = nColor.GetGreen();
        *pData++ = nColor.GetRed();
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
    virtual Color ReadPixel() override
    {
        const Color c = Color( (*pData & 0xf800) >> 8, (*pData & 0x07e0) >> 3 , (*pData & 0x001f) << 3 );
        pData++;
        return c;
    }
    virtual void WritePixel( Color nColor ) override
    {
        *pData++ =  (( nColor.GetRed() & 0xf8 ) << 8 ) |
                    (( nColor.GetGreen() & 0xfc ) << 3 ) |
                    (( nColor.GetBlue() & 0xf8 ) >> 3 );
    }
};

class ImplPixelFormat8 : public ImplPixelFormat
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;
    const sal_uInt16 mnPaletteCount;

public:
    explicit ImplPixelFormat8( const BitmapPalette& rPalette )
        : pData(nullptr)
        , mrPalette(rPalette)
        , mnPaletteCount(rPalette.GetEntryCount())
        {
        }
    virtual void StartLine( sal_uInt8* pLine ) override { pData = pLine; }
    virtual void SkipPixel( sal_uInt32 nPixel ) override
        {
            pData += nPixel;
        }
    virtual Color ReadPixel() override
        {
            const sal_uInt8 nIndex(*pData++);

            // Caution(!) rPalette.GetEntryCount() may be != (depth^^2)-1 (!)
            if(nIndex < mnPaletteCount)
                return mrPalette[nIndex].GetColor();
            else
                return COL_BLACK;
        }
    virtual void WritePixel( Color nColor ) override
        {
            *pData++ = static_cast< sal_uInt8 >( mrPalette.GetBestIndex( nColor ) );
        }
};

class ImplPixelFormat4 : public ImplPixelFormat
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;
    const sal_uInt16 mnPaletteCount;
    sal_uInt32 mnX;
    sal_uInt32 mnShift;

public:
    explicit ImplPixelFormat4( const BitmapPalette& rPalette )
        : pData(nullptr)
        , mrPalette(rPalette)
        , mnPaletteCount(rPalette.GetEntryCount())
        , mnX(0)
        , mnShift(0)
        {
        }
    virtual void SkipPixel( sal_uInt32 nPixel ) override
        {
            mnX += nPixel;
            if( nPixel & 1 )
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
    virtual Color ReadPixel() override
        {
            // Caution(!) rPalette.GetEntryCount() may be != (depth^^2)-1 (!)
            const sal_uInt8 nIndex(( pData[mnX >> 1] >> mnShift) & 0x0f);
            mnX++;
            mnShift ^= 4;

            if(nIndex < mnPaletteCount)
                return mrPalette[nIndex].GetColor();
            else
                return COL_BLACK;
        }
    virtual void WritePixel( Color nColor ) override
        {
            pData[mnX>>1] &= (0xf0 >> mnShift);
            pData[mnX>>1] |= (static_cast< sal_uInt8 >( mrPalette.GetBestIndex( nColor ) ) & 0x0f);
            mnX++;
            mnShift ^= 4;
        }
};

class ImplPixelFormat1 : public ImplPixelFormat
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;
    const sal_uInt16 mnPaletteCount;
    sal_uInt32 mnX;

public:
    explicit ImplPixelFormat1( const BitmapPalette& rPalette )
        : pData(nullptr)
        , mrPalette(rPalette)
        , mnPaletteCount(rPalette.GetEntryCount())
        , mnX(0)
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
    virtual Color ReadPixel() override
        {
            // Caution(!) rPalette.GetEntryCount() may be != (depth^^2)-1 (!)
            const sal_uInt8 nIndex( (pData[mnX >> 3 ] >> ( 7 - ( mnX & 7 ) )) & 1);
            mnX++;

            if(nIndex < mnPaletteCount)
                return mrPalette[nIndex].GetColor();
            else
                return COL_BLACK;
        }
    virtual void WritePixel( Color nColor ) override
        {
            if( mrPalette.GetBestIndex( nColor ) & 1 )
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

std::unique_ptr<ImplPixelFormat> ImplPixelFormat::GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette )
{
    switch( nBits )
    {
    case 1: return std::make_unique<ImplPixelFormat1>( rPalette );
    case 4: return std::make_unique<ImplPixelFormat4>( rPalette );
    case 8: return std::make_unique<ImplPixelFormat8>( rPalette );
    case 16: return std::make_unique<ImplPixelFormat16>();
    case 24: return std::make_unique<ImplPixelFormat24>();
    case 32: return std::make_unique<ImplPixelFormat32>();
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
        aSrcBuf.mnFormat = ScanlineFormat::N24BitTcBgr;
        aSrcBuf.mpBits = pSrcData;
        aSrcBuf.mnBitCount = nSrcBits;
        aSrcBuf.mnScanlineSize = nSrcBytesPerRow;
        BitmapBuffer aDstBuf;
        aDstBuf.mnFormat = ScanlineFormat::N32BitTcArgb;
        aDstBuf.mpBits = pDestData;
        aDstBuf.mnBitCount = nDestBits;
        aDstBuf.mnScanlineSize = nDestBytesPerRow;

        aSrcBuf.mnWidth = aDstBuf.mnWidth = nWidth;
        aSrcBuf.mnHeight = aDstBuf.mnHeight = nHeight;

        SalTwoRect aTwoRects(0, 0, mnWidth, mnHeight, 0, 0, mnWidth, mnHeight);
        bConverted = ::ImplFastBitmapConversion( aDstBuf, aSrcBuf, aTwoRects );
    }

    if( !bConverted )
    {
        // TODO: this implementation is for clarity, not for speed

        std::unique_ptr<ImplPixelFormat> pD = ImplPixelFormat::GetFormat( nDestBits, rDestPalette );
        std::unique_ptr<ImplPixelFormat> pS = ImplPixelFormat::GetFormat( nSrcBits, rSrcPalette );

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

static const BitmapPalette& GetDefaultPalette( int mnBits, bool bMonochrome )
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
    // TODO: AllocateUserData();
    if (!m_pUserBuffer.get())
        return nullptr;

    BitmapBuffer* pBuffer = new BitmapBuffer;
    pBuffer->mnWidth = mnWidth;
    pBuffer->mnHeight = mnHeight;
    pBuffer->maPalette = maPalette;
    pBuffer->mnScanlineSize = mnBytesPerRow;
    pBuffer->mpBits = m_pUserBuffer.get();
    pBuffer->mnBitCount = mnBits;
    switch( mnBits )
    {
        case 1:
            pBuffer->mnFormat = ScanlineFormat::N1BitMsbPal;
            break;
        case 4:
            pBuffer->mnFormat = ScanlineFormat::N4BitMsnPal;
            break;
        case 8:
            pBuffer->mnFormat = ScanlineFormat::N8BitPal;
            break;
        case 16:
        {
            pBuffer->mnFormat = ScanlineFormat::N16BitTcMsbMask;
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
            pBuffer->mnFormat = ScanlineFormat::N24BitTcBgr;
            break;
        case 32:
        {
            pBuffer->mnFormat = ScanlineFormat::N32BitTcArgb;
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

    // some BitmapBuffer users depend on a complete palette
    if( (mnBits <= 8) && !maPalette )
        pBuffer->maPalette = GetDefaultPalette( mnBits, true );

    return pBuffer;
}

void QuartzSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
    // invalidate graphic context if we have different data
    if( nMode == BitmapAccessMode::Write )
    {
        maPalette = pBuffer->maPalette;
        if( mxGraphicContext )
        {
            DestroyContext();
        }
        InvalidateChecksum();
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
    std::free( const_cast<void*>(data) );
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
    if( !CGImageIsMask(xMask) || rMask.GetBitCount() != 8)//(CGImageGetColorSpace(xMask) != GetSalData()->mxGraySpace) )
    {
        const CGRect xImageRect=CGRectMake( 0, 0, nWidth, nHeight );//the rect has no offset

        // create the alpha mask image fitting our image
        // TODO: is caching the full mask or the subimage mask worth it?
        int nMaskBytesPerRow = ((nWidth + 3) & ~3);
        void* pMaskMem = std::malloc( nMaskBytesPerRow * nHeight );
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
    SAL_INFO("vcl.cg", "CFRelease(" << xMask << ")" );
    CFRelease( xMask );
    SAL_INFO("vcl.cg", "CFRelease(" << xImage << ")" );
    CFRelease( xImage );
    return xMaskedImage;
}

/** creates an image from the given rectangle, replacing all black pixels
    with nMaskColor and make all other full transparent */
CGImageRef QuartzSalBitmap::CreateColorMask( int nX, int nY, int nWidth,
                                             int nHeight, Color nMaskColor ) const
{
    CGImageRef xMask = nullptr;
    if (m_pUserBuffer.get() && (nX + nWidth <= mnWidth) && (nY + nHeight <= mnHeight))
    {
        const sal_uInt32 nDestBytesPerRow = nWidth << 2;
        std::unique_ptr<sal_uInt32[]> pMaskBuffer(new (std::nothrow) sal_uInt32[ nHeight * nDestBytesPerRow / 4] );
        sal_uInt32* pDest = pMaskBuffer.get();

        std::unique_ptr<ImplPixelFormat> pSourcePixels = ImplPixelFormat::GetFormat( mnBits, maPalette );

        if( pMaskBuffer && pSourcePixels )
        {
            sal_uInt32 nColor;
            reinterpret_cast<sal_uInt8*>(&nColor)[0] = 0xff;
            reinterpret_cast<sal_uInt8*>(&nColor)[1] = nMaskColor.GetRed();
            reinterpret_cast<sal_uInt8*>(&nColor)[2] = nMaskColor.GetGreen();
            reinterpret_cast<sal_uInt8*>(&nColor)[3] = nMaskColor.GetBlue();

            sal_uInt8* pSource = m_pUserBuffer.get();
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

            CGDataProviderRef xDataProvider( CGDataProviderCreateWithData(nullptr, pMaskBuffer.release(), nHeight * nDestBytesPerRow, &CFRTLFree) );
            xMask = CGImageCreate(nWidth, nHeight, 8, 32, nDestBytesPerRow, GetSalData()->mxRGBSpace, kCGImageAlphaPremultipliedFirst, xDataProvider, nullptr, true, kCGRenderingIntentDefault);
            SAL_INFO("vcl.cg", "CGImageCreate(" << nWidth << "x" << nHeight << "x8) = " << xMask );
            CFRelease(xDataProvider);
        }
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
             *
             * Not sure what the above comment means. We don't use Cairo on macOS or iOS.
             *
             * This whole if statement was originally (before 2011) inside #ifdef CAIRO. Did we use Cairo on Mac back then?
             * Anyway, nowadays (since many years, I think) we don't, so should this if statement be dropped? Fun.
             */
            SAL_INFO("vcl.cg", "QuartzSalBitmap::" << __func__ << "(): kCGBitmapByteOrder32Host not found => inserting it.");

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

        rData.mnWidth = mnWidth;
        rData.mnHeight = mnHeight;
    }

    return bRet;
}

bool QuartzSalBitmap::ScalingSupported() const
{
    return false;
}

bool QuartzSalBitmap::Scale( const double& /*rScaleX*/, const double& /*rScaleY*/, BmpScaleFlag /*nScaleFlag*/ )
{
    return false;
}

bool QuartzSalBitmap::Replace( const Color& /*rSearchColor*/, const Color& /*rReplaceColor*/, sal_uInt8 /*nTol*/ )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
