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


#ifndef INCLUDED_OSL_PIPE_H
#define INCLUDED_OSL_PIPE_H

#include "sal/config.h"

#include "osl/security.h"
#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    osl_Pipe_E_None,                    /*< no error */
    osl_Pipe_E_NotFound,                /*< Pipe could not be found */
    osl_Pipe_E_AlreadyExists,           /*< Pipe already exists */
    osl_Pipe_E_NoProtocol,              /*< Protocol not available */
    osl_Pipe_E_NetworkReset,            /*< Network dropped connection because of reset */
    osl_Pipe_E_ConnectionAbort,         /*< Software caused connection abort */
    osl_Pipe_E_ConnectionReset,         /*< Connection reset by peer */
    osl_Pipe_E_NoBufferSpace,           /*< No buffer space available */
    osl_Pipe_E_TimedOut,                /*< Connection timed out */
    osl_Pipe_E_ConnectionRefused,       /*< Connection refused */
    osl_Pipe_E_invalidError,            /*< unmapped error: always last entry in enum! */
    osl_Pipe_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslPipeError;

/** Pipe creation options.

    A pipe can either be opened, or a new pipe can be created and opened.
*/
typedef sal_uInt32 oslPipeOptions;
#define osl_Pipe_OPEN        0x0000     /*< open existing pipe */
#define osl_Pipe_CREATE      0x0001     /*< create pipe and open it, fails if already exists */

typedef struct oslPipeImpl * oslPipe;

/** Create or open a pipe.

    @param[in] strPipeName  pipe name
    @param[in] Options      create or open the pipe
    @param[in] Security     pipe creator

    @returns nullptr on failure, otherwise returns the pipe handle

    @see osl_closePipe
*/
SAL_DLLPUBLIC oslPipe SAL_CALL osl_createPipe(
        rtl_uString *strPipeName, oslPipeOptions Options, oslSecurity Security);

/** Decreases the refcount of the pipe.

    If the refcount drops to zero, the handle is destroyed.

    @param[in] Pipe         pipe handle

    @see osl_acquirePipe
 */
SAL_DLLPUBLIC void    SAL_CALL osl_releasePipe(oslPipe Pipe);

/** Increases the refcount of the pipe.

    @param[in] Pipe         pipe handle

    @see osl_releasePipe
 */
SAL_DLLPUBLIC void    SAL_CALL osl_acquirePipe(oslPipe Pipe);

/** Close the pipe.

    Any read, write or accept actions stop immediately.

    @param[in] Pipe         pipe handle

    @see osl_createPipe
 */
SAL_DLLPUBLIC void    SAL_CALL osl_closePipe(oslPipe Pipe);


SAL_DLLPUBLIC oslPipe SAL_CALL osl_acceptPipe(oslPipe Pipe);

SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_sendPipe(oslPipe Pipe, const void* pBuffer, sal_Int32 BufferSize);
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_receivePipe(oslPipe Pipe, void* pBuffer, sal_Int32 BufferSize);

/** Reads blocking from the pipe.
    @return Number of read bytes. If less than BufferSize, the pipe was closed.
 */
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_readPipe( oslPipe Pipe, void *pBuffer, sal_Int32 BufferSize );

/** Writes blocking onto the pipe.
    @return Number of written bytes. If less than BufferSize, the pipe was closed.
 */
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_writePipe( oslPipe Pipe, const void *pBuffer, sal_Int32 BufferSize );

SAL_DLLPUBLIC oslPipeError SAL_CALL osl_getLastPipeError(oslPipe Pipe);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_PIPE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
