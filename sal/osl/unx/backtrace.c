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

#include "sal/types.h"


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
            fprintf( fp, "[%p]\n", *pFramePtr );
        }

        fflush( fp );
        fclose( fp );
    }
}

#endif /* defined SOLARIS */


#if defined FREEBSD || defined NETBSD
#include <dlfcn.h>
#include <pthread.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include "backtrace.h"

#define FRAME_PTR_OFFSET 3
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
                    fprintf( fp, "%s+0x%" SAL_PRI_PTRDIFFT "x", dli.dli_fname, offset );
                }
                if ( dli.dli_sname && dli.dli_saddr )
                {
                    offset = (ptrdiff_t)*pFramePtr - (ptrdiff_t)dli.dli_saddr;
                    fprintf( fp, "(%s+0x%" SAL_PRI_PTRDIFFT "x)", dli.dli_sname, offset );
                }
            }
            fprintf( fp, "[%p]\n", *pFramePtr );
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
            fprintf( fp, "[%p]\n", *pFramePtr );
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
            fprintf( fp, "[%p]\n", *pFramePtr );
        }

        fflush( fp );
        fclose( fp );
    }
}

#endif /* defined MACOSX */
