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

#ifndef IOS

#include "headless/svpbmp.hxx"
#include "headless/svpinst.hxx"

#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basebmp/color.hxx>

#include <vcl/salbtype.hxx>
#include <vcl/bitmap.hxx>

using namespace basebmp;
using namespace basegfx;

SvpSalBitmap::~SvpSalBitmap()
{
}

bool SvpSalBitmap::Create( const Size& rSize,
                           sal_uInt16 nBitCount,
                           const BitmapPalette& rPalette )
{
    SAL_INFO( "vcl.headless", "SvpSalBitmap::Create(" << rSize.Width() << "," << rSize.Height() << "," << nBitCount << ")" );

    SvpSalInstance* pInst = SvpSalInstance::s_pDefaultInstance;
    assert( pInst );
    basebmp::Format nFormat = SvpSalInstance::getBaseBmpFormatForBitCount( nBitCount );

    B2IVector aSize( rSize.Width(), rSize.Height() );
    if( aSize.getX() == 0 )
        aSize.setX( 1 );
    if( aSize.getY() == 0 )
        aSize.setY( 1 );
    sal_Int32 nStride = getBitmapDeviceStrideForWidth(nFormat, aSize.getX());
    if( nBitCount > 8 )
        m_aBitmap = createBitmapDevice( aSize, true, nFormat, nStride );
    else
    {
        // prepare palette
        unsigned int nEntries = 1U << nBitCount;
        std::vector<basebmp::Color>* pPalette =
            new std::vector<basebmp::Color>( nEntries, basebmp::Color(COL_WHITE) );
        unsigned int nColors = rPalette.GetEntryCount();
        for( unsigned int i = 0; i < nColors; i++ )
        {
            const BitmapColor& rCol = rPalette[i];
            (*pPalette)[i] = basebmp::Color( rCol.GetRed(), rCol.GetGreen(), rCol.GetBlue() );
        }
        m_aBitmap = createBitmapDevice( aSize, true, nFormat, nStride,
                                        basebmp::RawMemorySharedArray(),
                                        basebmp::PaletteMemorySharedVector( pPalette )
                                        );
    }
    return true;
}

bool SvpSalBitmap::Create( const SalBitmap& rSalBmp )
{
    const SvpSalBitmap& rSrc = static_cast<const SvpSalBitmap&>(rSalBmp);
    const BitmapDeviceSharedPtr& rSrcBmp = rSrc.getBitmap();
    if( rSrcBmp.get() )
    {
        B2IVector aSize = rSrcBmp->getSize();
        m_aBitmap = cloneBitmapDevice( aSize, rSrcBmp );
        B2IBox aRect( 0, 0, aSize.getX(), aSize.getY() );
        m_aBitmap->drawBitmap( rSrcBmp, aRect, aRect, DrawMode::Paint );
    }
    else
        m_aBitmap.reset();

    return true;
}

bool SvpSalBitmap::Create( const SalBitmap& /*rSalBmp*/,
                           SalGraphics* /*pGraphics*/ )
{
    return false;
}

bool SvpSalBitmap::Create( const SalBitmap& /*rSalBmp*/,
                           sal_uInt16 /*nNewBitCount*/ )
{
    return false;
}

bool SvpSalBitmap::Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& /*xBitmapCanvas*/, Size& /*rSize*/, bool /*bMask*/ )
{
    return false;
}

void SvpSalBitmap::Destroy()
{
    m_aBitmap.reset();
}

Size SvpSalBitmap::GetSize() const
{
    Size aSize;
    if( m_aBitmap.get() )
    {
        B2IVector aVec( m_aBitmap->getSize() );
        aSize = Size( aVec.getX(), aVec.getY() );
    }

    return aSize;
}

sal_uInt16 SvpSalBitmap::GetBitCount() const
{
    sal_uInt16 nDepth = 0;
    if( m_aBitmap.get() )
        nDepth = getBitCountFromScanlineFormat( m_aBitmap->getScanlineFormat() );
    return nDepth;
}

