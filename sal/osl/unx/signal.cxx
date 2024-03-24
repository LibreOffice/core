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

#include <signalshared.hxx>

#include <config_features.h>

#include "soffice.hxx"

#include "backtrace.h"

#define MAX_STACK_FRAMES 256

#include <osl/diagnose.h>
#include <osl/signal.h>
#include <sal/log.hxx>
#include <sal/macros.h>
#include <sal/backtrace.hxx>

#define ACT_IGNORE  1
#define ACT_EXIT    2
#define ACT_SYSTEM  3
#define ACT_HIDE    4
#define ACT_ABORT   5

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/memcheck.h>
#endif

#include <signal.h>
#include <unistd.h>

namespace
{
extern "C" using Handler1_t = void (*)(int);
extern "C" using Handler2_t = void (*)(int, siginfo_t *, void *);
struct SignalAction
{
    int Signal;
    int Action;
    union {
        Handler1_t Handler1;
        Handler2_t Handler2;
    };
    bool siginfo; // Handler2 is active
} Signals[] =
{
    { SIGHUP,    ACT_HIDE,   SIG_DFL, false }, /* hangup */
    { SIGINT,    ACT_EXIT,   SIG_DFL, false }, /* interrupt (rubout) */
    { SIGQUIT,   ACT_EXIT,   SIG_DFL, false }, /* quit (ASCII FS) */
    { SIGILL,    ACT_SYSTEM, SIG_DFL, false }, /* illegal instruction (not reset when caught) */
/* changed from ACT_ABOUT to ACT_SYSTEM to try and get collector to run*/
    { SIGTRAP,   ACT_ABORT,  SIG_DFL, false }, /* trace trap (not reset when caught) */
#if ( SIGIOT != SIGABRT )
    { SIGIOT,    ACT_ABORT,  SIG_DFL, false }, /* IOT instruction */
#endif
#if defined(FORCE_DEFAULT_SIGNAL)
    { SIGABRT,   ACT_SYSTEM, SIG_DFL, false }, /* used by abort, replace SIGIOT in the future */
#else
    { SIGABRT,   ACT_ABORT,  SIG_DFL, false }, /* used by abort, replace SIGIOT in the future */
#endif
#ifdef SIGEMT
    { SIGEMT,    ACT_SYSTEM, SIG_DFL, false }, /* EMT instruction */
/* changed from ACT_ABORT to ACT_SYSTEM to remove handler*/
/* SIGEMT may also be used by the profiler - so it is probably not a good
plan to have the new handler use this signal*/
#endif
    { SIGFPE,    ACT_ABORT,  SIG_DFL, false }, /* floating point exception */
    { SIGKILL,   ACT_SYSTEM, SIG_DFL, false }, /* kill (cannot be caught or ignored) */
    { SIGBUS,    ACT_ABORT,  SIG_DFL, false }, /* bus error */
#if defined(FORCE_DEFAULT_SIGNAL)
    { SIGSEGV,   ACT_SYSTEM, SIG_DFL, false }, /* segmentation violation */
#else
    { SIGSEGV,   ACT_ABORT,  SIG_DFL, false }, /* segmentation violation */
#endif
#ifdef SIGSYS
    { SIGSYS,    ACT_ABORT,  SIG_DFL, false }, /* bad argument to system call */
#endif
    { SIGPIPE,   ACT_HIDE,   SIG_DFL, false }, /* write on a pipe with no one to read it */
#if defined(FORCE_DEFAULT_SIGNAL)
    { SIGALRM,   ACT_SYSTEM, SIG_DFL, false }, /* alarm clock */
#else
    { SIGALRM,   ACT_EXIT,   SIG_DFL, false }, /* alarm clock */
#endif
    { SIGTERM,   ACT_EXIT,   SIG_DFL, false }, /* software termination signal from kill */
    { SIGUSR1,   ACT_SYSTEM, SIG_DFL, false }, /* user defined signal 1 */
    { SIGUSR2,   ACT_SYSTEM, SIG_DFL, false }, /* user defined signal 2 */
    { SIGCHLD,   ACT_SYSTEM, SIG_DFL, false }, /* child status change */
#ifdef SIGPWR
    { SIGPWR,    ACT_IGNORE, SIG_DFL, false }, /* power-fail restart */
#endif
    { SIGWINCH,  ACT_IGNORE, SIG_DFL, false }, /* window size change */
    { SIGURG,    ACT_EXIT,   SIG_DFL, false }, /* urgent socket condition */
#ifdef SIGPOLL
    { SIGPOLL,   ACT_EXIT,   SIG_DFL, false }, /* pollable event occurred */
#endif
    { SIGSTOP,   ACT_SYSTEM, SIG_DFL, false }, /* stop (cannot be caught or ignored) */
    { SIGTSTP,   ACT_SYSTEM, SIG_DFL, false }, /* user stop requested from tty */
    { SIGCONT,   ACT_SYSTEM, SIG_DFL, false }, /* stopped process has been continued */
    { SIGTTIN,   ACT_SYSTEM, SIG_DFL, false }, /* background tty read attempted */
    { SIGTTOU,   ACT_SYSTEM, SIG_DFL, false }, /* background tty write attempted */
    { SIGVTALRM, ACT_EXIT,   SIG_DFL, false }, /* virtual timer expired */
    { SIGPROF,   ACT_SYSTEM, SIG_DFL, false }, /* profiling timer expired */
/*Change from ACT_EXIT to ACT_SYSTEM for SIGPROF is so that profiling signals do
not get taken by the new handler - the new handler does not pass on context
information which causes 'collect' to crash. This is a way of avoiding
what looks like a bug in the new handler*/
    { SIGXCPU,   ACT_ABORT,  SIG_DFL, false }, /* exceeded cpu limit */
    { SIGXFSZ,   ACT_ABORT,  SIG_DFL, false }  /* exceeded file size limit */
};
const int NoSignals = SAL_N_ELEMENTS(Signals);

bool bSetSEGVHandler = false;
bool bSetWINCHHandler = false;
bool bSetILLHandler = false;

void signalHandlerFunction(int, siginfo_t *, void *);

#if HAVE_FEATURE_BREAKPAD
bool is_unset_signal(int signal)
{
#ifdef DBG_UTIL
    return (!bSetSEGVHandler && signal == SIGSEGV) ||
        (!bSetWINCHHandler && signal == SIGWINCH) ||
        (!bSetILLHandler && signal == SIGILL);
#else
    (void) signal;
    return false;
#endif
}
#endif

}

