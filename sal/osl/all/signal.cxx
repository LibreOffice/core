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

#include <config_features.h>

/* system headers */
#include "system.hxx"

#if defined( MACOSX )

#if defined( INTEL )
#include "backtrace.h"
#define INCLUDE_BACKTRACE
#endif /* INTEL */

#endif /* MACOSX */

#ifdef LINUX
#include <execinfo.h>
#include <link.h>
#define INCLUDE_BACKTRACE
#endif

#ifdef SOLARIS

#include "backtrace.h"
#define INCLUDE_BACKTRACE

#endif /* defined SOLARIS */

#if defined INCLUDE_BACKTRACE
#define MAX_STACK_FRAMES 256
#endif

#include <osl/diagnose.h>
#include <osl/mutex.h>
#include <osl/signal.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <sal/macros.h>
#include <rtl/bootstrap.h>
#include <rtl/digest.h>

#include "file_path_helper.hxx"

#define ACT_IGNORE  1
#define ACT_EXIT    2
#define ACT_SYSTEM  3
#define ACT_HIDE    4
#define ACT_ABORT   5

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/memcheck.h>
#endif

typedef struct _oslSignalHandlerImpl
{
    oslSignalHandlerFunction      Handler;
    void*                         pData;
    struct _oslSignalHandlerImpl* pNext;
} oslSignalHandlerImpl;

static struct SignalAction
{
    int Signal;
    int Action;
    void (*Handler)(int);
} Signals[] =
{
    { SIGHUP,    ACT_HIDE, nullptr },    /* hangup */
    { SIGINT,    ACT_EXIT,   nullptr },    /* interrupt (rubout) */
    { SIGQUIT,   ACT_EXIT,  nullptr },    /* quit (ASCII FS) */
    { SIGILL,    ACT_SYSTEM,  nullptr },    /* illegal instruction (not reset when caught) */
/* changed from ACT_ABOUT to ACT_SYSTEM to try and get collector to run*/
    { SIGTRAP,   ACT_ABORT,  nullptr },    /* trace trap (not reset when caught) */
#if ( SIGIOT != SIGABRT )
    { SIGIOT,    ACT_ABORT,  NULL },    /* IOT instruction */
#endif
    { SIGABRT,   ACT_ABORT,  nullptr },    /* used by abort, replace SIGIOT in the future */
#ifdef SIGEMT
    { SIGEMT,    ACT_SYSTEM,  nullptr },    /* EMT instruction */
/* changed from ACT_ABORT to ACT_SYSTEM to remove handler*/
/* SIGEMT may also be used by the profiler - so it is probably not a good
plan to have the new handler use this signal*/
#endif
    { SIGFPE,    ACT_ABORT,  nullptr },    /* floating point exception */
    { SIGKILL,   ACT_SYSTEM, nullptr },    /* kill (cannot be caught or ignored) */
    { SIGBUS,    ACT_ABORT,  nullptr },    /* bus error */
    { SIGSEGV,   ACT_ABORT,  nullptr },    /* segmentation violation */
#ifdef SIGSYS
    { SIGSYS,    ACT_ABORT,  nullptr },    /* bad argument to system call */
#endif
    { SIGPIPE,   ACT_HIDE,   nullptr },    /* write on a pipe with no one to read it */
    { SIGALRM,   ACT_EXIT,   nullptr },    /* alarm clock */
    { SIGTERM,   ACT_EXIT,   nullptr },    /* software termination signal from kill */
    { SIGUSR1,   ACT_SYSTEM, nullptr },    /* user defined signal 1 */
    { SIGUSR2,   ACT_SYSTEM, nullptr },    /* user defined signal 2 */
    { SIGCHLD,   ACT_SYSTEM, nullptr },    /* child status change */
#ifdef SIGPWR
    { SIGPWR,    ACT_IGNORE, nullptr },    /* power-fail restart */
#endif
    { SIGWINCH,  ACT_IGNORE, nullptr },    /* window size change */
    { SIGURG,    ACT_EXIT,   nullptr },    /* urgent socket condition */
#ifdef SIGPOLL
    { SIGPOLL,   ACT_EXIT,   nullptr },    /* pollable event occurred */
#endif
    { SIGSTOP,   ACT_SYSTEM, nullptr },    /* stop (cannot be caught or ignored) */
    { SIGTSTP,   ACT_SYSTEM, nullptr },    /* user stop requested from tty */
    { SIGCONT,   ACT_SYSTEM, nullptr },    /* stopped process has been continued */
    { SIGTTIN,   ACT_SYSTEM, nullptr },    /* background tty read attempted */
    { SIGTTOU,   ACT_SYSTEM, nullptr },    /* background tty write attempted */
    { SIGVTALRM, ACT_EXIT,   nullptr },    /* virtual timer expired */
    { SIGPROF,   ACT_SYSTEM,   nullptr },    /* profiling timer expired */
/*Change from ACT_EXIT to ACT_SYSTEM for SIGPROF is so that profiling signals do
not get taken by the new handler - the new handler does not pass on context
information which causes 'collect' to crash. This is a way of avoiding
what looks like a bug in the new handler*/
    { SIGXCPU,   ACT_ABORT,  nullptr },    /* exceeded cpu limit */
    { SIGXFSZ,   ACT_ABORT,  nullptr }     /* exceeded file size limit */
};
const int NoSignals = sizeof(Signals) / sizeof(struct SignalAction);

