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

#ifdef MACOSX
/*
 * Helper function for resolving Mac native alias files (not the same as unix alias files)
 * and to return the resolved alias as rtl::OString
 */
inline rtl::OString macxp_resolveAliasAndConvert(const rtl_uString* s)
{
    rtl::OString p = OUStringToOString(s);
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
#ifndef MACOSX
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
    return access(fn.getStr(), mode);
#else
    return access(macxp_resolveAliasAndConvert(pustrPath).getStr(), mode);
#endif
}

sal_Bool realpath_u(const rtl_uString* pustrFileName, rtl_uString** ppustrResolvedName)
{
#ifndef MACOSX
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
#else
    rtl::OString fn = macxp_resolveAliasAndConvert(pustrFileName);
#endif
    char  rp[PATH_MAX];
    bool  bRet = realpath(fn.getStr(), rp);

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
    return lstat(cpPath, buf);
}

int lstat_u(const rtl_uString* pustrPath, struct stat* buf)
{
#ifndef MACOSX
    rtl::OString fn = OUStringToOString(pustrPath);
    return lstat_c(fn.getStr(), buf);
#else
    return lstat(macxp_resolveAliasAndConvert(pustrPath).getStr(), buf);
#endif
}

int mkdir_u(const rtl_uString* path, mode_t mode)
{
    return mkdir(OUStringToOString(path).getStr(), mode);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
