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

#ifndef INCLUDED_FILE_IMPL_HXX
#define INCLUDED_FILE_IMPL_HXX

#include "osl/file.h"
#include <stddef.h>

struct DirectoryItem_Impl
{
    sal_Int32     m_RefCount;

    rtl_uString * m_ustrFilePath;       /* holds native file name */
    unsigned char m_DType;

    explicit DirectoryItem_Impl(
        rtl_uString * ustrFilePath, unsigned char DType = 0);
    ~DirectoryItem_Impl();

    static void * operator new(size_t n);
    static void operator delete (void * p, size_t);

    void acquire(); /* @see osl_acquireDirectoryItem() */
    void release(); /* @see osl_releaseDirectoryItem() */

    oslFileType getFileType() const;
};

#endif /* INCLUDED_FILE_IMPL_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
