/*************************************************************************
 *
 *  $RCSfile: mutex.h,v $
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

#ifndef _OSL_MUTEX_H_
#define _OSL_MUTEX_H_

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct _oslMutexImpl;
typedef struct _oslMutexImpl * oslMutex;

/** Create a thread-local mutex.
    @return 0 if the mutex could not be created, otherwise a handle to the mutex.
*/
oslMutex SAL_CALL osl_createMutex(void);

/** Release the OS-structures and free mutex data-structure.
    @param Mutex the mutex-handle
*/
void SAL_CALL osl_destroyMutex(oslMutex Mutex);

/** Acquire the mutex, block if already acquired by another thread.
    @param Mutex handle to a created mutex.
    @return False if system-call fails.
*/
sal_Bool SAL_CALL osl_acquireMutex(oslMutex Mutex);

/** Try to acquire the mutex without blocking.
    @param Mutex handle to a created mutex.
    @return False if it could not be acquired.
*/
sal_Bool SAL_CALL osl_tryToAcquireMutex(oslMutex Mutex);

/** Release the mutex.
    @param Mutex handle to a created mutex.
    @return False if system-call fails.
*/
sal_Bool SAL_CALL osl_releaseMutex(oslMutex Mutex);

oslMutex * SAL_CALL osl_getGlobalMutex();

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_MUTEX_H_ */

/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.6  2000/09/18 14:28:49  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.5  2000/05/17 14:30:14  mfe
*    comments upgraded
*
*    Revision 1.4  1999/12/08 10:40:52  jbu
*    #70413# changed oslMutex forward struct
*
*    Revision 1.3  1999/10/27 15:02:07  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.2  1999/09/28 13:04:31  kr
*    new: getGlobalMutex
*
*    Revision 1.1  1999/08/05 10:18:19  jsc
*    verschoben aus osl
*
*    Revision 1.4  1999/08/05 11:14:36  jsc
*    verschoben in root inc-Verzeichnis
*
*    Revision 1.3  1999/01/20 18:53:40  jsc
*    #61011# Typumstellung
*
*    Revision 1.2  1998/07/04 14:56:57  rh
*    Prototype for OS/2
*
*    Revision 1.1  1998/02/16 19:34:51  rh
*    Cleanup of ports, integration of Size_t, features for process
*
*************************************************************************/
