/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "basebmp/scanlineformats.hxx"
#include "basebmp/color.hxx"

#include "basegfx/vector/b2ivector.hxx"

#include "tools/color.hxx"

#include "vcl/bitmap.hxx" // for BitmapSystemData
#include "vcl/salbtype.hxx"

#include "aqua/salbmp.h"
#include "aqua/salinst.h"

#include "bmpfast.hxx"

// =======================================================================

static bool isValidBitCount( sal_uInt16 nBitCount )
{
    return (nBitCount == 1) || (nBitCount == 4) || (nBitCount == 8) || (nBitCount == 16) || (nBitCount == 24) || (nBitCount == 32);
}

// =======================================================================

AquaSalBitmap::AquaSalBitmap()
: mxGraphicContext( NULL )
, mxCachedImage( NULL )
, mnBits(0)
, mnWidth(0)
, mnHeight(0)
, mnBytesPerRow(0)
{
}

// ------------------------------------------------------------------

AquaSalBitmap::~AquaSalBitmap()
{
    Destroy();
}

// ------------------------------------------------------------------

bool AquaSalBitmap::Create( CGLayerRef xLayer, int nBitmapBits,
    int nX, int nY, int nWidth, int nHeight, bool /*bMirrorVert*/ )
{
    DBG_ASSERT( xLayer, "AquaSalBitmap::Create() from non-layered context" );

    // sanitize input parameters
    if( nX < 0 )
        nWidth += nX, nX = 0;
    if( nY < 0 )
        nHeight += nY, nY = 0;
    const CGSize aLayerSize = CGLayerGetSize( xLayer );
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
    const CGPoint aSrcPoint = { -nX, -nY };
    ::CGContextDrawLayerAtPoint( mxGraphicContext, aSrcPoint, xLayer );
    return true;
}

// ------------------------------------------------------------------

bool AquaSalBitmap::Create( const Size& rSize, sal_uInt16 nBits, const BitmapPalette& rBitmapPalette )
{
    if( !isValidBitCount( nBits ) )
        return false;
    maPalette = rBitmapPalette;
    mnBits = nBits;
    mnWidth = rSize.Width();
    mnHeight = rSize.Height();
    return AllocateUserData();
}

// ------------------------------------------------------------------

bool AquaSalBitmap::Create( const SalBitmap& rSalBmp )
{
    return Create( rSalBmp, rSalBmp.GetBitCount() );
}

// ------------------------------------------------------------------

bool AquaSalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    return Create( rSalBmp, pGraphics ? pGraphics->GetBitCount() : rSalBmp.GetBitCount() );
}

// ------------------------------------------------------------------

bool AquaSalBitmap::Create( const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount )
{
    const AquaSalBitmap& rSourceBitmap = static_cast<const AquaSalBitmap&>(rSalBmp);

    if( isValidBitCount( nNewBitCount ) &&  rSourceBitmap.maUserBuffer.get() )
    {
        mnBits = nNewBitCount;
        mnWidth = rSourceBitmap.mnWidth;
        mnHeight = rSourceBitmap.mnHeight;
        maPalette = rSourceBitmap.maPalette;

        if( AllocateUserData() )
        {
            ConvertBitmapData( mnWidth, mnHeight, mnBits, mnBytesPerRow, maPalette, maUserBuffer.get(), rSourceBitmap.mnBits, rSourceBitmap.mnBytesPerRow, rSourceBitmap.maPalette, rSourceBitmap.maUserBuffer.get() );
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------------

bool AquaSalBitmap::Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > /*xBitmapCanvas*/, Size& /*rSize*/, bool /*bMask*/ )
{
    return false;
}

// ------------------------------------------------------------------

void AquaSalBitmap::Destroy()
{
    DestroyContext();
    maUserBuffer.reset();
}

// ------------------------------------------------------------------

void AquaSalBitmap::DestroyContext()
{
    CGImageRelease( mxCachedImage );
    mxCachedImage = NULL;

    if( mxGraphicContext )
    {
        CGContextRelease( mxGraphicContext );
        mxGraphicContext = NULL;
        maContextBuffer.reset();
    }
}

// ------------------------------------------------------------------

bool AquaSalBitmap::CreateContext()
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
    else if( (mnBits == 8) && maPalette.IsGreyPalette() )
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

            if( !bSkipConversion )
                ConvertBitmapData( mnWidth, mnHeight,
                               32, nContextBytesPerRow, maPalette, maContextBuffer.get(),
                               mnBits, mnBytesPerRow, maPalette, maUserBuffer.get() );
        }
        catch( const std::bad_alloc& )
        {
            mxGraphicContext = 0;
        }
    }

    if( maContextBuffer.get() )
    {
        mxGraphicContext = ::CGBitmapContextCreate( maContextBuffer.get(), mnWidth, mnHeight,
            bitsPerComponent, nContextBytesPerRow, aCGColorSpace, aCGBmpInfo );
    }

    if( !mxGraphicContext )
        maContextBuffer.reset();

    return mxGraphicContext != NULL;
}

