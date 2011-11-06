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



#ifndef INCLUDED_TOOLS_APPENDUNIXSHELLWORD_HXX
#define INCLUDED_TOOLS_APPENDUNIXSHELLWORD_HXX

#include "sal/config.h"

#if defined UNX

#include "tools/toolsdllapi.h"

namespace rtl {
    class OString;
    class OStringBuffer;
}

namespace tools {

// append arbitrary bytes as a properly quoted Unix-style shell word
//
// @param accumulator
// the string buffer to which the word is appended (without any surrounding
// whitespace); must not be null
//
// @param text
// the text to add
TOOLS_DLLPUBLIC void appendUnixShellWord(
    rtl::OStringBuffer * accumulator, rtl::OString const & text);

}

#endif

#endif
