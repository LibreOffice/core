/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pipe.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:30:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _OSL_PIPE_H_
#define _OSL_PIPE_H_

#ifndef _RTL_USTRING_H
#   include <rtl/ustring.h>
#endif


#ifndef _OSL_SECURITY_H_
#   include <osl/security.h>
#endif

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