// ------------------------------------------------------------------

bool AquaSalBitmap::AllocateUserData()
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
            OSL_FAIL("vcl::AquaSalBitmap::AllocateUserData(), illegal bitcount!");
        }
    }

    try
    {
        if( mnBytesPerRow )
            maUserBuffer.reset( new sal_uInt8[mnBytesPerRow * mnHeight] );
    }
    catch( const std::bad_alloc& )
    {
        OSL_FAIL( "vcl::AquaSalBitmap::AllocateUserData: bad alloc" );
        maUserBuffer.reset( NULL );
        mnBytesPerRow = 0;
    }

    return maUserBuffer.get() != 0;
}

// ------------------------------------------------------------------

class ImplPixelFormat
{
protected:
    sal_uInt8* pData;
public:
    static ImplPixelFormat* GetFormat( sal_uInt16 nBits, const BitmapPalette& rPalette );

    virtual void StartLine( sal_uInt8* pLine ) { pData = pLine; }
    virtual void SkipPixel( sal_uInt32 nPixel ) = 0;
    virtual ColorData ReadPixel() = 0;
    virtual void WritePixel( ColorData nColor ) = 0;
    virtual ~ImplPixelFormat() { }
};

class ImplPixelFormat32 : public ImplPixelFormat
// currently ARGB-format for 32bit depth
{
public:
    virtual void SkipPixel( sal_uInt32 nPixel )
    {
        pData += nPixel << 2;
    }
    virtual ColorData ReadPixel()
    {
        const ColorData c = RGB_COLORDATA( pData[1], pData[2], pData[3] );
        pData += 4;
        return c;
    }
    virtual void WritePixel( ColorData nColor )
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
public:
    virtual void SkipPixel( sal_uInt32 nPixel )
    {
        pData += (nPixel << 1) + nPixel;
    }
    virtual ColorData ReadPixel()
    {
        const ColorData c = RGB_COLORDATA( pData[2], pData[1], pData[0] );
        pData += 3;
        return c;
    }
    virtual void WritePixel( ColorData nColor )
    {
        *pData++ = COLORDATA_BLUE( nColor );
        *pData++ = COLORDATA_GREEN( nColor );
        *pData++ = COLORDATA_RED( nColor );
    }
};

class ImplPixelFormat16 : public ImplPixelFormat
// currently R5G6B5-format for 16bit depth
{
protected:
    sal_uInt16* pData16;
public:

    virtual void StartLine( sal_uInt8* pLine )
    {
        pData16 = (sal_uInt16*)pLine;
    }
    virtual void SkipPixel( sal_uInt32 nPixel )
    {
        pData += nPixel;
    }
    virtual ColorData ReadPixel()
    {
        const ColorData c = RGB_COLORDATA( (*pData & 0x7c00) >> 7, (*pData & 0x03e0) >> 2 , (*pData & 0x001f) << 3 );
        pData++;
        return c;
    }
    virtual void WritePixel( ColorData nColor )
    {
        *pData++ =  ((COLORDATA_RED( nColor ) & 0xf8 ) << 7 ) ||
                    ((COLORDATA_GREEN( nColor ) & 0xf8 ) << 2 ) ||
                    ((COLORDATA_BLUE( nColor ) & 0xf8 ) >> 3 );
    }
};

class ImplPixelFormat8 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;

