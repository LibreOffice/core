/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
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
*
************************************************************************/

#include "sal/config.h"

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
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "test/gettestargument.hxx"
#include "test/officeconnection.hxx"
#include "test/oustringostreaminserter.hxx"
#include "test/toabsolutefileurl.hxx"

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
    result.condition.wait();
    CPPUNIT_ASSERT(result.success);
    CPPUNIT_ASSERT_EQUAL(rtl::OUString(), result.result);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
