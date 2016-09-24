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

#ifndef INCLUDED_SAL_OSL_UNX_FILE_IMPL_HXX
#define INCLUDED_SAL_OSL_UNX_FILE_IMPL_HXX

#include "osl/file.h"
#include <stddef.h>
#include <sys/types.h>

struct DirectoryItem_Impl
{
    sal_Int32     m_RefCount;

    rtl_uString * m_ustrFilePath;       /* holds native file name */
    unsigned char m_DType;

    explicit DirectoryItem_Impl(
        rtl_uString * ustrFilePath, unsigned char DType = 0);
    ~DirectoryItem_Impl();

    static void * operator new(size_t n);
    static void operator delete (void * p);

    void acquire(); /* @see osl_acquireDirectoryItem() */
    void release(); /* @see osl_releaseDirectoryItem() */

    oslFileType getFileType() const;
};

oslFileError openFile(
    rtl_uString * pustrFileURL, oslFileHandle * pHandle, sal_uInt32 uFlags,
    mode_t mode);

oslFileError openFilePath(
    const char *cpFilePath,
    oslFileHandle* pHandle,
    sal_uInt32 uFlags, mode_t mode );

#endif // INCLUDED_SAL_OSL_UNX_FILE_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
