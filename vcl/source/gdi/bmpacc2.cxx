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

#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapTools.hxx>

BitmapColor BitmapReadAccess::GetPixelForN1BitMsbPal(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 );
}

void BitmapReadAccess::SetPixelForN1BitMsbPal(const Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 3 ];

    if ( rBitmapColor.GetIndex() & 1 )
        rByte |= 1 << ( 7 - ( nX & 7 ) );
    else
        rByte &= ~( 1 << ( 7 - ( nX & 7 ) ) );
}

BitmapColor BitmapReadAccess::GetPixelForN1BitLsbPal(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX >> 3 ] & ( 1 << ( nX & 7 ) ) ? 1 : 0 );
}

void BitmapReadAccess::SetPixelForN1BitLsbPal(const Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 3 ];

    if ( rBitmapColor.GetIndex() & 1 )
        rByte |= 1 << ( nX & 7 );
    else
        rByte &= ~( 1 << ( nX & 7 ) );
}

BitmapColor BitmapReadAccess::GetPixelForN4BitMsnPal(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    return BitmapColor( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 0 : 4 ) ) & 0x0f );
}

void BitmapReadAccess::SetPixelForN4BitMsnPal(const Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 1 ];

    if ( nX & 1 )
    {
        rByte &= 0xf0;
        rByte |= ( rBitmapColor.GetIndex() & 0x0f );
    }
    else
    {
        rByte &= 0x0f;
        rByte |= ( rBitmapColor.GetIndex() << 4 );
    }
}

BitmapColor BitmapReadAccess::GetPixelForN4BitLsnPal(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    return BitmapColor( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 4 : 0 ) ) & 0x0f );
}

void BitmapReadAccess::SetPixelForN4BitLsnPal(const Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    sal_uInt8& rByte = pScanline[ nX >> 1 ];

    if ( nX & 1 )
    {
        rByte &= 0x0f;
        rByte |= ( rBitmapColor.GetIndex() << 4 );
    }
    else
    {
        rByte &= 0xf0;
        rByte |= ( rBitmapColor.GetIndex() & 0x0f );
    }
}

BitmapColor BitmapReadAccess::GetPixelForN8BitPal(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    return BitmapColor( pScanline[ nX ] );
}

void BitmapReadAccess::SetPixelForN8BitPal(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline[ nX ] = rBitmapColor.GetIndex();
}

BitmapColor BitmapReadAccess::GetPixelForN24BitTcBgr(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + nX * 3;
    aBitmapColor.SetBlue( *pScanline++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN24BitTcBgr(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 3;
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

BitmapColor BitmapReadAccess::GetPixelForN24BitTcRgb(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + nX * 3;
    aBitmapColor.SetRed( *pScanline++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN24BitTcRgb(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 3;
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcAbgr(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 a = *pScanline++;
    sal_uInt8 b = *pScanline++;
    sal_uInt8 g = *pScanline++;
    sal_uInt8 r = *pScanline;

    return BitmapColor(
            vcl::bitmap::unpremultiply(r, a),
            vcl::bitmap::unpremultiply(g, a),
            vcl::bitmap::unpremultiply(b, a),
            0xFF - a);
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcXbgr(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + ( nX << 2 ) + 1;
    aBitmapColor.SetBlue( *pScanline++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcAbgr(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 alpha = 0xFF - rBitmapColor.GetAlpha();
    *pScanline++ = alpha;
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetBlue(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetGreen(), alpha);
    *pScanline   = vcl::bitmap::premultiply(rBitmapColor.GetRed(), alpha);
}

void BitmapReadAccess::SetPixelForN32BitTcXbgr(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + ( nX << 2 );
    *pScanline++ = 0xFF;
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcArgb(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 a = *pScanline++;
    sal_uInt8 r = *pScanline++;
    sal_uInt8 g = *pScanline++;
    sal_uInt8 b = *pScanline;

    return BitmapColor(
            vcl::bitmap::unpremultiply(r, a),
            vcl::bitmap::unpremultiply(g, a),
            vcl::bitmap::unpremultiply(b, a),
            0xFF - a);
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcXrgb(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + ( nX << 2 ) + 1;
    aBitmapColor.SetRed( *pScanline++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcArgb(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 alpha = 0xFF - rBitmapColor.GetAlpha();
    *pScanline++ = alpha;
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetRed(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetGreen(), alpha);
    *pScanline   = vcl::bitmap::premultiply(rBitmapColor.GetBlue(), alpha);
}

void BitmapReadAccess::SetPixelForN32BitTcXrgb(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + ( nX << 2 );
    *pScanline++ = 0xFF;
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcBgra(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 b = *pScanline++;
    sal_uInt8 g = *pScanline++;
    sal_uInt8 r = *pScanline++;
    sal_uInt8 a = *pScanline;

    return BitmapColor(
            vcl::bitmap::unpremultiply(r, a),
            vcl::bitmap::unpremultiply(g, a),
            vcl::bitmap::unpremultiply(b, a),
            0xFF - a);
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcBgrx(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + ( nX << 2 );
    aBitmapColor.SetBlue( *pScanline++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcBgra(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 alpha = 0xFF - rBitmapColor.GetAlpha();
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetBlue(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetGreen(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetRed(), alpha);
    *pScanline = alpha;
}

void BitmapReadAccess::SetPixelForN32BitTcBgrx(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + ( nX << 2 );
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline = 0xFF;
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcRgba(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 r = *pScanline++;
    sal_uInt8 g = *pScanline++;
    sal_uInt8 b = *pScanline++;
    sal_uInt8 a = *pScanline;

    return BitmapColor(
            vcl::bitmap::unpremultiply(r, a),
            vcl::bitmap::unpremultiply(g, a),
            vcl::bitmap::unpremultiply(b, a),
            0xFF - a);
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcRgbx(ConstScanline pScanline, tools::Long nX, const ColorMask&)
{
    BitmapColor aBitmapColor;

    pScanline = pScanline + ( nX << 2 );
    aBitmapColor.SetRed( *pScanline++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

void BitmapReadAccess::SetPixelForN32BitTcRgba(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + nX * 4;

    sal_uInt8 alpha = 0xFF - rBitmapColor.GetAlpha();
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetRed(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetGreen(), alpha);
    *pScanline++ = vcl::bitmap::premultiply(rBitmapColor.GetBlue(), alpha);
    *pScanline = alpha;
}

void BitmapReadAccess::SetPixelForN32BitTcRgbx(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask&)
{
    pScanline = pScanline + ( nX << 2 );
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline = 0xFF;
}

BitmapColor BitmapReadAccess::GetPixelForN32BitTcMask(ConstScanline pScanline, tools::Long nX, const ColorMask& rMask)
{
    BitmapColor aColor;
    rMask.GetColorFor32Bit( aColor, pScanline + ( nX << 2 ) );
    return aColor;
}

void BitmapReadAccess::SetPixelForN32BitTcMask(Scanline pScanline, tools::Long nX, const BitmapColor& rBitmapColor, const ColorMask& rMask)
{
    rMask.SetColorFor32Bit( rBitmapColor, pScanline + ( nX << 2 ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
