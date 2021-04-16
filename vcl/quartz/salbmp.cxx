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
#include <tools/color.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/BitmapAccessMode.hxx>
#include <vcl/BitmapBuffer.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapPalette.hxx>
#include <vcl/ColorMask.hxx>
#include <vcl/Scanline.hxx>

#include <bitmap/bmpfast.hxx>
#include <quartz/salbmp.h>
#include <quartz/utils.h>
#include <bitmap/ScanlineTools.hxx>

#ifdef MACOSX
#include <osx/saldata.hxx>
#else
#include "saldatabasic.hxx"
#endif

const unsigned long k32BitRedColorMask   = 0x00ff0000;
const unsigned long k32BitGreenColorMask = 0x0000ff00;
const unsigned long k32BitBlueColorMask  = 0x000000ff;

QuartzSalBitmap::QuartzSalBitmap()
  : mxCachedImage( nullptr )
  , mnBits(0)
  , mnWidth(0)
  , mnHeight(0)
  , mnBytesPerRow(0)
{
}

QuartzSalBitmap::~QuartzSalBitmap()
{
    doDestroy();
}

bool QuartzSalBitmap::Create( const Size& rSize, vcl::PixelFormat ePixelFormat, const BitmapPalette& rBitmapPalette )
{
    if (ePixelFormat == vcl::PixelFormat::INVALID)
        return false;

    maPalette = rBitmapPalette;
    mnBits = vcl::pixelFormatBitCount(ePixelFormat);
    mnWidth = rSize.Width();
    mnHeight = rSize.Height();
    return AllocateUserData();
}

bool QuartzSalBitmap::Create( const SalBitmap& rSalBmp )
{
    vcl::PixelFormat ePixelFormat = vcl::bitDepthToPixelFormat(rSalBmp.GetBitCount());
    return Create( rSalBmp, ePixelFormat);
}

bool QuartzSalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    vcl::PixelFormat ePixelFormat = vcl::PixelFormat::INVALID;
    if (pGraphics)
        ePixelFormat = vcl::bitDepthToPixelFormat(pGraphics->GetBitCount());
    else
        ePixelFormat = vcl::bitDepthToPixelFormat(rSalBmp.GetBitCount());

    return Create( rSalBmp, ePixelFormat);
}

