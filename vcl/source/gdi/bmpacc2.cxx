/*************************************************************************
 *
 *  $RCSfile: bmpacc2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_BMPACC2_CXX

#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif

// ----------------
// - BitmapAccess -
// ----------------

IMPL_FORMAT_GETPIXEL( _1BIT_MSB_PAL )
{
    return( pScanline[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 );
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _1BIT_MSB_PAL )
{
    BYTE& rByte = pScanline[ nX >> 3 ];

    ( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( 7 - ( nX & 7 ) ) ) :
                                      ( rByte &= ~( 1 << ( 7 - ( nX & 7 ) ) ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _1BIT_LSB_PAL )
{
    return( pScanline[ nX >> 3 ] & ( 1 << ( nX & 7 ) ) ? 1 : 0 );
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _1BIT_LSB_PAL )
{
    BYTE& rByte = pScanline[ nX >> 3 ];

    ( rBitmapColor.GetIndex() & 1 ) ? ( rByte |= 1 << ( nX & 7 ) ) :
                                      ( rByte &= ~( 1 << ( nX & 7 ) ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _4BIT_MSN_PAL )
{
    return( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 0 : 4 ) ) & 0x0f );
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _4BIT_MSN_PAL )
{
    BYTE& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) ) :
                 ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _4BIT_LSN_PAL )
{
    return( ( pScanline[ nX >> 1 ] >> ( nX & 1 ? 4 : 0 ) ) & 0x0f );
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _4BIT_LSN_PAL )
{
    BYTE& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0x0f, rByte |= ( rBitmapColor.GetIndex() << 4 ) ) :
                 ( rByte &= 0xf0, rByte |= ( rBitmapColor.GetIndex() & 0x0f ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _8BIT_PAL )
{
    return pScanline[ nX ];
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _8BIT_PAL )
{
    pScanline[ nX ] = rBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _8BIT_TC_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor8Bit( aColor, pScanline + nX );
    return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _8BIT_TC_MASK )
{
    rMask.SetColorFor8Bit( rBitmapColor, pScanline + nX );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _16BIT_TC_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor16Bit( aColor, pScanline + ( nX << 1UL ) );
    return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _16BIT_TC_MASK )
{
    rMask.SetColorFor16Bit( rBitmapColor, pScanline + ( nX << 1UL ) );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _24BIT_TC_BGR )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + nX * 3 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _24BIT_TC_BGR )
{
    *( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _24BIT_TC_RGB )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + nX * 3 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _24BIT_TC_RGB )
{
    *( pScanline = pScanline + nX * 3 )++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _24BIT_TC_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor24Bit( aColor, pScanline + nX * 3L );
    return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _24BIT_TC_MASK )
{
    rMask.SetColorFor24Bit( rBitmapColor, pScanline + nX * 3L );
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _32BIT_TC_ABGR )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _32BIT_TC_ABGR )
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = 0;
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetRed();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _32BIT_TC_ARGB )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) + 1 )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _32BIT_TC_ARGB )
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = 0;
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline = rBitmapColor.GetBlue();
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _32BIT_TC_BGRA )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetBlue( *( pScanline = pScanline + ( nX << 2 ) )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetRed( *pScanline );

    return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _32BIT_TC_BGRA )
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetBlue();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetRed();
    *pScanline = 0;
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _32BIT_TC_RGBA )
{
    BitmapColor aBitmapColor;

    aBitmapColor.SetRed( *( pScanline = pScanline + ( nX << 2 ) )++ );
    aBitmapColor.SetGreen( *pScanline++ );
    aBitmapColor.SetBlue( *pScanline );

    return aBitmapColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _32BIT_TC_RGBA )
{
    *( pScanline = pScanline + ( nX << 2 ) )++ = rBitmapColor.GetRed();
    *pScanline++ = rBitmapColor.GetGreen();
    *pScanline++ = rBitmapColor.GetBlue();
    *pScanline = 0;
}

// ------------------------------------------------------------------

IMPL_FORMAT_GETPIXEL( _32BIT_TC_MASK )
{
    BitmapColor aColor;
    rMask.GetColorFor32Bit( aColor, pScanline + ( nX << 2UL ) );
    return aColor;
}

// ------------------------------------------------------------------

IMPL_FORMAT_SETPIXEL( _32BIT_TC_MASK )
{
    rMask.SetColorFor32Bit( rBitmapColor, pScanline + ( nX << 2UL ) );
}
