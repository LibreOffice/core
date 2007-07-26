/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: surrogates.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-26 09:06:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_SAL_RTL_SOURCE_SURROGATES_H
#define INCLUDED_SAL_RTL_SOURCE_SURROGATES_H

#include "sal/config.h"

#define SAL_RTL_FIRST_HIGH_SURROGATE 0xD800
#define SAL_RTL_LAST_HIGH_SURROGATE 0xDBFF
#define SAL_RTL_FIRST_LOW_SURROGATE 0xDC00
#define SAL_RTL_LAST_LOW_SURROGATE 0xDFFF

#define SAL_RTL_IS_HIGH_SURROGATE(utf16) \
    ((utf16) >= SAL_RTL_FIRST_HIGH_SURROGATE && \
     (utf16) <= SAL_RTL_LAST_HIGH_SURROGATE)

#define SAL_RTL_IS_LOW_SURROGATE(utf16) \
    ((utf16) >= SAL_RTL_FIRST_LOW_SURROGATE && \
     (utf16) <= SAL_RTL_LAST_LOW_SURROGATE)

#define SAL_RTL_COMBINE_SURROGATES(high, low) \
    ((((high) - SAL_RTL_FIRST_HIGH_SURROGATE) << 10) + \
     ((low) - SAL_RTL_FIRST_LOW_SURROGATE) + 0x10000)

#endif
