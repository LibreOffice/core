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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

 #ifndef _ERRNO_H
 #include <errno.h>
 #endif

 #ifndef _FILE_ERROR_TRANSL_H_
 #include "file_error_transl.h"
 #endif

 #ifndef _OSL_DIAGNOSE_H_
 #include <osl/diagnose.h>
 #endif


/********************************************
 * oslTranslateFileError
 *******************************************/

oslFileError oslTranslateFileError(sal_Bool bIsError, int Errno)
{
    oslFileError osl_error = osl_File_E_invalidError;

    OSL_ENSURE((bIsError && (0 != Errno)) || (!bIsError && (0 == Errno)), "oslTranslateFileError strange input combination!");

    /* Have a look at file_error_transl.h for
       the reason that we do this here */
    if (bIsError && (0 == Errno))
        return osl_error;

    switch(Errno)
    {
        case 0:
            osl_error = osl_File_E_None;
            break;

        case EPERM:
            osl_error = osl_File_E_PERM;
            break;

        case ENOENT:
            osl_error = osl_File_E_NOENT;
            break;

        case ESRCH:
            osl_error = osl_File_E_SRCH;
            break;

        case EINTR:
            osl_error = osl_File_E_INTR;
            break;

        case EIO:
            osl_error = osl_File_E_IO;
            break;

        case ENXIO:
            osl_error = osl_File_E_IO;
            break;

        case E2BIG:
            osl_error = osl_File_E_2BIG;
            break;

        case ENOEXEC:
            osl_error = osl_File_E_NOEXEC;
            break;

        case EBADF:
            osl_error = osl_File_E_BADF;
            break;

        case ECHILD:
            osl_error = osl_File_E_CHILD;
            break;

        case EAGAIN:
            osl_error = osl_File_E_AGAIN;
            break;

        case ENOMEM:
            osl_error = osl_File_E_NOMEM;
            break;

        case EACCES:
            osl_error = osl_File_E_ACCES;
            break;

        case EFAULT:
            osl_error = osl_File_E_FAULT;
            break;

        case EBUSY:
            osl_error = osl_File_E_BUSY;
            break;

        case EEXIST:
            osl_error = osl_File_E_EXIST;
            break;

        case EXDEV:
            osl_error = osl_File_E_XDEV;
            break;

        case ENODEV:
            osl_error = osl_File_E_NODEV;
            break;

        case ENOTDIR:
            osl_error = osl_File_E_NOTDIR;
            break;

        case EISDIR:
            osl_error = osl_File_E_ISDIR;
            break;

        case EINVAL:
            osl_error = osl_File_E_INVAL;
            break;

        case ENFILE:
            osl_error = osl_File_E_NFILE;
            break;

        case EMFILE:
            osl_error = osl_File_E_MFILE;
            break;

        case ENOTTY:
            osl_error = osl_File_E_NOTTY;
            break;

        case EFBIG:
            osl_error = osl_File_E_FBIG;
            break;

        case ENOSPC:
            osl_error = osl_File_E_NOSPC;
            break;

        case ESPIPE:
            osl_error = osl_File_E_SPIPE;
            break;

        case EROFS:
            osl_error = osl_File_E_ROFS;
            break;

        case EMLINK:
            osl_error = osl_File_E_MLINK;
            break;

        case EPIPE:
            osl_error = osl_File_E_PIPE;
            break;

        case EDOM:
            osl_error = osl_File_E_DOM;
            break;

        case ERANGE:
            osl_error = osl_File_E_RANGE;
            break;

        case EDEADLK:
            osl_error = osl_File_E_DEADLK;
            break;

        case ENAMETOOLONG:
            osl_error = osl_File_E_NAMETOOLONG;
            break;

        case ENOLCK:
            osl_error = osl_File_E_NOLCK;
            break;

        case ENOSYS:
           osl_error = osl_File_E_NOSYS;
            break;

        case ENOTEMPTY:
            osl_error = osl_File_E_NOTEMPTY;
            break;

        case ELOOP:
            osl_error = osl_File_E_LOOP;
            break;

#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
        case EILSEQ:
            osl_error = osl_File_E_ILSEQ;
            break;
#endif /* MACOSX */

#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
        case ENOLINK:
            osl_error = osl_File_E_NOLINK;
            break;
#endif /* MACOSX */

#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
        case EMULTIHOP:
            osl_error = osl_File_E_MULTIHOP;
            break;
#endif /* MACOSX */

        case EUSERS:
            osl_error = osl_File_E_USERS;
            break;

        case EOVERFLOW:
            osl_error = osl_File_E_OVERFLOW;
            break;

        case ETIMEDOUT:
            osl_error = osl_File_E_TIMEDOUT;
            break;

        default:
            /* FIXME translateFileError: is this alright? Or add a new one: osl_File_E_Unknown? */
            osl_error = osl_File_E_invalidError;
            break;
    }

    return osl_error;
}

