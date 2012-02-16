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

/* make Python.h go first as a hack to work around _POSIX_C_SOURCE redefinition
   warnings: */
#include "Python.h"

#include "sal/config.h"

#include <stdlib.h>
#include <string.h>

#if defined LINUX && !defined __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>

#include "rtl/string.h"

/* A wrapper around libpyuno.so, making sure the latter is loaded RTLD_GLOBAL
   so that C++ exception handling works with old GCC versions (that determine
   RTTI identity by comparing string addresses rather than string content).
*/

static void * load(void * address, char const * symbol) {
    Dl_info dl_info;
    char * slash;
    size_t len;
    char * libname;
    void * h;
    void * func;
    if (dladdr(address, &dl_info) == 0) {
        abort();
    }
    slash = strrchr(dl_info.dli_fname, '/');
    if (slash == NULL) {
        abort();
    }
    len = slash - dl_info.dli_fname + 1;
    libname = malloc(
        len + RTL_CONSTASCII_LENGTH(SAL_DLLPREFIX "pyuno" SAL_DLLEXTENSION)
        + 1);
    if (libname == 0) {
        abort();
    }
    strncpy(libname, dl_info.dli_fname, len);
    strcpy(libname + len, SAL_DLLPREFIX "pyuno" SAL_DLLEXTENSION);
    h = dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
    free(libname);
    if (h == NULL) {
        abort();
    }
    func = dlsym(h, symbol);
    if (func == NULL) {
        abort();
    }
    return func;
}

#if PY_MAJOR_VERSION >= 3

PyObject * PyInit_pyuno(void) {
    return
        ((PyObject * (*)(void)) load((void *) &PyInit_pyuno, "PyInit_pyuno"))();
}

#else

void initpyuno(void) {
    ((void (*)(void)) load((void *) &initpyuno, "initpyuno"))();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
