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



#include "precompiled_tools.hxx"
#include "sal/config.h"

#include <cstddef>

#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/bootstrap.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "tools/getprocessworkingdir.hxx"

namespace tools {

bool getProcessWorkingDir(rtl::OUString * url) {
    OSL_ASSERT(url != NULL);
    rtl::OUString s(RTL_CONSTASCII_USTRINGPARAM("$OOO_CWD"));
    rtl::Bootstrap::expandMacros(s);
    if (s.getLength() == 0) {
        if (osl_getProcessWorkingDir(&url->pData) == osl_Process_E_None) {
            return true;
        }
    } else if (s[0] == '1') {
        *url = s.copy(1);
        return true;
    } else if (s[0] == '2' &&
               (osl::FileBase::getFileURLFromSystemPath(s.copy(1), *url) ==
                osl::FileBase::E_None))
    {
        return true;
    }
    *url = rtl::OUString();
    return false;
}

}
