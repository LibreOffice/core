/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
#pragma warning(push,1) /* disable warnings within system headers */
#include <windows.h>
#pragma warning(pop)

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
