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
#include <vcl/bmpacc.hxx>

IMPL_FORMAT_GETPIXEL_NOMASK( _1BIT_MSB_PAL )
{
    return( pScanline[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 );
}

IMPL_FORMAT_SETPIXEL_NOMASK( _1BIT_MSB_PAL )
{
    sal_uInt8& rByte = pScanline[ nX >> 3 ];

    ( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( 7 - ( nX & 7 ) ) ) :
                                      ( rByte &= ~( 1 << ( 7 - ( nX & 7 ) ) ) );
}

IMPL_FORMAT_GETPIXEL_NOMASK( _1BIT_LSB_PAL )
{
    return( pScanline[ nX >> 3 ] & ( 1 << ( nX & 7 ) ) ? 1 : 0 );
}

IMPL_FORMAT_SETPIXEL_NOMASK( _1BIT_LSB_PAL )
{
    sal_uInt8& rByte = pScanline[ nX >> 3 ];

    ( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( nX & 7 ) ) :
                                      ( rByte &= ~( 1 << ( nX & 7 ) ) );
}

IMPL_FORMAT_GETPIXEL_NOMASK( _4BIT_MSN_PAL )
{
    return( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 0 : 4 ) ) & 0x0f );
}

IMPL_FORMAT_SETPIXEL_NOMASK( _4BIT_MSN_PAL )
{
    sal_uInt8& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) ) :
                 ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) );
}

IMPL_FORMAT_GETPIXEL_NOMASK( _4BIT_LSN_PAL )
{
    return( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 4 : 0 ) ) & 0x0f );
}

IMPL_FORMAT_SETPIXEL_NOMASK( _4BIT_LSN_PAL )
{
    sal_uInt8& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) ) :
                 ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) );
}

IMPL_FORMAT_GETPIXEL_NOMASK( _8BIT_PAL )
{
    return pScanline[ nX ];
}

IMPL_FORMAT_SETPIXEL_NOMASK( _8BIT_PAL )
{
    pScanline[ nX ] = rBitmapColor.GetIndex();
}

IMPL_FORMAT_GETPIXEL( _8BIT_TC_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor8Bit( aColor, pScanline + nX );
    return aColor;
}

IMPL_FORMAT_SETPIXEL( _8BIT_TC_MASK )
{
    rMask.SetColorFor8Bit( rBitmapColor, pScanline + nX );
}

IMPL_FORMAT_GETPIXEL( _16BIT_TC_MSB_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor16BitMSB( aColor, pScanline + ( nX << 1UL ) );
    return aColor;
}

IMPL_FORMAT_SETPIXEL( _16BIT_TC_MSB_MASK )
{
    rMask.SetColorFor16BitMSB( rBitmapColor, pScanline + ( nX << 1UL ) );
}

IMPL_FORMAT_GETPIXEL( _16BIT_TC_LSB_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor16BitLSB( aColor, pScanline + ( nX << 1UL ) );
    return aColor;
}

IMPL_FORMAT_SETPIXEL( _16BIT_TC_LSB_MASK )
{
    rMask.SetColorFor16BitLSB( rBitmapColor, pScanline + ( nX << 1UL ) );
}

IMPL_FORMAT_GETPIXEL_NOMASK( _24BIT_TC_BGR )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + nX * 3 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

IMPL_FORMAT_SETPIXEL_NOMASK( _24BIT_TC_BGR )
{
    *( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

IMPL_FORMAT_GETPIXEL_NOMASK( _24BIT_TC_RGB )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + nX * 3 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

IMPL_FORMAT_SETPIXEL_NOMASK( _24BIT_TC_RGB )
{
    *( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

IMPL_FORMAT_GETPIXEL( _24BIT_TC_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor24Bit( aColor, pScanline + nX * 3L );
    return aColor;
}

IMPL_FORMAT_SETPIXEL( _24BIT_TC_MASK )
{
    rMask.SetColorFor24Bit( rBitmapColor, pScanline + nX * 3L );
}

IMPL_FORMAT_GETPIXEL_NOMASK( _32BIT_TC_ABGR )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

IMPL_FORMAT_SETPIXEL_NOMASK( _32BIT_TC_ABGR )
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = 0;
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

IMPL_FORMAT_GETPIXEL_NOMASK( _32BIT_TC_ARGB )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

IMPL_FORMAT_SETPIXEL_NOMASK( _32BIT_TC_ARGB )
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = 0;
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

IMPL_FORMAT_GETPIXEL_NOMASK( _32BIT_TC_BGRA )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

IMPL_FORMAT_SETPIXEL_NOMASK( _32BIT_TC_BGRA )
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline = 0;
}

IMPL_FORMAT_GETPIXEL_NOMASK( _32BIT_TC_RGBA )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

IMPL_FORMAT_SETPIXEL_NOMASK( _32BIT_TC_RGBA )
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline = 0;
}

IMPL_FORMAT_GETPIXEL( _32BIT_TC_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor32Bit( aColor, pScanline + ( nX << 2UL ) );
    return aColor;
}

IMPL_FORMAT_SETPIXEL( _32BIT_TC_MASK )
{
    rMask.SetColorFor32Bit( rBitmapColor, pScanline + ( nX << 2UL ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
