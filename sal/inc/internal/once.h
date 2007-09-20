/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: once.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 15:05:29 $
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

#ifndef INCLUDED_SAL_INTERNAL_ONCE_H
#define INCLUDED_SAL_INTERNAL_ONCE_H

/** sal_once_type
 *  (platform dependent)
 */

#if defined(SAL_UNX) || defined(SAL_OS2)

#include <pthread.h>

typedef pthread_once_t sal_once_type;

#define SAL_ONCE_INIT PTHREAD_ONCE_INIT
#define SAL_ONCE(once, init) pthread_once((once), (init))

#elif defined(SAL_W32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct sal_once_st sal_once_type;
struct sal_once_st
{
    LONG volatile m_done;
    LONG volatile m_lock;
};

#define SAL_ONCE_INIT { 0, 0 }
#define SAL_ONCE(once, init) \
{ \
    sal_once_type * control = (once); \
    if (!(control->m_done)) \
    { \
        while (InterlockedExchange(&(control->m_lock), 1) == 1) Sleep(0); \
        if (!(control->m_done)) \
        { \
            void (*init_routine)(void) = (init); \
            (*init_routine)(); \
            control->m_done = 1; \
        } \
        InterlockedExchange(&(control->m_lock), 0); \
    } \
}

#else
#error Unknown platform
#endif /* SAL_UNX | SAL_W32 */

#endif /* INCLUDED_SAL_INTERNAL_ONCE_H */
