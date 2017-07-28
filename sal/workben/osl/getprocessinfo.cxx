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

#include <sal/main.h>
#include <osl/process.h>

#include <cstdio>

SAL_IMPLEMENT_MAIN()
{
    oslProcessInfo info;
    info.Size = sizeof(info);

    fprintf(stdout, "Getting current process information.\n");

    osl_getProcessInfo(nullptr, osl_Process_IDENTIFIER | osl_Process_CPUTIMES | osl_Process_HEAPUSAGE, &info);

    fprintf(stdout, "    Process ID: %d\n", info.Ident);
    fprintf(stdout, "    Heap usage: %d%\n", info.HeapUsage);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
