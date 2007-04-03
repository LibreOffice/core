/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strimp.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 14:05:38 $
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

#ifndef INCLUDED_RTL_SOURCE_STRIMP_H
#define INCLUDED_RTL_SOURCE_STRIMP_H

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#include "sal/types.h"

/* ======================================================================= */
/* Help functions for String and UString                                   */
/* ======================================================================= */

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * refCount is opaqueincludes 2 bit-fields;
 * MSB:   'interned' - is stored in the intern hash
 * MSB-1: 'static'   - is a const / static string,
 *                     do no ref counting
 */
#define SAL_STRING_INTERN_FLAG 0x80000000
#define SAL_STRING_STATIC_FLAG 0x40000000
#define SAL_STRING_REFCOUNT(a) ((a) & 0x3fffffff)

#define SAL_STRING_IS_INTERN(a) ((a)->refCount & SAL_STRING_INTERN_FLAG)
#define SAL_STRING_IS_STATIC(a) ((a)->refCount & SAL_STRING_STATIC_FLAG)

sal_Int16 rtl_ImplGetDigit( sal_Unicode ch, sal_Int16 nRadix );

sal_Bool rtl_ImplIsWhitespace( sal_Unicode c );

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDED_RTL_SOURCE_STRIMP_H */
