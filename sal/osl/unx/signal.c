/*************************************************************************
 *
 *  $RCSfile: signal.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kr $ $Date: 2000-11-21 10:07:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


/* system headers */
#include "system.h"

#include <osl/diagnose.h>
#include <osl/mutex.h>
#include <osl/signal.h>
#include <osl/process.h>


#define ACT_IGNORE  1
#define ACT_ABORT   2
#define ACT_EXIT    3
#define ACT_SYSTEM  4
#define ACT_HIDE    5

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
    { SIGHUP,    ACT_IGNORE,   NULL },    /* hangup */
    { SIGINT,    ACT_EXIT,   NULL },    /* interrupt (rubout) */
    { SIGQUIT,   ACT_ABORT,  NULL },    /* quit (ASCII FS) */
    { SIGILL,    ACT_ABORT,  NULL },    /* illegal instruction (not reset when caught) */
    { SIGTRAP,   ACT_ABORT,  NULL },    /* trace trap (not reset when caught) */
#if ( SIGIOT != SIGABRT )
    { SIGIOT,    ACT_ABORT,  NULL },    /* IOT instruction */
#endif
    { SIGABRT,   ACT_ABORT,  NULL },    /* used by abort, replace SIGIOT in the future */
#ifdef SIGEMT
    { SIGEMT,    ACT_ABORT,  NULL },    /* EMT instruction */
#endif
    { SIGFPE,    ACT_ABORT,  NULL },    /* floating point exception */
    { SIGKILL,   ACT_SYSTEM, NULL },    /* kill (cannot be caught or ignored) */
    { SIGBUS,    ACT_ABORT,  NULL },    /* bus error */
    { SIGSEGV,   ACT_ABORT,  NULL },    /* segmentation violation */
#ifdef SIGSYS
    { SIGSYS,    ACT_ABORT,  NULL },    /* bad argument to system call */
#endif
    { SIGPIPE,   ACT_HIDE,   NULL },    /* write on a pipe with no one to read it */
    { SIGALRM,   ACT_EXIT,   NULL },    /* alarm clock */
    { SIGTERM,   ACT_EXIT,   NULL },    /* software termination signal from kill */
    { SIGUSR1,   ACT_SYSTEM, NULL },    /* user defined signal 1 */
    { SIGUSR2,   ACT_SYSTEM, NULL },    /* user defined signal 2 */
    { SIGCHLD,   ACT_SYSTEM, NULL },    /* child status change */
#ifdef SIGPWR
    { SIGPWR,    ACT_IGNORE, NULL },    /* power-fail restart */
#endif
    { SIGWINCH,  ACT_IGNORE, NULL },    /* window size change */
    { SIGURG,    ACT_EXIT,   NULL },    /* urgent socket condition */
#ifdef SIGPOLL
    { SIGPOLL,   ACT_EXIT,   NULL },    /* pollable event occured */
#endif
    { SIGSTOP,   ACT_SYSTEM, NULL },    /* stop (cannot be caught or ignored) */
    { SIGTSTP,   ACT_SYSTEM, NULL },    /* user stop requested from tty */
    { SIGCONT,   ACT_SYSTEM, NULL },    /* stopped process has been continued */
    { SIGTTIN,   ACT_SYSTEM, NULL },    /* background tty read attempted */
    { SIGTTOU,   ACT_SYSTEM, NULL },    /* background tty write attempted */
    { SIGVTALRM, ACT_EXIT,   NULL },    /* virtual timer expired */
    { SIGPROF,   ACT_EXIT,   NULL },    /* profiling timer expired */
    { SIGXCPU,   ACT_ABORT,  NULL },    /* exceeded cpu limit */
    { SIGXFSZ,   ACT_ABORT,  NULL }     /* exceeded file size limit */
};
const int NoSignals = sizeof(Signals) / sizeof(struct SignalAction);

static sal_Bool               bInitSignal = sal_False;
static oslMutex               SignalListMutex;
static oslSignalHandlerImpl*  SignalList;
static sal_Bool               bDoHardKill = sal_False;
static sal_Bool               bSetSEGVHandler = sal_False;
static sal_Bool               bSetWINCHHandler = sal_False;
static sal_Bool               bSetILLHandler = sal_False;

