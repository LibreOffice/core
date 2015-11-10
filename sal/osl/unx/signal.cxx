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
    { SIGEMT,    ACT_SYSTEM,  NULL },    /* EMT instruction */
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

static void getExecutableName_Impl (rtl_String ** ppstrProgName)
{
    rtl_uString * ustrProgFile = nullptr;
    osl_getExecutableFile (&ustrProgFile);
    if (ustrProgFile)
    {
        rtl_uString * ustrProgName = nullptr;
        osl_systemPathGetFileNameOrLastDirectoryPart (ustrProgFile, &ustrProgName);
        if (ustrProgName != nullptr)
        {
            rtl_uString2String (
                ppstrProgName,
                rtl_uString_getStr (ustrProgName), rtl_uString_getLength (ustrProgName),
                osl_getThreadTextEncoding(),
                OUSTRING_TO_OSTRING_CVTFLAGS);
            rtl_uString_release (ustrProgName);
        }
        rtl_uString_release (ustrProgFile);
    }
}

static bool is_soffice_Impl()
{
    sal_Int32    idx       = -1;
    rtl_String * strProgName = nullptr;

    getExecutableName_Impl (&strProgName);
    if (strProgName)
    {
        idx = rtl_str_indexOfStr (rtl_string_getStr (strProgName), "soffice");
        rtl_string_release (strProgName);
    }
    return (idx != -1);
}

static bool InitSignal()
{
    int i;
    struct sigaction act;
    struct sigaction oact;
    sigset_t unset;

    if (is_soffice_Impl())
    {
        // WORKAROUND FOR SEGV HANDLER CONFLICT
        //
        // the java jit needs SIGSEGV for proper work
        // and we need SIGSEGV for the office crashguard
        //
        // TEMPORARY SOLUTION:
        //   the office sets the signal handler during startup
        //   java can than overwrite it, if needed
        bSetSEGVHandler = true;

        // WORKAROUND FOR WINCH HANDLER (SEE ABOVE)
        bSetWINCHHandler = true;

        // WORKAROUND FOR ILLEGAL INSTRUCTION HANDLER (SEE ABOVE)
        bSetILLHandler = true;
    }

#ifdef DBG_UTIL
    bSetSEGVHandler = bSetWINCHHandler = bSetILLHandler = false;
#endif

    SignalListMutex = osl_createMutex();

    act.sa_handler = SignalHandlerFunction;
    act.sa_flags   = SA_RESTART;

    sigfillset(&(act.sa_mask));

    /* Initialize the rest of the signals */
    for (i = 0; i < NoSignals; ++i)
    {
#if defined HAVE_VALGRIND_HEADERS
        if (Signals[i].Signal == SIGUSR2 && RUNNING_ON_VALGRIND)
            Signals[i].Action = ACT_IGNORE;
#endif

        /* hack: stomcatd is attaching JavaVM which does not work with an sigaction(SEGV) */
        if ((bSetSEGVHandler || Signals[i].Signal != SIGSEGV)
        && (bSetWINCHHandler || Signals[i].Signal != SIGWINCH)
        && (bSetILLHandler   || Signals[i].Signal != SIGILL))
        {
            if (Signals[i].Action != ACT_SYSTEM)
            {
                if (Signals[i].Action == ACT_HIDE)
                {
                    struct sigaction ign;

                    ign.sa_handler = SIG_IGN;
                    ign.sa_flags   = 0;
                    sigemptyset(&ign.sa_mask);

                    if (sigaction(Signals[i].Signal, &ign, &oact) == 0)
                        Signals[i].Handler = oact.sa_handler;
                    else
                        Signals[i].Handler = SIG_DFL;
                }
                else
                {
                    if (sigaction(Signals[i].Signal, &act, &oact) == 0)
                        Signals[i].Handler = oact.sa_handler;
                    else
                        Signals[i].Handler = SIG_DFL;
                }
            }
        }
    }

    /* Clear signal mask inherited from parent process (on Mac OS X, upon a
       crash soffice re-execs itself from within the signal handler, so the
       second soffice would have the guilty signal blocked and would freeze upon
       encountering a similar crash again): */
    if (sigemptyset(&unset) < 0 ||
        pthread_sigmask(SIG_SETMASK, &unset, nullptr) < 0)
    {
        OSL_TRACE("sigemptyset or pthread_sigmask failed");
    }

    return true;
}

