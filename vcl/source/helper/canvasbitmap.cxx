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


#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>

#include <rtl/instance.hxx>
#include <osl/mutex.hxx>

#include <tools/diagnose_ex.h>
#include <canvasbitmap.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>


using namespace ::vcl::unotools;
using namespace ::com::sun::star;

namespace
{
    // TODO(Q3): move to o3tl bithacks or somesuch. A similar method is in canvas/canvastools.hxx

    // Good ole HAKMEM tradition. Calc number of 1 bits in 32bit word,
    // unrolled loop. See e.g. Hackers Delight, p. 66
    inline sal_Int32 bitcount( sal_uInt32 val )
    {
        val = val - ((val >> 1) & 0x55555555);
        val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
        val = (val + (val >> 4)) & 0x0F0F0F0F;
        val = val + (val >> 8);
        val = val + (val >> 16);
        return sal_Int32(val & 0x0000003F);
    }
}

void VclCanvasBitmap::setComponentInfo( sal_uLong redShift, sal_uLong greenShift, sal_uLong blueShift )
{
    // sort channels in increasing order of appearance in the pixel
    // (starting with the least significant bits)
    sal_Int8 redPos(0);
    sal_Int8 greenPos(1);
    sal_Int8 bluePos(2);

    if( redShift > greenShift )
    {
        std::swap(redPos,greenPos);
        if( redShift > blueShift )
        {
            std::swap(redPos,bluePos);
            if( greenShift > blueShift )
                std::swap(greenPos,bluePos);
        }
    }
    else
    {
        if( greenShift > blueShift )
        {
            std::swap(greenPos,bluePos);
            if( redShift > blueShift )
                std::swap(redPos,bluePos);
        }
    }

    m_aComponentTags.realloc(3);
    sal_Int8* pTags = m_aComponentTags.getArray();
    pTags[redPos]   = rendering::ColorComponentTag::RGB_RED;
    pTags[greenPos] = rendering::ColorComponentTag::RGB_GREEN;
    pTags[bluePos]  = rendering::ColorComponentTag::RGB_BLUE;

    m_aComponentBitCounts.realloc(3);
    sal_Int32* pCounts = m_aComponentBitCounts.getArray();
    pCounts[redPos]    = bitcount(sal::static_int_cast<sal_uInt32>(redShift));
    pCounts[greenPos]  = bitcount(sal::static_int_cast<sal_uInt32>(greenShift));
    pCounts[bluePos]   = bitcount(sal::static_int_cast<sal_uInt32>(blueShift));
}

