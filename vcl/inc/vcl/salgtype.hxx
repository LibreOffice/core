/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salgtype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:06:46 $
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

#ifndef _SV_SALGTYPE_HXX
#define _SV_SALGTYPE_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

// ------------
// - SalColor -
// ------------

typedef UINT32 SalColor;
#define MAKE_SALCOLOR( r, g, b )    ((SalColor)(((UINT32)((UINT8)(b))))|(((UINT32)((UINT8)(g)))<<8)|(((UINT32)((UINT8)(r)))<<16))
#define SALCOLOR_RED( n )           ((UINT8)((n)>>16))
#define SALCOLOR_GREEN( n )         ((UINT8)(((UINT16)(n)) >> 8))
#define SALCOLOR_BLUE( n )          ((UINT8)(n))

// ------------
// - SalPoint -
// ------------

// must equal to class Point
struct SalPoint
{
    long        mnX;
    long        mnY;
};

typedef const SalPoint*   PCONSTSALPOINT;

// --------------
// - SalTwoRect -
// --------------

struct SalTwoRect
{
    long        mnSrcX;
    long        mnSrcY;
    long        mnSrcWidth;
    long        mnSrcHeight;
    long        mnDestX;
    long        mnDestY;
    long        mnDestWidth;
    long        mnDestHeight;
};

// ---------------
// - SalROPColor -
// ---------------

typedef USHORT SalROPColor;
#define SAL_ROP_0                   ((SalROPColor)0)
#define SAL_ROP_1                   ((SalROPColor)1)
#define SAL_ROP_INVERT              ((SalROPColor)2)

// -------------
// - SalInvert -
// -------------

typedef USHORT SalInvert;
#define SAL_INVERT_HIGHLIGHT        ((SalInvert)0x0001)
#define SAL_INVERT_50               ((SalInvert)0x0002)
#define SAL_INVERT_TRACKFRAME       ((SalInvert)0x0004)

#endif // _SV_SALGTYPE_HXX