static bool DeInitSignal()
{
    int i;
    struct sigaction act;

    act.sa_flags   = 0;
    sigemptyset(&(act.sa_mask));

    /* Initialize the rest of the signals */
    for (i = NoSignals - 1; i >= 0; i--)
        if (Signals[i].Action != ACT_SYSTEM)
        {
            act.sa_handler = Signals[i].Handler;

            sigaction(Signals[i].Signal, &act, nullptr);
        }

    osl_destroyMutex(SignalListMutex);

    return false;
}

static void PrintStack( int sig )
{
#ifdef INCLUDE_BACKTRACE
    void *buffer[MAX_STACK_FRAMES];
    int size = backtrace( buffer, SAL_N_ELEMENTS(buffer) );
#endif

    fprintf( stderr, "\n\nFatal exception: Signal %d\n", sig );

#if defined( MACOSX ) && !defined( INCLUDE_BACKTRACE )
    fprintf( stderr, "Please turn on Enable Crash Reporting and\nAutomatic Display of Crashlogs in the Console application\n" );
#else
#ifdef INCLUDE_BACKTRACE
    if ( size > 0 )
    {
        fputs( "Stack:\n", stderr );
        backtrace_symbols_fd( buffer, size, fileno(stderr) );
    }
#endif
#endif
}

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

void CallSystemHandler(int Signal)
{
    int i;
    struct sigaction act;

    for (i = 0; i < NoSignals; i++)
    {
        if (Signals[i].Signal == Signal)
            break;
    }

    if (i < NoSignals)
    {
        if ((Signals[i].Handler == nullptr)    ||
            (Signals[i].Handler == SIG_DFL) ||
            (Signals[i].Handler == SIG_IGN) ||
             (Signals[i].Handler == SIG_ERR))
        {
            switch (Signals[i].Action)
            {
                case ACT_EXIT:      /* terminate */
                    /* prevent dumping core on exit() */
                    _exit(255);
                    break;

                case ACT_ABORT:     /* terminate witch core dump */
                    act.sa_handler = SIG_DFL;
                    act.sa_flags   = 0;
                    sigemptyset(&(act.sa_mask));
                    sigaction(SIGABRT, &act, nullptr);
                    PrintStack( Signal );
                    abort();
                    break;

                case ACT_IGNORE:    /* ignore */
                    break;

                default:            /* should never happen */
                    OSL_ASSERT(false);
            }
        }
        else
            (*Signals[i].Handler)(Signal);
    }
}

#if defined HAVE_VALGRIND_HEADERS
static void DUMPCURRENTALLOCS()
{
    VALGRIND_PRINTF( "=== start memcheck dump of active allocations ===\n" );

#if __GNUC__ && !defined(__clang__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

    VALGRIND_DO_LEAK_CHECK;

#if __GNUC__ && !defined(__clang__)
#   pragma GCC diagnostic pop
#endif

    VALGRIND_PRINTF( "=== end memcheck dump of active allocations ===\n" );
}
#endif

void SignalHandlerFunction(int Signal)
{
    oslSignalInfo    Info;
    struct sigaction act;

    Info.UserSignal = Signal;
    Info.UserData   = nullptr;

    switch (Signal)
    {
        case SIGBUS:
        case SIGILL:
        case SIGSEGV:
        case SIGIOT:
#if ( SIGIOT != SIGABRT )
        case SIGABRT:
#endif
            Info.Signal = osl_Signal_AccessViolation;
            break;

        case -1:
            Info.Signal = osl_Signal_IntegerDivideByZero;
            break;

        case SIGFPE:
            Info.Signal = osl_Signal_FloatDivideByZero;
            break;

        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
            Info.Signal = osl_Signal_Terminate;
            break;

#if defined HAVE_VALGRIND_HEADERS
        case SIGUSR2:
            if (RUNNING_ON_VALGRIND)
                DUMPCURRENTALLOCS();
            Info.Signal = osl_Signal_System;
            break;
#endif

        default:
            Info.Signal = osl_Signal_System;
            break;
    }

    switch (CallSignalHandler(&Info))
    {
    case osl_Signal_ActCallNextHdl:
        CallSystemHandler(Signal);
        break;

    case osl_Signal_ActAbortApp:
        act.sa_handler = SIG_DFL;
        act.sa_flags   = 0;
        sigemptyset(&(act.sa_mask));
        sigaction(SIGABRT, &act, nullptr);
        PrintStack( Signal );
        abort();
        break;

    case osl_Signal_ActKillApp:
        /* prevent dumping core on exit() */
        _exit(255);
        break;
    default:
        break;
    }
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
