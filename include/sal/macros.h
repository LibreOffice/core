/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SAL_MACROS_H
#define INCLUDED_SAL_MACROS_H

#include <stddef.h>

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

#ifndef SAL_STRINGIFY
#   define SAL_STRINGIFY_ARG(x) #x
#   define SAL_STRINGIFY(x) SAL_STRINGIFY_ARG(x)
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
