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

#ifndef _SAL_MACROS_H_
#define _SAL_MACROS_H_

#include <stddef.h>

/* we want to be able to compile release code while retaining the abort capabilities of assert().
 * This presume that this include is called early... i.e before anyone else included assert.h
 * which should more often than not be the case since macros.h is included by sal/types.h
 * which is included at the top of most source (or should be)
 */
#ifdef ASSERT_ALWAYS_ABORT
#    ifdef NDEBUG
#        undef NDEBUG
#        include <assert.h>
#        define NDEBUG
#    else /* Ndef NDEBUG */
#        include <assert.h>
#    endif /* Ndef NDEBUG */
#else /* NDef ASSERT_ALWAYS_ABORT */
#        include <assert.h>
#endif /* NDef ASSERT_ALWAYS_ABORT */

#ifndef SAL_N_ELEMENTS
#    if defined(__cplusplus) && ( defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L )
        /*
         * Magic template to calculate at compile time the number of elements
         * in an array. Enforcing that the argument must be a array and not
         * a pointer, e.g.
         *  char *pFoo="foo";
         *  SAL_N_ELEMENTS(pFoo);
         * fails while
         *  SAL_N_ELEMENTS("foo");
         * or
         *  char aFoo[]="foo";
         *  SAL_N_ELEMENTS(aFoo);
         * pass
         *
         * Unfortunately if arr is an array of an anonymous class then we need
         * C++0x, i.e. see
         * http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#757
         */
         template <typename T, size_t S> char (&sal_n_array_size( T(&)[S] ))[S];
#        define SAL_N_ELEMENTS(arr)     (sizeof(sal_n_array_size(arr)))
#    else
#        define SAL_N_ELEMENTS(arr)     (sizeof (arr) / sizeof ((arr)[0]))
#    endif
#endif

#ifndef SAL_BOUND
#    define SAL_BOUND(x,l,h)        ((x) <= (l) ? (l) : ((x) >= (h) ? (h) : (x)))
#endif

#ifndef SAL_ABS
#    define SAL_ABS(a)              (((a) < 0) ? (-(a)) : (a))
#endif

#ifndef SAL_STRINGIFY
#   define SAL_STRINGIFY_ARG(x) #x
#   define SAL_STRINGIFY(x) SAL_STRINGIFY_ARG(x)
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
