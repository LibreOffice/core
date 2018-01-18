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

#ifndef INCLUDED_SAL_MAIN_H
#define INCLUDED_SAL_MAIN_H

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#if defined AIX
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

SAL_DLLPUBLIC void SAL_CALL sal_detail_initialize(int argc, char ** argv);
SAL_DLLPUBLIC void SAL_CALL sal_detail_deinitialize(void);

#if defined IOS || defined ANDROID

#error No code that includes this should be built for iOS or Android

#else

#define SAL_MAIN_WITH_ARGS_IMPL \
int SAL_DLLPUBLIC_EXPORT SAL_CALL main(int argc, char ** argv) \
{ \
    int ret; \
    sal_detail_initialize(argc, argv);   \
    ret = sal_main_with_args(argc, argv); \
    sal_detail_deinitialize(); \
    return ret; \
}

#define SAL_MAIN_IMPL \
int SAL_DLLPUBLIC_EXPORT SAL_CALL main(int argc, char ** argv) \
{ \
    int ret; \
    sal_detail_initialize(argc, argv); \
    ret = sal_main(); \
    sal_detail_deinitialize(); \
    return ret; \
}

#endif


/* Definition macros for CRT entries */

#ifdef _WIN32

#include <stdlib.h>

/* Sorry but this is necessary cause HINSTANCE is a typedef that differs (C++ causes an error) */

#ifndef WINAPI
#   define WINAPI   __stdcall
#endif

#if !defined(DECLARE_HANDLE)
#   ifdef STRICT
        typedef void *HANDLE;
#       define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#   else
        typedef void *PVOID;
        typedef PVOID HANDLE;
#       define DECLARE_HANDLE(name) typedef HANDLE name
#   endif
DECLARE_HANDLE(HINSTANCE);
#endif



#define SAL_WIN_WinMain \
int WINAPI WinMain( HINSTANCE _hinst, HINSTANCE _dummy, char* _cmdline, int _nshow ) \
{ \
    int argc = __argc; char ** argv = __argv; \
    (void) _hinst; (void) _dummy; (void) _cmdline; (void) _nshow; /* unused */ \
    return main(argc, argv); \
}

#else   /* ! _WIN32 */

# define SAL_WIN_WinMain

#endif /* ! _WIN32 */

/* Implementation macro */

#define SAL_IMPLEMENT_MAIN_WITH_ARGS(_argc_, _argv_) \
    static int  SAL_CALL sal_main_with_args (int _argc_, char ** _argv_); \
    SAL_MAIN_WITH_ARGS_IMPL \
    SAL_WIN_WinMain \
    static int SAL_CALL sal_main_with_args(int _argc_, char ** _argv_)

#define SAL_IMPLEMENT_MAIN() \
    static int  SAL_CALL sal_main(void); \
    SAL_MAIN_IMPL \
    SAL_WIN_WinMain \
    static int SAL_CALL sal_main(void)

/*
    "How to use" Examples:

    #include <sal/main.h>

    SAL_IMPLEMENT_MAIN()
    {
        DoSomething();

        return 0;
    }

    SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
    {
        DoSomethingWithArgs(argc, argv);

        return 0;
    }

*/

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif // INCLUDED_SAL_MAIN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
