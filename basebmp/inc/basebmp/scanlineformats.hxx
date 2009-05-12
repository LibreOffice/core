/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scanlineformats.hxx,v $
 * $Revision: 1.10 $
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

#ifndef INCLUDED_BASEBMP_SCANLINEFORMATS_HXX
#define INCLUDED_BASEBMP_SCANLINEFORMATS_HXX

#include <sal/types.h>

/* Definition of Scanline formats */

namespace basebmp { namespace Format
{
    static const sal_Int32 NONE                         = 0;
    static const sal_Int32 ONE_BIT_MSB_GREY             = (sal_Int32)0x01;
    static const sal_Int32 ONE_BIT_LSB_GREY             = (sal_Int32)0x02;
    static const sal_Int32 ONE_BIT_MSB_PAL              = (sal_Int32)0x03;
    static const sal_Int32 ONE_BIT_LSB_PAL              = (sal_Int32)0x04;
    static const sal_Int32 FOUR_BIT_MSB_GREY            = (sal_Int32)0x05;
    static const sal_Int32 FOUR_BIT_LSB_GREY            = (sal_Int32)0x06;
    static const sal_Int32 FOUR_BIT_MSB_PAL             = (sal_Int32)0x07;
    static const sal_Int32 FOUR_BIT_LSB_PAL             = (sal_Int32)0x08;
    static const sal_Int32 EIGHT_BIT_PAL                = (sal_Int32)0x09;
    static const sal_Int32 EIGHT_BIT_GREY               = (sal_Int32)0x0A;
    static const sal_Int32 SIXTEEN_BIT_LSB_TC_MASK      = (sal_Int32)0x0B;
    static const sal_Int32 SIXTEEN_BIT_MSB_TC_MASK      = (sal_Int32)0x0C;
    static const sal_Int32 TWENTYFOUR_BIT_TC_MASK       = (sal_Int32)0x0D;
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK        = (sal_Int32)0x0E;
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK_ARGB   = (sal_Int32)0x0F;
    static const sal_Int32 MAX                          = (sal_Int32)0x0F;
} }

#endif /* INCLUDED_BASEBMP_SCANLINEFORMATS_HXX */
