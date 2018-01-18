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

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

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

#ifdef _WIN32
#   pragma pack(push, 8)
#endif

typedef struct
{
    oslSignal   Signal;
    sal_Int32   UserSignal;
    void*       UserData;
} oslSignalInfo;

#if defined( _WIN32)
#   pragma pack(pop)
#endif

/** The function-ptr representing the signal handler-function.
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

    @param[in]  bEnable Enables or disables error reporting.

    @retval sal_True if previous state of error reporting was enabled
    @retval sal_False if previous state of error reporting was disabled
*/

SAL_DLLPUBLIC sal_Bool SAL_CALL osl_setErrorReporting(
        sal_Bool bEnable );

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_SIGNAL_H


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
