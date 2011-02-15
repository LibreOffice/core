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


#ifdef SOLARIS

#include <dlfcn.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/frame.h>
#include "backtrace.h"

#if defined(SPARC)

#if defined IS_LP64

#define FRAME_PTR_OFFSET 1
#define FRAME_OFFSET     0
#define STACK_BIAS       0x7ff

#else

#define FRAME_PTR_OFFSET 1
#define FRAME_OFFSET     0
#define STACK_BIAS       0

#endif

#elif defined( INTEL )

#define FRAME_PTR_OFFSET 3
#define FRAME_OFFSET     0
#define STACK_BIAS       0

#else

#error Unknown Solaris target platform.

#endif /* defined SPARC or INTEL */


int backtrace( void **buffer, int max_frames )
{
    jmp_buf       ctx;
    long          fpval;
    struct frame *fp;
    int i;

    /* flush register windows */
#ifdef SPARC
    asm("ta 3");
#endif

    /* get stack- and framepointer */
    setjmp(ctx);

    fpval = ((long*)(ctx))[FRAME_PTR_OFFSET];
    fp = (struct frame*)((char*)(fpval) + STACK_BIAS);

    for (i = 0; (i < FRAME_OFFSET) && (fp != 0); i++)
        fp = (struct frame*)((char*)(fp->fr_savfp) + STACK_BIAS);

    /* iterate through backtrace */
    for (i = 0; (fp != 0) && (fp->fr_savpc != 0) && (i < max_frames); i++)
    {
        /* saved (prev) frame */
        struct frame * prev = (struct frame*)((char*)(fp->fr_savfp) + STACK_BIAS);

        /* store frame */
        *(buffer++) = (void*)(fp->fr_savpc);

        /* prev frame (w/ stack growing top down) */
        fp = (prev > fp) ? prev : 0;
    }

    /* return number of frames stored */
    return i;
}

void backtrace_symbols_fd( void **buffer, int size, int fd )
{
    FILE    *fp = fdopen( fd, "w" );

    if ( fp )
    {
        void **pFramePtr;

        for ( pFramePtr = buffer; size > 0 && pFramePtr && *pFramePtr; pFramePtr++, size-- )
        {
            Dl_info     dli;
            ptrdiff_t   offset;

            if ( 0 != dladdr( *pFramePtr, &dli ) )
            {
                if ( dli.dli_fname && dli.dli_fbase )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_fbase;
                    fprintf( fp, "%s+0x%x", dli.dli_fname, offset );
                }
                if ( dli.dli_sname && dli.dli_saddr )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_saddr;
                    fprintf( fp, "(%s+0x%x)", dli.dli_sname, offset );
                }
            }
            fprintf( fp, "[0x%x]\n", *pFramePtr );
        }

        fflush( fp );
        fclose( fp );
    }
}

#endif /* defined SOLARIS */


#if defined FREEBSD || defined NETBSD || defined OPENBSD || defined(DRAGONFLY)
#include <dlfcn.h>
#include <pthread.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include "backtrace.h"

#define FRAME_PTR_OFFSET 1
#define FRAME_OFFSET 0

int backtrace( void **buffer, int max_frames )
{
    struct frame *fp;
    jmp_buf ctx;
    int i;
    /* get stack- and framepointer */
    setjmp(ctx);
    fp = (struct frame*)(((size_t*)(ctx))[FRAME_PTR_OFFSET]);
    for ( i=0; (i<FRAME_OFFSET) && (fp!=0); i++)
        fp = fp->fr_savfp;
    /* iterate through backtrace */
    for (i=0; fp && fp->fr_savpc && i<max_frames; i++)
    {
        /* store frame */
        *(buffer++) = (void *)fp->fr_savpc;
        /* next frame */
        fp=fp->fr_savfp;
    }
    return i;
}

void backtrace_symbols_fd( void **buffer, int size, int fd )
{
    FILE    *fp = fdopen( fd, "w" );

    if ( fp )
    {
        void **pFramePtr;
        for ( pFramePtr = buffer; size > 0 && pFramePtr && *pFramePtr; pFramePtr++, size-- )
        {
            Dl_info     dli;
            ptrdiff_t   offset;

            if ( 0 != dladdr( *pFramePtr, &dli ) )
            {
                if ( dli.dli_fname && dli.dli_fbase )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_fbase;
                    fprintf( fp, "%s+0x%x", dli.dli_fname, offset );
                }
                if ( dli.dli_sname && dli.dli_saddr )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_saddr;
                    fprintf( fp, "(%s+0x%x)", dli.dli_sname, offset );
                }
            }
            fprintf( fp, "[0x%x]\n", *pFramePtr );
        }
        fflush( fp );
        fclose( fp );
    }
}
#endif /* defined FREEBSD */

