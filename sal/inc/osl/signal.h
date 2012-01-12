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



#ifndef _OSL_SIGNAL_H_
#define _OSL_SIGNAL_H_

#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OSL_SIGNAL_USER_RESERVED    0

#define OSL_SIGNAL_USER_RESOURCEFAILURE     (OSL_SIGNAL_USER_RESERVED - 1)
#define OSL_SIGNAL_USER_X11SUBSYSTEMERROR   (OSL_SIGNAL_USER_RESERVED - 2)
#define OSL_SIGNAL_USER_RVPCONNECTIONERROR  (OSL_SIGNAL_USER_RESERVED - 3)

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
#elif defined(SAL_OS2)
#   pragma pack(push, 4)
#endif

typedef struct
{
    oslSignal   Signal;
    sal_Int32   UserSignal;
    void*       UserData;
} oslSignalInfo;

#if defined( SAL_W32) ||  defined(SAL_OS2)
#   pragma pack(pop)
#endif

/** the function-ptr. representing the signal handler-function.
*/
typedef oslSignalAction (SAL_CALL *oslSignalHandlerFunction)(void* pData, oslSignalInfo* pInfo);

oslSignalHandler SAL_CALL osl_addSignalHandler(oslSignalHandlerFunction Handler, void* pData);

sal_Bool SAL_CALL osl_removeSignalHandler(oslSignalHandler hHandler);

oslSignalAction SAL_CALL osl_raiseSignal(sal_Int32 UserSignal, void* UserData);

/** Enables or disables error reporting

    On default error reporting is enabled after process startup.

    @param  bEnable [in]
    Enables or disables error reporting.

    @return
    sal_True if previous state of error reporting was enabled<br>
    sal_False if previous state of error reporting was disbaled<br>
*/

sal_Bool SAL_CALL osl_setErrorReporting( sal_Bool bEnable );

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_SIGNAL_H_ */


