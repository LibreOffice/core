/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: converter.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:36:29 $
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

#ifndef INCLUDED_RTL_TEXTENC_CONVERTER_H
#define INCLUDED_RTL_TEXTENC_CONVERTER_H

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#if defined __cplusplus
extern "C" {
#endif /* __cpluscplus */

typedef enum
{
    IMPL_BAD_INPUT_STOP,
    IMPL_BAD_INPUT_CONTINUE,
    IMPL_BAD_INPUT_NO_OUTPUT
} ImplBadInputConversionAction;

ImplBadInputConversionAction
ImplHandleBadInputTextToUnicodeConversion(
    sal_Bool bUndefined, sal_Bool bMultiByte, sal_Char cByte, sal_uInt32 nFlags,
    sal_Unicode ** pDestBufPtr, sal_Unicode * pDestBufEnd, sal_uInt32 * pInfo)
    SAL_THROW_EXTERN_C();

ImplBadInputConversionAction
ImplHandleBadInputUnicodeToTextConversion(sal_Bool bUndefined,
                                          sal_uInt32 nUtf32,
                                          sal_uInt32 nFlags,
                                          sal_Char ** pDestBufPtr,
                                          sal_Char * pDestBufEnd,
                                          sal_uInt32 * pInfo,
                                          sal_Char const * pPrefix,
                                          sal_Size nPrefixLen,
                                          sal_Bool * pPrefixWritten)
    SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cpluscplus */

#endif /* INCLUDED_RTL_TEXTENC_CONVERTER_H */