BitmapBuffer* SvpSalBitmap::AcquireBuffer( BitmapAccessMode )
{
    BitmapBuffer* pBuf = nullptr;
    if( m_aBitmap.get() )
    {
        pBuf = new BitmapBuffer();
        sal_uInt16 nBitCount = 1;
        switch( m_aBitmap->getScanlineFormat() )
        {
            case Format::OneBitMsbGrey:
            case Format::OneBitMsbPal:
                nBitCount = 1;
                pBuf->mnFormat = BMP_FORMAT_1BIT_MSB_PAL;
                break;
            case Format::OneBitLsbGrey:
            case Format::OneBitLsbPal:
                nBitCount = 1;
                pBuf->mnFormat = BMP_FORMAT_1BIT_LSB_PAL;
                break;
            case Format::FourBitMsbGrey:
            case Format::FourBitMsbPal:
                nBitCount = 4;
                pBuf->mnFormat = BMP_FORMAT_4BIT_MSN_PAL;
                break;
            case Format::FourBitLsbGrey:
            case Format::FourBitLsbPal:
                nBitCount = 4;
                pBuf->mnFormat = BMP_FORMAT_4BIT_LSN_PAL;
                break;
            case Format::EightBitPal:
                nBitCount = 8;
                pBuf->mnFormat = BMP_FORMAT_8BIT_PAL;
                break;
            case Format::EightBitGrey:
                nBitCount = 8;
                pBuf->mnFormat = BMP_FORMAT_8BIT_PAL;
                break;
            case Format::SixteenBitLsbTcMask:
            {
                nBitCount = 16;
                pBuf->mnFormat = BMP_FORMAT_16BIT_TC_LSB_MASK;
                ColorMaskElement aRedMask(0xf800);
                aRedMask.CalcMaskShift();
                ColorMaskElement aGreenMask(0x07e0);
                aGreenMask.CalcMaskShift();
                ColorMaskElement aBlueMask(0x001f);
                aBlueMask.CalcMaskShift();
                pBuf->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
                break;
            }
            case Format::SixteenBitMsbTcMask:
            {
                nBitCount = 16;
                pBuf->mnFormat = BMP_FORMAT_16BIT_TC_MSB_MASK;
                ColorMaskElement aRedMask(0xf800);
                aRedMask.CalcMaskShift();
                ColorMaskElement aGreenMask(0x07e0);
                aGreenMask.CalcMaskShift();
                ColorMaskElement aBlueMask(0x001f);
                aBlueMask.CalcMaskShift();
                pBuf->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
                break;
            }
            case Format::TwentyFourBitTcMask:
                nBitCount = 24;
                pBuf->mnFormat = BMP_FORMAT_24BIT_TC_BGR;
                break;
            case Format::ThirtyTwoBitTcMaskBGRX:
            {
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                ColorMaskElement aRedMask(0x0000ff00);
                ColorMaskElement aGreenMask(0x00ff0000);
                ColorMaskElement aBlueMask(0xff000000);
#else
                ColorMaskElement aRedMask(0x00ff0000);
                ColorMaskElement aGreenMask(0x0000ff00);
                ColorMaskElement aBlueMask(0x000000ff);
#endif
                aBlueMask.CalcMaskShift();
                aRedMask.CalcMaskShift();
                aGreenMask.CalcMaskShift();
                pBuf->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
                break;
            }
            case Format::ThirtyTwoBitTcMaskBGRA:
            {
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                ColorMaskElement aRedMask(0x0000ff00);
                ColorMaskElement aGreenMask(0x00ff0000);
                ColorMaskElement aBlueMask(0xff000000);
                sal_uInt32 nAlphaChannel(0x000000ff);
#else
                ColorMaskElement aRedMask(0x00ff0000);
                ColorMaskElement aGreenMask(0x0000ff00);
                ColorMaskElement aBlueMask(0x000000ff);
                sal_uInt32 nAlphaChannel(0xff000000);
#endif
                aBlueMask.CalcMaskShift();
                aRedMask.CalcMaskShift();
                aGreenMask.CalcMaskShift();
                pBuf->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask, nAlphaChannel);
                break;
            }
            case Format::ThirtyTwoBitTcMaskARGB:
            {
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                ColorMaskElement aRedMask(0x00ff0000);
                ColorMaskElement aGreenMask(0x0000ff00);
                ColorMaskElement aBlueMask(0x000000ff);
                sal_uInt32 nAlphaChannel(0xff000000);
#else
                ColorMaskElement aRedMask(0x0000ff00);
                ColorMaskElement aGreenMask(0x00ff0000);
                ColorMaskElement aBlueMask(0xff000000);
                sal_uInt32 nAlphaChannel(0x000000ff);
#endif
                aBlueMask.CalcMaskShift();
                aRedMask.CalcMaskShift();
                aGreenMask.CalcMaskShift();
                pBuf->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask, nAlphaChannel);
                break;
            }
            case Format::ThirtyTwoBitTcMaskABGR:
            {
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                ColorMaskElement aRedMask(0x000000ff);
                ColorMaskElement aGreenMask(0x0000ff00);
                ColorMaskElement aBlueMask(0x00ff0000);
                sal_uInt32 nAlphaChannel(0xff000000);
#else
                ColorMaskElement aRedMask(0xff000000);
                ColorMaskElement aGreenMask(0x00ff0000);
                ColorMaskElement aBlueMask(0x0000ff00);
                sal_uInt32 nAlphaChannel(0x000000ff);
#endif
                aBlueMask.CalcMaskShift();
                aRedMask.CalcMaskShift();
                aGreenMask.CalcMaskShift();
                pBuf->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask, nAlphaChannel);
                break;
            }
            case Format::ThirtyTwoBitTcMaskRGBA:
            {
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                ColorMaskElement aRedMask(0xff000000);
                ColorMaskElement aGreenMask(0x00ff0000);
                ColorMaskElement aBlueMask(0x0000ff00);
                sal_uInt32 nAlphaChannel(0x000000ff);
#else
                ColorMaskElement aRedMask(0x000000ff);
                ColorMaskElement aGreenMask(0x0000ff00);
                ColorMaskElement aBlueMask(0x00ff0000);
                sal_uInt32 nAlphaChannel(0xff000000);
#endif
                aBlueMask.CalcMaskShift();
                aRedMask.CalcMaskShift();
                aGreenMask.CalcMaskShift();
                pBuf->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask, nAlphaChannel);
                break;
            }
            default:
                // this is an error case !!!!!
                nBitCount = 1;
                pBuf->mnFormat = BMP_FORMAT_1BIT_MSB_PAL;
                break;
        }
        if( m_aBitmap->isTopDown() )
            pBuf->mnFormat |= BMP_FORMAT_TOP_DOWN;

        B2IVector aSize = m_aBitmap->getSize();
        pBuf->mnWidth           = aSize.getX();
        pBuf->mnHeight          = aSize.getY();
        pBuf->mnScanlineSize    = m_aBitmap->getScanlineStride();
        pBuf->mnBitCount        = nBitCount;
        pBuf->mpBits            = m_aBitmap->getBuffer().get();
        if( nBitCount <= 8 )
        {
            if( m_aBitmap->getScanlineFormat() == Format::EightBitGrey ||
                m_aBitmap->getScanlineFormat() == Format::FourBitLsbGrey ||
                m_aBitmap->getScanlineFormat() == Format::FourBitMsbGrey ||
                m_aBitmap->getScanlineFormat() == Format::OneBitLsbGrey ||
                m_aBitmap->getScanlineFormat() == Format::OneBitMsbGrey
                )
                pBuf->maPalette = Bitmap::GetGreyPalette( 1U << nBitCount );
            else
            {
                basebmp::PaletteMemorySharedVector aPalette = m_aBitmap->getPalette();
                if( aPalette.get() )
                {
                    unsigned int nColors = aPalette->size();
                    if( nColors > 0 )
                    {
                        pBuf->maPalette.SetEntryCount( nColors );
                        for( unsigned int i = 0; i < nColors; i++ )
                        {
                            const basebmp::Color& rCol = (*aPalette)[i];
                            pBuf->maPalette[i] = BitmapColor( rCol.getRed(), rCol.getGreen(), rCol.getBlue() );
                        }
                    }
                }
            }
        }
    }

    return pBuf;
}

void SvpSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
    if( nMode == BITMAP_WRITE_ACCESS && pBuffer->maPalette.GetEntryCount() )
    {
        // palette might have changed, clone device (but recycle
        // memory)
        sal_uInt16 nBitCount = 0;
        switch( m_aBitmap->getScanlineFormat() )
        {
            case Format::OneBitMsbGrey:
                // FALLTHROUGH intended
            case Format::OneBitMsbPal:
                // FALLTHROUGH intended
            case Format::OneBitLsbGrey:
                // FALLTHROUGH intended
            case Format::OneBitLsbPal:
                nBitCount = 1;
                break;

            case Format::FourBitMsbGrey:
                // FALLTHROUGH intended
            case Format::FourBitMsbPal:
                // FALLTHROUGH intended
            case Format::FourBitLsbGrey:
                // FALLTHROUGH intended
            case Format::FourBitLsbPal:
                nBitCount = 4;
                break;

            case Format::EightBitPal:
                // FALLTHROUGH intended
            case Format::EightBitGrey:
                nBitCount = 8;
                break;

            default:
                break;
        }

        if( nBitCount )
        {
            sal_uInt32 nEntries = 1U << nBitCount;

            std::shared_ptr< std::vector<basebmp::Color> > pPal(
                new std::vector<basebmp::Color>( nEntries,
                                                 basebmp::Color(COL_WHITE)));
            const sal_uInt32 nColors = std::min(
                (sal_uInt32)pBuffer->maPalette.GetEntryCount(),
                nEntries);
            for( sal_uInt32 i = 0; i < nColors; i++ )
            {
                const BitmapColor& rCol = pBuffer->maPalette[i];
                (*pPal)[i] = basebmp::Color( rCol.GetRed(), rCol.GetGreen(), rCol.GetBlue() );
            }

            m_aBitmap = basebmp::createBitmapDevice( m_aBitmap->getSize(),
                                                     m_aBitmap->isTopDown(),
                                                     m_aBitmap->getScanlineFormat(),
                                                     m_aBitmap->getScanlineStride(),
                                                     m_aBitmap->getBuffer(),
                                                     pPal );
        }
    }

    delete pBuffer;
}

