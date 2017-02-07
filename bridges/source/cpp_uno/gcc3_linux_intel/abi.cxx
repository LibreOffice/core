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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include <typeinfo>
#include <cstddef>

#include <uno/any2.h>
#include <cppu/macros.hxx>

#include "share.hxx"

// At least Clang 3.8 can't compile our CPPU_CURRENT_NAMESPACE::__cxa_get_globals()
// decleration with <exception> or anything else that indirectly includes <cxxabi.h>
// included, as it sees it as a an overload of __cxxabi::__cxa_get_globals() with a
// different return type. Thus, it has to be placed in a file that doesn't include
// those, and wrapped in a function:
namespace CPPU_CURRENT_NAMESPACE
{
  extern "C" __cxa_eh_globals *__cxa_get_globals () throw();

  __cxa_eh_globals *__INTERNAL__cxa_get_globals() throw()
  {
    return __cxa_get_globals();
  }
}