static bool               bErrorReportingEnabled = true;
static bool               bInitSignal = false;
static oslMutex               SignalListMutex;
static oslSignalHandlerImpl*  SignalList;
static bool               bSetSEGVHandler = false;
static bool               bSetWINCHHandler = false;
static bool               bSetILLHandler = false;

static void SignalHandlerFunction(int);

static oslSignalAction CallSignalHandler(oslSignalInfo *pInfo)
{
    oslSignalHandlerImpl* pHandler = SignalList;
    oslSignalAction Action = osl_Signal_ActCallNextHdl;

    while (pHandler != nullptr)
    {
        if ((Action = pHandler->Handler(pHandler->pData, pInfo))
            != osl_Signal_ActCallNextHdl)
            break;

        pHandler = pHandler->pNext;
    }

    return Action;
}

oslSignalHandler SAL_CALL osl_addSignalHandler(oslSignalHandlerFunction Handler, void* pData)
{
    oslSignalHandlerImpl* pHandler;

    OSL_ASSERT(Handler != nullptr);
    if ( Handler == nullptr )
    {
        return nullptr;
    }

    if (! bInitSignal)
        bInitSignal = InitSignal();

    pHandler = static_cast<oslSignalHandlerImpl*>(calloc(1, sizeof(oslSignalHandlerImpl)));

    if (pHandler != nullptr)
    {
        pHandler->Handler = Handler;
        pHandler->pData   = pData;

        osl_acquireMutex(SignalListMutex);

        pHandler->pNext = SignalList;
        SignalList      = pHandler;

        osl_releaseMutex(SignalListMutex);

        return (pHandler);
    }

    return nullptr;
}

sal_Bool SAL_CALL osl_removeSignalHandler(oslSignalHandler Handler)
{
    oslSignalHandlerImpl *pHandler, *pPrevious = nullptr;

    OSL_ASSERT(Handler != nullptr);

    if (! bInitSignal)
        bInitSignal = InitSignal();

    osl_acquireMutex(SignalListMutex);

    pHandler = SignalList;

    while (pHandler != nullptr)
    {
        if (pHandler == Handler)
        {
            if (pPrevious)
                pPrevious->pNext = pHandler->pNext;
            else
                SignalList = pHandler->pNext;

            osl_releaseMutex(SignalListMutex);

            if (SignalList == nullptr)
                bInitSignal = DeInitSignal();

            free(pHandler);

            return sal_True;
        }

        pPrevious = pHandler;
        pHandler  = pHandler->pNext;
    }

    osl_releaseMutex(SignalListMutex);

    return sal_False;
}

oslSignalAction SAL_CALL osl_raiseSignal(sal_Int32 UserSignal, void* UserData)
{
    oslSignalInfo   Info;
    oslSignalAction Action;

    if (! bInitSignal)
        bInitSignal = InitSignal();

    osl_acquireMutex(SignalListMutex);

    Info.Signal     = osl_Signal_User;
    Info.UserSignal = UserSignal;
    Info.UserData   = UserData;

    Action = CallSignalHandler(&Info);

    osl_releaseMutex(SignalListMutex);

    return (Action);
}

sal_Bool SAL_CALL osl_setErrorReporting( sal_Bool bEnable )
{
    bool bOld = bErrorReportingEnabled;
    bErrorReportingEnabled = bEnable;

    return bOld;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
