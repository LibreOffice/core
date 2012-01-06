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

#ifndef INCLUDED_SAL_TEXTENC_CONVERTGB18030_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTGB18030_HXX

#include "sal/config.h"

#include "sal/types.h"

struct ImplGb180302000ToUnicodeRange
{
    sal_Int32 m_nNonRangeDataIndex;
    sal_uInt32 m_nFirstLinear;
    sal_uInt32 m_nPastLinear;
    sal_Unicode m_nFirstUnicode;
};

struct ImplUnicodeToGb180302000Range
{
    sal_Int32 m_nNonRangeDataIndex;
    sal_Unicode m_nFirstUnicode;
    sal_Unicode m_nLastUnicode;
    sal_uInt32 m_nFirstLinear;
};

struct ImplGb18030ConverterData
{
    sal_Unicode const * m_pGb18030ToUnicodeData;
    ImplGb180302000ToUnicodeRange const * m_pGb18030ToUnicodeRanges;
    sal_uInt32 const * m_pUnicodeToGb18030Data;
    ImplUnicodeToGb180302000Range const * m_pUnicodeToGb18030Ranges;
};

void * ImplCreateGb18030ToUnicodeContext();

void ImplResetGb18030ToUnicodeContext(void * pContext);

void ImplDestroyGb18030ToUnicodeContext(void * pContext);

sal_Size ImplConvertGb18030ToUnicode(void const * pData,
                                     void * pContext,
                                     char const * pSrcBuf,
                                     sal_Size nSrcBytes,
                                     sal_Unicode * pDestBuf,
                                     sal_Size nDestChars,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo,
                                     sal_Size * pSrcCvtBytes);

sal_Size ImplConvertUnicodeToGb18030(void const * pData,
                                     void * pContext,
                                     sal_Unicode const * pSrcBuf,
                                     sal_Size nSrcChars,
                                     char * pDestBuf,
                                     sal_Size nDestBytes,
                                     sal_uInt32 nFlags,
                                     sal_uInt32 * pInfo,
                                     sal_Size * pSrcCvtChars);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
