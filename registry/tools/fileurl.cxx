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

#include "fileurl.hxx"

#include "rtl/ustring.hxx"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "osl/thread.h"

#include <string.h>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using rtl::OUString;
using osl::FileBase;

namespace registry
{
namespace tools
{

OUString convertToFileUrl(char const * filename, size_t length)
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
        if ((filename[0] == '.') || (filename[0] != SEPARATOR))
        {
            // relative path name.
            OUString uWorkingDir;
            if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None)
            {
                OSL_ASSERT(false);
            }
            if (FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uFileUrl) != FileBase::E_None)
            {
                OSL_ASSERT(false);
            }
        }
        else
        {
            // absolute path name.
            if (FileBase::getFileURLFromSystemPath(uFileName, uFileUrl) != FileBase::E_None)
            {
                OSL_ASSERT(false);
            }
        }
    }
    return uFileUrl;
}

} // namespace tools
} // namespace registry