VclCanvasBitmap::VclCanvasBitmap( const BitmapEx& rBitmap ) :
    m_aBmpEx( rBitmap ),
    m_aBitmap( rBitmap.GetBitmap() ),
    m_aAlpha(),
    m_pBmpAcc( m_aBitmap.AcquireReadAccess() ),
    m_pAlphaAcc( NULL ),
    m_aComponentTags(),
    m_aComponentBitCounts(),
    m_aLayout(),
    m_nBitsPerInputPixel(0),
    m_nBitsPerOutputPixel(0),
    m_nRedIndex(-1),
    m_nGreenIndex(-1),
    m_nBlueIndex(-1),
    m_nAlphaIndex(-1),
    m_nIndexIndex(-1),
    m_nEndianness(0),
    m_bPalette(false)
{
    if( m_aBmpEx.IsTransparent() )
    {
        m_aAlpha = m_aBmpEx.IsAlpha() ? m_aBmpEx.GetAlpha().GetBitmap() : m_aBmpEx.GetMask();
        m_pAlphaAcc = m_aAlpha.AcquireReadAccess();
    }

    m_aLayout.ScanLines      = 0;
    m_aLayout.ScanLineBytes  = 0;
    m_aLayout.ScanLineStride = 0;
    m_aLayout.PlaneStride    = 0;
    m_aLayout.ColorSpace.clear();
    m_aLayout.Palette.clear();
    m_aLayout.IsMsbFirst     = sal_False;

    if( m_pBmpAcc )
    {
        m_aLayout.ScanLines      = m_pBmpAcc->Height();
        m_aLayout.ScanLineBytes  = (m_pBmpAcc->GetBitCount()*m_pBmpAcc->Width() + 7) / 8;
        m_aLayout.ScanLineStride = m_pBmpAcc->GetScanlineSize();
        m_aLayout.PlaneStride    = 0;

        switch( m_pBmpAcc->GetScanlineFormat() )
        {
            case BMP_FORMAT_1BIT_MSB_PAL:
                m_bPalette           = true;
                m_nBitsPerInputPixel = 1;
                m_nEndianness        = util::Endianness::LITTLE; // doesn't matter
                m_aLayout.IsMsbFirst = sal_True;
                break;

            case BMP_FORMAT_1BIT_LSB_PAL:
                m_bPalette           = true;
                m_nBitsPerInputPixel = 1;
                m_nEndianness        = util::Endianness::LITTLE; // doesn't matter
                m_aLayout.IsMsbFirst = sal_False;
                break;

            case BMP_FORMAT_4BIT_MSN_PAL:
                m_bPalette           = true;
                m_nBitsPerInputPixel = 4;
                m_nEndianness        = util::Endianness::LITTLE; // doesn't matter
                m_aLayout.IsMsbFirst = sal_True;
                break;

            case BMP_FORMAT_4BIT_LSN_PAL:
                m_bPalette           = true;
                m_nBitsPerInputPixel = 4;
                m_nEndianness        = util::Endianness::LITTLE; // doesn't matter
                m_aLayout.IsMsbFirst = sal_False;
                break;

            case BMP_FORMAT_8BIT_PAL:
                m_bPalette           = true;
                m_nBitsPerInputPixel = 8;
                m_nEndianness        = util::Endianness::LITTLE; // doesn't matter
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter
                break;

            case BMP_FORMAT_8BIT_TC_MASK:
                m_bPalette           = false;
                m_nBitsPerInputPixel = 8;
                m_nEndianness        = util::Endianness::LITTLE; // doesn't matter
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter
                setComponentInfo( m_pBmpAcc->GetColorMask().GetRedMask(),
                                  m_pBmpAcc->GetColorMask().GetGreenMask(),
                                  m_pBmpAcc->GetColorMask().GetBlueMask() );
                break;

            case BMP_FORMAT_16BIT_TC_MSB_MASK:
                m_bPalette           = false;
                m_nBitsPerInputPixel = 16;
                m_nEndianness        = util::Endianness::BIG;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter
                setComponentInfo( m_pBmpAcc->GetColorMask().GetRedMask(),
                                  m_pBmpAcc->GetColorMask().GetGreenMask(),
                                  m_pBmpAcc->GetColorMask().GetBlueMask() );
                break;

            case BMP_FORMAT_16BIT_TC_LSB_MASK:
                m_bPalette           = false;
                m_nBitsPerInputPixel = 16;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter
                setComponentInfo( m_pBmpAcc->GetColorMask().GetRedMask(),
                                  m_pBmpAcc->GetColorMask().GetGreenMask(),
                                  m_pBmpAcc->GetColorMask().GetBlueMask() );
                break;

            case BMP_FORMAT_24BIT_TC_BGR:
                m_bPalette           = false;
                m_nBitsPerInputPixel = 24;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter
                setComponentInfo( 0xff0000LL,
                                  0x00ff00LL,
                                  0x0000ffLL );
                break;

            case BMP_FORMAT_24BIT_TC_RGB:
                m_bPalette           = false;
                m_nBitsPerInputPixel = 24;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter
                setComponentInfo( 0x0000ffLL,
                                  0x00ff00LL,
                                  0xff0000LL );
                break;

            case BMP_FORMAT_24BIT_TC_MASK:
                m_bPalette           = false;
                m_nBitsPerInputPixel = 24;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter
                setComponentInfo( m_pBmpAcc->GetColorMask().GetRedMask(),
                                  m_pBmpAcc->GetColorMask().GetGreenMask(),
                                  m_pBmpAcc->GetColorMask().GetBlueMask() );
                break;

            case BMP_FORMAT_32BIT_TC_ABGR:
            {
                m_bPalette           = false;
                m_nBitsPerInputPixel = 32;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter

                m_aComponentTags.realloc(4);
                sal_Int8* pTags = m_aComponentTags.getArray();
                pTags[0]        = rendering::ColorComponentTag::ALPHA;
                pTags[1]        = rendering::ColorComponentTag::RGB_BLUE;
                pTags[2]        = rendering::ColorComponentTag::RGB_GREEN;
                pTags[3]        = rendering::ColorComponentTag::RGB_RED;

                m_aComponentBitCounts.realloc(4);
                sal_Int32* pCounts = m_aComponentBitCounts.getArray();
                pCounts[0]         = 8;
                pCounts[1]         = 8;
                pCounts[2]         = 8;
                pCounts[3]         = 8;

                m_nRedIndex   = 3;
                m_nGreenIndex = 2;
                m_nBlueIndex  = 1;
                m_nAlphaIndex = 0;
            }
            break;

            case BMP_FORMAT_32BIT_TC_ARGB:
            {
                m_bPalette           = false;
                m_nBitsPerInputPixel = 32;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter

                m_aComponentTags.realloc(4);
                sal_Int8* pTags = m_aComponentTags.getArray();
                pTags[0]        = rendering::ColorComponentTag::ALPHA;
                pTags[1]        = rendering::ColorComponentTag::RGB_RED;
                pTags[2]        = rendering::ColorComponentTag::RGB_GREEN;
                pTags[3]        = rendering::ColorComponentTag::RGB_BLUE;

                m_aComponentBitCounts.realloc(4);
                sal_Int32* pCounts = m_aComponentBitCounts.getArray();
                pCounts[0]         = 8;
                pCounts[1]         = 8;
                pCounts[2]         = 8;
                pCounts[3]         = 8;

                m_nRedIndex   = 1;
                m_nGreenIndex = 2;
                m_nBlueIndex  = 3;
                m_nAlphaIndex = 0;
            }
            break;

            case BMP_FORMAT_32BIT_TC_BGRA:
            {
                m_bPalette           = false;
                m_nBitsPerInputPixel = 32;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter

                m_aComponentTags.realloc(4);
                sal_Int8* pTags = m_aComponentTags.getArray();
                pTags[0]        = rendering::ColorComponentTag::RGB_BLUE;
                pTags[1]        = rendering::ColorComponentTag::RGB_GREEN;
                pTags[2]        = rendering::ColorComponentTag::RGB_RED;
                pTags[3]        = rendering::ColorComponentTag::ALPHA;

                m_aComponentBitCounts.realloc(4);
                sal_Int32* pCounts = m_aComponentBitCounts.getArray();
                pCounts[0]         = 8;
                pCounts[1]         = 8;
                pCounts[2]         = 8;
                pCounts[3]         = 8;

                m_nRedIndex   = 2;
                m_nGreenIndex = 1;
                m_nBlueIndex  = 0;
                m_nAlphaIndex = 3;
            }
            break;

            case BMP_FORMAT_32BIT_TC_RGBA:
            {
                m_bPalette           = false;
                m_nBitsPerInputPixel = 32;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter

                m_aComponentTags.realloc(4);
                sal_Int8* pTags = m_aComponentTags.getArray();
                pTags[0]        = rendering::ColorComponentTag::RGB_RED;
                pTags[1]        = rendering::ColorComponentTag::RGB_GREEN;
                pTags[2]        = rendering::ColorComponentTag::RGB_BLUE;
                pTags[3]        = rendering::ColorComponentTag::ALPHA;

                m_aComponentBitCounts.realloc(4);
                sal_Int32* pCounts = m_aComponentBitCounts.getArray();
                pCounts[0]         = 8;
                pCounts[1]         = 8;
                pCounts[2]         = 8;
                pCounts[3]         = 8;

                m_nRedIndex   = 0;
                m_nGreenIndex = 1;
                m_nBlueIndex  = 2;
                m_nAlphaIndex = 3;
            }
            break;

            case BMP_FORMAT_32BIT_TC_MASK:
                m_bPalette           = false;
                m_nBitsPerInputPixel = 32;
                m_nEndianness        = util::Endianness::LITTLE;
                m_aLayout.IsMsbFirst = sal_False; // doesn't matter
                setComponentInfo( m_pBmpAcc->GetColorMask().GetRedMask(),
                                  m_pBmpAcc->GetColorMask().GetGreenMask(),
                                  m_pBmpAcc->GetColorMask().GetBlueMask() );
                break;

            default:
                OSL_FAIL( "unsupported bitmap format" );
                break;
        }

        if( m_bPalette )
        {
            m_aComponentTags.realloc(1);
            m_aComponentTags[0] = rendering::ColorComponentTag::INDEX;

            m_aComponentBitCounts.realloc(1);
            m_aComponentBitCounts[0] = m_nBitsPerInputPixel;

            m_nIndexIndex = 0;
        }

        m_nBitsPerOutputPixel = m_nBitsPerInputPixel;
        if( m_aBmpEx.IsTransparent() )
        {
            // TODO(P1): need to interleave alpha with bitmap data -
            // won't fuss with less-than-8 bit for now
            m_nBitsPerOutputPixel = std::max(sal_Int32(8),m_nBitsPerInputPixel);

            // check whether alpha goes in front or behind the
            // bitcount sequence. If pixel format is little endian,
            // put it behind all the other channels. If it's big
            // endian, put it in front (because later, the actual data
            // always gets written after the pixel data)

            // TODO(Q1): slight catch - in the case of the
            // BMP_FORMAT_32BIT_XX_ARGB formats, duplicate alpha
            // channels might happen!
            m_aComponentTags.realloc(m_aComponentTags.getLength()+1);
            m_aComponentTags[m_aComponentTags.getLength()-1] = rendering::ColorComponentTag::ALPHA;

            m_aComponentBitCounts.realloc(m_aComponentBitCounts.getLength()+1);
            m_aComponentBitCounts[m_aComponentBitCounts.getLength()-1] = m_aBmpEx.IsAlpha() ? 8 : 1;

            if( m_nEndianness == util::Endianness::BIG )
            {
                // put alpha in front of all the color channels
                sal_Int8*  pTags  =m_aComponentTags.getArray();
                sal_Int32* pCounts=m_aComponentBitCounts.getArray();
                std::rotate(pTags,
                            pTags+m_aComponentTags.getLength()-1,
                            pTags+m_aComponentTags.getLength());
                std::rotate(pCounts,
                            pCounts+m_aComponentBitCounts.getLength()-1,
                            pCounts+m_aComponentBitCounts.getLength());
                ++m_nRedIndex;
                ++m_nGreenIndex;
                ++m_nBlueIndex;
                ++m_nIndexIndex;
                m_nAlphaIndex=0;
            }

            // always add a full byte to the pixel size, otherwise
            // pixel packing hell breaks loose.
            m_nBitsPerOutputPixel += 8;

            // adapt scanline parameters
            const Size aSize = m_aBitmap.GetSizePixel();
            m_aLayout.ScanLineBytes  =
            m_aLayout.ScanLineStride = (aSize.Width()*m_nBitsPerOutputPixel + 7)/8;
        }
    }
}

