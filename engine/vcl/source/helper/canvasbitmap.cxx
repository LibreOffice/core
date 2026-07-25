/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>

#include <comphelper/diagnose_ex.hxx>
#include <canvasbitmap.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>

using namespace vcl::unotools;
using namespace ::com::sun::star;

namespace
{
    // TODO(Q3): move to o3tl bithacks or somesuch. A similar method is in canvas/canvastools.hxx

    // Good ole HAKMEM tradition. Calc number of 1 bits in 32bit word,
    // unrolled loop. See e.g. Hackers Delight, p. 66
    sal_Int32 bitcount( sal_uInt32 val )
    {
        val = val - ((val >> 1) & 0x55555555);
        val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
        val = (val + (val >> 4)) & 0x0F0F0F0F;
        val = val + (val >> 8);
        val = val + (val >> 16);
        return sal_Int32(val & 0x0000003F);
    }
}

void VclCanvasBitmap::setComponentInfo( sal_uInt32 redShift, sal_uInt32 greenShift, sal_uInt32 blueShift )
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
    pCounts[redPos]   = bitcount(redShift);
    pCounts[greenPos] = bitcount(greenShift);
    pCounts[bluePos]  = bitcount(blueShift);
}

BitmapScopedReadAccess& VclCanvasBitmap::getBitmapReadAccess()
{
    // BitmapReadAccess is more expensive than BitmapInfoAccess,
    // as the latter requires also pixels, which may need converted
    // from the system format (and even fetched). Most calls here
    // need only info access, create read access only on demand.
    if(!m_pBmpReadAcc)
        m_pBmpReadAcc.emplace(m_aBmp);
    return *m_pBmpReadAcc;
}

