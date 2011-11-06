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
#include "precompiled_cppu.hxx"

#include "sal/config.h"

#include "osl/module.h"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"

#include "loadmodule.hxx"

namespace cppu { namespace detail {

::oslModule loadModule(rtl::OUString const & name) {
    rtl::OUStringBuffer b;
#if defined SAL_DLLPREFIX
    b.appendAscii(RTL_CONSTASCII_STRINGPARAM(SAL_DLLPREFIX));
#endif
    b.append(name);
    b.appendAscii(RTL_CONSTASCII_STRINGPARAM(SAL_DLLEXTENSION));
    return ::osl_loadModuleRelative(
        reinterpret_cast< oslGenericFunction >(&loadModule),
        b.makeStringAndClear().pData,
        SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY);
}

} }
