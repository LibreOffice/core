/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SAL_MAIN_H_
#define _SAL_MAIN_H_

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void SAL_CALL sal_detail_initialize(int argc, char ** argv);
void SAL_CALL sal_detail_deinitialize();

#ifdef SAL_OS2
#include <string.h>

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSMODULEMGR
#include <os2.h>
#define INCL_LOADEXCEPTQ
#include <exceptq.h>

#include <osl/process.h>

#define sal_detail_initialize(a,b) \
    EXCEPTIONREGISTRATIONRECORD exRegRec = {0}; \
    LoadExceptq(&exRegRec, NULL, NULL); \
    osl_setCommandArgs(argc, argv);
#define sal_detail_deinitialize() \
    UninstallExceptq(&exRegRec);
#endif // SAL_OS2

#define SAL_MAIN_WITH_ARGS_IMPL \
int SAL_CALL main(int argc, char ** argv) \
{ \
    int ret; \
    sal_detail_initialize(argc, argv);   \
    ret = sal_main_with_args(argc, argv); \
    sal_detail_deinitialize(); \
    return ret; \
}

#define SAL_MAIN_IMPL \
int SAL_CALL main(int argc, char ** argv) \
{ \
    int ret; \
    sal_detail_initialize(argc, argv); \
    ret = sal_main(); \
    sal_detail_deinitialize(); \
    return ret; \
}


/* Definition macros for CRT entries */

#ifdef SAL_W32

#ifndef INCLUDED_STDLIB_H
#include <stdlib.h>
#define INCLUDED_STDLIB_H
#endif

/* Sorry but this is neccessary cause HINSTANCE is a typedef that differs (C++ causes an error) */

#if 0

#ifndef _WINDOWS_
#include <windows.h>
#endif

#else /* Simulated what windows.h does */

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

#endif

#define SAL_WIN_WinMain \
int WINAPI WinMain( HINSTANCE _hinst, HINSTANCE _dummy, char* _cmdline, int _nshow ) \
{ \
    int argc = __argc; char ** argv = __argv; \
    (void) _hinst; (void) _dummy; (void) _cmdline; (void) _nshow; /* unused */ \
    return main(argc, argv); \
}

#else   /* ! SAL_W32 */

# define SAL_WIN_WinMain

#endif /* ! SAL_W32 */

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

#endif  /* _SAL_MAIN_H_ */