VclCanvasBitmap::~VclCanvasBitmap()
{
    if( m_pAlphaAcc )
        m_aAlpha.ReleaseAccess(m_pAlphaAcc);
    if( m_pBmpAcc )
        m_aBitmap.ReleaseAccess(m_pBmpAcc);
}

// XBitmap
geometry::IntegerSize2D SAL_CALL VclCanvasBitmap::getSize() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return integerSize2DFromSize( m_aBitmap.GetSizePixel() );
}

::sal_Bool SAL_CALL VclCanvasBitmap::hasAlpha() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return m_aBmpEx.IsTransparent();
}

uno::Reference< rendering::XBitmap > SAL_CALL VclCanvasBitmap::getScaledBitmap( const geometry::RealSize2D& newSize,
                                                                                sal_Bool beFast ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    BitmapEx aNewBmp( m_aBitmap );
    aNewBmp.Scale( sizeFromRealSize2D( newSize ), beFast ? BMP_SCALE_FAST : BMP_SCALE_DEFAULT );
    return uno::Reference<rendering::XBitmap>( new VclCanvasBitmap( aNewBmp ) );
}

// XIntegerReadOnlyBitmap
uno::Sequence< sal_Int8 > SAL_CALL VclCanvasBitmap::getData( rendering::IntegerBitmapLayout&     bitmapLayout,
                                                             const geometry::IntegerRectangle2D& rect ) throw( lang::IndexOutOfBoundsException,
                                                                                                               rendering::VolatileContentDestroyedException,
                                                                                                               uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    bitmapLayout = getMemoryLayout();

    const ::Rectangle aRequestedArea( vcl::unotools::rectangleFromIntegerRectangle2D(rect) );
    if( aRequestedArea.IsEmpty() )
        return uno::Sequence< sal_Int8 >();

    // Invalid/empty bitmap: no data available
    if( !m_pBmpAcc )
        throw lang::IndexOutOfBoundsException();
    if( m_aBmpEx.IsTransparent() && !m_pAlphaAcc )
        throw lang::IndexOutOfBoundsException();

    if( aRequestedArea.Left() < 0 || aRequestedArea.Top() < 0 ||
        aRequestedArea.Right() > m_pBmpAcc->Width() ||
        aRequestedArea.Bottom() > m_pBmpAcc->Height() )
    {
        throw lang::IndexOutOfBoundsException();
    }

    uno::Sequence< sal_Int8 > aRet;
    Rectangle aRequestedBytes( aRequestedArea );

    // adapt to byte boundaries
    aRequestedBytes.Left()  = aRequestedArea.Left()*m_nBitsPerOutputPixel/8;
    aRequestedBytes.Right() = (aRequestedArea.Right()*m_nBitsPerOutputPixel + 7)/8;

    // copy stuff to output sequence
    aRet.realloc(aRequestedBytes.getWidth()*aRequestedBytes.getHeight());
    sal_Int8* pOutBuf = aRet.getArray();

    bitmapLayout.ScanLines     = aRequestedBytes.getHeight();
    bitmapLayout.ScanLineBytes =
    bitmapLayout.ScanLineStride= aRequestedBytes.getWidth();

    sal_Int32 nScanlineStride=bitmapLayout.ScanLineStride;
    if( !(m_pBmpAcc->GetScanlineFormat() & BMP_FORMAT_TOP_DOWN) )
    {
        pOutBuf += bitmapLayout.ScanLineStride*(aRequestedBytes.getHeight()-1);
        nScanlineStride *= -1;
    }

    if( !m_aBmpEx.IsTransparent() )
    {
        OSL_ENSURE(m_pBmpAcc,"Invalid bmp read access");

        // can return bitmap data as-is
        for( long y=aRequestedBytes.Top(); y<aRequestedBytes.Bottom(); ++y )
        {
            Scanline pScan = m_pBmpAcc->GetScanline(y);
            memcpy(pOutBuf, pScan+aRequestedBytes.Left(), aRequestedBytes.getWidth());
            pOutBuf += nScanlineStride;
        }
    }
    else
    {
        OSL_ENSURE(m_pBmpAcc,"Invalid bmp read access");
        OSL_ENSURE(m_pAlphaAcc,"Invalid alpha read access");

        // interleave alpha with bitmap data - note, bitcount is
        // always integer multiple of 8
        OSL_ENSURE((m_nBitsPerOutputPixel & 0x07) == 0,
                   "Transparent bitmap bitcount not integer multiple of 8" );

        for( long y=aRequestedArea.Top(); y<aRequestedArea.Bottom(); ++y )
        {
            sal_Int8* pOutScan = pOutBuf;

            if( m_nBitsPerInputPixel < 8 )
            {
                // input less than a byte - copy via GetPixel()
                for( long x=aRequestedArea.Left(); x<aRequestedArea.Right(); ++x )
                {
                    *pOutScan++ = m_pBmpAcc->GetPixel(y,x);
                    *pOutScan++ = m_pAlphaAcc->GetPixel(y,x);
                }
            }
            else
            {
                const long nNonAlphaBytes( m_nBitsPerInputPixel/8 );
                const long nScanlineOffsetLeft(aRequestedArea.Left()*nNonAlphaBytes);
                Scanline  pScan = m_pBmpAcc->GetScanline(y) + nScanlineOffsetLeft;

                // input integer multiple of byte - copy directly
                for( long x=aRequestedArea.Left(); x<aRequestedArea.Right(); ++x )
                {
                    for( long i=0; i<nNonAlphaBytes; ++i )
                        *pOutScan++ = *pScan++;
                    *pOutScan++ = m_pAlphaAcc->GetPixel(y,x);
                }
            }

            pOutBuf += nScanlineStride;
        }
    }

    return aRet;
}

