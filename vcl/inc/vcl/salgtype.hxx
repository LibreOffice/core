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

#ifndef _SV_SALGTYPE_HXX
#define _SV_SALGTYPE_HXX

#include <vcl/sv.h>

// ------------
// - SalColor -
// ------------

typedef sal_uInt32 SalColor;
#define MAKE_SALCOLOR( r, g, b )    ((SalColor)(((sal_uInt32)((sal_uInt8)(b))))|(((sal_uInt32)((sal_uInt8)(g)))<<8)|(((sal_uInt32)((sal_uInt8)(r)))<<16))
#define SALCOLOR_RED( n )           ((sal_uInt8)((n)>>16))
#define SALCOLOR_GREEN( n )         ((sal_uInt8)(((sal_uInt16)(n)) >> 8))
#define SALCOLOR_BLUE( n )          ((sal_uInt8)(n))
#define SALCOLOR_NONE           (~(SalColor)0)
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

typedef sal_uInt16 SalROPColor;
#define SAL_ROP_0                   ((SalROPColor)0)
#define SAL_ROP_1                   ((SalROPColor)1)
#define SAL_ROP_INVERT              ((SalROPColor)2)

// -------------
// - SalInvert -
// -------------

typedef sal_uInt16 SalInvert;
#define SAL_INVERT_HIGHLIGHT        ((SalInvert)0x0001)
#define SAL_INVERT_50               ((SalInvert)0x0002)
#define SAL_INVERT_TRACKFRAME       ((SalInvert)0x0004)

#endif // _SV_SALGTYPE_HXX
