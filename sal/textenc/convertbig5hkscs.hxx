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

#ifndef INCLUDED_SAL_TEXTENC_CONVERTBIG5HKSCS_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTBIG5HKSCS_HXX

#include "sal/types.h"

#include "tenchelp.hxx"

struct ImplBig5HkscsConverterData
{
    sal_uInt16 const * m_pBig5Hkscs2001ToUnicodeData;
    sal_Int32 const * m_pBig5Hkscs2001ToUnicodeRowOffsets;
    ImplDBCSToUniLeadTab const * m_pBig5ToUnicodeData;
    sal_uInt16 const * m_pUnicodeToBig5Hkscs2001Data;
    sal_Int32 const * m_pUnicodeToBig5Hkscs2001PageOffsets;
    sal_Int32 const * m_pUnicodeToBig5Hkscs2001PlaneOffsets;
    ImplUniToDBCSHighTab const * m_pUnicodeToBig5Data;
    ImplDBCSEUDCData const * m_pEudcData;
    int m_nEudcCount;
};

void * ImplCreateBig5HkscsToUnicodeContext();

void ImplResetBig5HkscsToUnicodeContext(void * pContext);

void ImplDestroyBig5HkscsToUnicodeContext(void * pContext);

sal_Size ImplConvertBig5HkscsToUnicode(void const * pData,
                                       void * pContext,
                                       char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes);

sal_Size ImplConvertUnicodeToBig5Hkscs(void const * pData,
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