uno::Sequence< sal_Int8 > SAL_CALL VclCanvasBitmap::getPixel( rendering::IntegerBitmapLayout&   bitmapLayout,
                                                              const geometry::IntegerPoint2D&   pos ) throw (lang::IndexOutOfBoundsException,
                                                                                                             rendering::VolatileContentDestroyedException,
                                                                                                             uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    bitmapLayout = getMemoryLayout();

    // Invalid/empty bitmap: no data available
    if( !m_pBmpAcc )
        throw lang::IndexOutOfBoundsException();
    if( m_aBmpEx.IsTransparent() && !m_pAlphaAcc )
        throw lang::IndexOutOfBoundsException();

    if( pos.X < 0 || pos.Y < 0 ||
        pos.X > m_pBmpAcc->Width() || pos.Y > m_pBmpAcc->Height() )
    {
        throw lang::IndexOutOfBoundsException();
    }

    uno::Sequence< sal_Int8 > aRet((m_nBitsPerOutputPixel + 7)/8);
    sal_Int8* pOutBuf = aRet.getArray();

    // copy stuff to output sequence
    bitmapLayout.ScanLines     = 1;
    bitmapLayout.ScanLineBytes =
    bitmapLayout.ScanLineStride= aRet.getLength();

    const long nScanlineLeftOffset( pos.X*m_nBitsPerInputPixel/8 );
    if( !m_aBmpEx.IsTransparent() )
    {
        OSL_ENSURE(m_pBmpAcc,"Invalid bmp read access");

        // can return bitmap data as-is
        Scanline pScan = m_pBmpAcc->GetScanline(pos.Y);
        memcpy(pOutBuf, pScan+nScanlineLeftOffset, aRet.getLength() );
    }
    else
    {
        OSL_ENSURE(m_pBmpAcc,"Invalid bmp read access");
        OSL_ENSURE(m_pAlphaAcc,"Invalid alpha read access");

        // interleave alpha with bitmap data - note, bitcount is
        // always integer multiple of 8
        OSL_ENSURE((m_nBitsPerOutputPixel & 0x07) == 0,
                   "Transparent bitmap bitcount not integer multiple of 8" );

        if( m_nBitsPerInputPixel < 8 )
        {
            // input less than a byte - copy via GetPixel()
            *pOutBuf++ = m_pBmpAcc->GetPixel(pos.Y,pos.X);
            *pOutBuf   = m_pAlphaAcc->GetPixel(pos.Y,pos.X);
        }
        else
        {
            const long nNonAlphaBytes( m_nBitsPerInputPixel/8 );
            Scanline  pScan = m_pBmpAcc->GetScanline(pos.Y);

            // input integer multiple of byte - copy directly
            memcpy(pOutBuf, pScan+nScanlineLeftOffset, nNonAlphaBytes );
            pOutBuf += nNonAlphaBytes;
            *pOutBuf++ = m_pAlphaAcc->GetPixel(pos.Y,pos.X);
        }
    }

    return aRet;
}

uno::Reference< rendering::XBitmapPalette > SAL_CALL VclCanvasBitmap::getPalette() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference< XBitmapPalette > aRet;
    if( m_bPalette )
        aRet.set(this);

    return aRet;
}

rendering::IntegerBitmapLayout SAL_CALL VclCanvasBitmap::getMemoryLayout() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    rendering::IntegerBitmapLayout aLayout( m_aLayout );

    // only set references to self on separate copy of
    // IntegerBitmapLayout - if we'd set that on m_aLayout, we'd have
    // a circular reference!
    if( m_bPalette )
        aLayout.Palette.set( this );

    aLayout.ColorSpace.set( this );

    return aLayout;
}

sal_Int32 SAL_CALL VclCanvasBitmap::getNumberOfEntries() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( !m_pBmpAcc )
        return 0;

    return m_pBmpAcc->HasPalette() ? m_pBmpAcc->GetPaletteEntryCount() : 0 ;
}

sal_Bool SAL_CALL VclCanvasBitmap::getIndex( uno::Sequence< double >& o_entry, sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_uInt16 nCount( m_pBmpAcc ?
                         (m_pBmpAcc->HasPalette() ? m_pBmpAcc->GetPaletteEntryCount() : 0 ) : 0 );
    OSL_ENSURE(nIndex >= 0 && nIndex < nCount,"Palette index out of range");
    if( nIndex < 0 || nIndex >= nCount )
        throw lang::IndexOutOfBoundsException(::rtl::OUString("Palette index out of range"),
                                              static_cast<rendering::XBitmapPalette*>(this));

    const BitmapColor aCol = m_pBmpAcc->GetPaletteColor(sal::static_int_cast<sal_uInt16>(nIndex));
    o_entry.realloc(3);
    double* pColor=o_entry.getArray();
    pColor[0] = aCol.GetRed();
    pColor[1] = aCol.GetGreen();
    pColor[2] = aCol.GetBlue();

    return sal_True; // no palette transparency here.
}

sal_Bool SAL_CALL VclCanvasBitmap::setIndex( const uno::Sequence< double >&, sal_Bool, sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_uInt16 nCount( m_pBmpAcc ?
                         (m_pBmpAcc->HasPalette() ? m_pBmpAcc->GetPaletteEntryCount() : 0 ) : 0 );

    OSL_ENSURE(nIndex >= 0 && nIndex < nCount,"Palette index out of range");
    if( nIndex < 0 || nIndex >= nCount )
        throw lang::IndexOutOfBoundsException(::rtl::OUString("Palette index out of range"),
                                              static_cast<rendering::XBitmapPalette*>(this));

    return sal_False; // read-only implementation
}

namespace
{
    struct PaletteColorSpaceHolder: public rtl::StaticWithInit<uno::Reference<rendering::XColorSpace>,
                                                               PaletteColorSpaceHolder>
    {
        uno::Reference<rendering::XColorSpace> operator()()
        {
            return vcl::unotools::createStandardColorSpace();
        }
    };
}

uno::Reference< rendering::XColorSpace > SAL_CALL VclCanvasBitmap::getColorSpace(  ) throw (uno::RuntimeException)
{
    // this is the method from XBitmapPalette. Return palette color
    // space here
    return PaletteColorSpaceHolder::get();
}

sal_Int8 SAL_CALL VclCanvasBitmap::getType(  ) throw (uno::RuntimeException)
{
    return rendering::ColorSpaceType::RGB;
}

uno::Sequence< ::sal_Int8 > SAL_CALL VclCanvasBitmap::getComponentTags(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return m_aComponentTags;
}

sal_Int8 SAL_CALL VclCanvasBitmap::getRenderingIntent(  ) throw (uno::RuntimeException)
{
    return rendering::RenderingIntent::PERCEPTUAL;
}

