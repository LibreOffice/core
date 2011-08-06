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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#ifndef INCLUDED_RECENTLY_USED_FILE
#include "recently_used_file.hxx"
#endif
#include <rtl/ustring.hxx>
#include <osl/process.h>
#include <osl/security.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

const rtl::OUString RECENTLY_USED_FILE_NAME(RTL_CONSTASCII_USTRINGPARAM(".recently-used"));
const rtl::OUString SLASH(RTL_CONSTASCII_USTRINGPARAM("/"));

namespace /* private */ {

inline void ensure_final_slash(/*inout*/ rtl::OUString& path)
{
    if ((path.getLength() > 0) &&
        (SLASH.pData->buffer[0] != path.pData->buffer[path.getLength() - 1]))
        path += SLASH;
}

} // namespace private

//------------------------------------------------
recently_used_file::recently_used_file() :
    file_(NULL)
{
    osl::Security sec;
    rtl::OUString homedir_url;

    if (sec.getHomeDir(homedir_url))
    {
        rtl::OUString homedir;
        osl::FileBase::getSystemPathFromFileURL(homedir_url, homedir);

        rtl::OUString rufn = homedir;
        ensure_final_slash(rufn);
        rufn += RECENTLY_USED_FILE_NAME;

        rtl::OString tmp =
            rtl::OUStringToOString(rufn, osl_getThreadTextEncoding());

        file_ = fopen(tmp.getStr(), "r+");

        /* create if not exist */
        if (NULL == file_) {
            mode_t umask_ = umask(S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
            file_ = fopen(tmp.getStr(), "w+");
            umask(umask_);
        }

        if (NULL == file_)
            throw "I/O error opening ~/.recently-used";

        if (lockf(fileno(file_), F_LOCK, 0) != 0)
        {
            fclose(file_);
            throw "Cannot lock ~/.recently-used";
        }
    }
    else
        throw "Cannot determine user home directory";
}

//------------------------------------------------
recently_used_file::~recently_used_file()
{
    lockf(fileno(file_), F_ULOCK, 0);
    fclose(file_);
}

//------------------------------------------------
void recently_used_file::reset() const
{
    rewind(file_);
}

//------------------------------------------------
void recently_used_file::truncate(off_t length)
{
    if (ftruncate(fileno(file_), length) == -1)
        throw "I/O error: ftruncate failed";
}

//------------------------------------------------
size_t recently_used_file::read(char* buffer, size_t size) const
{
    size_t  r = fread(reinterpret_cast<void*>(buffer), sizeof(char), size, file_);

    if ((r < size) && ferror(file_))
        throw "I/O error: read failed";

    return r;
}

//----------------------------
void recently_used_file::write(const char* buffer, size_t size) const
{
    if (size != fwrite(reinterpret_cast<const void*>(buffer), sizeof(char), size, file_))
        throw "I/O error: write failed";
}

//----------------------------
bool recently_used_file::eof() const
{
    return feof(file_);
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
