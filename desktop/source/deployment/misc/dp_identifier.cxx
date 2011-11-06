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

#include "boost/optional.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"

#include "dp_identifier.hxx"

namespace {
    namespace css = ::com::sun::star;
}

namespace dp_misc {

::rtl::OUString generateIdentifier(
    ::boost::optional< ::rtl::OUString > const & optional,
    ::rtl::OUString const & fileName)
{
    return optional ? *optional : generateLegacyIdentifier(fileName);
}

::rtl::OUString getIdentifier(
    css::uno::Reference< css::deployment::XPackage > const & package)
{
    OSL_ASSERT(package.is());
    css::beans::Optional< ::rtl::OUString > id(package->getIdentifier());
    return id.IsPresent
        ? id.Value : generateLegacyIdentifier(package->getName());
}

::rtl::OUString generateLegacyIdentifier(::rtl::OUString const & fileName) {
    rtl::OUStringBuffer b;
    b.appendAscii(RTL_CONSTASCII_STRINGPARAM("org.openoffice.legacy."));
    b.append(fileName);
    return b.makeStringAndClear();
}

}
