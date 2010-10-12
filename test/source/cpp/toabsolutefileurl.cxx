/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
