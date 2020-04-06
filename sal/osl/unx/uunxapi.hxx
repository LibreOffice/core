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

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <rtl/ustring.h>

#include <rtl/ustring.hxx>

int stat_c(const char *cpPath, struct stat* buf);

int lstat_c(const char *cpPath, struct stat* buf);

int mkdir_c(OString const & path, mode_t mode);

int open_c(const char *cpPath, int oflag, int mode);

int utime_c(const char *cpPath, struct utimbuf *times);

int ftruncate_with_name(int fd, sal_uInt64 uSize, rtl_String* path);

namespace osl
{
    OString OUStringToOString(const OUString& s);

    int access(const OString& strPath, int mode);

    /***********************************
     osl::realpath

     @descr
     The return value differs from the
     realpath function

     @returns sal_True on success else
     sal_False

     @see realpath
     **********************************/

    bool realpath(
        const OUString& ustrFileName,
        OUString& ustrResolvedName);

    bool realpath(
        const OString& strFileName,
        OString& strResolvedName);

    int lstat(const OUString& ustrPath, struct stat& buf);

    int lstat(const OString& strPath, struct stat& buf);

    int mkdir(const OString& aPath, mode_t aMode);
} // end namespace osl

#endif // INCLUDED_SAL_OSL_UNX_UUNXAPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
