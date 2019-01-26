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



#pragma warning(push, 1)
#include <windows.h>
#pragma warning(pop)

#include "rtl/ustring.hxx"


class type_info;
typedef struct _uno_Any uno_Any;
typedef struct _uno_Mapping uno_Mapping;

namespace CPPU_CURRENT_NAMESPACE
{

const DWORD MSVC_ExceptionCode = 0xe06d7363;
const long MSVC_magic_number = 0x19930520L;

//==============================================================================
type_info * mscx_getRTTI( ::rtl::OUString const & rUNOname );

//==============================================================================
int mscx_filterCppException(
    EXCEPTION_POINTERS * pPointers, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno );

//==============================================================================
void mscx_raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

}

