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
#include <sal/log.hxx>
#include <rtl/ustring.hxx>
#include <rtl/alloc.h>
#include <osl/thread.h>
#include <osl/file.h>

#include <osl/process.h>

#include <cstdio>

SAL_IMPLEMENT_MAIN()
{
    oslProcess aProcess;

    fprintf(stdout, "Execute process.\n");

    rtl_uString *pustrExePath = nullptr;
    osl_getExecutableFile(&pustrExePath);

    rtl_uString *pTempExePath = nullptr;
    sal_uInt32 nLastChar;

    nLastChar = rtl_ustr_lastIndexOfChar(rtl_uString_getStr(pustrExePath), SAL_PATHDELIMITER);
    rtl_uString_newReplaceStrAt(&pTempExePath, pustrExePath, nLastChar, rtl_ustr_getLength(rtl_uString_getStr(pustrExePath)), nullptr);
    rtl_freeMemory(pustrExePath);
    pustrExePath = pTempExePath;

#if defined(_WIN32)
#  define BATCHFILE "\\..\\sal\\workben\\osl\\batch.bat"
#  define BATCHFILE_LENGTH 35
#else
#  define BATCHFILE "/../../../sal/workben/osl/batch.sh"
#  define BATCHFILE_LENGTH 34
#endif

    rtl_uString_newConcatAsciiL(&pustrExePath, pustrExePath, BATCHFILE, BATCHFILE_LENGTH);

    oslProcessError osl_error = osl_executeProcess(
        pustrExePath,           // process to execute
        nullptr,                // no arguments
        0,                      // no arguments
        osl_Process_NORMAL,     // process execution mode
        nullptr,                // security context is current user
        nullptr,                // current working directory inherited from parent process
        nullptr,                // no environment variables
        0,                      // no environment variables
        &aProcess);              // process handle

    rtl_freeMemory(pustrExePath);

    if (osl_error != osl_Process_E_None)
        fprintf(stderr, "Process failed\n");

    fprintf(stdout, "    Process running...\n");
    osl_joinProcess(aProcess);
    fprintf(stdout, "    ...process finished.\n");
    osl_freeProcessHandle(aProcess);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
