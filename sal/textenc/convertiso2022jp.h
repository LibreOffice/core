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



#ifndef INCLUDED_RTL_TEXTENC_CONVERTISO2022JP_H
#define INCLUDED_RTL_TEXTENC_CONVERTISO2022JP_H

#include "tenchelp.h"
#include "sal/types.h"

#if defined __cplusplus
extern "C" {
#endif /* __cpluscplus */

typedef struct
{
    ImplDBCSToUniLeadTab const * m_pJisX0208ToUnicodeData;
    ImplUniToDBCSHighTab const * m_pUnicodeToJisX0208Data;
} ImplIso2022JpConverterData;

void * ImplCreateIso2022JpToUnicodeContext(void) SAL_THROW_EXTERN_C();

void ImplResetIso2022JpToUnicodeContext(void * pContext) SAL_THROW_EXTERN_C();

sal_Size ImplConvertIso2022JpToUnicode(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
    SAL_THROW_EXTERN_C();

void * ImplCreateUnicodeToIso2022JpContext(void) SAL_THROW_EXTERN_C();

void ImplResetUnicodeToIso2022JpContext(void * pContext) SAL_THROW_EXTERN_C();

sal_Size ImplConvertUnicodeToIso2022Jp(ImplTextConverterData const * pData,
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

#endif /* INCLUDED_RTL_TEXTENC_CONVERTISO2022JP_H */
