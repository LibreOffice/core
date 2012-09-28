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

#include "osl/diagnose.h"
#include "system.h"

#ifndef HAVE_DLFCN_H

#if defined(LINUX) || defined(SOLARIS)
#define HAVE_DLFCN_H
#endif  /* LINUX || SOLARIS */

#endif  /* HAVE_DLFCN_H */


#ifdef  HAVE_DLFCN_H

#ifndef INCLUDED_DLFCN_H
#include <dlfcn.h>
#define INCLUDED_DLFCN_H
#endif

#endif  /* HAVE_DLFCN_H */

#ifndef INCLUDED_PTHREAD_H
#include <pthread.h>
#define INCLUDED_PTHREAD_H
#endif

#ifndef INCLUDED_STDDEF_H
#include <stddef.h>
#define INCLUDED_STDDEF_H
#endif

/************************************************************************/
/* Internal data structures and functions */
/************************************************************************/

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef pfunc_osl_printDebugMessage oslDebugMessageFunc;
static oslDebugMessageFunc volatile g_pDebugMessageFunc = 0;

typedef pfunc_osl_printDetailedDebugMessage oslDetailedDebugMessageFunc;
static oslDetailedDebugMessageFunc volatile g_pDetailedDebugMessageFunc = 0;

static void osl_diagnose_backtrace_Impl (
    oslDebugMessageFunc f);

#define OSL_DIAGNOSE_OUTPUTMESSAGE(f, s) \
((f != 0) ? (*(f))((s)) : (void)fprintf(stderr, "%s", (s)))

#if defined (LINUX) || defined (SOLARIS)
/************************************************************************/
/* osl_diagnose_frame_Impl */
/************************************************************************/
static void osl_diagnose_frame_Impl (
    oslDebugMessageFunc f,
    int                 depth,
    void *              pc)
{
    const char *fname = 0, *sname = 0;
    void       *fbase = 0, *saddr = 0;
    ptrdiff_t   offset;
    char        szMessage[1024];

#ifdef INCLUDED_DLFCN_H
    Dl_info dli;
    if (dladdr (pc, &dli) != 0)
    {
        fname = dli.dli_fname;
        fbase = dli.dli_fbase;
        sname = dli.dli_sname;
        saddr = dli.dli_saddr;
    }
#endif /* INCLUDED_DLFCN_H */

    if (saddr)
        offset = (ptrdiff_t)(pc) - (ptrdiff_t)(saddr);
    else if (fbase)
        offset = (ptrdiff_t)(pc) - (ptrdiff_t)(fbase);
    else
        offset = (ptrdiff_t)(pc);

    snprintf (szMessage, sizeof(szMessage),
              "Backtrace: [%d] %s: %s+0x%" SAL_PRI_PTRDIFFT "x\n",
              depth,
              fname ? fname : "<unknown>",
              sname ? sname : "???",
              offset);

    OSL_DIAGNOSE_OUTPUTMESSAGE(f, szMessage);
}
#endif

/************************************************************************/
/* osl_diagnose_backtrace_Impl */
/************************************************************************/
#if defined(LINUX)

#include <execinfo.h>

#define FRAME_COUNT 64
#define FRAME_OFFSET 1

static void osl_diagnose_backtrace_Impl (oslDebugMessageFunc f)
{
    void * ppFrames[FRAME_COUNT];
    int    i, n;

    n = backtrace (ppFrames, FRAME_COUNT);
    for (i = FRAME_OFFSET; i < n; i++)
    {
        osl_diagnose_frame_Impl (f, (i - FRAME_OFFSET), ppFrames[i]);
    }
}

#elif defined(SOLARIS)

#include <pthread.h>
#include <setjmp.h>
#include <sys/frame.h>

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

#elif defined(INTEL)

#define FRAME_PTR_OFFSET 3
#define FRAME_OFFSET     0
#define STACK_BIAS       0

#endif /* (SPARC || INTEL) */

