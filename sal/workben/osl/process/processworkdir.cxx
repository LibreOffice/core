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
#include <rtl/string.h>
#include <rtl/ustring.h>
#include <osl/thread.h>

#include <osl/process.h>

#include <cstdio>

SAL_IMPLEMENT_MAIN()
{
    rtl_uString *pustrWorkDir = nullptr;
    rtl_String *pstrWorkDir = nullptr;

    osl_getProcessWorkingDir(&pustrWorkDir);
    rtl_uString2String(&pstrWorkDir,
                       rtl_uString_getStr(pustrWorkDir),
                       rtl_uString_getLength(pustrWorkDir),
                       osl_getThreadTextEncoding(),
                       OUSTRING_TO_OSTRING_CVTFLAGS);
    sal_Char* pszWorkDir = rtl_string_getStr(pstrWorkDir);

    fprintf(stdout, "Process current working directory: %s\n", pszWorkDir);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
