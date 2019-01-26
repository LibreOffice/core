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

// This is an implementation of the Win64 (x64) ABI.

#include <typelib/typedescription.hxx>

namespace x86_64
{

const sal_uInt32 MAX_REGS = 4;

/** Does function that returns this type use a hidden parameter, or registers?

 The value can be returned either in a hidden 1st parameter (which is a
 pointer to a structure allocated by the caller), or in registers (rax, rdx
 for the integers, xmm0, xmm1 for the floating point numbers).
*/
bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef );

} // namespace x86_64

#endif // _BRIDGES_CPP_UNO_X86_64_ABI_HXX_