public:
    ImplPixelFormat8( const BitmapPalette& rPalette )
    : mrPalette( rPalette )
    {
    }
    virtual void SkipPixel( sal_uInt32 nPixel )
    {
        pData += nPixel;
    }
    virtual ColorData ReadPixel()
    {
        return mrPalette[ *pData++ ].operator Color().GetColor();
    }
    virtual void WritePixel( ColorData nColor )
    {
        const BitmapColor aColor( COLORDATA_RED( nColor ), COLORDATA_GREEN( nColor ), COLORDATA_BLUE( nColor ) );
        *pData++ = static_cast< sal_uInt8 >( mrPalette.GetBestIndex( aColor ) );
    }
};

class ImplPixelFormat4 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;
    sal_uInt32 mnShift;

public:
    ImplPixelFormat4( const BitmapPalette& rPalette )
    : mrPalette( rPalette )
    {
    }
    virtual void SkipPixel( sal_uInt32 nPixel )
    {
        mnX += nPixel;
        if( (nPixel & 1) )
            mnShift ^= 4;
    }
    virtual void StartLine( sal_uInt8* pLine )
    {
        pData = pLine;
        mnX = 0;
        mnShift = 4;
    }
    virtual ColorData ReadPixel()
    {
        const BitmapColor& rColor = mrPalette[( pData[mnX >> 1] >> mnShift) & 0x0f];
        mnX++;
        mnShift ^= 4;
        return rColor.operator Color().GetColor();
    }
    virtual void WritePixel( ColorData nColor )
    {
        const BitmapColor aColor( COLORDATA_RED( nColor ), COLORDATA_GREEN( nColor ), COLORDATA_BLUE( nColor ) );
        pData[mnX>>1] &= (0xf0 >> mnShift);
        pData[mnX>>1] |= (static_cast< sal_uInt8 >( mrPalette.GetBestIndex( aColor ) ) & 0x0f);
        mnX++;
        mnShift ^= 4;
    }
};

class ImplPixelFormat1 : public ImplPixelFormat
{
private:
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;

public:
    ImplPixelFormat1( const BitmapPalette& rPalette )
    : mrPalette( rPalette )
    {
    }
    virtual void SkipPixel( sal_uInt32 nPixel )
    {
        mnX += nPixel;
    }
    virtual void StartLine( sal_uInt8* pLine )
    {
        pData = pLine;
        mnX = 0;
    }
    virtual ColorData ReadPixel()
    {
        const BitmapColor& rColor = mrPalette[ (pData[mnX >> 3 ] >> ( 7 - ( mnX & 7 ) )) & 1];
        mnX++;
        return rColor.operator Color().GetColor();
    }
    virtual void WritePixel( ColorData nColor )
    {
        const BitmapColor aColor( COLORDATA_RED( nColor ), COLORDATA_GREEN( nColor ), COLORDATA_BLUE( nColor ) );
        if( mrPalette.GetBestIndex( aColor ) & 1 )
            pData[ mnX >> 3 ] |= 1 << ( 7 - ( mnX & 7 ) );
        else
            pData[ mnX >> 3 ] &= ~( 1 << ( 7 - ( mnX & 7 ) ) );
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
    }

    return 0;
}

void AquaSalBitmap::ConvertBitmapData( sal_uInt32 nWidth, sal_uInt32 nHeight,
                                       sal_uInt16 nDestBits, sal_uInt32 nDestBytesPerRow, const BitmapPalette& rDestPalette, sal_uInt8* pDestData,
                                       sal_uInt16 nSrcBits, sal_uInt32 nSrcBytesPerRow, const BitmapPalette& rSrcPalette, sal_uInt8* pSrcData )

{
    if( (nDestBytesPerRow == nSrcBytesPerRow) && (nDestBits == nSrcBits) && ((nSrcBits != 8) || (rDestPalette.operator==( rSrcPalette ))) )
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

        SalTwoRect aTwoRects;
        aTwoRects.mnSrcX = aTwoRects.mnDestX = 0;
        aTwoRects.mnSrcY = aTwoRects.mnDestY = 0;
        aTwoRects.mnSrcWidth = aTwoRects.mnDestWidth = mnWidth;
        aTwoRects.mnSrcHeight = aTwoRects.mnDestHeight = mnHeight;
        bConverted = ::ImplFastBitmapConversion( aDstBuf, aSrcBuf, aTwoRects );
    }

    if( !bConverted )
    {
        // TODO: this implementation is for clarety, not for speed

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
                    pD->WritePixel( pS->ReadPixel() );

                pSrcData += nSrcBytesPerRow;
                pDestData += nDestBytesPerRow;
            }
        }
        delete pS;
        delete pD;
    }
}

