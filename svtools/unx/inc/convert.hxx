/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convert.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:58:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