bool SvpSalBitmap::GetSystemData( BitmapSystemData& )
{
    return false;
}

bool SvpSalBitmap::Scale( const double& /*rScaleX*/, const double& /*rScaleY*/, BmpScaleFlag /*nScaleFlag*/ )
{
    return false;
}

bool SvpSalBitmap::Replace( const ::Color& /*rSearchColor*/, const ::Color& /*rReplaceColor*/, sal_uLong /*nTol*/ )
{
    return false;
}

sal_uInt32 SvpSalBitmap::getBitCountFromScanlineFormat( basebmp::Format nFormat )
{
    sal_uInt32 nBitCount = 1;
    switch( nFormat )
    {
        case Format::OneBitMsbGrey:
        case Format::OneBitLsbGrey:
        case Format::OneBitMsbPal:
        case Format::OneBitLsbPal:
            nBitCount = 1;
            break;
        case Format::FourBitMsbGrey:
        case Format::FourBitLsbGrey:
        case Format::FourBitMsbPal:
        case Format::FourBitLsbPal:
            nBitCount = 4;
            break;
        case Format::EightBitPal:
        case Format::EightBitGrey:
            nBitCount = 8;
            break;
        case Format::SixteenBitLsbTcMask:
        case Format::SixteenBitMsbTcMask:
            nBitCount = 16;
            break;
        case Format::TwentyFourBitTcMask:
            nBitCount = 24;
            break;
        case Format::ThirtyTwoBitTcMaskBGRX:
        case Format::ThirtyTwoBitTcMaskBGRA:
        case Format::ThirtyTwoBitTcMaskARGB:
        case Format::ThirtyTwoBitTcMaskABGR:
        case Format::ThirtyTwoBitTcMaskRGBA:
            nBitCount = 32;
            break;
        default:
        OSL_FAIL( "unsupported basebmp format" );
        break;
    }
    return nBitCount;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