uno::Sequence< ::beans::PropertyValue > SAL_CALL VclCanvasBitmap::getProperties(  ) throw (uno::RuntimeException)
{
    return uno::Sequence< ::beans::PropertyValue >();
}

uno::Sequence< double > SAL_CALL VclCanvasBitmap::convertColorSpace( const uno::Sequence< double >& deviceColor,
                                                                     const uno::Reference< ::rendering::XColorSpace >& targetColorSpace ) throw (uno::RuntimeException)
{
    // TODO(P3): if we know anything about target
    // colorspace, this can be greatly sped up
    uno::Sequence<rendering::ARGBColor> aIntermediate(
        convertToARGB(deviceColor));
    return targetColorSpace->convertFromARGB(aIntermediate);
}

uno::Sequence<rendering::RGBColor> SAL_CALL VclCanvasBitmap::convertToRGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( deviceColor.getLength() );
    const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());
    ENSURE_ARG_OR_THROW2(nLen%nComponentsPerPixel==0,
                         "number of channels no multiple of pixel element count",
                         static_cast<rendering::XBitmapPalette*>(this), 01);

    uno::Sequence< rendering::RGBColor > aRes(nLen/nComponentsPerPixel);
    rendering::RGBColor* pOut( aRes.getArray() );

    if( m_bPalette )
    {
        OSL_ENSURE(m_nIndexIndex != -1,
                   "Invalid color channel indices");
        ENSURE_OR_THROW(m_pBmpAcc,
                        "Unable to get BitmapAccess");

        for( sal_Size i=0; i<nLen; i+=nComponentsPerPixel )
        {
            const BitmapColor aCol = m_pBmpAcc->GetPaletteColor(
                sal::static_int_cast<sal_uInt16>(deviceColor[i+m_nIndexIndex]));

            // TODO(F3): Convert result to sRGB color space
            *pOut++ = rendering::RGBColor(toDoubleColor(aCol.GetRed()),
                                          toDoubleColor(aCol.GetGreen()),
                                          toDoubleColor(aCol.GetBlue()));
        }
    }
    else
    {
        OSL_ENSURE(m_nRedIndex != -1 && m_nGreenIndex != -1 && m_nBlueIndex != -1,
                   "Invalid color channel indices");

        for( sal_Size i=0; i<nLen; i+=nComponentsPerPixel )
        {
            // TODO(F3): Convert result to sRGB color space
            *pOut++ = rendering::RGBColor(
                deviceColor[i+m_nRedIndex],
                deviceColor[i+m_nGreenIndex],
                deviceColor[i+m_nBlueIndex]);
        }
    }

    return aRes;
}

uno::Sequence<rendering::ARGBColor> SAL_CALL VclCanvasBitmap::convertToARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( deviceColor.getLength() );
    const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());
    ENSURE_ARG_OR_THROW2(nLen%nComponentsPerPixel==0,
                         "number of channels no multiple of pixel element count",
                         static_cast<rendering::XBitmapPalette*>(this), 01);

    uno::Sequence< rendering::ARGBColor > aRes(nLen/nComponentsPerPixel);
    rendering::ARGBColor* pOut( aRes.getArray() );

    if( m_bPalette )
    {
        OSL_ENSURE(m_nIndexIndex != -1,
                   "Invalid color channel indices");
        ENSURE_OR_THROW(m_pBmpAcc,
                        "Unable to get BitmapAccess");

        for( sal_Size i=0; i<nLen; i+=nComponentsPerPixel )
        {
            const BitmapColor aCol = m_pBmpAcc->GetPaletteColor(
                sal::static_int_cast<sal_uInt16>(deviceColor[i+m_nIndexIndex]));

            // TODO(F3): Convert result to sRGB color space
            const double nAlpha( m_nAlphaIndex != -1 ? 1.0 - deviceColor[i+m_nAlphaIndex] : 1.0 );
            *pOut++ = rendering::ARGBColor(nAlpha,
                                           toDoubleColor(aCol.GetRed()),
                                           toDoubleColor(aCol.GetGreen()),
                                           toDoubleColor(aCol.GetBlue()));
        }
    }
    else
    {
        OSL_ENSURE(m_nRedIndex != -1 && m_nGreenIndex != -1 && m_nBlueIndex != -1,
                   "Invalid color channel indices");

        for( sal_Size i=0; i<nLen; i+=nComponentsPerPixel )
        {
            // TODO(F3): Convert result to sRGB color space
            const double nAlpha( m_nAlphaIndex != -1 ? 1.0 - deviceColor[i+m_nAlphaIndex] : 1.0 );
            *pOut++ = rendering::ARGBColor(
                nAlpha,
                deviceColor[i+m_nRedIndex],
                deviceColor[i+m_nGreenIndex],
                deviceColor[i+m_nBlueIndex]);
        }
    }

    return aRes;
}

uno::Sequence<rendering::ARGBColor> SAL_CALL VclCanvasBitmap::convertToPARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( deviceColor.getLength() );
    const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());
    ENSURE_ARG_OR_THROW2(nLen%nComponentsPerPixel==0,
                         "number of channels no multiple of pixel element count",
                         static_cast<rendering::XBitmapPalette*>(this), 01);

    uno::Sequence< rendering::ARGBColor > aRes(nLen/nComponentsPerPixel);
    rendering::ARGBColor* pOut( aRes.getArray() );

    if( m_bPalette )
    {
        OSL_ENSURE(m_nIndexIndex != -1,
                   "Invalid color channel indices");
        ENSURE_OR_THROW(m_pBmpAcc,
                        "Unable to get BitmapAccess");

        for( sal_Size i=0; i<nLen; i+=nComponentsPerPixel )
        {
            const BitmapColor aCol = m_pBmpAcc->GetPaletteColor(
                sal::static_int_cast<sal_uInt16>(deviceColor[i+m_nIndexIndex]));

            // TODO(F3): Convert result to sRGB color space
            const double nAlpha( m_nAlphaIndex != -1 ? 1.0 - deviceColor[i+m_nAlphaIndex] : 1.0 );
            *pOut++ = rendering::ARGBColor(nAlpha,
                                           nAlpha*toDoubleColor(aCol.GetRed()),
                                           nAlpha*toDoubleColor(aCol.GetGreen()),
                                           nAlpha*toDoubleColor(aCol.GetBlue()));
        }
    }
    else
    {
        OSL_ENSURE(m_nRedIndex != -1 && m_nGreenIndex != -1 && m_nBlueIndex != -1,
                   "Invalid color channel indices");

        for( sal_Size i=0; i<nLen; i+=nComponentsPerPixel )
        {
            // TODO(F3): Convert result to sRGB color space
            const double nAlpha( m_nAlphaIndex != -1 ? 1.0 - deviceColor[i+m_nAlphaIndex] : 1.0 );
            *pOut++ = rendering::ARGBColor(
                nAlpha,
                nAlpha*deviceColor[i+m_nRedIndex],
                nAlpha*deviceColor[i+m_nGreenIndex],
                nAlpha*deviceColor[i+m_nBlueIndex]);
        }
    }

    return aRes;
}

