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

#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>

BitmapColor BitmapReadAccess::GetPixelFor_1BIT_MSB_PAL(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 );
}

void BitmapReadAccess::SetPixelFor_1BIT_MSB_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 3 ];

    ( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( 7 - ( nX & 7 ) ) ) :
                                      ( rByte &= ~( 1 << ( 7 - ( nX & 7 ) ) ) );
}

BitmapColor BitmapReadAccess::GetPixelFor_1BIT_LSB_PAL(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX >> 3 ] & ( 1 << ( nX & 7 ) ) ? 1 : 0 );
}

void BitmapReadAccess::SetPixelFor_1BIT_LSB_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 3 ];

    ( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( nX & 7 ) ) :
                                      ( rByte &= ~( 1 << ( nX & 7 ) ) );
}

BitmapColor BitmapReadAccess::GetPixelFor_4BIT_MSN_PAL(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 0 : 4 ) ) & 0x0f );
}

void BitmapReadAccess::SetPixelFor_4BIT_MSN_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) ) :
                 ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) );
}

BitmapColor BitmapReadAccess::GetPixelFor_4BIT_LSN_PAL(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 4 : 0 ) ) & 0x0f );
}

void BitmapReadAccess::SetPixelFor_4BIT_LSN_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) ) :
                 ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) );
}

BitmapColor BitmapReadAccess::GetPixelFor_8BIT_PAL(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX ] );
}

void BitmapReadAccess::SetPixelFor_8BIT_PAL(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    if (rBitmapColor.IsIndex())
        pScanline[ nX ] = rBitmapColor.GetIndex();
    else
        // Let's hope that the RGB color values equal, so it doesn't matter what do we pick
        pScanline[ nX ] = rBitmapColor.GetBlueOrIndex();
}

BitmapColor BitmapReadAccess::GetPixelFor_8BIT_TC_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor8Bit( aColor, pScanline + nX );
    return aColor;
}

void BitmapReadAccess::SetPixelFor_8BIT_TC_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor8Bit( rBitmapColor, pScanline + nX );
}


BitmapColor BitmapReadAccess::GetPixelFor_16BIT_TC_MSB_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor16BitMSB( aColor, pScanline + ( nX << 1UL ) );
    return aColor;
}

void BitmapReadAccess::SetPixelFor_16BIT_TC_MSB_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor16BitMSB( rBitmapColor, pScanline + ( nX << 1UL ) );
}

BitmapColor BitmapReadAccess::GetPixelFor_16BIT_TC_LSB_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor16BitLSB( aColor, pScanline + ( nX << 1UL ) );
    return aColor;
}

void BitmapReadAccess::SetPixelFor_16BIT_TC_LSB_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor16BitLSB( rBitmapColor, pScanline + ( nX << 1UL ) );
}

BitmapColor BitmapReadAccess::GetPixelFor_24BIT_TC_BGR(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + nX * 3 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelFor_24BIT_TC_BGR(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

BitmapColor BitmapReadAccess::GetPixelFor_24BIT_TC_RGB(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + nX * 3 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelFor_24BIT_TC_RGB(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

BitmapColor BitmapReadAccess::GetPixelFor_24BIT_TC_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor24Bit( aColor, pScanline + nX * 3L );
    return aColor;
}

void BitmapReadAccess::SetPixelFor_24BIT_TC_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor24Bit( rBitmapColor, pScanline + nX * 3L );
}

BitmapColor BitmapReadAccess::GetPixelFor_32BIT_TC_ABGR(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelFor_32BIT_TC_ABGR(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = 0xFF;
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

BitmapColor BitmapReadAccess::GetPixelFor_32BIT_TC_ARGB(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelFor_32BIT_TC_ARGB(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = 0xFF;
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

BitmapColor BitmapReadAccess::GetPixelFor_32BIT_TC_BGRA(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelFor_32BIT_TC_BGRA(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline = 0xFF;
}

BitmapColor BitmapReadAccess::GetPixelFor_32BIT_TC_RGBA(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelFor_32BIT_TC_RGBA(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline = 0xFF;
}

BitmapColor BitmapReadAccess::GetPixelFor_32BIT_TC_MASK(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor32Bit( aColor, pScanline + ( nX << 2UL ) );
    return aColor;
}

void BitmapReadAccess::SetPixelFor_32BIT_TC_MASK(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor32Bit( rBitmapColor, pScanline + ( nX << 2UL ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
