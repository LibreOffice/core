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

#include "svpbmp.hxx"

#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/range/b2irange.hxx>
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
    sal_uInt32 nFormat = SVP_DEFAULT_BITMAP_FORMAT;
    switch( nBitCount )
    {
        case 1: nFormat = Format::ONE_BIT_MSB_PAL; break;
        case 4: nFormat = Format::FOUR_BIT_MSB_PAL; break;
        case 8: nFormat = Format::EIGHT_BIT_PAL; break;
#ifdef OSL_BIGENDIAN
        case 16: nFormat = Format::SIXTEEN_BIT_MSB_TC_MASK; break;
#else
        case 16: nFormat = Format::SIXTEEN_BIT_LSB_TC_MASK; break;
#endif
        case 24: nFormat = Format::TWENTYFOUR_BIT_TC_MASK; break;
        case 32: nFormat = Format::THIRTYTWO_BIT_TC_MASK; break;
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
        B2IRange aRect( 0, 0, aSize.getX(), aSize.getY() );
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
            case Format::ONE_BIT_MSB_GREY:
            case Format::ONE_BIT_MSB_PAL:
                nBitCount = 1;
                pBuf->mnFormat = BMP_FORMAT_1BIT_MSB_PAL;
                break;
            case Format::ONE_BIT_LSB_GREY:
            case Format::ONE_BIT_LSB_PAL:
                nBitCount = 1;
                pBuf->mnFormat = BMP_FORMAT_1BIT_LSB_PAL;
                break;
            case Format::FOUR_BIT_MSB_GREY:
            case Format::FOUR_BIT_MSB_PAL:
                nBitCount = 4;
                pBuf->mnFormat = BMP_FORMAT_4BIT_MSN_PAL;
                break;
            case Format::FOUR_BIT_LSB_GREY:
            case Format::FOUR_BIT_LSB_PAL:
                nBitCount = 4;
                pBuf->mnFormat = BMP_FORMAT_4BIT_LSN_PAL;
                break;
            case Format::EIGHT_BIT_PAL:
                nBitCount = 8;
                pBuf->mnFormat = BMP_FORMAT_8BIT_PAL;
                break;
            case Format::EIGHT_BIT_GREY:
                nBitCount = 8;
                pBuf->mnFormat = BMP_FORMAT_8BIT_PAL;
                break;
            case Format::SIXTEEN_BIT_LSB_TC_MASK:
                nBitCount = 16;
                pBuf->mnFormat = BMP_FORMAT_16BIT_TC_LSB_MASK;
                pBuf->maColorMask = ColorMask( 0xf800, 0x07e0, 0x001f );
                break;
            case Format::SIXTEEN_BIT_MSB_TC_MASK:
                nBitCount = 16;
                pBuf->mnFormat = BMP_FORMAT_16BIT_TC_MSB_MASK;
                pBuf->maColorMask = ColorMask( 0xf800, 0x07e0, 0x001f );
                break;
            case Format::TWENTYFOUR_BIT_TC_MASK:
                nBitCount = 24;
                pBuf->mnFormat = BMP_FORMAT_24BIT_TC_BGR;
                break;
            case Format::THIRTYTWO_BIT_TC_MASK:
                nBitCount = 32;
                pBuf->mnFormat = BMP_FORMAT_32BIT_TC_MASK;
#ifdef OSL_BIGENDIAN
                pBuf->maColorMask = ColorMask( 0x0000ff, 0x00ff00, 0xff0000 );
#else
                pBuf->maColorMask = ColorMask( 0xff0000, 0x00ff00, 0x0000ff );
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
            if( m_aBitmap->getScanlineFormat() == Format::EIGHT_BIT_GREY ||
                m_aBitmap->getScanlineFormat() == Format::FOUR_BIT_LSB_GREY ||
                m_aBitmap->getScanlineFormat() == Format::FOUR_BIT_MSB_GREY ||
                m_aBitmap->getScanlineFormat() == Format::ONE_BIT_LSB_GREY ||
                m_aBitmap->getScanlineFormat() == Format::ONE_BIT_MSB_GREY
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
            case Format::ONE_BIT_MSB_GREY:
                // FALLTHROUGH intended
            case Format::ONE_BIT_MSB_PAL:
                // FALLTHROUGH intended
            case Format::ONE_BIT_LSB_GREY:
                // FALLTHROUGH intended
            case Format::ONE_BIT_LSB_PAL:
                nBitCount = 1;
                break;

            case Format::FOUR_BIT_MSB_GREY:
                // FALLTHROUGH intended
            case Format::FOUR_BIT_MSB_PAL:
                // FALLTHROUGH intended
            case Format::FOUR_BIT_LSB_GREY:
                // FALLTHROUGH intended
            case Format::FOUR_BIT_LSB_PAL:
                nBitCount = 4;
                break;

            case Format::EIGHT_BIT_PAL:
                // FALLTHROUGH intended
            case Format::EIGHT_BIT_GREY:
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


