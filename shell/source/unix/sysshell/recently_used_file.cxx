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

#include "recently_used_file.hxx"
#include <rtl/ustring.hxx>
#include <osl/process.h>
#include <osl/security.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

const rtl::OUString RECENTLY_USED_FILE_NAME(".recently-used");
const rtl::OUString SLASH("/");

namespace /* private */ {

inline void ensure_final_slash(/*inout*/ rtl::OUString& path)
{
    if (!path.isEmpty() &&
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
    int ret = lockf(fileno(file_), F_ULOCK, 0);
    SAL_WARN_IF(ret != 0, "shell", "cannot unlock recently unused file");
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
