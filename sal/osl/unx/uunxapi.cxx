/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <config_features.h>

#include "uunxapi.h"
#include "system.h"
#include <limits.h>
#include <rtl/ustring.hxx>
#include <osl/thread.h>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

inline rtl::OString OUStringToOString(const rtl_uString* s)
{
    return rtl::OUStringToOString(rtl::OUString(const_cast<rtl_uString*>(s)),
                                  osl_getThreadTextEncoding());
}

#if defined(MACOSX) && MAC_OS_X_VERSION_MIN_REQUIRED >= 1070 && HAVE_FEATURE_MACOSX_SANDBOX

static NSUserDefaults *userDefaults = NULL;

static void get_user_defaults()
{
    userDefaults = [NSUserDefaults standardUserDefaults];
}

typedef struct {
    NSURL *scopeURL;
    NSAutoreleasePool *pool;
} accessFilePathState;

static accessFilePathState *
prepare_to_access_file_path( const char *cpFilePath )
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, &get_user_defaults);
    NSURL *fileURL = nil;
    NSData *data = nil;
    BOOL stale;
    accessFilePathState *state;

    // If malloc() fails we are screwed anyway
    state = (accessFilePathState*) malloc(sizeof(accessFilePathState));

    state->pool = [[NSAutoreleasePool alloc] init];
    state->scopeURL = nil;

    if (userDefaults != nil)
        fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:cpFilePath]];

    if (fileURL != nil)
        data = [userDefaults dataForKey:[@"bookmarkFor:" stringByAppendingString:[fileURL absoluteString]]];

    if (data != nil)
        state->scopeURL = [NSURL URLByResolvingBookmarkData:data
                                                    options:NSURLBookmarkResolutionWithSecurityScope
                                              relativeToURL:nil
                                        bookmarkDataIsStale:&stale
                                                      error:nil];
    if (state->scopeURL != nil)
        [state->scopeURL startAccessingSecurityScopedResource];

    return state;
}

static void
done_accessing_file_path( const char * /*cpFilePath*/, accessFilePathState *state )
{
    int saved_errno = errno;

    if (state->scopeURL != nil)
        [state->scopeURL stopAccessingSecurityScopedResource];
    [state->pool release];
    free(state);

    errno = saved_errno;
}

#else

typedef void accessFilePathState;

#define prepare_to_access_file_path( cpFilePath ) NULL

#define done_accessing_file_path( cpFilePath, state ) ((void) cpFilePath, (void) state)

#endif

#ifdef MACOSX
/*
 * Helper function for resolving Mac native alias files (not the same as unix alias files)
 * and to return the resolved alias as rtl::OString
 */
static rtl::OString macxp_resolveAliasAndConvert(rtl::OString p)
{
    sal_Char path[PATH_MAX];
    if (p.getLength() < PATH_MAX)
    {
        strcpy(path, p.getStr());
        macxp_resolveAlias(path, PATH_MAX);
        p = rtl::OString(path);
    }
    return p;
}
#endif /* MACOSX */

int access_u(const rtl_uString* pustrPath, int mode)
{
    rtl::OString fn = OUStringToOString(pustrPath);
#ifdef ANDROID
    if (strncmp(fn.getStr(), "/assets", sizeof("/assets")-1) == 0 &&
        (fn.getStr()[sizeof("/assets")-1] == '\0' ||
         fn.getStr()[sizeof("/assets")-1] == '/'))
    {
        struct stat stat;
        if (lo_apk_lstat(fn.getStr(), &stat) == -1)
            return -1;
        if (mode & W_OK)
        {
            errno = EACCES;
            return -1;
        }
        return 0;
    }
#endif

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

    int result = access(fn.getStr(), mode);

    done_accessing_file_path(fn.getStr(), state);

    return result;
}

sal_Bool realpath_u(const rtl_uString* pustrFileName, rtl_uString** ppustrResolvedName)
{
    rtl::OString fn = OUStringToOString(pustrFileName);
#ifdef ANDROID
    if (strncmp(fn.getStr(), "/assets", sizeof("/assets")-1) == 0 &&
        (fn.getStr()[sizeof("/assets")-1] == '\0' ||
         fn.getStr()[sizeof("/assets")-1] == '/'))
    {
        if (access_u(pustrFileName, F_OK) == -1)
            return sal_False;

        rtl_uString silly(*pustrFileName);
        rtl_uString_assign(ppustrResolvedName, &silly);

        return sal_True;
    }
#endif

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

    char  rp[PATH_MAX];
    bool  bRet = realpath(fn.getStr(), rp);

    done_accessing_file_path(fn.getStr(), state);

    if (bRet)
    {
        rtl::OUString resolved = rtl::OStringToOUString(rtl::OString(static_cast<sal_Char*>(rp)),
                                                        osl_getThreadTextEncoding());

        rtl_uString_assign(ppustrResolvedName, resolved.pData);
    }
    return bRet;
}

