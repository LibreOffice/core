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

#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "com/sun/star/connection/NoConnectException.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/bootstrap.hxx"

#include "cppunit/TestAssert.h"

#include "osl/process.h"
#include "osl/time.h"
#include "sal/types.h"
#include <sal/macros.h>
#include "test/getargument.hxx"
#include "test/officeconnection.hxx"
#include "test/toabsolutefileurl.hxx"

namespace {

namespace css = com::sun::star;

}

namespace test {

OfficeConnection::OfficeConnection(): process_(0) {}

OfficeConnection::~OfficeConnection() {}

void OfficeConnection::setUp() {
    rtl::OUString desc;
    rtl::OUString argSoffice;
    CPPUNIT_ASSERT(
        getArgument(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("soffice")),
            &argSoffice));
    if (argSoffice.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("path:"))) {
        oslProcessInfo info;
        info.Size = sizeof info;
        CPPUNIT_ASSERT_EQUAL(
            osl_Process_E_None,
            osl_getProcessInfo(0, osl_Process_IDENTIFIER, &info));
        desc = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pipe,name=oootest")) +
            rtl::OUString::valueOf(static_cast< sal_Int64 >(info.Ident));
        rtl::OUString noquickArg(
            RTL_CONSTASCII_USTRINGPARAM("--quickstart=no"));
        rtl::OUString nofirstArg(
            RTL_CONSTASCII_USTRINGPARAM("--nofirststartwizard"));
        rtl::OUString norestoreArg(RTL_CONSTASCII_USTRINGPARAM("--norestore"));
        //Disable use of the unix standalone splash screen app for the tests
        rtl::OUString noSplashArg(RTL_CONSTASCII_USTRINGPARAM("--no-oosplash"));
        rtl::OUString acceptArg(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("--accept=")) + desc +
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";urp")));
        rtl::OUString argUser;
        CPPUNIT_ASSERT(
            getArgument(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user")), &argUser));
        rtl::OUString userArg(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("-env:UserInstallation=")) +
            toAbsoluteFileUrl(argUser));
        rtl::OUString jreArg(
            RTL_CONSTASCII_USTRINGPARAM("-env:UNO_JAVA_JFW_ENV_JREHOME=true"));
        rtl::OUString classpathArg(
            RTL_CONSTASCII_USTRINGPARAM(
                "-env:UNO_JAVA_JFW_ENV_CLASSPATH=true"));
        rtl_uString * args[] = {
            noquickArg.pData, nofirstArg.pData, norestoreArg.pData, noSplashArg.pData,
            acceptArg.pData, userArg.pData, jreArg.pData, classpathArg.pData };
        rtl_uString ** envs = 0;
        rtl::OUString argEnv;
        if (getArgument(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("env")), &argEnv))
        {
            envs = &argEnv.pData;
        }
        CPPUNIT_ASSERT_EQUAL(
            osl_Process_E_None,
            osl_executeProcess(
                toAbsoluteFileUrl(
                    argSoffice.copy(RTL_CONSTASCII_LENGTH("path:"))).pData,
                args, SAL_N_ELEMENTS( args ), 0, 0, 0, envs,
                envs == 0 ? 0 : 1, &process_));
    } else if (argSoffice.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("connect:"))) {
        desc = argSoffice.copy(RTL_CONSTASCII_LENGTH("connect:"));
    } else {
        CPPUNIT_FAIL(
            "\"soffice\" argument starts with neither \"path:\" nor"
            " \"connect:\"");
    }
    css::uno::Reference< css::bridge::XUnoUrlResolver > resolver(
        css::bridge::UnoUrlResolver::create(
            cppu::defaultBootstrap_InitialComponentContext()));
    for (;;) {
        try {
            factory_ =
                css::uno::Reference< css::lang::XMultiServiceFactory >(
                    resolver->resolve(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:")) +
                        desc +
                        rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                ";urp;StarOffice.ServiceManager"))),
                    css::uno::UNO_QUERY_THROW);
            break;
        } catch (css::connection::NoConnectException &) {}
        if (process_ != 0) {
            TimeValue delay = { 1, 0 }; // 1 sec
            CPPUNIT_ASSERT_EQUAL(
                osl_Process_E_TimedOut,
                osl_joinProcessWithTimeout(process_, &delay));
        }
    }
}

void OfficeConnection::tearDown() {
    if (factory_.is()) {
        css::uno::Reference< css::frame::XDesktop > desktop(
            factory_->createInstance(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))),
            css::uno::UNO_QUERY_THROW);
        factory_.clear();
        try {
            CPPUNIT_ASSERT(desktop->terminate());
            desktop.clear();
        } catch (css::lang::DisposedException &) {}
            // it appears that DisposedExceptions can already happen while
            // receiving the response of the terminate call
    }
    if (process_ != 0) {
        CPPUNIT_ASSERT_EQUAL(osl_Process_E_None, osl_joinProcess(process_));
        oslProcessInfo info;
        info.Size = sizeof info;
        CPPUNIT_ASSERT_EQUAL(
            osl_Process_E_None,
            osl_getProcessInfo(process_, osl_Process_EXITCODE, &info));
        CPPUNIT_ASSERT_EQUAL(oslProcessExitCode(0), info.Code);
        osl_freeProcessHandle(process_);
    }
}

css::uno::Reference< css::lang::XMultiServiceFactory >
OfficeConnection::getFactory() const {
    return factory_;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
