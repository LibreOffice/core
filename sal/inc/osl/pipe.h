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


#ifndef _OSL_PIPE_H_
#define _OSL_PIPE_H_

#include "sal/config.h"

#include "osl/security.h"
#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    osl_Pipe_E_None,                /* no error */
    osl_Pipe_E_NotFound,            /* Pipe could not be found */
    osl_Pipe_E_AlreadyExists,       /* Pipe already exists */
    osl_Pipe_E_NoProtocol,          /* Protocol not available */
    osl_Pipe_E_NetworkReset,        /* Network dropped connection because of reset */
    osl_Pipe_E_ConnectionAbort,     /* Software caused connection abort */
    osl_Pipe_E_ConnectionReset,     /* Connection reset by peer */
    osl_Pipe_E_NoBufferSpace,       /* No buffer space available */
    osl_Pipe_E_TimedOut,            /* Connection timed out */
    osl_Pipe_E_ConnectionRefused,   /* Connection refused */
    osl_Pipe_E_invalidError,        /* unmapped error: always last entry in enum! */
    osl_Pipe_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslPipeError;

typedef sal_uInt32 oslPipeOptions;
#define osl_Pipe_OPEN        0x0000     /* open existing pipe */
#define osl_Pipe_CREATE      0x0001     /* create pipe and open it, fails if already existst */

typedef struct oslPipeImpl * oslPipe;

/**
 */
SAL_DLLPUBLIC oslPipe SAL_CALL osl_createPipe(
        rtl_uString *strPipeName, oslPipeOptions Options, oslSecurity Security);

/** decreases the refcount of the pipe.
    If the refcount drops to zero, the handle is destroyed.
 */
SAL_DLLPUBLIC void    SAL_CALL osl_releasePipe( oslPipe );

/** increases the refcount of the pipe.
 */
SAL_DLLPUBLIC void    SAL_CALL osl_acquirePipe( oslPipe Pipe );

/** closes the pipe, any read,write or accept actions stop immeadiatly.
 */
SAL_DLLPUBLIC void    SAL_CALL osl_closePipe( oslPipe );


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

#endif  /* _OSL_PIPE_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