VclCanvasBitmap::VclCanvasBitmap( const Bitmap& rBitmap ) :
    m_aBmp( rBitmap ),
    m_pBmpAcc( m_aBmp ),
    m_nBitsPerInputPixel(0),
    m_nBitsPerOutputPixel(0),
    m_nRedIndex(-1),
    m_nGreenIndex(-1),
    m_nBlueIndex(-1),
    m_nAlphaIndex(-1),
    m_nIndexIndex(-1),
    m_bPalette(false)
{
    m_aLayout.ScanLines      = 0;
    m_aLayout.ScanLineBytes  = 0;
    m_aLayout.ScanLineStride = 0;
    m_aLayout.PlaneStride    = 0;
    m_aLayout.ColorSpace.clear();
    m_aLayout.Palette.clear();
    m_aLayout.IsMsbFirst     = false;

    if( !m_pBmpAcc )
        return;

    m_aLayout.ScanLines      = m_pBmpAcc->Height();
    m_aLayout.ScanLineBytes  = (m_pBmpAcc->GetBitCount()*m_pBmpAcc->Width() + 7) / 8;
    m_aLayout.ScanLineStride = m_pBmpAcc->GetScanlineSize();
    m_aLayout.PlaneStride    = 0;

    switch( m_pBmpAcc->GetScanlineFormat() )
    {
        case ScanlineFormat::N8BitPal:
            m_bPalette           = true;
            m_nBitsPerInputPixel = 8;
            m_aLayout.IsMsbFirst = false; // doesn't matter
            break;

        case ScanlineFormat::N24BitTcBgr:
            m_bPalette           = false;
            m_nBitsPerInputPixel = 24;
            m_aLayout.IsMsbFirst = false; // doesn't matter
            setComponentInfo( static_cast<sal_uInt32>(0xff0000UL),
                              static_cast<sal_uInt32>(0x00ff00UL),
                              static_cast<sal_uInt32>(0x0000ffUL) );
            break;

        case ScanlineFormat::N24BitTcRgb:
            m_bPalette           = false;
            m_nBitsPerInputPixel = 24;
            m_aLayout.IsMsbFirst = false; // doesn't matter
            setComponentInfo( static_cast<sal_uInt32>(0x0000ffUL),
                              static_cast<sal_uInt32>(0x00ff00UL),
                              static_cast<sal_uInt32>(0xff0000UL) );
            break;

        case ScanlineFormat::N32BitTcAbgr:
        case ScanlineFormat::N32BitTcXbgr:
        {
            m_bPalette           = false;
            m_nBitsPerInputPixel = 24;
            m_aLayout.IsMsbFirst = false; // doesn't matter

            m_aComponentTags = { /* 0 */ rendering::ColorComponentTag::ALPHA,
                                 /* 1 */ rendering::ColorComponentTag::RGB_BLUE,
                                 /* 2 */ rendering::ColorComponentTag::RGB_GREEN,
                                 /* 3 */ rendering::ColorComponentTag::RGB_RED };

            m_aComponentBitCounts = { /* 0 */ 8,
                                      /* 1 */ 8,
                                      /* 2 */ 8,
                                      /* 3 */ 8 };

            m_nRedIndex   = 3;
            m_nGreenIndex = 2;
            m_nBlueIndex  = 1;
            m_nAlphaIndex = 0;
        }
        break;

        case ScanlineFormat::N32BitTcArgb:
        case ScanlineFormat::N32BitTcXrgb:
        {
            m_bPalette           = false;
            m_nBitsPerInputPixel = 24;
            m_aLayout.IsMsbFirst = false; // doesn't matter

            m_aComponentTags = { /* 0 */ rendering::ColorComponentTag::ALPHA,
                                 /* 1 */ rendering::ColorComponentTag::RGB_RED,
                                 /* 2 */ rendering::ColorComponentTag::RGB_GREEN,
                                 /* 3 */ rendering::ColorComponentTag::RGB_BLUE };

            m_aComponentBitCounts = { /* 0 */ 8,
                                      /* 1 */ 8,
                                      /* 2 */ 8,
                                      /* 3 */ 8 };

            m_nRedIndex   = 1;
            m_nGreenIndex = 2;
            m_nBlueIndex  = 3;
            m_nAlphaIndex = 0;
        }
        break;

        case ScanlineFormat::N32BitTcBgra:
        case ScanlineFormat::N32BitTcBgrx:
        {
            m_bPalette           = false;
            m_nBitsPerInputPixel = 24;
            m_aLayout.IsMsbFirst = false; // doesn't matter

            m_aComponentTags = { /* 0 */ rendering::ColorComponentTag::RGB_BLUE,
                                 /* 1 */ rendering::ColorComponentTag::RGB_GREEN,
                                 /* 2 */ rendering::ColorComponentTag::RGB_RED,
                                 /* 3 */ rendering::ColorComponentTag::ALPHA };

            m_aComponentBitCounts = { /* 0 */ 8,
                                      /* 1 */ 8,
                                      /* 2 */ 8,
                                      /* 3 */ 8 };

            m_nRedIndex   = 2;
            m_nGreenIndex = 1;
            m_nBlueIndex  = 0;
            m_nAlphaIndex = 3;
        }
        break;

        case ScanlineFormat::N32BitTcRgba:
        case ScanlineFormat::N32BitTcRgbx:
        {
            m_bPalette           = false;
            m_nBitsPerInputPixel = 24;
            m_aLayout.IsMsbFirst = false; // doesn't matter

            m_aComponentTags = { /* 0 */ rendering::ColorComponentTag::RGB_RED,
                                 /* 1 */ rendering::ColorComponentTag::RGB_GREEN,
                                 /* 2 */ rendering::ColorComponentTag::RGB_BLUE,
                                 /* 3 */ rendering::ColorComponentTag::ALPHA };

            m_aComponentBitCounts = { /* 0 */ 8,
                                      /* 1 */ 8,
                                      /* 2 */ 8,
                                      /* 3 */ 8 };

            m_nRedIndex   = 0;
            m_nGreenIndex = 1;
            m_nBlueIndex  = 2;
            m_nAlphaIndex = 3;
        }
        break;

        default:
            OSL_FAIL( "unsupported bitmap format" );
            break;
    }

    if( m_bPalette )
    {
        m_aComponentTags = { rendering::ColorComponentTag::INDEX };

        m_aComponentBitCounts = { m_nBitsPerInputPixel };

        m_nIndexIndex = 0;
    }

    m_nBitsPerOutputPixel = m_nBitsPerInputPixel;
    if( !m_aBmp.HasAlpha() )
        return;

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
    m_aComponentTags.getArray()[m_aComponentTags.getLength()-1] = rendering::ColorComponentTag::ALPHA;

    m_aComponentBitCounts.realloc(m_aComponentBitCounts.getLength()+1);
    m_aComponentBitCounts.getArray()[m_aComponentBitCounts.getLength()-1] = m_aBmp.HasAlpha() ? 8 : 1;

    // always add a full byte to the pixel size, otherwise
    // pixel packing hell breaks loose.
    m_nBitsPerOutputPixel += 8;

    // adapt scanline parameters
    const Size aSize = m_aBmp.GetSizePixel();
    m_aLayout.ScanLineBytes  =
    m_aLayout.ScanLineStride = (aSize.Width()*m_nBitsPerOutputPixel + 7)/8;
}