bool onInitSignal()
{
    if (sal::detail::isSoffice())
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

    struct sigaction act;
    act.sa_sigaction = signalHandlerFunction;
    act.sa_flags = SA_RESTART | SA_SIGINFO;

    sigfillset(&(act.sa_mask));

    /* Initialize the rest of the signals */
    for (SignalAction & rSignal : Signals)
    {
#if defined HAVE_VALGRIND_HEADERS
        if (rSignal.Signal == SIGUSR2 && RUNNING_ON_VALGRIND)
            rSignal.Action = ACT_IGNORE;
#endif

        /* hack: stomcatd is attaching JavaVM which does not work with an sigaction(SEGV) */
        if ((bSetSEGVHandler || rSignal.Signal != SIGSEGV)
        && (bSetWINCHHandler || rSignal.Signal != SIGWINCH)
        && (bSetILLHandler   || rSignal.Signal != SIGILL))
        {
            if (rSignal.Action != ACT_SYSTEM)
            {
                if (rSignal.Action == ACT_HIDE)
                {
                    struct sigaction ign;

                    ign.sa_handler = SIG_IGN;
                    ign.sa_flags   = 0;
                    sigemptyset(&ign.sa_mask);

                    struct sigaction oact;
                    if (sigaction(rSignal.Signal, &ign, &oact) == 0) {
                        rSignal.siginfo = (oact.sa_flags & SA_SIGINFO) != 0;
                        if (rSignal.siginfo) {
                            rSignal.Handler2 =
                                oact.sa_sigaction;
                        } else {
                            rSignal.Handler1 = oact.sa_handler;
                        }
                    } else {
                        rSignal.Handler1 = SIG_DFL;
                        rSignal.siginfo = false;
                    }
                }
                else
                {
                    struct sigaction oact;
                    if (sigaction(rSignal.Signal, &act, &oact) == 0) {
                        rSignal.siginfo = (oact.sa_flags & SA_SIGINFO) != 0;
                        if (rSignal.siginfo) {
                            rSignal.Handler2 =
                                oact.sa_sigaction;
                        } else {
                            rSignal.Handler1 = oact.sa_handler;
                        }
                    } else {
                        rSignal.Handler1 = SIG_DFL;
                        rSignal.siginfo = false;
                    }
                }
            }
        }
    }

    /* Clear signal mask inherited from parent process (on macOS, upon a
       crash soffice re-execs itself from within the signal handler, so the
       second soffice would have the guilty signal blocked and would freeze upon
       encountering a similar crash again): */
    sigset_t unset;
    if (sigemptyset(&unset) < 0 ||
        pthread_sigmask(SIG_SETMASK, &unset, nullptr) < 0)
    {
        SAL_WARN("sal.osl", "sigemptyset or pthread_sigmask failed");
    }

    return true;
}

