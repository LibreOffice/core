/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scanlineformats.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:47:09 $
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

#ifndef INCLUDED_BASEBMP_SCANLINEFORMATS_HXX
#define INCLUDED_BASEBMP_SCANLINEFORMATS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

/* Definition of Scanline formats */

namespace basebmp { namespace Format
{
    static const sal_Int32 NONE                   = 0;
    static const sal_Int32 ONE_BIT_MSB_GREY       = (sal_Int32)0x01;
    static const sal_Int32 ONE_BIT_LSB_GREY       = (sal_Int32)0x02;
    static const sal_Int32 ONE_BIT_MSB_PAL        = (sal_Int32)0x03;
    static const sal_Int32 ONE_BIT_LSB_PAL        = (sal_Int32)0x04;
    static const sal_Int32 FOUR_BIT_MSB_GREY      = (sal_Int32)0x05;
    static const sal_Int32 FOUR_BIT_LSB_GREY      = (sal_Int32)0x06;
    static const sal_Int32 FOUR_BIT_MSB_PAL       = (sal_Int32)0x07;
    static const sal_Int32 FOUR_BIT_LSB_PAL       = (sal_Int32)0x08;
    static const sal_Int32 EIGHT_BIT_PAL          = (sal_Int32)0x09;
    static const sal_Int32 EIGHT_BIT_GREY         = (sal_Int32)0x0A;
    static const sal_Int32 SIXTEEN_BIT_LSB_TC_MASK= (sal_Int32)0x0B;
    static const sal_Int32 SIXTEEN_BIT_MSB_TC_MASK= (sal_Int32)0x0C;
    static const sal_Int32 TWENTYFOUR_BIT_TC_MASK = (sal_Int32)0x0D;
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK  = (sal_Int32)0x0E;
    static const sal_Int32 MAX                    = (sal_Int32)0x0E;
} }

#endif /* INCLUDED_BASEBMP_SCANLINEFORMATS_HXX */
