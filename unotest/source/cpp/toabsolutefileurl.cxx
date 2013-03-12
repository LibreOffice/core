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

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/ustring.hxx"
#include "unotest/toabsolutefileurl.hxx"

namespace test {

rtl::OUString toAbsoluteFileUrl(rtl::OUString const & relativePathname) {
    rtl::OUString cwd;
    oslProcessError e1 = osl_getProcessWorkingDir(&cwd.pData);
    if (e1 != osl_Process_E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString("osl_getProcessWorkingDir failed with ") +
             OUString::number(e1)),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString url;
    osl::FileBase::RC e2 = osl::FileBase::getFileURLFromSystemPath(
        relativePathname, url);
    if (e2 != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString("osl::FileBase::getFileURLFromSystemPath(") +
             relativePathname +
             rtl::OUString(") failed with ") +
             OUString::number(e2)),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString absUrl;
    e2 = osl::FileBase::getAbsoluteFileURL(cwd, url, absUrl);
    if (e2 != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString("osl::FileBase::getAbsoluteFileURL(") +
             cwd + rtl::OUString(", ") + url +
             rtl::OUString(") failed with ") +
             OUString::number(e2)),
            css::uno::Reference< css::uno::XInterface >());
    }
    return absUrl;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
