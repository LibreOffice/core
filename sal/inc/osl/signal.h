/*************************************************************************
 *
 *  $RCSfile: signal.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:13 $
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

#ifndef _OSL_SIGNAL_H_
#define _OSL_SIGNAL_H_

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
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


#ifdef __cplusplus
}
#endif

#endif  /* _OSL_SIGNAL_H_ */


/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.8  2000/09/18 14:28:49  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.7  2000/08/11 16:46:28  pl
*    #77400# #76899# corrected define
*
*    Revision 1.6  2000/08/11 16:31:40  pl
*    #77400# add OSL_SIGNAL_USER_RVPCONNECTIONERROR
*
*    Revision 1.5  2000/05/29 16:45:16  hro
*    SRC591: Explicite SAL_CALL calling convention
*
*    Revision 1.4  2000/05/17 14:50:15  mfe
*    comments upgraded
*
*    Revision 1.3  1999/12/22 13:37:55  mfe
*    #71232# : added Alarm Signal Type
*
*    Revision 1.2  1999/10/27 15:02:08  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.1  1999/08/05 10:18:20  jsc
*    verschoben aus osl
*
*    Revision 1.5  1999/08/05 11:14:42  jsc
*    verschoben in root inc-Verzeichnis
*
*    Revision 1.4  1999/04/22 07:54:04  rh
*    #63033# Deliver breakpoints via signal TSignal_DebugBreak
*
*    Revision 1.3  1999/01/20 18:53:41  jsc
*    #61011# Typumstellung
*
*    Revision 1.2  1998/03/13 15:07:32  rh
*    Cleanup of enum chaos and implemntation of pipes
*
*    Revision 1.1  1998/03/06 15:42:09  rh
*    Added signal handling
*
*************************************************************************/
