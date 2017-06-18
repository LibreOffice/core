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

#ifndef INCLUDED_OSL_SIGNAL_H
#define INCLUDED_OSL_SIGNAL_H

#include <sal/config.h>

#include <sal/saldllapi.h>
#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OSL_SIGNAL_USER_RESERVED    0

#define OSL_SIGNAL_USER_RESOURCEFAILURE     (OSL_SIGNAL_USER_RESERVED - 1)
#define OSL_SIGNAL_USER_X11SUBSYSTEMERROR   (OSL_SIGNAL_USER_RESERVED - 2)

typedef void* oslSignalHandler;

typedef enum
{
    osl_Signal_System,
    osl_Signal_Terminate,
    osl_Signal_AccessViolation,
    osl_Signal_IntegerDivideByZero,
    osl_Signal_FloatDivideByZero,
    osl_Signal_DebugBreak,
    osl_Signal_User,
    osl_Signal_Alarm,
    osl_Signal_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSignal;

typedef enum
{
    osl_Signal_ActCallNextHdl,
    osl_Signal_ActIgnore,
    osl_Signal_ActAbortApp,
    osl_Signal_ActKillApp,
    osl_Signal_Act_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSignalAction;

#ifdef SAL_W32
#   pragma pack(push, 8)
#endif

typedef struct
{
    oslSignal   Signal;
    sal_Int32   UserSignal;
    void*       UserData;
} oslSignalInfo;

#if defined( SAL_W32)
#   pragma pack(pop)
#endif

/** the function-ptr. representing the signal handler-function.
*/
typedef oslSignalAction (SAL_CALL *oslSignalHandlerFunction)(void* pData, oslSignalInfo* pInfo);

SAL_DLLPUBLIC oslSignalHandler SAL_CALL osl_addSignalHandler(
        oslSignalHandlerFunction Handler, void* pData);

SAL_DLLPUBLIC sal_Bool SAL_CALL osl_removeSignalHandler(
        oslSignalHandler hHandler);

SAL_DLLPUBLIC oslSignalAction SAL_CALL osl_raiseSignal(
        sal_Int32 UserSignal, void* UserData);

/** Enables or disables error reporting

    On default error reporting is enabled after process startup.

    @param  bEnable [in]
    Enables or disables error reporting.

    @retval sal_True if previous state of error reporting was enabled
    @retval sal_False if previous state of error reporting was disabled
*/

SAL_DLLPUBLIC sal_Bool SAL_CALL osl_setErrorReporting(
        sal_Bool bEnable );

#ifdef UNX

#include <signal.h>

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/memcheck.h>
#endif

#define ACT_IGNORE  1
#define ACT_EXIT    2
#define ACT_SYSTEM  3
#define ACT_HIDE    4
#define ACT_ABORT   5

extern "C" using Handler1 = void (*)(int);
extern "C" using Handler2 = void (*)(int, siginfo_t *, void *);

typedef struct
{
    int Signal;
    int Action;
    Handler1 Handler;
    bool siginfo; // Handler's type is Handler2
} SignalAction;

static SignalAction Signals[] =
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

void SAL_DLLPUBLIC initSignal(SignalAction &rSignal);
#endif

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_SIGNAL_H


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
