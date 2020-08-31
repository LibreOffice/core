/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_folders.h>

#include <sal/config.h>

#include <stdlib.h>

#if defined UNX
#include <sys/resource.h>
#endif

#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/extendapplicationenvironment.hxx>

namespace tools {

void extendApplicationEnvironment() {
#if defined UNX
    // Try to set RLIMIT_NOFILE as large as possible (failure is harmless):
    rlimit lim;
    if (getrlimit(RLIMIT_NOFILE, &lim) == 0) {
        lim.rlim_cur = lim.rlim_max;
        setrlimit(RLIMIT_NOFILE, &lim);
    }
#endif

    // Make sure URE_BOOTSTRAP environment variable is set (failure is fatal):
    OUStringBuffer env(512);
    OUString envVar("URE_BOOTSTRAP");
    OUString uri;
    if (rtl::Bootstrap::get(envVar, uri))
    {
        if (!uri.matchIgnoreAsciiCase("vnd.sun.star.pathname:"))
        {
            uri = rtl::Bootstrap::encode(uri);
        }
        env.append(uri);
    } else {
        if (osl_getExecutableFile(&uri.pData) != osl_Process_E_None) {
            abort();
        }
        sal_Int32 lastDirSeparatorPos = uri.lastIndexOf('/');
        if (lastDirSeparatorPos >= 0) {
            uri = uri.copy(0, lastDirSeparatorPos + 1);
        }
        env.append(rtl::Bootstrap::encode(uri));
#ifdef MACOSX
        env.append("../" LIBO_SHARE_FOLDER "/");
#endif
        env.append(SAL_CONFIGFILE("fundamental"));
    }
    OUString envValue(env.makeStringAndClear());
    if (osl_setEnvironment(envVar.pData, envValue.pData) != osl_Process_E_None) {
        abort();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
