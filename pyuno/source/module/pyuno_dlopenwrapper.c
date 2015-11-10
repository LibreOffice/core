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
    if (libname == NULL) {
        abort();
    }
    strncpy(libname, dl_info.dli_fname, len);
    strcpy(libname + len, SAL_DLLPREFIX "pyuno" SAL_DLLEXTENSION);
    h = dlopen(libname, RTLD_LAZY | RTLD_GLOBAL);
    free(libname);
    if (h == NULL) {
        fprintf(stderr, "failed to load pyuno: '%s'\n", dlerror());
        abort();
    }
    func = dlsym(h, symbol);
    if (func == NULL) {
        dlclose(h);
        abort();
    }
    return func;
}

#if PY_MAJOR_VERSION >= 3

SAL_DLLPUBLIC_EXPORT PyObject * PyInit_pyuno(void) {
    return
        ((PyObject * (*)(void)) load((void *) &PyInit_pyuno, "PyInit_pyuno"))();
}

#else

SAL_DLLPUBLIC_EXPORT void initpyuno(void) {
    ((void (*)(void)) load((void *) &initpyuno, "initpyuno"))();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
