/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_RTL_TEXTENC_CONVERTBIG5HKSCS_H
#define INCLUDED_RTL_TEXTENC_CONVERTBIG5HKSCS_H

#include "tenchelp.h"
#include "sal/types.h"

#if defined __cplusplus
extern "C" {
#endif /* __cpluscplus */

typedef struct
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
} ImplBig5HkscsConverterData;

void * ImplCreateBig5HkscsToUnicodeContext(void) SAL_THROW_EXTERN_C();

void ImplResetBig5HkscsToUnicodeContext(void * pContext) SAL_THROW_EXTERN_C();

sal_Size ImplConvertBig5HkscsToUnicode(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
    SAL_THROW_EXTERN_C();

sal_Size ImplConvertUnicodeToBig5Hkscs(ImplTextConverterData const * pData,
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

#endif /* INCLUDED_RTL_TEXTENC_CONVERTBIG5HKSCS_H */
