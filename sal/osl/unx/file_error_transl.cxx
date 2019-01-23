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

#include <cassert>
#include <cerrno>

#include "file_error_transl.hxx"

oslFileError oslTranslateFileError(int Errno)
{
    switch(Errno)
    {
        case EPERM:
            return osl_File_E_PERM;

        case ENOENT:
            return osl_File_E_NOENT;

        case ESRCH:
            return osl_File_E_SRCH;

        case EINTR:
            return osl_File_E_INTR;

        case EIO:
            return osl_File_E_IO;

        case ENXIO:
            return osl_File_E_IO;

        case E2BIG:
            return osl_File_E_2BIG;

        case ENOEXEC:
            return osl_File_E_NOEXEC;

        case EBADF:
            return osl_File_E_BADF;

        case ECHILD:
            return osl_File_E_CHILD;

        case EAGAIN:
            return osl_File_E_AGAIN;

        case ENOMEM:
            return osl_File_E_NOMEM;

        case EACCES:
            return osl_File_E_ACCES;

        case EFAULT:
            return osl_File_E_FAULT;

        case EBUSY:
            return osl_File_E_BUSY;

        case EEXIST:
            return osl_File_E_EXIST;

        case EXDEV:
            return osl_File_E_XDEV;

        case ENODEV:
            return osl_File_E_NODEV;

        case ENOTDIR:
            return osl_File_E_NOTDIR;

        case EISDIR:
            return osl_File_E_ISDIR;

        case EINVAL:
            return osl_File_E_INVAL;

        case ENFILE:
            return osl_File_E_NFILE;

        case EMFILE:
            return osl_File_E_MFILE;

        case ENOTTY:
            return osl_File_E_NOTTY;

        case EFBIG:
            return osl_File_E_FBIG;

        case ENOSPC:
            return osl_File_E_NOSPC;

        case ESPIPE:
            return osl_File_E_SPIPE;

        case EROFS:
            return osl_File_E_ROFS;

        case EMLINK:
            return osl_File_E_MLINK;

        case EPIPE:
            return osl_File_E_PIPE;

        case EDOM:
            return osl_File_E_DOM;

        case ERANGE:
            return osl_File_E_RANGE;

        case EDEADLK:
            return osl_File_E_DEADLK;

        case ENAMETOOLONG:
            return osl_File_E_NAMETOOLONG;

        case ENOLCK:
            return osl_File_E_NOLCK;

        case ENOSYS:
        case ENOTSUP:
#if EOPNOTSUPP != ENOTSUP
        case EOPNOTSUPP:
#endif
           return osl_File_E_NOSYS;

#if !defined(AIX) || !(defined(_ALL_SOURCE) && !defined(_LINUX_SOURCE_COMPAT))
        case ENOTEMPTY:
            return osl_File_E_NOTEMPTY;
#endif

        case ELOOP:
            return osl_File_E_LOOP;

#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD) || defined(DRAGONFLY))
        case EILSEQ:
            return osl_File_E_ILSEQ;

        case ENOLINK:
            return osl_File_E_NOLINK;

        case EMULTIHOP:
            return osl_File_E_MULTIHOP;
#endif /* MACOSX */

#if !defined(HAIKU)
        case EUSERS:
            return osl_File_E_USERS;
#endif

        case EOVERFLOW:
            return osl_File_E_OVERFLOW;

        case ETIMEDOUT:
            return osl_File_E_TIMEDOUT;

        default:
            assert(Errno != 0);
            /* FIXME translateFileError: is this alright? Or add a new one: osl_File_E_Unknown? */
            return osl_File_E_invalidError;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
