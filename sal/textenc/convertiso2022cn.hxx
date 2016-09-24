/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SAL_TEXTENC_CONVERTISO2022CN_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTISO2022CN_HXX

#include "sal/config.h"
#include "sal/types.h"
#include "tenchelp.hxx"

struct ImplIso2022CnConverterData
{
    ImplDBCSToUniLeadTab const * m_pGb2312ToUnicodeData;
    sal_uInt16 const * m_pCns116431992ToUnicodeData;
    sal_Int32 const * m_pCns116431992ToUnicodeRowOffsets;
    sal_Int32 const * m_pCns116431992ToUnicodePlaneOffsets;
    ImplUniToDBCSHighTab const * m_pUnicodeToGb2312Data;
    sal_uInt8 const * m_pUnicodeToCns116431992Data;
    sal_Int32 const * m_pUnicodeToCns116431992PageOffsets;
    sal_Int32 const * m_pUnicodeToCns116431992PlaneOffsets;
};

void * ImplCreateIso2022CnToUnicodeContext();

void ImplResetIso2022CnToUnicodeContext(void * pContext);

void ImplDestroyIso2022CnToUnicodeContext(void * pContext);

sal_Size ImplConvertIso2022CnToUnicode(void const * pData,
                                       void * pContext,
                                       char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes);

void * ImplCreateUnicodeToIso2022CnContext();

void ImplResetUnicodeToIso2022CnContext(void * pContext);

void ImplDestroyUnicodeToIso2022CnContext(void * pContext);

sal_Size ImplConvertUnicodeToIso2022Cn(void const * pData,
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
