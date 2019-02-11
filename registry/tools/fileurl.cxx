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

#include "fileurl.hxx"

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>

#include <string.h>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using osl::FileBase;

namespace registry
{
namespace tools
{

OUString convertToFileUrl(char const * filename, sal_Int32 length)
{
    OUString const uFileName(filename, length, osl_getThreadTextEncoding());
    if (strncmp(filename, "file://", 7) == 0)
    {
        // already a FileUrl.
        return uFileName;
    }

    OUString uFileUrl;
    if (length > 0)
    {
        if (filename[0] != SEPARATOR)
        {
            // relative path name.
            OUString uWorkingDir;
            if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None)
            {
                assert(false);
            }
            if (FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uFileUrl) != FileBase::E_None)
            {
                assert(false);
            }
        }
        else
        {
            // absolute path name.
            if (FileBase::getFileURLFromSystemPath(uFileName, uFileUrl) != FileBase::E_None)
            {
                assert(false);
            }
        }
    }
    return uFileUrl;
}

} // namespace tools
} // namespace registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
