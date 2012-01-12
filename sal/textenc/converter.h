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



#ifndef INCLUDED_RTL_TEXTENC_CONVERTER_H
#define INCLUDED_RTL_TEXTENC_CONVERTER_H

#include "sal/types.h"

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
