/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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

#include <ostream>

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "com/sun/star/connection/NoConnectException.hpp"
#include "com/sun/star/document/MacroExecMode.hpp"
#include "com/sun/star/frame/DispatchResultEvent.hpp"
#include "com/sun/star/frame/DispatchResultState.hpp"
#include "com/sun/star/frame/XComponentLoader.hpp"
#include "com/sun/star/frame/XController.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XDispatchResultListener.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/XNotifyingDispatch.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/EventObject.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/util/URL.hpp"
#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "osl/conditn.hxx"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/process.h"
#include "osl/thread.h"
#include "osl/time.h"
#include "rtl/bootstrap.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/main.h"
#include "sal/types.h"

namespace {

namespace css = com::sun::star;

template< typename charT, typename traits > std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, rtl::OUString const & string)
{
    return stream <<
        rtl::OUStringToOString(string, osl_getThreadTextEncoding()).getStr();
        // best effort; potentially loses data due to conversion failures and
        // embedded null characters
}

rtl::OUString argumentName(rtl::OUString const & name) {
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("arg-")) + name;
}

rtl::OUString getArgument(rtl::OUString const & name) {
    rtl::OUString val;
    CPPUNIT_ASSERT(rtl::Bootstrap::get(argumentName(name), val));
    return val;
}

bool getOptionalArgument(rtl::OUString const & name, rtl::OUString * value) {
    OSL_ASSERT(value != 0);
    return rtl::Bootstrap::get(argumentName(name), *value);
}

rtl::OUString toUrl(rtl::OUString const & pathname) {
    rtl::OUString cwd;
    CPPUNIT_ASSERT_EQUAL(
        osl_Process_E_None, osl_getProcessWorkingDir(&cwd.pData));
    rtl::OUString url;
    CPPUNIT_ASSERT_EQUAL(
        osl::FileBase::E_None,
        osl::FileBase::getFileURLFromSystemPath(pathname, url));
    rtl::OUString absUrl;
    CPPUNIT_ASSERT_EQUAL(
        osl::FileBase::E_None,
        osl::FileBase::getAbsoluteFileURL(cwd, url, absUrl));
    return absUrl;
}

struct Result: private boost::noncopyable {
    osl::Condition condition;
    bool success;
    rtl::OUString result;
};

class Listener:
    public cppu::WeakImplHelper1< css::frame::XDispatchResultListener >
{
public:
    Listener(Result * result): result_(result) { OSL_ASSERT(result != 0); }

private:
    virtual void SAL_CALL disposing(css::lang::EventObject const &)
        throw (css::uno::RuntimeException) {}

    virtual void SAL_CALL dispatchFinished(
        css::frame::DispatchResultEvent const & Result)
        throw (css::uno::RuntimeException);

    Result * result_;
};

void Listener::dispatchFinished(css::frame::DispatchResultEvent const & Result)
    throw (css::uno::RuntimeException)
{
    result_->success =
        (Result.State == css::frame::DispatchResultState::SUCCESS) &&
        (Result.Result >>= result_->result);
    result_->condition.set();
}

class Test: public CppUnit::TestFixture {
public:
    Test(): process_(0) {}

    virtual void setUp();

    virtual void tearDown();

private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();

    oslProcess process_;
    css::uno::Reference< css::lang::XMultiServiceFactory > factory_;
};