bool QuartzSalBitmap::Create( const SalBitmap& rSalBmp, vcl::PixelFormat eNewPixelFormat )
{
    const QuartzSalBitmap& rSourceBitmap = static_cast<const QuartzSalBitmap&>(rSalBmp);

    if (eNewPixelFormat != vcl::PixelFormat::INVALID && rSourceBitmap.m_pUserBuffer)
    {
        mnBits = vcl::pixelFormatBitCount(eNewPixelFormat);
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
    doDestroy();
}

void QuartzSalBitmap::doDestroy()
{
    DestroyContext();
    m_pUserBuffer.reset();
}

void QuartzSalBitmap::DestroyContext()
{
    if( mxCachedImage )
    {
        CGImageRelease( mxCachedImage );
        mxCachedImage = nullptr;
    }

    if (maGraphicContext.isSet())
    {
        CGContextRelease(maGraphicContext.get());
        maGraphicContext.set(nullptr);
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
    size_t bitsPerComponent = 8;
    sal_uInt32 nContextBytesPerRow = mnBytesPerRow;
    if( mnBits == 32 )
    {
        // no conversion needed for truecolor
        m_pContextBuffer = m_pUserBuffer;
    }
    else if( mnBits == 8 && maPalette.IsGreyPalette8Bit() )
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
            maGraphicContext.set(nullptr);
        }
    }

    if (m_pContextBuffer)
    {
        maGraphicContext.set(CGBitmapContextCreate(m_pContextBuffer.get(), mnWidth, mnHeight,
                                                   bitsPerComponent, nContextBytesPerRow,
                                                   aCGColorSpace, aCGBmpInfo));
    }

    if (!maGraphicContext.isSet())
        m_pContextBuffer.reset();

    return maGraphicContext.isSet();
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
        case 24:    mnBytesPerRow = (mnWidth << 1) + mnWidth; break;
        case 32:    mnBytesPerRow = mnWidth << 2; break;
        default:
            assert(false && "vcl::QuartzSalBitmap::AllocateUserData(), illegal bitcount!");
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
        m_pUserBuffer.reset();
        mnBytesPerRow = 0;
    }

    return bool(m_pUserBuffer);
}

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

        auto pTarget = vcl::bitmap::getScanlineTransformer(nDestBits, rDestPalette);
        auto pSource = vcl::bitmap::getScanlineTransformer(nSrcBits, rSrcPalette);

        if (pTarget && pSource)
        {
            sal_uInt32 nY = nHeight;
            while( nY-- )
            {
                pTarget->startLine(pDestData);
                pSource->startLine(pSrcData);

                sal_uInt32 nX = nWidth;
                while( nX-- )
                {
                    pTarget->writePixel(pSource->readPixel());
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

namespace {

struct pal_entry
{
    sal_uInt8 mnRed;
    sal_uInt8 mnGreen;
    sal_uInt8 mnBlue;
};

}

pal_entry const aImplSalSysPalEntryAry[ 16 ] =
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
    if (!m_pUserBuffer)
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
        case 8:
            pBuffer->mnFormat = ScanlineFormat::N8BitPal;
            break;
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
        default: assert(false);
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
        if (maGraphicContext.isSet())
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
        if (!maGraphicContext.isSet())
        {
            if( !const_cast<QuartzSalBitmap*>(this)->CreateContext() )
            {
                return nullptr;
            }
        }
        mxCachedImage = CGBitmapContextCreateImage(maGraphicContext.get());
    }

    CGImageRef xCroppedImage = nullptr;
    // short circuit if there is nothing to crop
    if( !nX && !nY && (mnWidth == nNewWidth) && (mnHeight == nNewHeight) )
    {
          xCroppedImage = mxCachedImage;
          CFRetain( xCroppedImage );
    }
    else
    {
        nY = mnHeight - (nY + nNewHeight); // adjust for y-mirrored context
        const CGRect aCropRect = { { static_cast<CGFloat>(nX), static_cast<CGFloat>(nY) }, { static_cast<CGFloat>(nNewWidth), static_cast<CGFloat>(nNewHeight) } };
        xCroppedImage = CGImageCreateWithImageInRect( mxCachedImage, aCropRect );
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
    if( rMask.GetBitCount() != 8)//(CGImageGetColorSpace(xMask) != GetSalData()->mxGraySpace) )
    {
        const CGRect xImageRect=CGRectMake( 0, 0, nWidth, nHeight );//the rect has no offset

        // create the alpha mask image fitting our image
        // TODO: is caching the full mask or the subimage mask worth it?
        int nMaskBytesPerRow = ((nWidth + 3) & ~3);
        void* pMaskMem = std::malloc( nMaskBytesPerRow * nHeight );
        CGContextRef xMaskContext = CGBitmapContextCreate( pMaskMem,
            nWidth, nHeight, 8, nMaskBytesPerRow, GetSalData()->mxGraySpace, kCGImageAlphaNone );
        CGContextDrawImage( xMaskContext, xImageRect, xMask );
        CFRelease( xMask );
        CGDataProviderRef xDataProvider( CGDataProviderCreateWithData( nullptr,
            pMaskMem, nHeight * nMaskBytesPerRow, &CFRTLFree ) );

        static const CGFloat* pDecode = nullptr;
        xMask = CGImageMaskCreate( nWidth, nHeight, 8, 8, nMaskBytesPerRow, xDataProvider, pDecode, false );
        CFRelease( xDataProvider );
        CFRelease( xMaskContext );
    }

    if( !xMask )
        return xImage;

    // combine image and alpha mask
    CGImageRef xMaskedImage = CGImageCreateWithMask( xImage, xMask );
    CFRelease( xMask );
    CFRelease( xImage );
    return xMaskedImage;
}

/** creates an image from the given rectangle, replacing all black pixels
    with nMaskColor and make all other full transparent */
CGImageRef QuartzSalBitmap::CreateColorMask( int nX, int nY, int nWidth,
                                             int nHeight, Color nMaskColor ) const
{
    CGImageRef xMask = nullptr;
    if (m_pUserBuffer && (nX + nWidth <= mnWidth) && (nY + nHeight <= mnHeight))
    {
        const sal_uInt32 nDestBytesPerRow = nWidth << 2;
        std::unique_ptr<sal_uInt32[]> pMaskBuffer(new (std::nothrow) sal_uInt32[ nHeight * nDestBytesPerRow / 4] );
        sal_uInt32* pDest = pMaskBuffer.get();

        auto pSourcePixels = vcl::bitmap::getScanlineTransformer(mnBits, maPalette);

        if( pMaskBuffer && pSourcePixels )
        {
            sal_uInt32 nColor;
            reinterpret_cast<sal_uInt8*>(&nColor)[0] = 0xff;
            reinterpret_cast<sal_uInt8*>(&nColor)[1] = nMaskColor.GetRed();
            reinterpret_cast<sal_uInt8*>(&nColor)[2] = nMaskColor.GetGreen();
            reinterpret_cast<sal_uInt8*>(&nColor)[3] = nMaskColor.GetBlue();

            sal_uInt8* pSource = m_pUserBuffer.get();
            // First to nY on y-axis, as that is our starting point (sub-image)
            if( nY )
                pSource += nY * mnBytesPerRow;

            int y = nHeight;
            while( y-- )
            {
                pSourcePixels->startLine( pSource );
                pSourcePixels->skipPixel(nX); // Skip on x axis to nX
                sal_uInt32 x = nWidth;
                while( x-- )
                {
                    *pDest++ = (pSourcePixels->readPixel() == 0) ? nColor : 0;
                }
                pSource += mnBytesPerRow;
            }

            CGDataProviderRef xDataProvider( CGDataProviderCreateWithData(nullptr, pMaskBuffer.release(), nHeight * nDestBytesPerRow, &CFRTLFree) );
            xMask = CGImageCreate(nWidth, nHeight, 8, 32, nDestBytesPerRow, GetSalData()->mxRGBSpace, kCGImageAlphaPremultipliedFirst, xDataProvider, nullptr, true, kCGRenderingIntentDefault);
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

    if (!maGraphicContext.isSet())
        CreateContext();

    if (maGraphicContext.isSet())
    {
        bRet = true;

        if ((CGBitmapContextGetBitsPerPixel(maGraphicContext.get()) == 32) &&
            (CGBitmapContextGetBitmapInfo(maGraphicContext.get()) & kCGBitmapByteOrderMask) != kCGBitmapByteOrder32Host)
        {
            /**
             * We need to hack things because VCL does not use kCGBitmapByteOrder32Host, while Cairo requires it.
             *
             * Not sure what the above comment means. We don't use Cairo on macOS or iOS.
             *
             * This whole if statement was originally (before 2011) inside #ifdef CAIRO. Did we use Cairo on Mac back then?
             * Anyway, nowadays (since many years, I think) we don't, so should this if statement be dropped? Fun.
             */

            CGImageRef xImage = CGBitmapContextCreateImage(maGraphicContext.get());

            // re-create the context with single change: include kCGBitmapByteOrder32Host flag.
            CGContextHolder aGraphicContextNew(CGBitmapContextCreate(CGBitmapContextGetData(maGraphicContext.get()),
                                                                     CGBitmapContextGetWidth(maGraphicContext.get()),
                                                                     CGBitmapContextGetHeight(maGraphicContext.get()),
                                                                     CGBitmapContextGetBitsPerComponent(maGraphicContext.get()),
                                                                     CGBitmapContextGetBytesPerRow(maGraphicContext.get()),
                                                                     CGBitmapContextGetColorSpace(maGraphicContext.get()),
                                                                     CGBitmapContextGetBitmapInfo(maGraphicContext.get()) | kCGBitmapByteOrder32Host));
            CFRelease(maGraphicContext.get());

            // Needs to be flipped
            aGraphicContextNew.saveState();
            CGContextTranslateCTM (aGraphicContextNew.get(), 0, CGBitmapContextGetHeight(aGraphicContextNew.get()));
            CGContextScaleCTM (aGraphicContextNew.get(), 1.0, -1.0);

            CGContextDrawImage(aGraphicContextNew.get(), CGRectMake( 0, 0, CGImageGetWidth(xImage), CGImageGetHeight(xImage)), xImage);

            // Flip back
            CGContextRestoreGState( aGraphicContextNew.get() );
            CGImageRelease( xImage );
            maGraphicContext = aGraphicContextNew;
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
