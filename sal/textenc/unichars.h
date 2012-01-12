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



#ifndef INCLUDED_RTL_TEXTENC_UNICHARS_H
#define INCLUDED_RTL_TEXTENC_UNICHARS_H

#include "sal/types.h"

#if defined __cplusplus
extern "C" {
#endif /* __cpluscplus */

#define RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER 0xFFFD

int ImplIsNoncharacter(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

int ImplIsControlOrFormat(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

int ImplIsHighSurrogate(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

int ImplIsLowSurrogate(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

int ImplIsPrivateUse(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

int ImplIsZeroWidth(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_uInt32 ImplGetHighSurrogate(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_uInt32 ImplGetLowSurrogate(sal_uInt32 nUtf32) SAL_THROW_EXTERN_C();

sal_uInt32 ImplCombineSurrogates(sal_uInt32 nHigh, sal_uInt32 nLow)
    SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cpluscplus */

#endif /* INCLUDED_RTL_TEXTENC_UNICHARS_H */
