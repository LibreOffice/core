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

#ifndef _SAL_MAIN_H_
#define _SAL_MAIN_H_

#include <sal/types.h>
#if defined(AIX)
#   include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void SAL_CALL sal_detail_initialize(int argc, char ** argv);
void SAL_CALL sal_detail_deinitialize();

#ifdef IOS

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

#define SAL_MAIN_WITH_ARGS_IMPL \
int SAL_CALL main(int argc, char ** argv) \
{ \
    sal_detail_initialize(argc, argv);   \
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; \
    int retVal = UIApplicationMain (argc, argv, @"UIApplication", @"salAppDelegate"); \
    [pool release]; \
    sal_detail_deinitialize(); \
    return retVal; \
} \
 \
static int sal_main_with_args(int argc, char **argv); \
 \
static int \
sal_main(void) \
{ \
    char *argv[] = { NULL }; \
    return sal_main_with_args(0, argv); \
}

#define SAL_MAIN_IMPL \
int SAL_CALL main(int argc, char ** argv) \
{ \
    sal_detail_initialize(argc, argv);   \
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; \
    int retVal = UIApplicationMain (argc, argv, @"UIApplication", @"salAppDelegate"); \
    [pool release]; \
    sal_detail_deinitialize(); \
    return retVal; \
}

#define SAL_MAIN_WITH_GUI_IMPL \
int SAL_CALL main(int argc, char ** argv) \
{ \
    sal_detail_initialize(argc, argv);   \
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; \
    int retVal = sal_main(); \
    [pool release]; \
    sal_detail_deinitialize(); \
    return retVal; \
}

@interface salAppDelegate : NSObject <UIApplicationDelegate> {
}
@property (nonatomic, retain) UIWindow *window;
@end

static int sal_main(void);

@implementation salAppDelegate

@synthesize window=_window;

- (BOOL)application: (UIApplication *) application didFinishLaunchingWithOptions: (NSDictionary *) launchOptions
{
  (void) application;
  (void) launchOptions;
  UIWindow *uiw = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  uiw.backgroundColor = [UIColor redColor];
  self.window = uiw;
  [uiw release];

  sal_main();

  [self.window makeKeyAndVisible];
  return YES;
}

@end

#else

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

#define SAL_MAIN_WITH_GUI_IMPL SAL_MAIN_IMPL

#endif


/* Definition macros for CRT entries */

#ifdef SAL_W32

#ifndef INCLUDED_STDLIB_H
#include <stdlib.h>
#define INCLUDED_STDLIB_H
#endif

/* Sorry but this is neccessary cause HINSTANCE is a typedef that differs (C++ causes an error) */

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

/* Use SAL_IMPLEMENT_MAIN_WITH_GUI in programs that actually have a
 * VCL GUI. The difference is meaningful only for iOS support, which
 * of course is a highly experimental work in progress. So actually,
 * don't bother, just let developers who care for iOS take care of it
 * when/if necessary.
 */

#define SAL_IMPLEMENT_MAIN_WITH_GUI() \
    static int  SAL_CALL sal_main(void); \
    SAL_MAIN_WITH_GUI_IMPL \
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
