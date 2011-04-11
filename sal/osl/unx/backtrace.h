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


#if defined (SOLARIS) || defined (FREEBSD) || defined (NETBSD) || \
    defined (OPENBSD) || defined (DRAGONFLY)

#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

void backtrace_symbols_fd( void **buffer, int size, int fd );

/* no frame.h on FreeBSD */
#if defined (FREEBSD) || defined (NETBSD) || defined (OPENBSD) || \
    defined (DRAGONFLY)
struct frame {
    long    arg0[8];
    long    arg1[6];
    struct frame *fr_savfp;
    long    fr_savpc;
};
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined SOLARIS || FREEBSD || NETBSD || OPENBSD */

#if defined (LINUX) && defined (SPARC)
#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

void backtrace_symbols_fd( void **buffer, int size, int fd );

/* no frame.h on linux sparc */
struct frame {
    long    arg0[8];
    long    arg1[6];
    struct frame *fr_savfp;
    long    fr_savpc;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined LINUX && SPARC */

#if defined (MACOSX)

#ifdef __cplusplus
extern "C" {
#endif

/* backtrace function with same behaviour as defined in GNU libc */

int backtrace( void **buffer, int max_frames );

void backtrace_symbols_fd( void **buffer, int size, int fd );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* defined MACOSX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
