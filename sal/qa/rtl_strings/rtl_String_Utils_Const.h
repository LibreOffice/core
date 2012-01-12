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



//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _RTL_STRING_UTILS_CONST_H_
#define _RTL_STRING_UTILS_CONST_H_

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
    #include <sal/types.h>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Int32 kErrCompareAStringToUString     = -2;
static const sal_Int32 kErrCompareNAStringToUString    = -3;
static const sal_Int32 kErrCompareAStringToRTLUString  = -4;
static const sal_Int32 kErrCompareNAStringToRTLUString = -5;
static const sal_Int32 kErrAStringToByteStringCompare  = -6;
static const sal_Int32 kErrAStringToByteStringNCompare = -7;
static const sal_Int32 kErrCompareAStringToString      = -8;
static const sal_Int32 kErrCompareNAStringToString     = -9;

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif /* _RTL_STRING_UTILS_CONST_H_ */