VclCanvasBitmap::~VclCanvasBitmap()
{
}

// XBitmap
geometry::IntegerSize2D SAL_CALL VclCanvasBitmap::getSize()
{
    SolarMutexGuard aGuard;
    return integerSize2DFromSize( m_aBmp.GetSizePixel() );
}

bool SAL_CALL VclCanvasBitmap::hasAlpha()
{
    SolarMutexGuard aGuard;
    return m_aBmp.HasAlpha();
}

cpo::uno::Sequence<rendering::ARGBColor> SAL_CALL VclCanvasBitmap::convertToARGB( const cpo::uno::Sequence< double >& deviceColor )
{
    SolarMutexGuard aGuard;

    const std::size_t nLen( deviceColor.getLength() );
    const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());
    ENSURE_ARG_OR_THROW2(nLen%nComponentsPerPixel==0,
                         "number of channels no multiple of pixel element count",
                         static_cast<rendering::XBitmapPalette*>(this), 01);

    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/nComponentsPerPixel);
    rendering::ARGBColor* pOut( aRes.getArray() );

    if( m_bPalette )
    {
        OSL_ENSURE(m_nIndexIndex != -1,
                   "Invalid color channel indices");
        ENSURE_OR_THROW(m_pBmpAcc,
                        "Unable to get BitmapAccess");

        for( std::size_t i=0; i<nLen; i+=nComponentsPerPixel )
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

        for( std::size_t i=0; i<nLen; i+=nComponentsPerPixel )
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

cpo::uno::Sequence< double > SAL_CALL VclCanvasBitmap::convertFromARGB( const cpo::uno::Sequence<rendering::ARGBColor>& rgbColor )
{
    SolarMutexGuard aGuard;

    const std::size_t nLen( rgbColor.getLength() );
    const sal_Int32 nComponentsPerPixel(m_aComponentTags.getLength());

    cpo::uno::Sequence< double > aRes(nLen*nComponentsPerPixel);
    double* pColors=aRes.getArray();

    if( m_bPalette )
    {
        for( const auto& rIn : rgbColor )
        {
            pColors[m_nIndexIndex] = m_pBmpAcc->GetBestPaletteIndex(
                    BitmapColor(toByteColor(rIn.Red),
                                toByteColor(rIn.Green),
                                toByteColor(rIn.Blue)));
            if( m_nAlphaIndex != -1 )
                pColors[m_nAlphaIndex] = rIn.Alpha;

            pColors += nComponentsPerPixel;
        }
    }
    else
    {
        for( const auto& rIn : rgbColor )
        {
            pColors[m_nRedIndex]   = rIn.Red;
            pColors[m_nGreenIndex] = rIn.Green;
            pColors[m_nBlueIndex]  = rIn.Blue;
            if( m_nAlphaIndex != -1 )
                pColors[m_nAlphaIndex] = rIn.Alpha;

            pColors += nComponentsPerPixel;
        }
    }
    return aRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
