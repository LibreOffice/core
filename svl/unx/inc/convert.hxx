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
#ifndef _CONVERT_HXX
#define _CONVERT_HXX

/*
#define _SWAPSHORT(x) ((((x) & 0xFF00)>>8) | (((x) & 0x00FF)<<8))
#define _SWAPLONG(x)  ((((x) & 0xFF000000)>>24) | (((x) & 0x00FF0000)>>8) |  \
                      (((x) & 0x0000FF00)<<8) | (((x) & 0x000000FF)<<24))
*/
class Convert
{
public:
    static void     Swap( long & nValue )
                    { nValue = SWAPLONG( nValue ); }
    static void     Swap( ULONG & nValue )
                    { nValue = SWAPLONG( nValue ); }
    static void     Swap( short & nValue )
                    { nValue = SWAPSHORT( nValue ); }
    static void     Swap( USHORT & nValue )
                    { nValue = SWAPSHORT( nValue ); }
    static void     Swap( Point & aPtr )
                    { Swap( aPtr.X() ); Swap( aPtr.Y() ); }
    static void     Swap( Size & aSize )
                    { Swap( aSize.Width() ); Swap( aSize.Height() ); }
    static void     Swap( Rectangle & rRect )
                    { Swap( rRect.Top() ); Swap( rRect.Bottom() );
                      Swap( rRect.Left() ); Swap( rRect.Right() ); }
/*
    static USHORT   AnsiFloatSize() const { return 6; }
    static float    AnsiToFloat( void * pAnsiFloat )
                    { return 0; }
    static USHORT   AnsiDoubleSize() const { return 12; }
    static double   AnsiToDouble( void * pAnsiDouble )
                    { return 0; }
*/
};

#endif // _CONVERT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
