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

#include "precompiled_tools.hxx"
#include "sal/config.h"

#include <stdlib.h>

#if defined UNX
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#include "osl/process.h"
#include "osl/thread.h"
#include "rtl/bootstrap.hxx"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "tools/extendapplicationenvironment.hxx"

namespace tools {

void extendApplicationEnvironment() {
#if defined UNX
    // Try to set RLIMIT_NOFILE as large as possible (failure is harmless):
    rlimit l;
    if (getrlimit(RLIMIT_NOFILE, &l) == 0) {
        l.rlim_cur = l.rlim_max;
        setrlimit(RLIMIT_NOFILE, &l);
    }
#endif

    // Make sure URE_BOOTSTRAP environment variable is set (failure is fatal):
    rtl::OUStringBuffer env;
    rtl::OUString envVar(RTL_CONSTASCII_USTRINGPARAM("URE_BOOTSTRAP"));
    rtl::OUString uri;
    if (rtl::Bootstrap::get(envVar, uri))
    {
        if (!uri.matchIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pathname:")))
        {
            uri = rtl::Bootstrap::encode(uri);
        }
        env.append(uri);
    } else {
        if (osl_getExecutableFile(&uri.pData) != osl_Process_E_None) {
            abort();
        }
        sal_Int32 i = uri.lastIndexOf('/');
        if (i >= 0) {
            uri = uri.copy(0, i + 1);
        }
        env.append(rtl::Bootstrap::encode(uri));
        env.appendAscii(
            RTL_CONSTASCII_STRINGPARAM(SAL_CONFIGFILE("fundamental")));
    }
    rtl::OUString envValue(env.makeStringAndClear());
    if (osl_setEnvironment(envVar.pData, envValue.pData) != osl_Process_E_None) {
        abort();
    }
}

}
