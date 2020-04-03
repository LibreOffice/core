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

#ifndef INCLUDED_SAL_OSL_UNX_UUNXAPI_HXX
#define INCLUDED_SAL_OSL_UNX_UUNXAPI_HXX

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <rtl/ustring.h>

#include <rtl/ustring.hxx>

int access_u(const rtl_uString* pustrPath, int mode);

/***********************************
 @descr
 The return value differs from the
 realpath function

 @returns sal_True on success else
 sal_False

 @see realpath
 **********************************/
bool realpath_u(
    const rtl_uString* pustrFileName,
    rtl_uString** ppustrResolvedName);

int stat_c(const char *cpPath, struct stat* buf);

int lstat_c(const char *cpPath, struct stat* buf);

int lstat_u(const rtl_uString* pustrPath, struct stat* buf);

int mkdir_u(const rtl_uString* path, mode_t mode);

int open_c(const char *cpPath, int oflag, int mode);

int utime_c(const char *cpPath, struct utimbuf *times);

int ftruncate_with_name(int fd, sal_uInt64 uSize, rtl_String* path);

namespace osl
{
    inline int access(const OUString& ustrPath, int mode)
    {
        return access_u(ustrPath.pData, mode);
    }

    /***********************************
     osl::realpath

     @descr
     The return value differs from the
     realpath function

     @returns sal_True on success else
     sal_False

     @see realpath
     **********************************/

    inline bool realpath(
        const OUString& ustrFileName,
        OUString& ustrResolvedName)
    {
        return realpath_u(ustrFileName.pData, &ustrResolvedName.pData);
    }

    inline int lstat(const OUString& ustrPath, struct stat& buf)
    {
        return lstat_u(ustrPath.pData, &buf);
    }

    inline int mkdir(const OUString& aPath, mode_t aMode)
    {
        return mkdir_u(aPath.pData, aMode);
    }

    inline OString openFlagsToString(int flags)
    {
        OString result;
        switch (flags & O_ACCMODE)
        {
        case O_RDONLY:
            result = "O_RDONLY";
            break;
        case O_WRONLY:
            result = "O_WRONLY";
            break;
            break;
        case O_RDWR:
            result = "O_RDWR";
            break;
        }
        if (flags & O_CREAT)
            result += "|O_CREAT";
        if (flags & O_EXCL)
            result += "|O_EXCL";
        if (flags & O_NOCTTY)
            result += "|O_NOCTTY";
        if (flags & O_TRUNC)
            result += "|O_TRUNC";
        if (flags & O_NONBLOCK)
            result += "|O_NONBLOCK";
        if (flags & O_NONBLOCK)
            result += "|O_NONBLOCK";
        if ((flags & O_SYNC) == O_SYNC)
            result += "|O_SYNC";
        if (flags & O_ASYNC)
            result += "|O_ASYNC";

#if defined __O_LARGEFILE
        if (flags & __O_LARGEFILE)
            result += "|O_LARGEFILE";
#endif

#if defined __O_DIRECTORY
        if (flags & __O_DIRECTORY)
            result += "|O_DIRECTORY";
#elif defined O_DIRECTORY
        if (flags & O_DIRECTORY)
            result += "|O_DIRECTORY";
#endif

#if defined __O_NOFOLLOW
        if (flags & __O_NOFOLLOW)
            result += "|O_NOFOLLOW";
#elif defined O_NOFOLLOW
        if (flags & O_NOFOLLOW)
            result += "|O_NOFOLLOW";
#endif

#if defined __O_CLOEXEC
        if (flags & __O_CLOEXEC)
            result += "|O_CLOEXEC";
#elif defined O_CLOEXEC
        if (flags & O_CLOEXEC)
            result += "|O_CLOEXEC";
#endif

#if defined __O_DIRECT
        if (flags & __O_DIRECT)
            result += "|O_DIRECT";
#endif

#if defined __O_NOATIME
        if (flags & __O_NOATIME)
            result += "|O_NOATIME";
#endif

#if defined __O_PATH
        if (flags & __O_PATH)
            result += "|O_PATH";
#endif

#if defined __O_DSYNC
        if (flags & __O_DSYNC)
            result += "|O_DSYNC";
#endif

#if defined __O_TMPFILE
        if ((flags & __O_TMPFILE) == __O_TMPFILE)
            result += "|O_TMPFILE";
#endif

        return result;
    }

    inline OString openModeToString(int mode)
    {
        if (mode == 0)
            return "0";
        else
            return "0" + OString::number(mode, 8);
    }

    inline OString accessModeToString(int mode)
    {
        if (mode == F_OK)
            return "F_OK";
        OString result;
        if (mode & R_OK)
            result = "R_OK";
        if (mode & W_OK)
        {
            if (result != "")
                result += "|";
            result += "W_OK";
        }
        if (mode & X_OK)
        {
            if (result != "")
                result += "|";
            result += "X_OK";
        }
        return result;
    }

} // end namespace osl

#endif // INCLUDED_SAL_OSL_UNX_UUNXAPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
