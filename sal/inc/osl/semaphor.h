/*************************************************************************
 *
 *  $RCSfile: semaphor.h,v $
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

#ifndef _OSL_SEMAPHORE_H_
#define _OSL_SEMAPHORE_H_

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* oslSemaphore;

/** Creates a semaphore.<BR>
    @param InitialCount denotes the starting value the semaphore. If you set it to
    zero, the first acquire() blocks. Otherwise InitialCount acquire()s  are
    immedeatly  successfull.
    @return 0 if the semaphore could not be created, otherwise a handle to the sem.
*/
oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount);

/** Release the OS-structures and free semaphore data-structure
    @return fbbb
*/
void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore);

/** acquire() decreases the count. It will block if it tries to
    decrease below zero.
    @return False if the system-call failed.
*/
sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore);

/** tryToAcquire() tries to decreases the count. It will
    return with False if it would decrease the count below zero.
    (When acquire() would block.) If it could successfully
    decrease the count, it will return True.
*/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore);

/** release() increases the count.
    @return False if the system-call failed.
*/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_SEMAPHORE_H_  */

/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.4  2000/09/18 14:28:49  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.3  2000/05/17 14:50:15  mfe
*    comments upgraded
*
*    Revision 1.2  1999/10/27 15:02:08  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.1  1999/08/05 10:18:20  jsc
*    verschoben aus osl
*
*    Revision 1.6  1999/08/05 11:14:41  jsc
*    verschoben in root inc-Verzeichnis
*
*    Revision 1.5  1999/01/20 18:53:41  jsc
*    #61011# Typumstellung
*
*    Revision 1.4  1998/02/16 19:34:52  rh
*    Cleanup of ports, integration of Size_t, features for process
*
*    Revision 1.3  1997/07/31 15:28:40  ts
*    *** empty log message ***
*
*    Revision 1.2  1997/07/17 11:02:28  rh
*    Header adapted and profile added
*
*    Revision 1.1  1997/06/19 13:10:10  bho
*    first version of OSL.
*
*************************************************************************/