void Test::setUp() {
    oslProcessInfo info;
    info.Size = sizeof info;
    CPPUNIT_ASSERT_EQUAL(
        osl_Process_E_None,
        osl_getProcessInfo(0, osl_Process_IDENTIFIER, &info));
    rtl::OUString desc(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("pipe,name=oootest")) +
        rtl::OUString::valueOf(static_cast< sal_Int64 >(info.Ident)) +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";urp")));
    rtl::OUString noquickArg(
        RTL_CONSTASCII_USTRINGPARAM("-quickstart=no"));
    rtl::OUString nofirstArg(
        RTL_CONSTASCII_USTRINGPARAM("-nofirststartwizard"));
    rtl::OUString acceptArg(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-accept=")) + desc);
    rtl::OUString userArg(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-env:UserInstallation=")) +
        toUrl(getArgument(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user")))));
    rtl::OUString jreArg(
        RTL_CONSTASCII_USTRINGPARAM("-env:UNO_JAVA_JFW_ENV_JREHOME=true"));
    rtl::OUString classpathArg(
        RTL_CONSTASCII_USTRINGPARAM("-env:UNO_JAVA_JFW_ENV_CLASSPATH=true"));
    rtl_uString * args[] = {
        noquickArg.pData, nofirstArg.pData, acceptArg.pData, userArg.pData,
        jreArg.pData, classpathArg.pData };
    rtl_uString ** envs = 0;
    rtl::OUString argEnv;
    if (getOptionalArgument(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("env")), &argEnv))
    {
        envs = &argEnv.pData;
    }
    CPPUNIT_ASSERT_EQUAL(
        osl_Process_E_None,
        osl_executeProcess(
            (toUrl(
                getArgument(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("path")))).
             pData),
            args, sizeof args / sizeof args[0], 0, 0, 0, envs,
            envs == 0 ? 0 : 1, &process_));
    try {
        css::uno::Reference< css::uno::XComponentContext > context(
            cppu::defaultBootstrap_InitialComponentContext());
        css::uno::Reference< css::bridge::XUnoUrlResolver > resolver(
            css::bridge::UnoUrlResolver::create(context));
        for (int i = 0;; ++i) {
            try {
                factory_ =
                    css::uno::Reference< css::lang::XMultiServiceFactory >(
                        resolver->resolve(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:")) +
                            desc +
                            rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    ";StarOffice.ServiceManager"))),
                        css::uno::UNO_QUERY_THROW);
                break;
            } catch (css::connection::NoConnectException &) {
                if (i == 600) { // 600 sec
                    throw;
                }
            }
            TimeValue delay = { 1, 0 }; // 1 sec
            CPPUNIT_ASSERT_EQUAL(
                osl_Process_E_TimedOut,
                osl_joinProcessWithTimeout(process_, &delay));
        }
    } catch (...) {
        CPPUNIT_ASSERT_EQUAL(
            osl_Process_E_None, osl_terminateProcess(process_));
        osl_freeProcessHandle(process_);
        process_ = 0;
        throw;
    }
}

void Test::tearDown() {
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

void Test::test() {
    css::uno::Sequence< css::beans::PropertyValue > args(1);
    args[0].Name = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("MacroExecutionMode"));
    args[0].Handle = -1;
    args[0].Value <<=
        com::sun::star::document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
    args[0].State = css::beans::PropertyState_DIRECT_VALUE;
    css::util::URL url;
    url.Complete = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "vnd.sun.star.script:Standard.Global.StartTestWithDefaultOptions?"
            "language=Basic&location=document"));
    Result result;
    css::uno::Reference< css::frame::XNotifyingDispatch >(
        css::uno::Reference< css::frame::XDispatchProvider >(
            css::uno::Reference< css::frame::XController >(
                css::uno::Reference< css::frame::XModel >(
                    css::uno::Reference< css::frame::XComponentLoader >(
                        factory_->createInstance(
                            rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.frame.Desktop"))),
                        css::uno::UNO_QUERY_THROW)->loadComponentFromURL(
                            toUrl(
                                getArgument(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM("doc")))),
                            rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM("_default")),
                            0, args),
                    css::uno::UNO_QUERY_THROW)->getCurrentController(),
                css::uno::UNO_SET_THROW)->getFrame(),
            css::uno::UNO_QUERY_THROW)->queryDispatch(
                url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")), 0),
        css::uno::UNO_QUERY_THROW)->dispatchWithNotification(
            url, css::uno::Sequence< css::beans::PropertyValue >(),
            new Listener(&result));
    result.condition.wait();
    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), result.result);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();
