/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
