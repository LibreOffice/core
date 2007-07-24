/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bmpfast.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 09:57:51 $
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

#ifndef _SV_BMPFAST_HXX
#define _SV_BMPFAST_HXX

class BitmapWriteAccess;
class BitmapReadAccess;
struct BitmapBuffer;
class BitmapColor;
class Size;
class Point;
struct SalTwoRect;

// the bmpfast functions have signatures with good compatibility to
// their canonic counterparts, which employ the GetPixel/SetPixel methods

bool ImplFastBitmapConversion( BitmapBuffer& rDst, const BitmapBuffer& rSrc,
        const SalTwoRect& rTwoRect );

bool ImplFastBitmapBlending( BitmapWriteAccess& rDst,
    const BitmapReadAccess& rSrc, const BitmapReadAccess& rMask,
    const SalTwoRect& rTwoRect );

bool ImplFastEraseBitmap( BitmapBuffer&, const BitmapColor& );

#endif // _SV_BMPFAST_HXX
