/*************************************************************************
 *
 *  $RCSfile: unichars.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2001-10-17 14:35:30 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
