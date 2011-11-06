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



#include <cstddef>
#include <rtl/string.hxx>
#include <typeinfo>

typedef struct _uno_Any uno_Any;
typedef struct _uno_Mapping uno_Mapping;

// private C50 structures and functions
namespace __Crun
{
    struct static_type_info
    {
        std::ptrdiff_t m_pClassName;
        int         m_nSkip1; // must be 0
        void*       m_pMagic; // points to some magic data
        int         m_nMagic[ 4 ];
        int         m_nSkip2[2]; // must be 0
    };
    void* ex_alloc(unsigned);
    void ex_throw( void*, const static_type_info*, void(*)(void*));
    void* ex_get();
    void ex_rethrow_q();
}

namespace __Cimpl
{
    const char* ex_name();
}

extern "C" void _ex_register( void*, int );

namespace CPPU_CURRENT_NAMESPACE
{

//##################################################################################################
//#### exceptions ##################################################################################
//##################################################################################################

void cc50_solaris_intel_raiseException(
    uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

void cc50_solaris_intel_fillUnoException(
    void*, const char*,
    uno_Any*, uno_Mapping * pCpp2Uno );
}