bool onDeInitSignal()
{
    struct sigaction act;

    sigemptyset(&(act.sa_mask));

    /* Initialize the rest of the signals */
    for (int i = NoSignals - 1; i >= 0; i--)
        if (Signals[i].Action != ACT_SYSTEM
            && ((bSetSEGVHandler || Signals[i].Signal != SIGSEGV)
                && (bSetWINCHHandler || Signals[i].Signal != SIGWINCH)
                && (bSetILLHandler || Signals[i].Signal != SIGILL)))
        {
            if (Signals[i].siginfo) {
                act.sa_sigaction =
                    Signals[i].Handler2;
                act.sa_flags = SA_SIGINFO;
            } else {
                act.sa_handler = Signals[i].Handler1;
                act.sa_flags = 0;
            }

            sigaction(Signals[i].Signal, &act, nullptr);
        }

    return false;
}

namespace
{
void printStack(int sig)
{
    std::unique_ptr<sal::BacktraceState> bs = sal::backtrace_get(MAX_STACK_FRAMES);

    fprintf( stderr, "\n\nFatal exception: Signal %d\n", sig );

#if ! HAVE_FEATURE_BACKTRACE && defined( MACOSX ) && !defined( INTEL )
    fprintf( stderr, "Please turn on Enable Crash Reporting and\nAutomatic Display of Crashlogs in the Console application\n" );
#endif

    fputs( "Stack:\n", stderr );
    fprintf( stderr, "%s\n", OUStringToOString( sal::backtrace_to_string(bs.get()), RTL_TEXTENCODING_UTF8 ).getStr() );
}

void callSystemHandler(int signal, siginfo_t * info, void * context)
{
    int i;

    for (i = 0; i < NoSignals; i++)
    {
        if (Signals[i].Signal == signal)
            break;
    }

    if (i >= NoSignals)
        return;

    if ((Signals[i].Handler1 == SIG_DFL) ||
        (Signals[i].Handler1 == SIG_IGN) ||
         (Signals[i].Handler1 == SIG_ERR))
    {
        switch (Signals[i].Action)
        {
            case ACT_EXIT:      /* terminate */
                /* prevent dumping core on exit() */
                _exit(255);
                break;

            case ACT_ABORT:     /* terminate with core dump */
                struct sigaction act;
                act.sa_handler = SIG_DFL;
                act.sa_flags   = 0;
                sigemptyset(&(act.sa_mask));
                sigaction(SIGABRT, &act, nullptr);
                printStack( signal );
                abort();
                break;

            case ACT_IGNORE:    /* ignore */
                break;

            default:            /* should never happen */
                OSL_ASSERT(false);
        }
    }
    else if (Signals[i].siginfo) {
        (*Signals[i].Handler2)(
            signal, info, context);
    } else {
        (*Signals[i].Handler1)(signal);
    }
}

#if defined HAVE_VALGRIND_HEADERS
void DUMPCURRENTALLOCS()
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

void signalHandlerFunction(int signal, siginfo_t * info, void * context)
{
    oslSignalInfo Info;

    Info.UserSignal = signal;
    Info.UserData   = nullptr;

    switch (signal)
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

#if HAVE_FEATURE_BREAKPAD
    if ((Info.Signal == osl_Signal_AccessViolation ||
            Info.Signal == osl_Signal_IntegerDivideByZero ||
            Info.Signal == osl_Signal_FloatDivideByZero) && !is_unset_signal(signal))
    {
        callSystemHandler(signal, info, context);
    }
#endif

    switch (callSignalHandler(&Info))
    {
    case osl_Signal_ActCallNextHdl:
        callSystemHandler(signal, info, context);
        break;

    case osl_Signal_ActAbortApp:
        struct sigaction act;
        act.sa_handler = SIG_DFL;
        act.sa_flags   = 0;
        sigemptyset(&(act.sa_mask));
        sigaction(SIGABRT, &act, nullptr);
        printStack( signal );
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