uno::Sequence< double > SAL_CALL VclCanvasBitmap::convertFromRGB( const uno::Sequence<rendering::RGBColor>& rgbColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( rgbColor.getLength() );
    const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());

    uno::Sequence< double > aRes(nLen*nComponentsPerPixel);
    double* pColors=aRes.getArray();

    if( m_bPalette )
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            pColors[m_nIndexIndex] = m_pBmpAcc->GetBestPaletteIndex(
                    BitmapColor(toByteColor(rgbColor[i].Red),
                                toByteColor(rgbColor[i].Green),
                                toByteColor(rgbColor[i].Blue)));
            if( m_nAlphaIndex != -1 )
                pColors[m_nAlphaIndex] = 1.0;

            pColors += nComponentsPerPixel;
        }
    }
    else
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            pColors[m_nRedIndex]   = rgbColor[i].Red;
            pColors[m_nGreenIndex] = rgbColor[i].Green;
            pColors[m_nBlueIndex]  = rgbColor[i].Blue;
            if( m_nAlphaIndex != -1 )
                pColors[m_nAlphaIndex] = 1.0;

            pColors += nComponentsPerPixel;
        }
    }
    return aRes;
}

uno::Sequence< double > SAL_CALL VclCanvasBitmap::convertFromARGB( const uno::Sequence<rendering::ARGBColor>& rgbColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( rgbColor.getLength() );
    const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());

    uno::Sequence< double > aRes(nLen*nComponentsPerPixel);
    double* pColors=aRes.getArray();

    if( m_bPalette )
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            pColors[m_nIndexIndex] = m_pBmpAcc->GetBestPaletteIndex(
                    BitmapColor(toByteColor(rgbColor[i].Red),
                                toByteColor(rgbColor[i].Green),
                                toByteColor(rgbColor[i].Blue)));
            if( m_nAlphaIndex != -1 )
                pColors[m_nAlphaIndex] = rgbColor[i].Alpha;

            pColors += nComponentsPerPixel;
        }
    }
    else
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            pColors[m_nRedIndex]   = rgbColor[i].Red;
            pColors[m_nGreenIndex] = rgbColor[i].Green;
            pColors[m_nBlueIndex]  = rgbColor[i].Blue;
            if( m_nAlphaIndex != -1 )
                pColors[m_nAlphaIndex] = rgbColor[i].Alpha;

            pColors += nComponentsPerPixel;
        }
    }
    return aRes;
}

uno::Sequence< double > SAL_CALL VclCanvasBitmap::convertFromPARGB( const uno::Sequence<rendering::ARGBColor>& rgbColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( rgbColor.getLength() );
    const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());

    uno::Sequence< double > aRes(nLen*nComponentsPerPixel);
    double* pColors=aRes.getArray();

    if( m_bPalette )
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            const double nAlpha( rgbColor[i].Alpha );
            pColors[m_nIndexIndex] = m_pBmpAcc->GetBestPaletteIndex(
                    BitmapColor(toByteColor(rgbColor[i].Red / nAlpha),
                                toByteColor(rgbColor[i].Green / nAlpha),
                                toByteColor(rgbColor[i].Blue / nAlpha)));
            if( m_nAlphaIndex != -1 )
                pColors[m_nAlphaIndex] = nAlpha;

            pColors += nComponentsPerPixel;
        }
    }
    else
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            const double nAlpha( rgbColor[i].Alpha );
            pColors[m_nRedIndex]   = rgbColor[i].Red / nAlpha;
            pColors[m_nGreenIndex] = rgbColor[i].Green / nAlpha;
            pColors[m_nBlueIndex]  = rgbColor[i].Blue / nAlpha;
            if( m_nAlphaIndex != -1 )
                pColors[m_nAlphaIndex] = nAlpha;

            pColors += nComponentsPerPixel;
        }
    }
    return aRes;
}

sal_Int32 SAL_CALL VclCanvasBitmap::getBitsPerPixel(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return m_nBitsPerOutputPixel;
}

uno::Sequence< ::sal_Int32 > SAL_CALL VclCanvasBitmap::getComponentBitCounts(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return m_aComponentBitCounts;
}

sal_Int8 SAL_CALL VclCanvasBitmap::getEndianness(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return m_nEndianness;
}

uno::Sequence<double> SAL_CALL VclCanvasBitmap::convertFromIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                              const uno::Reference< ::rendering::XColorSpace >& targetColorSpace ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    if( dynamic_cast<VclCanvasBitmap*>(targetColorSpace.get()) )
    {
        SolarMutexGuard aGuard;

        const sal_Size  nLen( deviceColor.getLength() );
        const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());
        ENSURE_ARG_OR_THROW2(nLen%nComponentsPerPixel==0,
                             "number of channels no multiple of pixel element count",
                             static_cast<rendering::XBitmapPalette*>(this), 01);

        uno::Sequence<double> aRes(nLen);
        double* pOut( aRes.getArray() );

        if( m_bPalette )
        {
            OSL_ENSURE(m_nIndexIndex != -1,
                       "Invalid color channel indices");
            ENSURE_OR_THROW(m_pBmpAcc,
                            "Unable to get BitmapAccess");

            for( sal_Size i=0; i<nLen; i+=nComponentsPerPixel )
            {
                const BitmapColor aCol = m_pBmpAcc->GetPaletteColor(
                    sal::static_int_cast<sal_uInt16>(deviceColor[i+m_nIndexIndex]));

                // TODO(F3): Convert result to sRGB color space
                const double nAlpha( m_nAlphaIndex != -1 ? 1.0 - deviceColor[i+m_nAlphaIndex] : 1.0 );
                *pOut++ = toDoubleColor(aCol.GetRed());
                *pOut++ = toDoubleColor(aCol.GetGreen());
                *pOut++ = toDoubleColor(aCol.GetBlue());
                *pOut++ = nAlpha;
            }
        }
        else
        {
            OSL_ENSURE(m_nRedIndex != -1 && m_nGreenIndex != -1 && m_nBlueIndex != -1,
                       "Invalid color channel indices");

            for( sal_Size i=0; i<nLen; i+=nComponentsPerPixel )
            {
                // TODO(F3): Convert result to sRGB color space
                const double nAlpha( m_nAlphaIndex != -1 ? 1.0 - deviceColor[i+m_nAlphaIndex] : 1.0 );
                *pOut++ = deviceColor[i+m_nRedIndex];
                *pOut++ = deviceColor[i+m_nGreenIndex];
                *pOut++ = deviceColor[i+m_nBlueIndex];
                *pOut++ = nAlpha;
            }
        }

        return aRes;
    }
    else
    {
        // TODO(P3): if we know anything about target
        // colorspace, this can be greatly sped up
        uno::Sequence<rendering::ARGBColor> aIntermediate(
            convertIntegerToARGB(deviceColor));
        return targetColorSpace->convertFromARGB(aIntermediate);
    }
}

