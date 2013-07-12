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

#include "headless/svpbmp.hxx"

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
    basebmp::Format nFormat = SVP_DEFAULT_BITMAP_FORMAT;
    SAL_INFO( "vcl.headless", "SvpSalBitmap::Create(" << rSize.Width() << "," << rSize.Height() << "," << nBitCount << ")" );
    switch( nBitCount )
    {
        case 1: nFormat = FORMAT_ONE_BIT_MSB_PAL; break;
        case 4: nFormat = FORMAT_FOUR_BIT_MSB_PAL; break;
        case 8: nFormat = FORMAT_EIGHT_BIT_PAL; break;
#ifdef OSL_BIGENDIAN
        case 16: nFormat = FORMAT_SIXTEEN_BIT_MSB_TC_MASK; break;
#else
        case 16: nFormat = FORMAT_SIXTEEN_BIT_LSB_TC_MASK; break;
#endif
        case 24: nFormat = FORMAT_TWENTYFOUR_BIT_TC_MASK; break;
#if defined(ANDROID) || defined(IOS)
        case 32: nFormat = FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA; break;
#else
        case 32: nFormat = FORMAT_THIRTYTWO_BIT_TC_MASK_BGRA; break;
#endif
    }
    B2IVector aSize( rSize.Width(), rSize.Height() );
    if( aSize.getX() == 0 )
        aSize.setX( 1 );
    if( aSize.getY() == 0 )
        aSize.setY( 1 );
    if( nBitCount > 8 )
        m_aBitmap = createBitmapDevice( aSize, false, nFormat );
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
        m_aBitmap = createBitmapDevice( aSize, false, nFormat,
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
        m_aBitmap->drawBitmap( rSrcBmp, aRect, aRect, DrawMode_PAINT );
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

bool SvpSalBitmap::Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > /*xBitmapCanvas*/, Size& /*rSize*/, bool /*bMask*/ )
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

