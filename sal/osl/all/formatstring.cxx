/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 Stephan Bergmann, Red Hat Inc. <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "precompiled_sal.hxx"
#include "sal/config.h"

#include <cstdarg>
#include <cstring>
#include <stdio.h> // vsnprintf not in C++03 <cstdio>, only C99 <stdio.h>

#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/string.hxx"

rtl_String * osl_detail_formatString(char const * format, ...)
    SAL_THROW_EXTERN_C()
{
    // Avoid the use of other sal code as much as possible, so that this code
    // can be called from other sal code without causing endless recursion:
    char buf[1024];
    int n1 = sizeof buf - RTL_CONSTASCII_LENGTH("...");
    std::va_list args;
    va_start(args, format);
    int n2 = vsnprintf(buf, n1, format, args);
    va_end(args);
    if (n2 < 0) {
        std::strcpy(buf, "???");
        n2 = RTL_CONSTASCII_LENGTH("???");
    } else if (n2 >= n1) {
        std::strcpy(buf + n1 - 1, "...");
        n2 = sizeof buf - 1;
    }
    rtl::OString s(buf, n2);
    rtl_string_acquire(s.pData);
    return s.pData;
}
