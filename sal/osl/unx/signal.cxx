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

#ifdef __sun

#include "backtrace.h"
#define INCLUDE_BACKTRACE

#endif /* defined __sun */

#if defined INCLUDE_BACKTRACE
#define MAX_STACK_FRAMES 256
#endif

#include <osl/diagnose.h>
#include <osl/signal.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <sal/macros.h>
#include <rtl/bootstrap.h>
#include <rtl/digest.h>

#include "file_path_helper.hxx"

namespace
{
const int NoSignals = SAL_N_ELEMENTS(Signals);

void signalHandlerFunction(int, siginfo_t *, void *);
}

bool onInitSignal()
{
    /* Initialize the rest of the signals */
    for (SignalAction &rSignal : Signals)
    {
        // Don't wipe out JVM signal handlers
        switch (rSignal.Signal)
        {
            case SIGSEGV:
            case SIGWINCH: // caters for Apache, this is the signal for graceful shutdown
            case SIGILL:
                break;
            default:
                initSignal(rSignal);
        }
    }

    /* Clear signal mask inherited from parent process (on Mac OS X, upon a
       crash soffice re-execs itself from within the signal handler, so the
       second soffice would have the guilty signal blocked and would freeze upon
       encountering a similar crash again) */
    sigset_t unset;

    if (sigemptyset(&unset) < 0 || pthread_sigmask(SIG_SETMASK, &unset, nullptr) < 0)
        SAL_WARN("sal.osl", "sigemptyset or pthread_sigmask failed");

    return true;
}

void initSignal(SignalAction &rSignal)
{
    struct sigaction act;
    act.sa_sigaction = signalHandlerFunction;
    act.sa_flags = SA_RESTART | SA_SIGINFO;

    sigfillset(&(act.sa_mask));

#if defined HAVE_VALGRIND_HEADERS
    if (rSignal.Signal == SIGUSR2 && RUNNING_ON_VALGRIND)
        rSignal.Action = ACT_IGNORE;
#endif

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
                    rSignal.Handler = reinterpret_cast<Handler1>(
                        oact.sa_sigaction);
                } else {
                    rSignal.Handler = oact.sa_handler;
                }
            } else {
                rSignal.Handler = SIG_DFL;
                rSignal.siginfo = false;
            }
        }
        else
        {
            struct sigaction oact;
            if (sigaction(rSignal.Signal, &act, &oact) == 0) {
                rSignal.siginfo = (oact.sa_flags & SA_SIGINFO) != 0;
                if (rSignal.siginfo) {
                    rSignal.Handler = reinterpret_cast<Handler1>(
                        oact.sa_sigaction);
                } else {
                    rSignal.Handler = oact.sa_handler;
                }
            } else {
                rSignal.Handler = SIG_DFL;
                rSignal.siginfo = false;
            }
        }
    }

    /* Clear signal mask inherited from parent process (on Mac OS X, upon a
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
        if (Signals[i].Action != ACT_SYSTEM)
	    {
            if (Signals[i].siginfo) {
                act.sa_sigaction = reinterpret_cast<Handler2>(
                    Signals[i].Handler);
                act.sa_flags = SA_SIGINFO;
            } else {
                act.sa_handler = Signals[i].Handler;
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

void callSystemHandler(int signal, siginfo_t * info, void * context)
{
    int i;

    for (i = 0; i < NoSignals; i++)
    {
        if (Signals[i].Signal == signal)
            break;
    }

    if (i < NoSignals)
    {
        if ((Signals[i].Handler == SIG_DFL) ||
            (Signals[i].Handler == SIG_IGN) ||
             (Signals[i].Handler == SIG_ERR))
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
            (*reinterpret_cast<Handler2>(Signals[i].Handler))(
                signal, info, context);
        } else {
            (*Signals[i].Handler)(signal);
        }
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
         Info.Signal == osl_Signal_FloatDivideByZero))
    {
        for (SignalAction &rSignal : Signals)
        {
            if (rSignal.Signal == signal)
            {
                if (rSignal.siginfo)
                {
                    (*reinterpret_cast<Handler2>(
                        rSignal.Handler))(signal, info, context);
                }
                else
                {
                    rSignal.Handler(signal);
                }
                break;
            }
        }
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
