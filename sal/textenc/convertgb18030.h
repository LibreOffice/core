/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convertgb18030.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:31:30 $
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

#ifndef INCLUDED_RTL_TEXTENC_CONVERTGB18030_H
#define INCLUDED_RTL_TEXTENC_CONVERTGB18030_H

#ifndef INCLUDED_RTL_TEXTENC_TENCHELP_H
#include "tenchelp.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#if defined __cplusplus
extern "C" {
#endif /* __cpluscplus */

typedef struct
{
    sal_Int32 m_nNonRangeDataIndex;
    sal_uInt32 m_nFirstLinear;
    sal_uInt32 m_nPastLinear;
    sal_Unicode m_nFirstUnicode;
} ImplGb180302000ToUnicodeRange;

typedef struct
{
    sal_Int32 m_nNonRangeDataIndex;
    sal_Unicode m_nFirstUnicode;
    sal_Unicode m_nLastUnicode;
    sal_uInt32 m_nFirstLinear;
} ImplUnicodeToGb180302000Range;

typedef struct
{
    sal_Unicode const * m_pGb18030ToUnicodeData;
    ImplGb180302000ToUnicodeRange const * m_pGb18030ToUnicodeRanges;
    sal_uInt32 const * m_pUnicodeToGb18030Data;
    ImplUnicodeToGb180302000Range const * m_pUnicodeToGb18030Ranges;
} ImplGb18030ConverterData;

void * ImplCreateGb18030ToUnicodeContext(void) SAL_THROW_EXTERN_C();

void ImplResetGb18030ToUnicodeContext(void * pContext) SAL_THROW_EXTERN_C();

sal_Size ImplConvertGb18030ToUnicode(ImplTextConverterData const * pData,
                                     void * pContext,
                                     sal_Char const * pSrcBuf,
                                     sal_Size nSrcBytes,
                                     sal_Unicode * pDestBuf,
                                     sal_Size nDestChars,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo,
                                     sal_Size * pSrcCvtBytes)
    SAL_THROW_EXTERN_C();

sal_Size ImplConvertUnicodeToGb18030(ImplTextConverterData const * pData,
                                     void * pContext,
                                     sal_Unicode const * pSrcBuf,
                                     sal_Size nSrcChars,
                                     sal_Char * pDestBuf,
                                     sal_Size nDestBytes,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo,
                                     sal_Size * pSrcCvtChars)
    SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cpluscplus */

#endif /* INCLUDED_RTL_TEXTENC_CONVERTGB18030_H */
