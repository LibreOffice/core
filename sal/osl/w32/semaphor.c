/*************************************************************************
 *
 *  $RCSfile: semaphor.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:23 $
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

#include "system.h"

#include <osl/diagnose.h>
#include <osl/semaphor.h>

/*
    Implemetation notes:
    The void* represented by oslSemaphore is used
    to store a WIN32 HANDLE.
*/


/*****************************************************************************/
/* osl_createSemaphore */
/*****************************************************************************/
oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount)
{
    oslSemaphore Semaphore;

    Semaphore= CreateSemaphore(0, initialCount, INT_MAX, 0);

    /* create failed? */
    if((HANDLE)Semaphore == INVALID_HANDLE_VALUE)
    {
        Semaphore= 0;
    }

    return Semaphore;
}

/*****************************************************************************/
/* osl_destroySemaphore */
/*****************************************************************************/
void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore)
{


    if(Semaphore != 0)
    {
        CloseHandle((HANDLE)Semaphore);
    }

}

/*****************************************************************************/
/* osl_acquireSemaphore */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore)
{
    OSL_ASSERT(Semaphore != 0);

    while(1)
    {
        switch (MsgWaitForMultipleObjects(1, &((HANDLE)Semaphore),
                                             FALSE, INFINITE, QS_SENDMESSAGE))
        {
            case WAIT_OBJECT_0:
                return sal_True;

            case WAIT_OBJECT_0 + 1:
            {
                MSG msg;

                  while (PeekMessage(&msg, NULL, WM_USER - 1, WM_USER - 1, PM_REMOVE))
                   {
                    TranslateMessage(&msg);
                      DispatchMessage(&msg);
                   }
                break;
            }

            default:
                return (sal_False);
        }
    }
}

/*****************************************************************************/
/* osl_tryToAcquireSemaphore */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore)
{
    OSL_ASSERT(Semaphore != 0);
    return (WaitForSingleObject((HANDLE)Semaphore, 0) == WAIT_OBJECT_0);
}


/*****************************************************************************/
/* osl_releaseSemaphore */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore)
{
    OSL_ASSERT(Semaphore != 0);

    return (ReleaseSemaphore((HANDLE)Semaphore,
                             1,                 /* increase count by one */
                             NULL));            /* not interested in previous count */
}



/*************************************************************************
*
*     $Log: not supported by cvs2svn $
*     Revision 1.10  2000/09/18 14:29:02  willem.vandorp
*     OpenOffice header added.
*
*     Revision 1.9  1999/10/27 15:13:36  mfe
*     Change of Copyright, removed compiler warnings, code clean up, ...
*
*     Revision 1.8  1999/10/13 10:42:54  rt
*     undo last changes
*
*     Revision 1.7  1999/10/11 12:47:09  rt
*     gcc for windows: syntax change
*
*     Revision 1.6  1999/01/20 19:09:50  jsc
*     #61011# Typumstellung
*
*     Revision 1.5  1998/07/16 10:41:02  rh
*     Beautify the source
*
*     Revision 1.4  1998/07/16 10:25:15  rh
*     Change WaitForObject to MsgWaitForObject to process Messages while waiting
*
*     Revision 1.3  1997/07/15 19:02:04  rh
*     system.h inserted
*
*     Revision 1.2  1997/07/02 10:54:01  rh
*     Anpassung fuer Diagnose Macros
*
*     Revision 1.1  1997/06/19 13:10:43  bho
*     first version of OSL.
*
*************************************************************************/
