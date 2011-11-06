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



#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_FLUSHCODE_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_FLUSHCODE_HXX

#include "sal/config.h"

extern "C" void sync_instruction_memory(caddr_t addr, int len); // from libc

namespace bridges { namespace cpp_uno { namespace cc5_solaris_sparc64 {

/**
 * Flush a region of memory into which code has been written dynamically.
 */
inline void flushCode(void const * begin, void const * end) {
    sync_instruction_memory(
        static_cast< caddr_t >(const_cast< void * >(begin)),
        static_cast< char const * >(end) - static_cast< char const * >(begin));
}

} } }

#endif