uno::Sequence< ::sal_Int8 > SAL_CALL VclCanvasBitmap::convertToIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                  const uno::Reference< ::rendering::XIntegerBitmapColorSpace >& targetColorSpace ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    if( dynamic_cast<VclCanvasBitmap*>(targetColorSpace.get()) )
    {
        // it's us, so simply pass-through the data
        return deviceColor;
    }
    else
    {
        // TODO(P3): if we know anything about target
        // colorspace, this can be greatly sped up
        uno::Sequence<rendering::ARGBColor> aIntermediate(
            convertIntegerToARGB(deviceColor));
        return targetColorSpace->convertIntegerFromARGB(aIntermediate);
    }
}

uno::Sequence<rendering::RGBColor> SAL_CALL VclCanvasBitmap::convertIntegerToRGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_uInt8*     pIn( reinterpret_cast<const sal_uInt8*>(deviceColor.getConstArray()) );
    const sal_Size  nLen( deviceColor.getLength() );
    const sal_Int32 nNumColors((nLen*8 + m_nBitsPerOutputPixel-1)/m_nBitsPerOutputPixel);

    uno::Sequence< rendering::RGBColor > aRes(nNumColors);
    rendering::RGBColor* pOut( aRes.getArray() );

    ENSURE_OR_THROW(m_pBmpAcc,
                    "Unable to get BitmapAccess");

    if( m_aBmpEx.IsTransparent() )
    {
        const sal_Int32 nBytesPerPixel((m_nBitsPerOutputPixel+7)/8);
        for( sal_Size i=0; i<nLen; i+=nBytesPerPixel )
        {
            // if palette, index is guaranteed to be 8 bit
            const BitmapColor aCol =
                m_bPalette ?
                m_pBmpAcc->GetPaletteColor(*pIn) :
                m_pBmpAcc->GetPixelFromData(pIn,0);

            // TODO(F3): Convert result to sRGB color space
            *pOut++ = rendering::RGBColor(toDoubleColor(aCol.GetRed()),
                                          toDoubleColor(aCol.GetGreen()),
                                          toDoubleColor(aCol.GetBlue()));
            // skips alpha
            pIn += nBytesPerPixel;
        }
    }
    else
    {
        for( sal_Int32 i=0; i<nNumColors; ++i )
        {
            const BitmapColor aCol =
                m_bPalette ?
                m_pBmpAcc->GetPaletteColor(
                    sal::static_int_cast<sal_uInt16>(
                        m_pBmpAcc->GetPixelFromData(
                            pIn, i ))) :
                m_pBmpAcc->GetPixelFromData(pIn, i);

            // TODO(F3): Convert result to sRGB color space
            *pOut++ = rendering::RGBColor(toDoubleColor(aCol.GetRed()),
                                          toDoubleColor(aCol.GetGreen()),
                                          toDoubleColor(aCol.GetBlue()));
        }
    }

    return aRes;
}

uno::Sequence<rendering::ARGBColor> SAL_CALL VclCanvasBitmap::convertIntegerToARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_uInt8*     pIn( reinterpret_cast<const sal_uInt8*>(deviceColor.getConstArray()) );
    const sal_Size  nLen( deviceColor.getLength() );
    const sal_Int32 nNumColors((nLen*8 + m_nBitsPerOutputPixel-1)/m_nBitsPerOutputPixel);

    uno::Sequence< rendering::ARGBColor > aRes(nNumColors);
    rendering::ARGBColor* pOut( aRes.getArray() );

    ENSURE_OR_THROW(m_pBmpAcc,
                    "Unable to get BitmapAccess");

    if( m_aBmpEx.IsTransparent() )
    {
        const long      nNonAlphaBytes( (m_nBitsPerInputPixel+7)/8 );
        const sal_Int32 nBytesPerPixel((m_nBitsPerOutputPixel+7)/8);
        const sal_uInt8 nAlphaFactor( m_aBmpEx.IsAlpha() ? 1 : 255 );
        for( sal_Size i=0; i<nLen; i+=nBytesPerPixel )
        {
            // if palette, index is guaranteed to be 8 bit
            const BitmapColor aCol =
                m_bPalette ?
                m_pBmpAcc->GetPaletteColor(*pIn) :
                m_pBmpAcc->GetPixelFromData(pIn,0);

            // TODO(F3): Convert result to sRGB color space
            *pOut++ = rendering::ARGBColor(1.0 - toDoubleColor(nAlphaFactor*pIn[nNonAlphaBytes]),
                                           toDoubleColor(aCol.GetRed()),
                                           toDoubleColor(aCol.GetGreen()),
                                           toDoubleColor(aCol.GetBlue()));
            pIn += nBytesPerPixel;
        }
    }
    else
    {
        for( sal_Int32 i=0; i<nNumColors; ++i )
        {
            const BitmapColor aCol =
                m_bPalette ?
                m_pBmpAcc->GetPaletteColor(
                    sal::static_int_cast<sal_uInt16>(
                        m_pBmpAcc->GetPixelFromData(
                            pIn, i ))) :
                m_pBmpAcc->GetPixelFromData(pIn, i);

            // TODO(F3): Convert result to sRGB color space
            *pOut++ = rendering::ARGBColor(1.0,
                                           toDoubleColor(aCol.GetRed()),
                                           toDoubleColor(aCol.GetGreen()),
                                           toDoubleColor(aCol.GetBlue()));
        }
    }

    return aRes;
}

uno::Sequence<rendering::ARGBColor> SAL_CALL VclCanvasBitmap::convertIntegerToPARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_uInt8*     pIn( reinterpret_cast<const sal_uInt8*>(deviceColor.getConstArray()) );
    const sal_Size  nLen( deviceColor.getLength() );
    const sal_Int32 nNumColors((nLen*8 + m_nBitsPerOutputPixel-1)/m_nBitsPerOutputPixel);

    uno::Sequence< rendering::ARGBColor > aRes(nNumColors);
    rendering::ARGBColor* pOut( aRes.getArray() );

    ENSURE_OR_THROW(m_pBmpAcc,
                    "Unable to get BitmapAccess");

    if( m_aBmpEx.IsTransparent() )
    {
        const long      nNonAlphaBytes( (m_nBitsPerInputPixel+7)/8 );
        const sal_Int32 nBytesPerPixel((m_nBitsPerOutputPixel+7)/8);
        const sal_uInt8 nAlphaFactor( m_aBmpEx.IsAlpha() ? 1 : 255 );
        for( sal_Size i=0; i<nLen; i+=nBytesPerPixel )
        {
            // if palette, index is guaranteed to be 8 bit
            const BitmapColor aCol =
                m_bPalette ?
                m_pBmpAcc->GetPaletteColor(*pIn) :
                m_pBmpAcc->GetPixelFromData(pIn,0);

            // TODO(F3): Convert result to sRGB color space
            const double nAlpha( 1.0 - toDoubleColor(nAlphaFactor*pIn[nNonAlphaBytes]) );
            *pOut++ = rendering::ARGBColor(nAlpha,
                                           nAlpha*toDoubleColor(aCol.GetRed()),
                                           nAlpha*toDoubleColor(aCol.GetGreen()),
                                           nAlpha*toDoubleColor(aCol.GetBlue()));
            pIn += nBytesPerPixel;
        }
    }
    else
    {
        for( sal_Int32 i=0; i<nNumColors; ++i )
        {
            const BitmapColor aCol =
                m_bPalette ?
                m_pBmpAcc->GetPaletteColor(
                    sal::static_int_cast<sal_uInt16>(
                        m_pBmpAcc->GetPixelFromData(
                            pIn, i ))) :
                m_pBmpAcc->GetPixelFromData(pIn, i);

            // TODO(F3): Convert result to sRGB color space
            *pOut++ = rendering::ARGBColor(1.0,
                                           toDoubleColor(aCol.GetRed()),
                                           toDoubleColor(aCol.GetGreen()),
                                           toDoubleColor(aCol.GetBlue()));
        }
    }

    return aRes;
}

