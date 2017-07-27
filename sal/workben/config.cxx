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

#include <sal/config.h>
#include <sal/main.h>

#include <cstdio>

// _WIN32 for some reason has no SAL_DLLPREFIX
#ifndef SAL_DLLPREFIX
#define SAL_DLLPREFIX ""
#endif

SAL_IMPLEMENT_MAIN()
{
    fprintf(stdout, "Platform specific quirks via sal/config.h.\n");
    fprintf(stdout, "    Library name:    %sexample%s\n", SAL_DLLPREFIX, SAL_DLLEXTENSION);
    fprintf(stdout, "    Executable name: example%s\n", SAL_EXEEXTENSION);
    fprintf(stdout, "    PATH syntax:     PATH=%cpath%cto%cexecutable1%c%cpath%cto%cexecutable2\n",
                            SAL_PATHDELIMITER, SAL_PATHDELIMITER, SAL_PATHDELIMITER, SAL_PATHSEPARATOR,
                            SAL_PATHDELIMITER, SAL_PATHDELIMITER, SAL_PATHDELIMITER);
    fprintf(stdout, "    Config file:     %s\n", SAL_CONFIGFILE("config"));
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