static void osl_diagnose_backtrace_Impl (oslDebugMessageFunc f)
{
    jmp_buf        ctx;
    long           fpval;
    struct frame * fp;
    int            i;

#if defined(SPARC)
    asm("ta 3");
#endif /* SPARC */
    setjmp (ctx);

    fpval = ((long*)(ctx))[FRAME_PTR_OFFSET];
    fp = (struct frame*)((char*)(fpval) + STACK_BIAS);

    for (i = 0; (i < FRAME_OFFSET) && (fp != 0); i++)
        fp = (struct frame*)((char*)(fp->fr_savfp) + STACK_BIAS);

    for (i = 0; (fp != 0) && (fp->fr_savpc != 0); i++)
    {
        struct frame * prev = (struct frame*)((char*)(fp->fr_savfp) + STACK_BIAS);
        osl_diagnose_frame_Impl (f, i, (void*)(fp->fr_savpc));
        fp = (prev > fp) ? prev : 0;
    }
}

#else  /* (LINUX || SOLARIS) */

static void osl_diagnose_backtrace_Impl (oslDebugMessageFunc f)
{
    (void) f;
    /* not yet implemented */
}

#endif /* (LINUX || SOLARIS) */

/************************************************************************/
/* osl_assertFailedLine */
/************************************************************************/

namespace {

// getenv is not thread safe, so minimize use of result:
bool isEnv(char const * name) {
    char * p = getenv(name);
    return p != NULL && *p != '\0';
}

}

sal_Bool SAL_CALL osl_assertFailedLine (
    const sal_Char* pszFileName,
    sal_Int32       nLine,
    const sal_Char* pszMessage)
{
    oslDebugMessageFunc f = g_pDebugMessageFunc;
    char                szMessage[1024];

    // after reporting the assertion, abort if told so by SAL_DIAGNOSE_ABORT, but *not* if
    // assertions are routed to some external instance
    static bool envAbort = isEnv( "SAL_DIAGNOSE_ABORT" );
    static bool envBacktrace = isEnv( "SAL_DIAGNOSE_BACKTRACE" );
    sal_Bool const doAbort = envAbort && f == NULL;

    /* If there's a callback for detailed messages, use it */
    if ( g_pDetailedDebugMessageFunc != NULL )
    {
        g_pDetailedDebugMessageFunc( pszFileName, nLine, pszMessage );
        return sal_False;
    }

    /* format message into buffer */
    if (pszMessage != 0)
    {
        snprintf(szMessage, sizeof(szMessage),
                 "Error: File %s, Line %" SAL_PRIdINT32 ": %s\n",
                 pszFileName, nLine, pszMessage);
    }
    else
    {
        snprintf(szMessage, sizeof(szMessage),
                 "Error: File %s, Line %" SAL_PRIdINT32 "\n",
                 pszFileName, nLine);
    }

    /* acquire lock to serialize output message(s) */
    pthread_mutex_lock(&g_mutex);

    /* output message buffer */
    OSL_DIAGNOSE_OUTPUTMESSAGE(f, szMessage);

    /* should we output backtrace? */
    if( envBacktrace )
        osl_diagnose_backtrace_Impl(f);

    /* release lock and leave */
    pthread_mutex_unlock(&g_mutex);

    return doAbort;
}

/************************************************************************/
/* osl_breakDebug */
/************************************************************************/
void SAL_CALL osl_breakDebug()
{
    abort();
}

/************************************************************************/
/* osl_reportError */
/************************************************************************/
sal_Int32 SAL_CALL osl_reportError (
    sal_uInt32      nType,
    const sal_Char* pszMessage)
{
    (void) nType; /* unused */
    fputs(pszMessage, stderr);
    return 0;
}

/************************************************************************/
/* osl_setDebugMessageFunc */
/************************************************************************/
oslDebugMessageFunc SAL_CALL osl_setDebugMessageFunc (
    oslDebugMessageFunc pNewFunc)
{
    oslDebugMessageFunc pOldFunc = g_pDebugMessageFunc;
    g_pDebugMessageFunc = pNewFunc;
    return pOldFunc;
}

/************************************************************************/
/* osl_setDetailedDebugMessageFunc */
/************************************************************************/
pfunc_osl_printDetailedDebugMessage SAL_CALL osl_setDetailedDebugMessageFunc (
    pfunc_osl_printDetailedDebugMessage pNewFunc)
{
    oslDetailedDebugMessageFunc pOldFunc = g_pDetailedDebugMessageFunc;
    g_pDetailedDebugMessageFunc = pNewFunc;
    return pOldFunc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
