/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_RTL_TEXTENC_CONVERTGB18030_H
#define INCLUDED_RTL_TEXTENC_CONVERTGB18030_H

#include "tenchelp.h"
#include "sal/types.h"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
