/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: signal.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:13:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OSL_SIGNAL_H_
#define _OSL_SIGNAL_H_

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

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
#   pragma pack(1)
#endif

typedef struct
{
    oslSignal   Signal;
    sal_Int32   UserSignal;
    void*       UserData;
} oslSignalInfo;

#ifdef SAL_W32
#   pragma pack(pop)
#elif defined(SAL_OS2)
#   pragma pack()
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


