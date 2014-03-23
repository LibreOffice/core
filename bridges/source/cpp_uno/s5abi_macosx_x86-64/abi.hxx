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



#ifndef _BRIDGES_CPP_UNO_X86_64_ABI_HXX_
#define _BRIDGES_CPP_UNO_X86_64_ABI_HXX_

// This is an implementation of the x86-64 ABI as described in 'System V
// Application Binary Interface, AMD64 Architecture Processor Supplement'
// (http://www.x86-64.org/documentation/abi-0.95.pdf)

#include <typelib/typedescription.hxx>

namespace x86_64
{

/* 6 general purpose registers are used for parameter passing */
const sal_uInt32 MAX_GPR_REGS = 6;

/* 8 SSE registers are used for parameter passing */
const sal_uInt32 MAX_SSE_REGS = 8;

/* Count number of required registers.

 Examine the argument and return set number of register required in each
 class.

 Return false iff parameter should be passed in memory.
*/
bool examine_argument( typelib_TypeDescriptionReference *pTypeRef, bool bInReturn, int &nUsedGPR, int &nUsedSSE );

/** Does function that returns this type use a hidden parameter, or registers?

 The value can be returned either in a hidden 1st parameter (which is a
 pointer to a structure allocated by the caller), or in registers (rax, rdx
 for the integers, xmm0, xmm1 for the floating point numbers).
*/
bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef );

void fill_struct( typelib_TypeDescriptionReference *pTypeRef, const sal_uInt64* pGPR, const double* pSSE, void *pStruct );

} // namespace x86_64

#endif // _BRIDGES_CPP_UNO_X86_64_ABI_HXX_
