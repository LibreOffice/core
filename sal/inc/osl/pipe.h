/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef _OSL_PIPE_H_
#define _OSL_PIPE_H_

#   include <rtl/ustring.h>


#   include <osl/security.h>

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
oslPipe SAL_CALL osl_createPipe(rtl_uString *strPipeName, oslPipeOptions Options, oslSecurity Security);

/** decreases the refcount of the pipe.
    If the refcount drops to zero, the handle is destroyed.
 */
void    SAL_CALL osl_releasePipe( oslPipe );

/** increases the refcount of the pipe.
 */
void    SAL_CALL osl_acquirePipe( oslPipe Pipe );

/** closes the pipe, any read,write or accept actions stop immeadiatly.
 */
void    SAL_CALL osl_closePipe( oslPipe );


oslPipe SAL_CALL osl_acceptPipe(oslPipe Pipe);

sal_Int32 SAL_CALL osl_sendPipe(oslPipe Pipe, const void* pBuffer, sal_Int32 BufferSize);
sal_Int32 SAL_CALL osl_receivePipe(oslPipe Pipe, void* pBuffer, sal_Int32 BufferSize);

/** Reads blocking from the pipe.
    @return Number of read bytes. If less than BufferSize, the pipe was closed.
 */
sal_Int32 SAL_CALL osl_readPipe( oslPipe Pipe, void *pBuffer, sal_Int32 BufferSize );

/** Writes blocking onto the pipe.
    @return Number of written bytes. If less than BufferSize, the pipe was closed.
 */
sal_Int32 SAL_CALL osl_writePipe( oslPipe Pipe, const void *pBuffer, sal_Int32 BufferSize );

oslPipeError SAL_CALL osl_getLastPipeError(oslPipe Pipe);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_PIPE_H_ */

