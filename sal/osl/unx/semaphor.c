/*************************************************************************
 *
 *  $RCSfile: semaphor.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:21 $
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

#include <osl/semaphor.h>
#include <osl/diagnose.h>

#ifndef OSL_USE_SYS_V_SEMAPHORE

/* This is the (default) POSIX thread-local semaphore variant */

/*
    Implemetation notes:
    The void* represented by oslSemaphore is used
    as a pointer to an sem_t struct
*/

/*****************************************************************************/
/* osl_createSemaphore  */
/*****************************************************************************/

#ifdef LINUX
/* sem_t from glibc-2.1 (/usr/include/semaphore.h) */

typedef struct {
    struct { long status; int spinlock; } sem_lock;
    int sem_value;
    _pthread_descr sem_waiting;
} glibc_21_sem_t;
#endif

oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount)
{
    int ret = 0;
    oslSemaphore Semaphore;

    /*
     *  XXX this a hack of course. since sizeof(sem_t) changed
     * from glibc-2.0.7 to glibc-2.1.x, we have to allocate the
     * larger of both
     * XXX
     */
#ifdef LINUX
    if ( sizeof(glibc_21_sem_t) > sizeof(sem_t) )
        Semaphore= malloc(sizeof(glibc_21_sem_t));
    else
#endif
        Semaphore= malloc(sizeof(sem_t));

    OSL_ASSERT(Semaphore);      /* ptr valid? */

    if ( Semaphore == 0 )
    {
        return 0;
    }

    /* unnamed semaphore, not shared between processes */

       ret= sem_init((sem_t*)Semaphore, 0, initialCount);

    /* create failed? */
    if (ret != 0)
    {
        OSL_TRACE("osl_createSemaphore failed. Errno: %d; %s\n",
                  errno,
                  strerror(errno));

        free(Semaphore);
        Semaphore = NULL;
    }

    return Semaphore;
}

/*****************************************************************************/
/* osl_destroySemaphore  */
/*****************************************************************************/
void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore)
{
    if(Semaphore)           /* ptr valid? */
    {
        sem_destroy((sem_t*)Semaphore);
        free(Semaphore);
    }
}

/*****************************************************************************/
/* osl_acquireSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore) {

    OSL_ASSERT(Semaphore != 0); /* abort in debug mode */

    if (Semaphore != 0)     /* be tolerant in release mode */
    {
        return (sem_wait((sem_t*)Semaphore) == 0);
    }

    return sal_False;
}

/*****************************************************************************/
/* osl_tryToAcquireSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore) {

    OSL_ASSERT(Semaphore != 0); /* abort in debug mode */
    if (Semaphore != 0)     /* be tolerant in release mode */
    {
        return (sem_trywait((sem_t*)Semaphore) == 0);
    }

    return sal_False;
}

/*****************************************************************************/
/* osl_releaseSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore) {

    OSL_ASSERT(Semaphore != 0);     /* abort in debug mode */

    if (Semaphore != 0)         /* be tolerant in release mode */
    {
        return (sem_post((sem_t*)Semaphore) == 0);
    }

    return sal_False;
}

#else /* OSL_USE_SYS_V_SEMAPHORE */

/*******************************************************************************/

/* This is the SYS V private semaphore variant */

/*
    Implemetation notes:
    The void* represented by oslSemaphore is used
    as a pointer to an osl_TSemImpl struct
*/

typedef struct _osl_TSemImpl
{
  int m_Id;

} osl_TSemImpl;

/*****************************************************************************/
/* osl_createSemaphore  */
/*****************************************************************************/
oslSemaphore SAL_CALL osl_createSemaphore(sal_uInt32 initialCount)
{
    union semun arg;

    oslSemaphore Semaphore;
    osl_TSemImpl* pSem;

    Semaphore= malloc(sizeof(osl_TSemImpl));
    OSL_POSTCOND(Semaphore, "malloc failed\n");     /* ptr valid? */

    pSem= (osl_TSemImpl*)Semaphore;


    /* unnamed (private) semaphore */

       pSem->m_Id= semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);


    /* create failed? */
    if (pSem->m_Id < 0)
    {
        OSL_TRACE("osl_createSemaphore failed (semget). Errno: %d; %s\n",
               errno,
               strerror(errno));

        free(Semaphore);
        return 0;
    }

    /* set initial count */

    arg.val= initialCount;

    if(semctl(pSem->m_Id, 0, SETVAL, arg) < 0)
    {
        OSL_TRACE("osl_createSemaphore failed (semctl(SETVAL)). Errno: %d; %s\n",
               errno,
               strerror(errno));

        if(semctl(pSem->m_Id, 0, IPC_RMID, arg) < 0)
        {
            OSL_TRACE("semctl(IPC_RMID) failed. Errno: %d; %s\n", errno, strerror(errno));
        }

        free(Semaphore);
        return 0;
    }


    return Semaphore;
}

/*****************************************************************************/
/* osl_destroySemaphore  */
/*****************************************************************************/
void SAL_CALL osl_destroySemaphore(oslSemaphore Semaphore) {

    if(Semaphore)           /* ptr valid? */
    {
        union semun arg;

        osl_TSemImpl* pSem= (osl_TSemImpl*)Semaphore;

        if(semctl(pSem->m_Id, 0, IPC_RMID, arg) < 0)

        {
            OSL_TRACE("osl_destroySemaphore failed. (semctl(IPC_RMID)). Errno: %d; %s\n",
                   errno,
                   strerror(errno));
        }

        free(Semaphore);
    }
}

/*****************************************************************************/
/* osl_acquireSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_acquireSemaphore(oslSemaphore Semaphore) {

    /* abort in debug mode */
    OSL_PRECOND(Semaphore != 0, "Semaphore not created\n");


    if (Semaphore != 0)     /* be tolerant in release mode */
    {
        struct sembuf op;
        osl_TSemImpl* pSem= (osl_TSemImpl*)Semaphore;

        op.sem_num= 0;
        op.sem_op= -1;
        op.sem_flg= SEM_UNDO;

        return semop(pSem->m_Id, &op, 1) >= 0;

    }

    return sal_False;
}

/*****************************************************************************/
/* osl_tryToAcquireSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_tryToAcquireSemaphore(oslSemaphore Semaphore) {

    /* abort in debug mode */
    OSL_PRECOND(Semaphore != 0, "Semaphore not created\n");

    if (Semaphore != 0)     /* be tolerant in release mode */
    {
        struct sembuf op;
        osl_TSemImpl* pSem= (osl_TSemImpl*)Semaphore;

        op.sem_num= 0;
        op.sem_op= -1;
        op.sem_flg= SEM_UNDO | IPC_NOWAIT;

        return semop(pSem->m_Id, &op, 1) >= 0;
    }

    return sal_False;
}

/*****************************************************************************/
/* osl_releaseSemaphore  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_releaseSemaphore(oslSemaphore Semaphore)
{

    /* abort in debug mode */
    OSL_PRECOND(Semaphore != 0, "Semaphore not created\n");

    if (Semaphore != 0)         /* be tolerant in release mode */
    {
        struct sembuf op;
        osl_TSemImpl* pSem= (osl_TSemImpl*)Semaphore;

        op.sem_num= 0;
        op.sem_op= 1;
        op.sem_flg= SEM_UNDO;

        return semop(pSem->m_Id, &op, 1) >= 0;
    }

    return sal_False;
}

#endif /* OSL_USE_SYS_V_SEMAPHORE */