#ifdef LINUX

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include "backtrace.h"

#if defined(SPARC)

#define FRAME_PTR_OFFSET 1
#define FRAME_OFFSET 0

#else

#error Unknown Linux target platform.

#endif /* defined SPARC or INTEL */

typedef int ptrdiff_t;

int backtrace( void **buffer, int max_frames )
{
    struct frame *fp;
    jmp_buf ctx;
    int i;

    /* flush register windows */
#ifdef SPARC
    asm("ta 3");
#endif
    /* get stack- and framepointer */
    setjmp(ctx);
    fp = (struct frame*)(((size_t*)(ctx))[FRAME_PTR_OFFSET]);
    for ( i=0; (i<FRAME_OFFSET) && (fp!=0); i++)
        fp = fp->fr_savfp;

    /* iterate through backtrace */
    for (i=0; fp && fp->fr_savpc && i<max_frames; i++)
    {
        /* store frame */
        *(buffer++) = (void *)fp->fr_savpc;
        /* next frame */
        fp=fp->fr_savfp;
    }
    return i;
}

void backtrace_symbols_fd( void **buffer, int size, int fd )
{
    FILE    *fp = fdopen( fd, "w" );

    if ( fp )
    {
        void **pFramePtr;

        for ( pFramePtr = buffer; size > 0 && pFramePtr && *pFramePtr; pFramePtr++, size-- )
        {
            Dl_info     dli;

            if ( 0 != dladdr( *pFramePtr, &dli ) )
            {
                ptrdiff_t offset;

                if ( dli.dli_fname && dli.dli_fbase )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_fbase;
                    fprintf( fp, "%s+0x%x", dli.dli_fname, offset );
                }
                if ( dli.dli_sname && dli.dli_saddr )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_saddr;
                    fprintf( fp, "(%s+0x%x)", dli.dli_sname, offset );
                }
            }
            fprintf( fp, "[0x%x]\n", *pFramePtr );
        }

        fflush( fp );
        fclose( fp );
    }
}

#endif /* defined LINUX */

#if defined( MACOSX )

#include <dlfcn.h>
#include <stdio.h>
#include "backtrace.h"

typedef unsigned     ptrdiff_t;

/* glib backtrace is only available on MacOsX 10.5 or higher
   so we do it on our own */

int backtrace( void **buffer, int max_frames )
{
    void **frame = (void **)__builtin_frame_address(0);
    void **bp = ( void **)(*frame);
    void *ip = frame[1];
    int i;

    for ( i = 0; bp && ip && i < max_frames; i++ )
    {
        *(buffer++) = ip;

        ip = bp[1];
        bp = (void**)(bp[0]);
    }

    return i;
}


void backtrace_symbols_fd( void **buffer, int size, int fd )
{
    FILE    *fp = fdopen( fd, "w" );

    if ( fp )
    {
        void **pFramePtr;

        for ( pFramePtr = buffer; size > 0 && pFramePtr && *pFramePtr; pFramePtr++, size-- )
        {
            Dl_info     dli;

            if ( 0 != dladdr( *pFramePtr, &dli ) )
            {
                ptrdiff_t offset;

                if ( dli.dli_fname && dli.dli_fbase )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_fbase;
                    fprintf( fp, "%s+0x%x", dli.dli_fname, offset );
                }
                if ( dli.dli_sname && dli.dli_saddr )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_saddr;
                    fprintf( fp, "(%s+0x%x)", dli.dli_sname, offset );
                }
            }
            fprintf( fp, "[0x%x]\n", (unsigned int)*pFramePtr );
        }

        fflush( fp );
        fclose( fp );
    }
}

#endif /* defined MACOSX */

#if defined(AIX)
int backtrace( void **buffer, int max_frames )
{
    return 0;
}

void backtrace_symbols_fd( void **buffer, int size, int fd )
{
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