// ------------------------------------------------------------------

Size AquaSalBitmap::GetSize() const
{
    return Size( mnWidth, mnHeight );
}

// ------------------------------------------------------------------

sal_uInt16 AquaSalBitmap::GetBitCount() const
{
    return mnBits;
}

// ------------------------------------------------------------------

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

BitmapBuffer* AquaSalBitmap::AcquireBuffer( bool /*bReadOnly*/ )
{
    if( !maUserBuffer.get() )
//  || maContextBuffer.get() && (maUserBuffer.get() != maContextBuffer.get()) )
    {
        fprintf(stderr,"ASB::Acq(%dx%d,d=%d)\n",mnWidth,mnHeight,mnBits);
        // TODO: AllocateUserData();
        return NULL;
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
    case 1:     pBuffer->mnFormat = BMP_FORMAT_1BIT_MSB_PAL; break;
    case 4:     pBuffer->mnFormat = BMP_FORMAT_4BIT_MSN_PAL; break;
    case 8:     pBuffer->mnFormat = BMP_FORMAT_8BIT_PAL; break;
    case 16:    pBuffer->mnFormat = BMP_FORMAT_16BIT_TC_MSB_MASK;
                pBuffer->maColorMask  = ColorMask( k16BitRedColorMask, k16BitGreenColorMask, k16BitBlueColorMask );
                break;
    case 24:    pBuffer->mnFormat = BMP_FORMAT_24BIT_TC_BGR; break;
    case 32:    pBuffer->mnFormat = BMP_FORMAT_32BIT_TC_ARGB;
                pBuffer->maColorMask  = ColorMask( k32BitRedColorMask, k32BitGreenColorMask, k32BitBlueColorMask );
                break;
    }
    pBuffer->mnFormat |= BMP_FORMAT_BOTTOM_UP;

    // some BitmapBuffer users depend on a complete palette
    if( (mnBits <= 8) && !maPalette )
        pBuffer->maPalette = GetDefaultPalette( mnBits, true );

    return pBuffer;
}

// ------------------------------------------------------------------

void AquaSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly )
{
    // invalidate graphic context if we have different data
    if( !bReadOnly )
    {
        maPalette = pBuffer->maPalette;
        if( mxGraphicContext )
            DestroyContext();
    }

    delete pBuffer;
}

// ------------------------------------------------------------------

CGImageRef AquaSalBitmap::CreateCroppedImage( int nX, int nY, int nNewWidth, int nNewHeight ) const
{
    if( !mxCachedImage )
    {
        if( !mxGraphicContext )
            if( !const_cast<AquaSalBitmap*>(this)->CreateContext() )
                return NULL;

        mxCachedImage = CGBitmapContextCreateImage( mxGraphicContext );
    }

    CGImageRef xCroppedImage = NULL;
    // short circuit if there is nothing to crop
    if( !nX && !nY && (mnWidth == nNewWidth) && (mnHeight == nNewHeight) )
    {
          xCroppedImage = mxCachedImage;
          CFRetain( xCroppedImage );
    }
    else
    {
        nY = mnHeight - (nY + nNewHeight); // adjust for y-mirrored context
        const CGRect aCropRect = {{nX, nY}, {nNewWidth, nNewHeight}};
        xCroppedImage = CGImageCreateWithImageInRect( mxCachedImage, aCropRect );
    }

    return xCroppedImage;
}

// ------------------------------------------------------------------

static void CFRTLFree(void* /*info*/, const void* data, size_t /*size*/)
{
    rtl_freeMemory( const_cast<void*>(data) );
}

