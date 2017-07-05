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

#include <errno.h>
#include "file_error_transl.hxx"
#include <sal/log.hxx>

oslFileError oslTranslateFileError(bool bIsError, int Errno)
{
    oslFileError osl_error = osl_File_E_invalidError;

    SAL_WARN_IF(!(bIsError && (Errno != 0)) || (!bIsError && (Errno == 0)),
            "sal.file", "oslTranslateFileError strange input combination!");

    /* Have a look at file_error_transl.hxx for
       the reason that we do this here */
    if (bIsError && (Errno == 0))
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

#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD) || defined(DRAGONFLY))
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
