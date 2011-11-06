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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include "sal/config.h"

#include <cstddef>

#include "osl/diagnose.h"
#include "osl/module.h"
#include "osl/module.hxx"
#include "osl/thread.h"
#include "rtl/malformeduriexception.hxx"
#include "rtl/uri.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

extern "C" {

oslModule SAL_CALL osl_loadModuleRelative(
    oslGenericFunction const baseModule, rtl_uString * const relativePath,
    sal_Int32 const mode)
{
    ::rtl::OUString base;
    if (!::osl::Module::getUrlFromAddress(baseModule, base)) {
        OSL_TRACE("osl::Module::getUrlFromAddress failed");
        return NULL;
    }
    ::rtl::OUString abs;
    try {
        abs = ::rtl::Uri::convertRelToAbs(base, relativePath);
    } catch (::rtl::MalformedUriException & e) {
        (void) e; // avoid warnings
        OSL_TRACE(
            "rtl::MalformedUriException <%s>",
            rtl::OUStringToOString(e.getMessage(), osl_getThreadTextEncoding()).
                getStr());
            //TODO: let some OSL_TRACE variant take care of text conversion?
        return NULL;
    }
    return ::osl_loadModule(abs.pData, mode);
}

}
