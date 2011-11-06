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



#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/ustring.hxx"
#include "test/toabsolutefileurl.hxx"

namespace {

namespace css = com::sun::star;

}

namespace test {

rtl::OUString toAbsoluteFileUrl(rtl::OUString const & relativePathname) {
    rtl::OUString cwd;
    oslProcessError e1 = osl_getProcessWorkingDir(&cwd.pData);
    if (e1 != osl_Process_E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "osl_getProcessWorkingDir failed with ")) +
             rtl::OUString::valueOf(static_cast< sal_Int32 >(e1))),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString url;
    osl::FileBase::RC e2 = osl::FileBase::getFileURLFromSystemPath(
        relativePathname, url);
    if (e2 != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "osl::FileBase::getFileURLFromSystemPath(")) +
             relativePathname +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(") failed with ")) +
             rtl::OUString::valueOf(static_cast< sal_Int32 >(e2))),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString absUrl;
    e2 = osl::FileBase::getAbsoluteFileURL(cwd, url, absUrl);
    if (e2 != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "osl::FileBase::getAbsoluteFileURL(")) +
             cwd + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(", ")) + url +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(") failed with ")) +
             rtl::OUString::valueOf(static_cast< sal_Int32 >(e2))),
            css::uno::Reference< css::uno::XInterface >());
    }
    return absUrl;
}

}