BitmapBuffer* SvpSalBitmap::AcquireBuffer( bool )
{
    BitmapBuffer* pBuf = NULL;
    if( m_aBitmap.get() )
    {
        pBuf = new BitmapBuffer();
        sal_uInt16 nBitCount = 1;
        switch( m_aBitmap->getScanlineFormat() )
        {
            case FORMAT_ONE_BIT_MSB_GREY:
            case FORMAT_ONE_BIT_MSB_PAL:
                nBitCount = 1;
                pBuf->mnFormat = BMP_FORMAT_1BIT_MSB_PAL;
                break;
            case FORMAT_ONE_BIT_LSB_GREY:
            case FORMAT_ONE_BIT_LSB_PAL:
                nBitCount = 1;
                pBuf->mnFormat = BMP_FORMAT_1BIT_LSB_PAL;
                break;
            case FORMAT_FOUR_BIT_MSB_GREY:
            case FORMAT_FOUR_BIT_MSB_PAL:
                nBitCount = 4;
                pBuf->mnFormat = BMP_FORMAT_4BIT_MSN_PAL;
                break;
            case FORMAT_FOUR_BIT_LSB_GREY:
            case FORMAT_FOUR_BIT_LSB_PAL:
                nBitCount = 4;
                pBuf->mnFormat = BMP_FORMAT_4BIT_LSN_PAL;
                break;
            case FORMAT_EIGHT_BIT_PAL:
                nBitCount = 8;
                pBuf->mnFormat = BMP_FORMAT_8BIT_PAL;
                break;
            case FORMAT_EIGHT_BIT_GREY:
                nBitCount = 8;
                pBuf->mnFormat = BMP_FORMAT_8BIT_PAL;
                break;
            case FORMAT_SIXTEEN_BIT_LSB_TC_MASK:
                nBitCount = 16;
                pBuf->mnFormat = BMP_FORMAT_16BIT_TC_LSB_MASK;
                pBuf->maColorMask = ColorMask( 0xf800, 0x07e0, 0x001f );
                break;
            case FORMAT_SIXTEEN_BIT_MSB_TC_MASK:
                nBitCount = 16;
                pBuf->mnFormat = BMP_FORMAT_16BIT_TC_MSB_MASK;
                pBuf->maColorMask = ColorMask( 0xf800, 0x07e0, 0x001f );
                break;
            case FORMAT_TWENTYFOUR_BIT_TC_MASK:
                nBitCount = 24;
                pBuf->mnFormat = BMP_FORMAT_24BIT_TC_BGR;
                break;
            case FORMAT_THIRTYTWO_BIT_TC_MASK_BGRA:
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                pBuf->maColorMask = ColorMask( 0x0000ff00, 0x00ff0000, 0xff000000 );
#else
                pBuf->maColorMask = ColorMask( 0x00ff0000, 0x0000ff00, 0x000000ff );
#endif
                break;
            case FORMAT_THIRTYTWO_BIT_TC_MASK_ARGB:
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                pBuf->maColorMask = ColorMask( 0x00ff0000, 0x0000ff00, 0x000000ff );
#else
                pBuf->maColorMask = ColorMask( 0x0000ff00, 0x00ff0000, 0xff000000 );
#endif
                break;
            case FORMAT_THIRTYTWO_BIT_TC_MASK_ABGR:
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                pBuf->maColorMask = ColorMask( 0x000000ff, 0x0000ff00, 0x00ff0000 );
#else
                pBuf->maColorMask = ColorMask( 0xff000000, 0x00ff0000, 0x0000ff00 );
#endif
                break;
            case FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA:
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                pBuf->maColorMask = ColorMask( 0xff000000, 0x00ff0000, 0x0000ff00 );
#else
                pBuf->maColorMask = ColorMask( 0x000000ff, 0x0000ff00, 0x00ff0000 );
#endif
                break;

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
        pBuf->mpBits            = (sal_uInt8*)m_aBitmap->getBuffer().get();
        if( nBitCount <= 8 )
        {
            if( m_aBitmap->getScanlineFormat() == FORMAT_EIGHT_BIT_GREY ||
                m_aBitmap->getScanlineFormat() == FORMAT_FOUR_BIT_LSB_GREY ||
                m_aBitmap->getScanlineFormat() == FORMAT_FOUR_BIT_MSB_GREY ||
                m_aBitmap->getScanlineFormat() == FORMAT_ONE_BIT_LSB_GREY ||
                m_aBitmap->getScanlineFormat() == FORMAT_ONE_BIT_MSB_GREY
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

void SvpSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly )
{
    if( !bReadOnly && pBuffer->maPalette.GetEntryCount() )
    {
        // palette might have changed, clone device (but recycle
        // memory)
        sal_uInt16 nBitCount = 0;
        switch( m_aBitmap->getScanlineFormat() )
        {
            case FORMAT_ONE_BIT_MSB_GREY:
                // FALLTHROUGH intended
            case FORMAT_ONE_BIT_MSB_PAL:
                // FALLTHROUGH intended
            case FORMAT_ONE_BIT_LSB_GREY:
                // FALLTHROUGH intended
            case FORMAT_ONE_BIT_LSB_PAL:
                nBitCount = 1;
                break;

            case FORMAT_FOUR_BIT_MSB_GREY:
                // FALLTHROUGH intended
            case FORMAT_FOUR_BIT_MSB_PAL:
                // FALLTHROUGH intended
            case FORMAT_FOUR_BIT_LSB_GREY:
                // FALLTHROUGH intended
            case FORMAT_FOUR_BIT_LSB_PAL:
                nBitCount = 4;
                break;

            case FORMAT_EIGHT_BIT_PAL:
                // FALLTHROUGH intended
            case FORMAT_EIGHT_BIT_GREY:
                nBitCount = 8;
                break;

            default:
                break;
        }

        if( nBitCount )
        {
            sal_uInt32 nEntries = 1U << nBitCount;

            boost::shared_ptr< std::vector<basebmp::Color> > pPal(
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

sal_uInt32 SvpSalBitmap::getBitCountFromScanlineFormat( basebmp::Format nFormat )
{
    sal_uInt32 nBitCount = 1;
    switch( nFormat )
    {
        case FORMAT_ONE_BIT_MSB_GREY:
        case FORMAT_ONE_BIT_LSB_GREY:
        case FORMAT_ONE_BIT_MSB_PAL:
        case FORMAT_ONE_BIT_LSB_PAL:
            nBitCount = 1;
            break;
        case FORMAT_FOUR_BIT_MSB_GREY:
        case FORMAT_FOUR_BIT_LSB_GREY:
        case FORMAT_FOUR_BIT_MSB_PAL:
        case FORMAT_FOUR_BIT_LSB_PAL:
            nBitCount = 4;
            break;
        case FORMAT_EIGHT_BIT_PAL:
        case FORMAT_EIGHT_BIT_GREY:
            nBitCount = 8;
            break;
        case FORMAT_SIXTEEN_BIT_LSB_TC_MASK:
        case FORMAT_SIXTEEN_BIT_MSB_TC_MASK:
            nBitCount = 16;
            break;
        case FORMAT_TWENTYFOUR_BIT_TC_MASK:
            nBitCount = 24;
            break;
        case FORMAT_THIRTYTWO_BIT_TC_MASK_BGRA:
        case FORMAT_THIRTYTWO_BIT_TC_MASK_ARGB:
        case FORMAT_THIRTYTWO_BIT_TC_MASK_ABGR:
        case FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA:
            nBitCount = 32;
            break;
        default:
        OSL_FAIL( "unsupported basebmp format" );
        break;
    }
    return nBitCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
