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
#include <chrono>
#include <com/sun/star/awt/AsyncCallback.hpp>
#include <com/sun/star/awt/XCallback.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/URL.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <osl/conditn.hxx>
#include <osl/diagnose.h>
#include <osl/time.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <unotest/gettestargument.hxx>
#include <unotest/officeconnection.hxx>
#include <unotest/toabsolutefileurl.hxx>
#include <utility>

namespace {

struct Result {
    osl::Condition condition;
    bool success;
    OUString result;
    Result()
        : success(false)
    {
    }
    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
};

class Listener:
    public cppu::WeakImplHelper< css::frame::XDispatchResultListener >
{
public:
    explicit Listener(Result * result): result_(result) { OSL_ASSERT(result != nullptr); }

private:
    virtual void SAL_CALL disposing(css::lang::EventObject const &) override {}

    virtual void SAL_CALL dispatchFinished(
        css::frame::DispatchResultEvent const & Result) override;

    Result * result_;
};

void Listener::dispatchFinished(css::frame::DispatchResultEvent const & Result)
{
    result_->success =
        (Result.State == css::frame::DispatchResultState::SUCCESS) &&
        (Result.Result >>= result_->result);
    result_->condition.set();
}

class Callback: public cppu::WeakImplHelper< css::awt::XCallback > {
public:
    Callback(
        css::uno::Reference< css::frame::XNotifyingDispatch > const & dispatch,
        css::util::URL url,
        css::uno::Sequence< css::beans::PropertyValue > const & arguments,
        css::uno::Reference< css::frame::XDispatchResultListener > listener):
        dispatch_(dispatch), url_(std::move(url)), arguments_(arguments),
        listener_(std::move(listener))
    { OSL_ASSERT(dispatch.is()); }

private:
    virtual void SAL_CALL notify(css::uno::Any const &) override
    { dispatch_->dispatchWithNotification(url_, arguments_, listener_); }

    css::uno::Reference< css::frame::XNotifyingDispatch > dispatch_;
    css::util::URL url_;
    css::uno::Sequence< css::beans::PropertyValue > arguments_;
    css::uno::Reference< css::frame::XDispatchResultListener > listener_;
};

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp() override;

    virtual void tearDown() override;

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
    OUString doc;
    CPPUNIT_ASSERT(
        test::getTestArgument(
            u"smoketest.doc", &doc));
    css::uno::Sequence< css::beans::PropertyValue > args{
        { /* Name   */ u"MacroExecutionMode"_ustr,
          /* Handle */ -1,
          /* Value  */ css::uno::Any(css::document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN),
          /* State  */ css::beans::PropertyState_DIRECT_VALUE },
        { /* Name   */ u"ReadOnly"_ustr,
          /* Handle */ -1,
          /* Value  */ css::uno::Any(true),
          /* State  */ css::beans::PropertyState_DIRECT_VALUE }
    };
    css::util::URL url;
    url.Complete = "vnd.sun.star.script:Standard.Global.StartTestWithDefaultOptions?"
            "language=Basic&location=document";

    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create(connection_.getComponentContext());

    css::uno::Reference< css::frame::XNotifyingDispatch > disp(
        css::uno::Reference< css::frame::XDispatchProvider >(
            css::uno::Reference< css::frame::XController >(
                css::uno::Reference< css::frame::XModel >(
                    xDesktop->loadComponentFromURL(
                            test::toAbsoluteFileUrl(doc),
                            u"_default"_ustr,
                            0, args),
                    css::uno::UNO_QUERY_THROW)->getCurrentController(),
                css::uno::UNO_SET_THROW)->getFrame(),
            css::uno::UNO_QUERY_THROW)->queryDispatch(
                url, u"_self"_ustr, 0),
        css::uno::UNO_QUERY_THROW);
    Result result;
    // Shifted to main thread to work around potential deadlocks (i112867):
    css::awt::AsyncCallback::create(
        connection_.getComponentContext())->addCallback(
            new Callback(
                disp, url, css::uno::Sequence< css::beans::PropertyValue >(),
                new Listener(&result)),
            css::uno::Any());
    // Wait for result.condition or connection_ going stale:
    for (;;) {
        osl::Condition::Result res = result.condition.wait(std::chrono::seconds(1)); // 1 sec delay
        if (res == osl::Condition::result_ok) {
            break;
        }
        CPPUNIT_ASSERT_EQUAL(osl::Condition::result_timeout, res);
        CPPUNIT_ASSERT(connection_.isStillAlive());
    }
    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(OUString(), result.result);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
