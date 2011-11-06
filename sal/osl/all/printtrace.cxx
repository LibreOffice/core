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



#include "precompiled_sal.hxx"

#include "sal/config.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <stdio.h> // snprintf, vsnprintf

#include "osl/diagnose.h"
#include "osl/thread.hxx"
#include "rtl/string.h"
#include "sal/types.h"

#include "printtrace.h"

void printTrace(unsigned long pid, char const * format, std::va_list arguments)
{
    char buf[1024];
    int n1 = snprintf(
        buf, sizeof buf, "Trace %lu/%" SAL_PRIuUINT32 ": \"", pid,
        osl::Thread::getCurrentIdentifier());
    OSL_ASSERT(
        n1 >= 0 &&
        (static_cast< unsigned int >(n1) <
         sizeof buf - RTL_CONSTASCII_LENGTH("\"...\n")));
    int n2 = sizeof buf - n1 - RTL_CONSTASCII_LENGTH("\"...\n");
    int n3 = vsnprintf(buf + n1, n2, format, arguments);
    if (n3 < 0) {
        std::strcpy(buf + n1, "\"???\n");
    } else if (n3 < n2) {
        std::strcpy(buf + n1 + n3, "\"\n");
    } else {
        std::strcpy(buf + n1 + n2 - 1, "\"...\n");
    }
    std::fputs(buf, stderr);
}