static void SignalHandlerFunction(int);
extern oslProcessError SAL_CALL osl_psz_getExecutableFile(sal_Char* pszBuffer, sal_uInt32 Max);
oslProcessError SAL_CALL osl_getCommandArgs(sal_Char* pszBuffer, sal_uInt32 Max);

static sal_Bool InitSignal()
{
    int i;
    struct sigaction act;
    struct sigaction oact;

    char ProgFile[512];

    if ((osl_psz_getExecutableFile(ProgFile, sizeof(ProgFile)) ==  osl_Process_E_None) &&
        (strstr(ProgFile, "soffice") != NULL))
    {
        char CmdLine[512];

        if ((osl_getCommandArgs(CmdLine, sizeof(CmdLine)) ==  osl_Process_E_None) &&
             (strstr(CmdLine, "-bean") != NULL))
            bDoHardKill = sal_True;

        // WORKAROUND FOR SEGV HANDLER CONFLICT
        //
        // the java jit needs SIGSEGV for proper work
        // and we need SIGSEGV for the office crashguard
        //
        // TEMPORARY SOLUTION:
        //   the office sets the signal handler during startup
        //   java can than overwrite it, if needed
        bSetSEGVHandler = sal_True;

        // WORKAROUND FOR WINCH HANDLER (SEE ABOVE)
        bSetWINCHHandler = sal_True;

        // WORKAROUND FOR ILLEGAL INSTRUCTION HANDLER (SEE ABOVE)
        bSetILLHandler = sal_True;
    }

    SignalListMutex = osl_createMutex();

    act.sa_handler = SignalHandlerFunction;
    act.sa_flags   = SA_RESTART;

    sigfillset(&(act.sa_mask));

    /* Initialize the rest of the signals */
    for (i = 0; i < NoSignals; i++)
    {
        /* hack: stomcatd is attaching JavaVM wich dont work with an sigaction(SEGV) */
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

                    if (sigaction(Signals[i].Signal, &ign, &oact) == 0)
                        Signals[i].Handler = oact.sa_handler;
                    else
                        Signals[i].Handler = SIG_DFL;
                }
                else
                    if (sigaction(Signals[i].Signal, &act, &oact) == 0)
                        Signals[i].Handler = oact.sa_handler;
                    else
                        Signals[i].Handler = SIG_DFL;
            }
        }
    }

    return sal_True;
}

static sal_Bool DeInitSignal()
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

            sigaction(Signals[i].Signal, &act, NULL);
        }

    osl_destroyMutex(SignalListMutex);

    return sal_False;
}

static oslSignalAction CallSignalHandler(oslSignalInfo *pInfo)
{
    oslSignalHandlerImpl* pHandler = SignalList;
    oslSignalAction Action = osl_Signal_ActCallNextHdl;

    /* PORTAL EA SOFFICE TERMINATION HACK BEGIN */
    struct sigaction act;
    struct sigaction oldact;
    int nRet=0;

    act.sa_flags   = 0;
    sigemptyset(&(act.sa_mask));

    act.sa_handler=SIG_DFL;
    nRet = sigaction(SIGALRM,&act,&oldact);
    if ( nRet < 0 )
    {
        OSL_TRACE("sigaction failed : '%s'\n",strerror(errno));
    }

    alarm(30);
    /* PORTAL EA SOFFICE TERMINATION HACK END */

    while (pHandler != NULL)
    {
        if ((Action = pHandler->Handler(pHandler->pData, pInfo))
            != osl_Signal_ActCallNextHdl)
            break;

        pHandler = pHandler->pNext;
    }

    /* PORTAL EA SOFFICE TERMINATION HACK BEGIN */
    alarm(0);
    /* PORTAL EA SOFFICE TERMINATION HACK END */


    return Action;
}

