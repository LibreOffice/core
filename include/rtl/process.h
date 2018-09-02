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
#ifndef INCLUDED_RTL_PROCESS_H
#define INCLUDED_RTL_PROCESS_H

#include "sal/config.h"

#include "osl/process.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
    gets a 16-byte fixed size identifier which is guaranteed not to change
    during the current process.

    The current implementation creates a 16-byte uuid without using
    the ethernet address of system. Thus the
    identifier is different from identifiers created
    in other processes with a very probability.

    @param pTargetUUID 16 byte of memory
    @see rtl_createUuid()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_getGlobalProcessId( sal_uInt8 *pTargetUUID );

/** Get the nArg-th command-line argument passed to the main-function of this process.

    This function differs from osl_getCommandArg() in filtering any bootstrap values
    given by command args, that means that all arguments starting with "-env:" will be
    ignored by this function.

    @param[in]  nArg          The number of the argument to return.
    @param[out] strCommandArg The string receives the nArg-th command-line argument.
    @retval osl_Process_E_None or does not return.
    @see osl_getCommandArg()
    @see rtl_getAppCommandArgCount()
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL rtl_getAppCommandArg(sal_uInt32 nArg, rtl_uString **strCommandArg);

/** Returns the number of command line arguments at process start.

    This function differs from osl_getCommandArg() in filtering any bootstrap values
    given by command args, that means that all arguments starting with "-env:" will be
    ignored by this function.

    @return the number of commandline arguments passed to the main-function of this process.
    @see osl_getCommandArgCount()
    @see rtl_getAppCommandArg()
*/
SAL_DLLPUBLIC sal_uInt32 SAL_CALL rtl_getAppCommandArgCount(void);

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
