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

BitmapColor BitmapReadAccess::GetPixelForN1BitMsbPal(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 );
}

void BitmapReadAccess::SetPixelForN1BitMsbPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 3 ];

    ( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( 7 - ( nX & 7 ) ) ) :
                                      ( rByte &= ~( 1 << ( 7 - ( nX & 7 ) ) ) );
}

BitmapColor BitmapReadAccess::GetPixelForN1BitLsbPal(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX >> 3 ] & ( 1 << ( nX & 7 ) ) ? 1 : 0 );
}

void BitmapReadAccess::SetPixelForN1BitLsbPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 3 ];

    ( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( nX & 7 ) ) :
                                      ( rByte &= ~( 1 << ( nX & 7 ) ) );
}

BitmapColor BitmapReadAccess::GetPixelForN4BitMsnPal(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 0 : 4 ) ) & 0x0f );
}

void BitmapReadAccess::SetPixelForN4BitMsnPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) ) :
                 ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) );
}

BitmapColor BitmapReadAccess::GetPixelForN4BitLsnPal(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 4 : 0 ) ) & 0x0f );
}

void BitmapReadAccess::SetPixelForN4BitLsnPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) ) :
                 ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) );
}

BitmapColor BitmapReadAccess::GetPixelForN8BitPal(ConstScanline pScanline, long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX ] );
}

void BitmapReadAccess::SetPixelForN8BitPal(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    if (rBitmapColor.IsIndex())
        pScanline[ nX ] = rBitmapColor.GetIndex();
    else
        // Let's hope that the RGB color values equal, so it doesn't matter what do we pick
        pScanline[ nX ] = rBitmapColor.GetBlueOrIndex();
}

BitmapColor BitmapReadAccess::GetPixelForN8BitTcMask(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor8Bit( aColor, pScanline + nX );
    return aColor;
}

void BitmapReadAccess::SetPixelForN8BitTcMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor8Bit( rBitmapColor, pScanline + nX );
}


BitmapColor BitmapReadAccess::GetPixelForN16BitTcMsbMask(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor16BitMSB( aColor, pScanline + ( nX << 1UL ) );
    return aColor;
}

void BitmapReadAccess::SetPixelForN16BitTcMsbMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor16BitMSB( rBitmapColor, pScanline + ( nX << 1UL ) );
}

BitmapColor BitmapReadAccess::GetPixelForN16BitTcLsbMask(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor16BitLSB( aColor, pScanline + ( nX << 1UL ) );
    return aColor;
}

void BitmapReadAccess::SetPixelForN16BitTcLsbMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor16BitLSB( rBitmapColor, pScanline + ( nX << 1UL ) );
}

BitmapColor BitmapReadAccess::GetPixelForN24BitTcBgr(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + nX * 3 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN24BitTcBgr(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

BitmapColor BitmapReadAccess::GetPixelForN24BitTcRgb(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + nX * 3 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN24BitTcRgb(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

BitmapColor BitmapReadAccess::GetPixelForN24BitTcMask(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor24Bit( aColor, pScanline + nX * 3L );
    return aColor;
}

void BitmapReadAccess::SetPixelForN24BitTcMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor24Bit( rBitmapColor, pScanline + nX * 3L );
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcAbgr(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcAbgr(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = 0xFF;
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcArgb(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcArgb(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = 0xFF;
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcBgra(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcBgra(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline = 0xFF;
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcRgba(ConstScanline pScanline, long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcRgba(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline = 0xFF;
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcMask(ConstScanline pScanline, long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor32Bit( aColor, pScanline + ( nX << 2UL ) );
    return aColor;
}

void BitmapReadAccess::SetPixelForN32BitTcMask(Scanline pScanline, long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor32Bit( rBitmapColor, pScanline + ( nX << 2UL ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
