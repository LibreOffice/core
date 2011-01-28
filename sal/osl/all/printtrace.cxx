/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
*
************************************************************************/

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
