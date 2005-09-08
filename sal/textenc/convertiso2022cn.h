/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convertiso2022cn.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:32:31 $
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

#ifndef INCLUDED_RTL_TEXTENC_CONVERTISO2022CN_H
#define INCLUDED_RTL_TEXTENC_CONVERTISO2022CN_H

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
    ImplDBCSToUniLeadTab const * m_pGb2312ToUnicodeData;
    sal_uInt16 const * m_pCns116431992ToUnicodeData;
    sal_Int32 const * m_pCns116431992ToUnicodeRowOffsets;
    sal_Int32 const * m_pCns116431992ToUnicodePlaneOffsets;
    ImplUniToDBCSHighTab const * m_pUnicodeToGb2312Data;
    sal_uInt8 const * m_pUnicodeToCns116431992Data;
    sal_Int32 const * m_pUnicodeToCns116431992PageOffsets;
    sal_Int32 const * m_pUnicodeToCns116431992PlaneOffsets;
} ImplIso2022CnConverterData;

void * ImplCreateIso2022CnToUnicodeContext(void) SAL_THROW_EXTERN_C();

void ImplResetIso2022CnToUnicodeContext(void * pContext) SAL_THROW_EXTERN_C();

sal_Size ImplConvertIso2022CnToUnicode(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
    SAL_THROW_EXTERN_C();

void * ImplCreateUnicodeToIso2022CnContext(void) SAL_THROW_EXTERN_C();

void ImplResetUnicodeToIso2022CnContext(void * pContext) SAL_THROW_EXTERN_C();

sal_Size ImplConvertUnicodeToIso2022Cn(ImplTextConverterData const * pData,
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

#endif /* INCLUDED_RTL_TEXTENC_CONVERTISO2022CN_H */
