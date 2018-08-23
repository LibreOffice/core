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
#include "precompiled_cppumaker.hxx"

#include "dumputils.hxx"

#include "codemaker/global.hxx"
#include "codemaker/commoncpp.hxx"

#include "rtl/ustring.hxx"
#include "sal/types.h"


namespace codemaker { namespace cppumaker {

bool dumpNamespaceOpen(
    FileStream & out, rtl::OString const & registryType, bool fullModuleType)
{
    bool output = false;
    if (registryType != "/") {
        bool first = true;
        for (sal_Int32 i = 0; i >= 0;) {
            rtl::OString id(registryType.getToken(0, '/', i));
            if (fullModuleType || i >= 0) {
                if (!first) {
                    out << " ";
                }
                out << "namespace " << id << " {";
                first = false;
                output = true;
            }
        }
    }
    return output;
}

bool dumpNamespaceClose(
    FileStream & out, rtl::OString const & registryType, bool fullModuleType)
{
    bool output = false;
    if (registryType != "/") {
        bool first = true;
        for (sal_Int32 i = 0; i >= 0;) {
            i = registryType.indexOf('/', i);
            if (i >= 0) {
                ++i;
            }
            if (fullModuleType || i >= 0) {
                if (!first) {
                    out << " ";
                }
                out << "}";
                first = false;
                output = true;
            }
        }
    }
    return output;
}

void dumpTypeIdentifier(FileStream & out, rtl::OString const & registryType) {
    out << registryType.copy(registryType.lastIndexOf('/') + 1);
}

} }
