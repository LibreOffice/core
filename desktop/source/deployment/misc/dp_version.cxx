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
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include "com/sun/star/deployment/XPackage.hpp"
#include "rtl/ustring.hxx"

#include "dp_version.hxx"

namespace {

namespace css = ::com::sun::star;

::rtl::OUString getElement(::rtl::OUString const & version, ::sal_Int32 * index)
{
    while (*index < version.getLength() && version[*index] == '0') {
        ++*index;
    }
    return version.getToken(0, '.', *index);
}

}

namespace dp_misc {

::dp_misc::Order compareVersions(
    ::rtl::OUString const & version1, ::rtl::OUString const & version2)
{
    for (::sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0;) {
        ::rtl::OUString e1(getElement(version1, &i1));
        ::rtl::OUString e2(getElement(version2, &i2));
        if (e1.getLength() < e2.getLength()) {
            return ::dp_misc::LESS;
        } else if (e1.getLength() > e2.getLength()) {
            return ::dp_misc::GREATER;
        } else if (e1 < e2) {
            return ::dp_misc::LESS;
        } else if (e1 > e2) {
            return ::dp_misc::GREATER;
        }
    }
    return ::dp_misc::EQUAL;
}


}
