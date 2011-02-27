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

#if !defined(AIX) || !(defined(_ALL_SOURCE) && !defined(_LINUX_SOURCE_COMPAT))
        case ENOTEMPTY:
            osl_error = osl_File_E_NOTEMPTY;
            break;
#endif

        case ELOOP:
            osl_error = osl_File_E_LOOP;
            break;

#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD))
        case EILSEQ:
            osl_error = osl_File_E_ILSEQ;
            break;

        case ENOLINK:
            osl_error = osl_File_E_NOLINK;
            break;

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
