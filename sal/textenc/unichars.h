/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unichars.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:47:03 $
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

#ifndef INCLUDED_RTL_TEXTENC_UNICHARS_H
#define INCLUDED_RTL_TEXTENC_UNICHARS_H

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#if defined __cplusplus
extern "C" {
#endif /* __cpluscplus */

#define RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER 0xFFFD

sal_Bool ImplIsNoncharacter(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_Bool ImplIsControlOrFormat(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_Bool ImplIsHighSurrogate(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_Bool ImplIsLowSurrogate(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_Bool ImplIsPrivateUse(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_Bool ImplIsZeroWidth(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_uInt32 ImplGetHighSurrogate(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_uInt32 ImplGetLowSurrogate(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_uInt32 ImplCombineSurrogates(sal_uInt32 nHigh, sal_uInt32 nLow)
    SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cpluscplus */

#endif /* INCLUDED_RTL_TEXTENC_UNICHARS_H */