int stat_c(const char* cpPath, struct stat* buf)
{
#ifdef ANDROID
    if (strncmp(cpPath, "/assets", sizeof("/assets")-1) == 0 &&
        (cpPath[sizeof("/assets")-1] == '\0' ||
         cpPath[sizeof("/assets")-1] == '/'))
        return lo_apk_lstat(cpPath, buf);
#endif

    accessFilePathState *state = prepare_to_access_file_path(cpPath);

    int result = stat(cpPath, buf);

    done_accessing_file_path(cpPath, state);

    return result;
}

int lstat_c(const char* cpPath, struct stat* buf)
{
#ifdef ANDROID
    if (strncmp(cpPath, "/assets", sizeof("/assets")-1) == 0 &&
        (cpPath[sizeof("/assets")-1] == '\0' ||
         cpPath[sizeof("/assets")-1] == '/'))
        return lo_apk_lstat(cpPath, buf);
#endif

    accessFilePathState *state = prepare_to_access_file_path(cpPath);

    int result = lstat(cpPath, buf);

    done_accessing_file_path(cpPath, state);

    return result;
}

int lstat_u(const rtl_uString* pustrPath, struct stat* buf)
{
    rtl::OString fn = OUStringToOString(pustrPath);

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

    return lstat_c(fn.getStr(), buf);
}

int mkdir_u(const rtl_uString* path, mode_t mode)
{
    rtl::OString fn = OUStringToOString(path);

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

    int result = mkdir(OUStringToOString(path).getStr(), mode);

    done_accessing_file_path(fn.getStr(), state);

    return result;
}

int open_c(const char *cpPath, int oflag, int mode)
{
    accessFilePathState *state = prepare_to_access_file_path(cpPath);

    int result = open(cpPath, oflag, mode);

#if defined(MACOSX) && MAC_OS_X_VERSION_MIN_REQUIRED >= 1070 && HAVE_FEATURE_MACOSX_SANDBOX
    if (result != -1 && (oflag & O_CREAT) && (oflag & O_EXCL))
    {
        // A new file was created. Check if it is outside the sandbox.
        // (In that case it must be one the user selected as export or
        // save destination in a file dialog, otherwise we wouldn't
        // have been able to crete it.) Create and store a security
        // scoped bookmark for it so that we can access the file in
        // the future, too. (For the "Recent Files" functionality.)
        const char *sandbox = [NSHomeDirectory() UTF8String];
        if (!(strncmp(sandbox, cpPath, strlen(sandbox)) == 0 &&
              cpPath[strlen(sandbox)] == '/'))
        {
            NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:cpPath]];
            NSData *data = [url bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope
                         includingResourceValuesForKeys:nil
                                          relativeToURL:nil
                                                  error:nil];
            if (data != NULL)
            {
                [userDefaults setObject:data
                                 forKey:[@"bookmarkFor:" stringByAppendingString:[url absoluteString]]];
            }
        }
    }
#endif

    done_accessing_file_path(cpPath, state);

    return result;
}

int utime_c(const char *cpPath, struct utimbuf *times)
{
    accessFilePathState *state = prepare_to_access_file_path(cpPath);

    int result = utime(cpPath, times);

    done_accessing_file_path(cpPath, state);

    return result;
}

int ftruncate_with_name(int fd, sal_uInt64 uSize, rtl_String* path)
{
    /* When sandboxed on OS X, ftruncate(), even if it takes an
     * already open file descriptor which was retuned from an open()
     * call already checked by the sandbox, still requires a security
     * scope bookmark for the file to be active in case the file is
     * one that the sandbox doesn't otherwise allow access to. Luckily
     * LibreOffice usually calls ftruncate() through the helpful C++
     * abstraction layer that keeps the pathname around.
     */

    rtl::OString fn = rtl::OString(path);

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

    int result = ftruncate(fd, uSize);

    done_accessing_file_path(fn.getStr(), state);

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
