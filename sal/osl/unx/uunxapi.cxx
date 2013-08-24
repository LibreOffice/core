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
#ifndef MACOSX
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
    return access(fn.getStr(), mode);
#else

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

    int result = access(macxp_resolveAliasAndConvert(fn).getStr(), mode);

    done_accessing_file_path(fn.getStr(), state);

    return result;

#endif
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

    accessFilePathState *state = prepare_to_access_file_path(fn.getStr());

#ifdef MACOSX
    fn = macxp_resolveAliasAndConvert(fn);
#endif

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
    return stat(cpPath, buf);
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
#ifndef MACOSX
    return lstat_c(fn.getStr(), buf);
#else
    return lstat(macxp_resolveAliasAndConvert(fn).getStr(), buf);
#endif
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
