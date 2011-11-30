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




/* system headers */
#include "system.h"

#include <osl/diagnose.h>
#include <osl/mutex.h>
#include <osl/signal.h>

typedef struct _oslSignalHandlerImpl
{
    oslSignalHandlerFunction      Handler;
    void*                         pData;
    struct _oslSignalHandlerImpl* pNext;
} oslSignalHandlerImpl;

static sal_Bool               bErrorReportingEnabled = sal_True;
static sal_Bool               bInitSignal = sal_False;
static oslMutex               SignalListMutex;
static oslSignalHandlerImpl*  SignalList;

/*static*//* ULONG _Export APIENTRY SignalHandlerFunction(PEXCEPTIONREPORTRECORD pERepRec,
                                            PEXCEPTIONREGISTRATIONRECORD,
                                            PCONTEXTRECORD, PVOID);
*/
/*static*/ ULONG __declspec(dllexport) APIENTRY SignalHandlerFunction(PEXCEPTIONREPORTRECORD pERepRec,
                                            PEXCEPTIONREGISTRATIONRECORD,
                                            PCONTEXTRECORD, PVOID);
static EXCEPTIONREGISTRATIONRECORD ExcptHandler = { 0, SignalHandlerFunction };

static sal_Bool InitSignal( void )
{
    SignalListMutex = osl_createMutex();

    ExcptHandler.ExceptionHandler = (_ERR *) &SignalHandlerFunction;
    /* DosSetExceptionHandler(&ExcptHandler); */

    return sal_True;
}

static sal_Bool DeInitSignal( void )
{
    /* DosUnsetExceptionHandler(&ExcptHandler); */

    osl_destroyMutex(SignalListMutex);

    return sal_False;
}

static oslSignalAction CallSignalHandler(oslSignalInfo *pInfo)
{
    oslSignalHandlerImpl* pHandler = SignalList;
    oslSignalAction Action = osl_Signal_ActCallNextHdl;

    while (pHandler != NULL)
    {
        if ((Action = pHandler->Handler(pHandler->pData, pInfo)) != osl_Signal_ActCallNextHdl)
            break;

        pHandler = pHandler->pNext;
    }

    return Action;
}

/*****************************************************************************/
/* SignalHandlerFunction    */
/*****************************************************************************/
/*static*/ ULONG __declspec(dllexport) APIENTRY SignalHandlerFunction(PEXCEPTIONREPORTRECORD pERepRec,
                                            PEXCEPTIONREGISTRATIONRECORD pERegRec,
                                            PCONTEXTRECORD pConRec, PVOID pReserved)
{
    oslSignalInfo    Info;

    Info.UserSignal = pERepRec->ExceptionNum;
    Info.UserData   = NULL;

    switch (pERepRec->ExceptionNum)
    {
        case XCPT_ACCESS_VIOLATION:
            Info.Signal = osl_Signal_AccessViolation;
            break;

        case XCPT_INTEGER_DIVIDE_BY_ZERO:
            Info.Signal = osl_Signal_IntegerDivideByZero;
            break;

        case XCPT_BREAKPOINT:
            Info.Signal = osl_Signal_DebugBreak;
            break;

        default:
            Info.Signal = osl_Signal_System;
            break;
    }

    switch (CallSignalHandler(&Info))
    {
        case osl_Signal_ActCallNextHdl:
            return (XCPT_CONTINUE_SEARCH);

        case osl_Signal_ActAbortApp:
            return (XCPT_CONTINUE_SEARCH);

        case osl_Signal_ActKillApp:
            exit(255);
            break;
    }

    return (XCPT_CONTINUE_SEARCH);
}

/*****************************************************************************/
/* osl_addSignalHandler */
/*****************************************************************************/
oslSignalHandler SAL_CALL osl_addSignalHandler(oslSignalHandlerFunction Handler, void* pData)
{
    oslSignalHandlerImpl* pHandler;

    OSL_ASSERT(Handler != NULL);

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

            if (SignalList == NULL )
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

/*****************************************************************************/
/* osl_setErrorReporting */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setErrorReporting( sal_Bool bEnable )
{
    sal_Bool bOld = bErrorReportingEnabled;
    bErrorReportingEnabled = bEnable;

    return bOld;
}

