/*************************************************************************
 *
 *  $RCSfile: scanlineformats.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-11 11:38:55 $
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
