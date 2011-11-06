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



#include "precompiled_tools.hxx"
#include "sal/config.h"

#if defined UNX

#include <cstddef>

#include "osl/diagnose.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "sal/types.h"
#include "tools/appendunixshellword.hxx"

namespace tools {

void appendUnixShellWord(
    rtl::OStringBuffer * accumulator, rtl::OString const & text)
{
    OSL_ASSERT(accumulator != NULL);
    if (text.getLength() == 0) {
        accumulator->append(RTL_CONSTASCII_STRINGPARAM("''"));
    } else {
        bool quoted = false;
        for (sal_Int32 i = 0; i < text.getLength(); ++i) {
            char c = text[i];
            if (c == '\'') {
                if (quoted) {
                    accumulator->append('\'');
                    quoted = false;
                }
                accumulator->append(RTL_CONSTASCII_STRINGPARAM("\\'"));
            } else {
                if (!quoted) {
                    accumulator->append('\'');
                    quoted = true;
                }
                accumulator->append(c);
            }
        }
        if (quoted) {
            accumulator->append('\'');
        }
    }
}

}

#endif
