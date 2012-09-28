/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _RTL_PROCESS_H_
#define _RTL_PROCESS_H_

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
    @see rtl_createUiid()
 */
SAL_DLLPUBLIC void SAL_CALL rtl_getGlobalProcessId( sal_uInt8 *pTargetUUID );

/** Get the nArg-th command-line argument passed to the main-function of this process.

    This functions differs from osl_getCommandArg() in filtering any bootstrap values
    given by command args, that means that all arguments starting with "-env:" will be
    ignored by this function.

    @param nArg [in] The number of the argument to return.
    @param strCommandArg [out] The string receives the nArg-th command-line argument.
    @return osl_Process_E_None or does not return.
    @see osl_getCommandArg()
    @see rtl_getCommandArgCount()
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL rtl_getAppCommandArg(sal_uInt32 nArg, rtl_uString **strCommandArg);

/** Returns the number of command line arguments at process start.

    This functions differs from osl_getCommandArg() in filtering any bootstrap values
    given by command args, that means that all arguments starting with "-env:" will be
    ignored by this function.

    @return the number of commandline arguments passed to the main-function of this process.
    @see osl_getCommandArgCount()
    @see rtl_getCommandArg()
*/
SAL_DLLPUBLIC sal_uInt32 SAL_CALL rtl_getAppCommandArgCount();

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