CGImageRef AquaSalBitmap::CreateWithMask( const AquaSalBitmap& rMask,
    int nX, int nY, int nWidth, int nHeight ) const
{
    CGImageRef xImage( CreateCroppedImage( nX, nY, nWidth, nHeight ) );
    if( !xImage )
        return NULL;

    CGImageRef xMask = rMask.CreateCroppedImage( nX, nY, nWidth, nHeight );
    if( !xMask )
        return xImage;

    // CGImageCreateWithMask() only likes masks or greyscale images => convert if needed
    // TODO: isolate in an extra method?
    if( !CGImageIsMask(xMask) || (CGImageGetColorSpace(xMask) != GetSalData()->mxGraySpace) )
    {
        const CGRect xImageRect=CGRectMake( 0, 0, nWidth, nHeight );//the rect has no offset

        // create the alpha mask image fitting our image
        // TODO: is caching the full mask or the subimage mask worth it?
        int nMaskBytesPerRow = ((nWidth + 3) & ~3);
        void* pMaskMem = rtl_allocateMemory( nMaskBytesPerRow * nHeight );
        CGContextRef xMaskContext = CGBitmapContextCreate( pMaskMem,
            nWidth, nHeight, 8, nMaskBytesPerRow, GetSalData()->mxGraySpace, kCGImageAlphaNone );
        CGContextDrawImage( xMaskContext, xImageRect, xMask );
        CFRelease( xMask );
        CGDataProviderRef xDataProvider( CGDataProviderCreateWithData( NULL,
        pMaskMem, nHeight * nMaskBytesPerRow, &CFRTLFree ) );
        static const CGFloat* pDecode = NULL;
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

// ------------------------------------------------------------------

/** creates an image from the given rectangle, replacing all black pixels with nMaskColor and make all other full transparent */
CGImageRef AquaSalBitmap::CreateColorMask( int nX, int nY, int nWidth, int nHeight, SalColor nMaskColor ) const
{
    CGImageRef xMask = 0;
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

            CGDataProviderRef xDataProvider( CGDataProviderCreateWithData(NULL, pMaskBuffer, nHeight * nDestBytesPerRow, &CFRTLFree) );
            xMask = CGImageCreate(nWidth, nHeight, 8, 32, nDestBytesPerRow, GetSalData()->mxRGBSpace, kCGImageAlphaPremultipliedFirst, xDataProvider, NULL, true, kCGRenderingIntentDefault);
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

// =======================================================================

/** AquaSalBitmap::GetSystemData Get platform native image data from existing image
 *
 *  @param rData struct BitmapSystemData, defined in vcl/inc/bitmap.hxx
 *  @return true if successful
**/
bool AquaSalBitmap::GetSystemData( BitmapSystemData& rData )
{
    bool bRet = false;

    if( !mxGraphicContext )
        CreateContext();

    if ( mxGraphicContext )
    {
        bRet = true;

        if ((CGBitmapContextGetBitsPerPixel(mxGraphicContext) == 32) &&
            (CGBitmapContextGetBitmapInfo(mxGraphicContext) & kCGBitmapByteOrderMask) != kCGBitmapByteOrder32Host) {
            /**
             * We need to hack things because VCL does not use kCGBitmapByteOrder32Host, while Cairo requires it.
             */
            OSL_TRACE("AquaSalBitmap::%s(): kCGBitmapByteOrder32Host not found => inserting it.",__func__);

            CGImageRef xImage = CGBitmapContextCreateImage (mxGraphicContext);

            // re-create the context with single change: include kCGBitmapByteOrder32Host flag.
            CGContextRef mxGraphicContextNew = CGBitmapContextCreate( CGBitmapContextGetData(mxGraphicContext),
                                                                      CGBitmapContextGetWidth(mxGraphicContext),
                                                                      CGBitmapContextGetHeight(mxGraphicContext),
                                                                      CGBitmapContextGetBitsPerComponent(mxGraphicContext),
                                                                      CGBitmapContextGetBytesPerRow(mxGraphicContext),
                                                                      CGBitmapContextGetColorSpace(mxGraphicContext),
                                                                      CGBitmapContextGetBitmapInfo(mxGraphicContext) | kCGBitmapByteOrder32Host);
            CFRelease(mxGraphicContext);

            // Needs to be flipped
            CGContextSaveGState( mxGraphicContextNew );
            CGContextTranslateCTM (mxGraphicContextNew, 0, CGBitmapContextGetHeight(mxGraphicContextNew));
            CGContextScaleCTM (mxGraphicContextNew, 1.0, -1.0);

            CGContextDrawImage(mxGraphicContextNew, CGRectMake( 0, 0, CGImageGetWidth(xImage), CGImageGetHeight(xImage)), xImage);

            // Flip back
            CGContextRestoreGState( mxGraphicContextNew );

            CGImageRelease( xImage );
            mxGraphicContext = mxGraphicContextNew;
        }

        rData.rImageContext = (void *) mxGraphicContext;
        rData.mnWidth = mnWidth;
        rData.mnHeight = mnHeight;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