static void CallSystemHandler(int Signal)
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
        if ((Signals[i].Handler == NULL)    ||
            (Signals[i].Handler == SIG_DFL) ||
            (Signals[i].Handler == SIG_IGN) ||
             (Signals[i].Handler == SIG_ERR))
        {
            switch (Signals[i].Action)
            {
                case ACT_EXIT:      /* terminate */
                    exit(255);
                    break;

                case ACT_ABORT:     /* terminate witch core dump */
                    act.sa_handler = SIG_DFL;
                    act.sa_flags   = 0;
                    sigemptyset(&(act.sa_mask));
                    sigaction(SIGABRT, &act, NULL);
                    abort();
                    break;

                case ACT_IGNORE:    /* ignore */
                    break;

                default:            /* should never happen */
                    OSL_ASSERT(0);
            }
        }
        else
            (*Signals[i].Handler)(Signal);
    }
}

/*****************************************************************************/
/* SignalHandlerFunction    */
/*****************************************************************************/
static void SignalHandlerFunction(int Signal)
{
    oslSignalInfo    Info;
    struct sigaction act;

    Info.UserSignal = Signal;
    Info.UserData   = NULL;

    switch (Signal)
    {
        case SIGBUS:
        case SIGILL:
        case SIGSEGV:
            Info.Signal = osl_Signal_AccessViolation;
            break;

        case -1:
            Info.Signal = osl_Signal_IntegerDivideByZero;
            break;

        case SIGFPE:
            Info.Signal = osl_Signal_FloatDivideByZero;
            break;

        case SIGINT:
        case SIGIOT:
        case SIGTERM:
    case SIGQUIT:
    case SIGHUP:
            Info.Signal = osl_Signal_Terminate;
            break;

        default:
            Info.Signal = osl_Signal_System;
            break;
    }

    /* Portal Demo HACK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    if (bDoHardKill && (Info.Signal == osl_Signal_AccessViolation))
        _exit(255);
    /* Portal Demo HACK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    switch (CallSignalHandler(&Info))
    {
    case osl_Signal_ActCallNextHdl:
        CallSystemHandler(Signal);
        break;

    case osl_Signal_ActAbortApp:
        act.sa_handler = SIG_DFL;
        act.sa_flags   = 0;
        sigemptyset(&(act.sa_mask));
        sigaction(SIGABRT, &act, NULL);
        abort();
        break;

    case osl_Signal_ActKillApp:
        exit(255);
        break;
    default:
        break;
    }
}

/*****************************************************************************/
/* osl_addSignalHandler */
/*****************************************************************************/
oslSignalHandler SAL_CALL osl_addSignalHandler(oslSignalHandlerFunction Handler, void* pData)
{
    oslSignalHandlerImpl* pHandler;

    OSL_ASSERT(Handler != NULL);
    if ( Handler == 0 )
    {
        return 0;
    }

    if (! bInitSignal)
        bInitSignal = InitSignal();

    pHandler = (oslSignalHandlerImpl*) calloc(1, sizeof(oslSignalHandlerImpl));

    if (pHandler != NULL)
    {
        pHandler->Handler = Handler;
        pHandler->pData   = pData;

        osl_acquireMutex(SignalListMutex);

        pHandler->pNext = SignalList;
        SignalList      = pHandler;

        osl_releaseMutex(SignalListMutex);

        return (pHandler);
    }

    return (NULL);
}

/*****************************************************************************/
/* osl_removeSignalHandler */
/*****************************************************************************/
sal_Bool SAL_CALL osl_removeSignalHandler(oslSignalHandler Handler)
{
    oslSignalHandlerImpl *pHandler, *pPrevious = NULL;

    OSL_ASSERT(Handler != NULL);

    if (! bInitSignal)
        bInitSignal = InitSignal();

    osl_acquireMutex(SignalListMutex);

    pHandler = SignalList;

    while (pHandler != NULL)
    {
        if (pHandler == Handler)
        {
            if (pPrevious)
                pPrevious->pNext = pHandler->pNext;
            else
                SignalList = pHandler->pNext;

            osl_releaseMutex(SignalListMutex);

            if (SignalList == NULL)
                bInitSignal = DeInitSignal();

            free(pHandler);

            return (sal_True);
        }

        pPrevious = pHandler;
        pHandler  = pHandler->pNext;
    }

    osl_releaseMutex(SignalListMutex);

    return (sal_False);
}

/*****************************************************************************/
/* osl_raiseSignal */
/*****************************************************************************/
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