uno::Sequence< ::sal_Int8 > SAL_CALL VclCanvasBitmap::convertIntegerFromRGB( const uno::Sequence<rendering::RGBColor>& rgbColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( rgbColor.getLength() );
    const sal_Int32 nNumBytes((nLen*m_nBitsPerOutputPixel+7)/8);

    uno::Sequence< sal_Int8 > aRes(nNumBytes);
    sal_uInt8* pColors=reinterpret_cast<sal_uInt8*>(aRes.getArray());

    if( m_aBmpEx.IsTransparent() )
    {
        const long nNonAlphaBytes( (m_nBitsPerInputPixel+7)/8 );
        for( sal_Size i=0; i<nLen; ++i )
        {
            const BitmapColor aCol(toByteColor(rgbColor[i].Red),
                                   toByteColor(rgbColor[i].Green),
                                   toByteColor(rgbColor[i].Blue));
            const BitmapColor aCol2 =
                m_bPalette ?
                BitmapColor(
                    sal::static_int_cast<sal_uInt8>(m_pBmpAcc->GetBestPaletteIndex( aCol ))) :
                aCol;

            m_pBmpAcc->SetPixelOnData(pColors,i,aCol2);
            pColors   += nNonAlphaBytes;
            *pColors++ = sal_uInt8(255);
        }
    }
    else
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            const BitmapColor aCol(toByteColor(rgbColor[i].Red),
                                   toByteColor(rgbColor[i].Green),
                                   toByteColor(rgbColor[i].Blue));
            const BitmapColor aCol2 =
                m_bPalette ?
                BitmapColor(
                    sal::static_int_cast<sal_uInt8>(m_pBmpAcc->GetBestPaletteIndex( aCol ))) :
                aCol;

            m_pBmpAcc->SetPixelOnData(pColors,i,aCol2);
        }
    }

    return aRes;
}

uno::Sequence< ::sal_Int8 > SAL_CALL VclCanvasBitmap::convertIntegerFromARGB( const uno::Sequence<rendering::ARGBColor>& rgbColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( rgbColor.getLength() );
    const sal_Int32 nNumBytes((nLen*m_nBitsPerOutputPixel+7)/8);

    uno::Sequence< sal_Int8 > aRes(nNumBytes);
    sal_uInt8* pColors=reinterpret_cast<sal_uInt8*>(aRes.getArray());

    if( m_aBmpEx.IsTransparent() )
    {
        const long nNonAlphaBytes( (m_nBitsPerInputPixel+7)/8 );
        for( sal_Size i=0; i<nLen; ++i )
        {
            const BitmapColor aCol(toByteColor(rgbColor[i].Red),
                                   toByteColor(rgbColor[i].Green),
                                   toByteColor(rgbColor[i].Blue));
            const BitmapColor aCol2 =
                m_bPalette ?
                BitmapColor(
                    sal::static_int_cast<sal_uInt8>(m_pBmpAcc->GetBestPaletteIndex( aCol ))) :
                aCol;

            m_pBmpAcc->SetPixelOnData(pColors,i,aCol2);
            pColors   += nNonAlphaBytes;
            *pColors++ = 255 - toByteColor(rgbColor[i].Alpha);
        }
    }
    else
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            const BitmapColor aCol(toByteColor(rgbColor[i].Red),
                                   toByteColor(rgbColor[i].Green),
                                   toByteColor(rgbColor[i].Blue));
            const BitmapColor aCol2 =
                m_bPalette ?
                BitmapColor(
                    sal::static_int_cast<sal_uInt8>(m_pBmpAcc->GetBestPaletteIndex( aCol ))) :
                aCol;

            m_pBmpAcc->SetPixelOnData(pColors,i,aCol2);
        }
    }

    return aRes;
}

uno::Sequence< ::sal_Int8 > SAL_CALL VclCanvasBitmap::convertIntegerFromPARGB( const uno::Sequence<rendering::ARGBColor>& rgbColor ) throw (lang::IllegalArgumentException,uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    const sal_Size  nLen( rgbColor.getLength() );
    const sal_Int32 nNumBytes((nLen*m_nBitsPerOutputPixel+7)/8);

    uno::Sequence< sal_Int8 > aRes(nNumBytes);
    sal_uInt8* pColors=reinterpret_cast<sal_uInt8*>(aRes.getArray());

    if( m_aBmpEx.IsTransparent() )
    {
        const long nNonAlphaBytes( (m_nBitsPerInputPixel+7)/8 );
        for( sal_Size i=0; i<nLen; ++i )
        {
            const double nAlpha( rgbColor[i].Alpha );
            const BitmapColor aCol(toByteColor(rgbColor[i].Red / nAlpha),
                                   toByteColor(rgbColor[i].Green / nAlpha),
                                   toByteColor(rgbColor[i].Blue / nAlpha));
            const BitmapColor aCol2 =
                m_bPalette ?
                BitmapColor(
                    sal::static_int_cast<sal_uInt8>(m_pBmpAcc->GetBestPaletteIndex( aCol ))) :
                aCol;

            m_pBmpAcc->SetPixelOnData(pColors,i,aCol2);
            pColors   += nNonAlphaBytes;
            *pColors++ = 255 - toByteColor(nAlpha);
        }
    }
    else
    {
        for( sal_Size i=0; i<nLen; ++i )
        {
            const BitmapColor aCol(toByteColor(rgbColor[i].Red),
                                   toByteColor(rgbColor[i].Green),
                                   toByteColor(rgbColor[i].Blue));
            const BitmapColor aCol2 =
                m_bPalette ?
                BitmapColor(
                    sal::static_int_cast<sal_uInt8>(m_pBmpAcc->GetBestPaletteIndex( aCol ))) :
                aCol;

            m_pBmpAcc->SetPixelOnData(pColors,i,aCol2);
        }
    }

    return aRes;
}

BitmapEx VclCanvasBitmap::getBitmapEx() const
{
    return m_aBmpEx;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
