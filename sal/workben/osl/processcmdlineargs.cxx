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
#include <sal/log.hxx>
#include <rtl/ustring.h>
#include <rtl/alloc.h>
#include <osl/thread.h>

#include <osl/process.h>

#include <cstdio>

SAL_IMPLEMENT_MAIN()
{
    rtl_uString *pustrExePath = nullptr;
    rtl_String *pstrExePath = nullptr;

    fprintf(stdout, "List process arguments\n");

    osl_getExecutableFile(&pustrExePath);
    rtl_uString2String(&pstrExePath,
                       rtl_uString_getStr(pustrExePath),
                       rtl_uString_getLength(pustrExePath),
                       osl_getThreadTextEncoding(),
                       OUSTRING_TO_OSTRING_CVTFLAGS);
    rtl_freeMemory(pustrExePath);

    fprintf(stdout, "    Process executable image: %s\n", rtl_string_getStr(pstrExePath));
    rtl_freeMemory(pstrExePath);

    sal_uInt32 argc = osl_getCommandArgCount();

    if (argc > 0)
    {
        for (sal_uInt32 i = 0; i < argc; i++)
        {
            rtl_uString *pustrArgument = nullptr;
            rtl_String *pstrArgument = nullptr;

            osl_getCommandArg(i, &pustrArgument);
            rtl_uString2String(&pstrArgument,
                               rtl_uString_getStr(pustrArgument),
                               rtl_uString_getLength(pustrArgument),
                               osl_getThreadTextEncoding(),
                               OUSTRING_TO_OSTRING_CVTFLAGS);

            rtl_freeMemory(pustrArgument);

            fprintf(stdout, "    Process argument %d:       %s\n", i, rtl_string_getStr(pstrArgument));

            rtl_freeMemory(pstrArgument);
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
