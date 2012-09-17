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

#include <sal/types.h>
#include "boost/noncopyable.hpp"
#include "com/sun/star/awt/AsyncCallback.hpp"
#include "com/sun/star/awt/XCallback.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/document/MacroExecMode.hpp"
#include "com/sun/star/frame/DispatchResultEvent.hpp"
#include "com/sun/star/frame/DispatchResultState.hpp"
#include "com/sun/star/frame/XComponentLoader.hpp"
#include "com/sun/star/frame/XController.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XDispatchResultListener.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/XNotifyingDispatch.hpp"
#include "com/sun/star/lang/EventObject.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/util/URL.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "osl/conditn.hxx"
#include "osl/diagnose.h"
#include "osl/time.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "unotest/gettestargument.hxx"
#include "unotest/officeconnection.hxx"
#include "unotest/toabsolutefileurl.hxx"

namespace {

namespace css = com::sun::star;

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

class Callback: public cppu::WeakImplHelper1< css::awt::XCallback > {
public:
    Callback(
        css::uno::Reference< css::frame::XNotifyingDispatch > const & dispatch,
        css::util::URL const & url,
        css::uno::Sequence< css::beans::PropertyValue > const & arguments,
        css::uno::Reference< css::frame::XDispatchResultListener > const &
            listener):
        dispatch_(dispatch), url_(url), arguments_(arguments),
        listener_(listener)
    { OSL_ASSERT(dispatch.is()); }

private:
    virtual void SAL_CALL notify(css::uno::Any const &)
        throw (css::uno::RuntimeException)
    { dispatch_->dispatchWithNotification(url_, arguments_, listener_); }

    css::uno::Reference< css::frame::XNotifyingDispatch > dispatch_;
    css::util::URL url_;
    css::uno::Sequence< css::beans::PropertyValue > arguments_;
    css::uno::Reference< css::frame::XDispatchResultListener > listener_;
};

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp();

    virtual void tearDown();

private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();

    test::OfficeConnection connection_;
};

void Test::setUp() {
    connection_.setUp();
}

void Test::tearDown() {
    connection_.tearDown();
}

void Test::test() {
    rtl::OUString doc;
    CPPUNIT_ASSERT(
        test::getTestArgument(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smoketest.doc")), &doc));
    css::uno::Sequence< css::beans::PropertyValue > args(2);
    args[0].Name = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("MacroExecutionMode"));
    args[0].Handle = -1;
    args[0].Value <<=
        com::sun::star::document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
    args[0].State = css::beans::PropertyState_DIRECT_VALUE;
    args[1].Name = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
    args[1].Handle = -1;
    args[1].Value <<= sal_True;
    args[1].State = css::beans::PropertyState_DIRECT_VALUE;
    css::util::URL url;
    url.Complete = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "vnd.sun.star.script:Standard.Global.StartTestWithDefaultOptions?"
            "language=Basic&location=document"));
    css::uno::Reference< css::frame::XNotifyingDispatch > disp(
        css::uno::Reference< css::frame::XDispatchProvider >(
            css::uno::Reference< css::frame::XController >(
                css::uno::Reference< css::frame::XModel >(
                    css::uno::Reference< css::frame::XComponentLoader >(
                        (connection_.getComponentContext()->
                         getServiceManager()->createInstanceWithContext(
                             rtl::OUString(
                                 RTL_CONSTASCII_USTRINGPARAM(
                                     "com.sun.star.frame.Desktop")),
                             connection_.getComponentContext())),
                        css::uno::UNO_QUERY_THROW)->loadComponentFromURL(
                            test::toAbsoluteFileUrl(doc),
                            rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM("_default")),
                            0, args),
                    css::uno::UNO_QUERY_THROW)->getCurrentController(),
                css::uno::UNO_SET_THROW)->getFrame(),
            css::uno::UNO_QUERY_THROW)->queryDispatch(
                url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")), 0),
        css::uno::UNO_QUERY_THROW);
    Result result;
    // Shifted to main thread to work around potential deadlocks (i112867):
    com::sun::star::awt::AsyncCallback::create(
        connection_.getComponentContext())->addCallback(
            new Callback(
                disp, url, css::uno::Sequence< css::beans::PropertyValue >(),
                new Listener(&result)),
            css::uno::Any());
    // Wait for result.condition or connection_ going stale:
    for (;;) {
        TimeValue delay = { 1, 0 }; // 1 sec
        osl::Condition::Result res = result.condition.wait(&delay);
        if (res == osl::Condition::result_ok) {
            break;
        }
        CPPUNIT_ASSERT_EQUAL(osl::Condition::result_timeout, res);
        CPPUNIT_ASSERT(connection_.isStillAlive());
    }
    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), result.result);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
