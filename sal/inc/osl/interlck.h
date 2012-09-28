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

#ifndef _OSL_INTERLOCK_H_
#define _OSL_INTERLOCK_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef sal_Int32 oslInterlockedCount;

/** Increments the count variable addressed by pCount.
    @param pCount Address of count variable
    @return The adjusted value of the count variable.
*/
SAL_DLLPUBLIC oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount);

/** Decrement the count variable addressed by pCount.
    @param pCount Address of count variable
    @return The adjusted value of the count variable.
*/
SAL_DLLPUBLIC oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount);


/// @cond INTERNAL

/** Increments the count variable addressed by p.

    @attention This functionality should only be used internally within
    LibreOffice.

    @param p Address of count variable
    @return The adjusted value of the count variable.

    @since LibreOffice 3.7
*/
#if defined( HAVE_GCC_BUILTIN_ATOMIC )
#    define osl_atomic_increment(p)  __sync_add_and_fetch((p), 1)
#elif defined( _MSC_VER )
#    define osl_atomic_increment(p) _InterlockedIncrement((p))
#else
#    define osl_atomic_increment(p) osl_incrementInterlockedCount((p))
#endif


/** Decrement the count variable addressed by p.

    @attention This functionality should only be used internally within
    LibreOffice.

    @param p Address of count variable
    @return The adjusted value of the count variable.

    @since LibreOffice 3.7
*/
#if defined( HAVE_GCC_BUILTIN_ATOMIC )
#    define osl_atomic_decrement(p) __sync_sub_and_fetch((p), 1)
#elif defined( _MSC_VER )
#    define osl_atomic_decrement(p) _InterlockedDecrement((p))
#else
#    define osl_atomic_decrement(p) osl_decrementInterlockedCount((p))
#endif

/// @endcond

#ifdef __cplusplus
}
#endif


#endif  /* _OSL_INTERLOCK_H_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
