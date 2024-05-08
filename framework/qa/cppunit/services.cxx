/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/propertyvalue.hxx>
#include <salhelper/thread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/wrkwin.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers framework/source/services/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/framework/qa/cppunit/data/"_ustr)
    {
    }
};

/// Invokes XFrameImpl::loadComponentFromURL() on a thread.
class TestThread : public salhelper::Thread
{
    uno::Reference<frame::XComponentLoader> mxComponentLoader;
    uno::Reference<lang::XComponent>& mrComponent;

public:
    TestThread(const uno::Reference<frame::XComponentLoader>& xComponentLoader,
               uno::Reference<lang::XComponent>& rComponent);
    void execute() override;
};

TestThread::TestThread(const uno::Reference<frame::XComponentLoader>& xComponentLoader,
                       uno::Reference<lang::XComponent>& rComponent)
    : salhelper::Thread("TestThread")
    , mxComponentLoader(xComponentLoader)
    , mrComponent(rComponent)
{
}

void TestThread::execute()
{
    sal_Int32 nSearchFlags = frame::FrameSearchFlag::AUTO;
    uno::Sequence<beans::PropertyValue> aArguments = {
        comphelper::makePropertyValue(u"OnMainThread"_ustr, true),
    };
    // Note how this is invoking loadComponentFromURL() on a frame, not on the desktop, as usual.
    mrComponent = mxComponentLoader->loadComponentFromURL(u"private:factory/swriter"_ustr,
                                                          u"_self"_ustr, nSearchFlags, aArguments);
}

CPPUNIT_TEST_FIXTURE(Test, testLoadComponentFromURL)
{
    // Without the accompanying fix in place, this test would have failed with:
    // thread 1: comphelper::SolarMutex::doRelease end: m_nCount is 1
    // thread 2: vcl::SolarThreadExecutor::execute: before SolarMutexReleaser ctor
    // thread 2: comphelper::SolarMutex::doRelease start: m_nCount is 1, bUnlockAll is 1
    // thread 2: comphelper::SolarMutex::doRelease: failed IsCurrentThread() check, will abort
    // Notice how thread 2 attempts to release the solar mutex while thread 1 holds it.

    // Create a default window, so by the time the thread would post a user event, it doesn't need
    // the solar mutex to process a SendMessageW() call on Windows.
    ScopedVclPtrInstance<WorkWindow> xWindow(nullptr, WB_APP | WB_STDWORK);
    // Variable is not used, it holds the default window.
    (void)xWindow;

    rtl::Reference<TestThread> xThread;
    {
        // Start the thread that will load the component, but hold the solar mutex for now, so we
        // can see if it blocks.
        SolarMutexGuard guard;
        uno::Reference<frame::XFrame> xFrame
            = mxDesktop->findFrame(u"_blank"_ustr, /*nSearchFlags=*/0);
        uno::Reference<frame::XComponentLoader> xComponentLoader(xFrame, uno::UNO_QUERY);
        xThread = new TestThread(xComponentLoader, mxComponent);
        xThread->launch();
        // If loadComponentFromURL() doesn't lock the solar mutex, the test will abort here.
        osl::Thread::wait(std::chrono::seconds(1));
    }
    {
        // Now release the solar mutex, so the thread can post the task on the main loop.
        SolarMutexReleaser releaser;
        osl::Thread::wait(std::chrono::seconds(1));
    }
    {
        // Spin the main loop.
        SolarMutexGuard guard;
        Scheduler::ProcessEventsToIdle();
    }
    {
        // Stop the thread.
        SolarMutexReleaser releaser;
        xThread->join();
    }
}

CPPUNIT_TEST_FIXTURE(Test, testURLTransformer_parseSmart)
{
    // Without the accompanying fix in place, this test would have failed with
    // "www.example.com:" treated as scheme, "/8080/foo/" as path, "bar?q=baz"
    // as name, and "F" as fragment.

    css::util::URL aURL;
    aURL.Complete = "www.example.com:8080/foo/bar?q=baz#F";
    css::uno::Reference xParser(css::util::URLTransformer::create(mxComponentContext));
    CPPUNIT_ASSERT(xParser->parseSmart(aURL, u"http:"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"http://www.example.com:8080/foo/bar?q=baz#F"_ustr, aURL.Complete);
    CPPUNIT_ASSERT_EQUAL(u"http://www.example.com:8080/foo/bar"_ustr, aURL.Main);
    CPPUNIT_ASSERT_EQUAL(u"http://"_ustr, aURL.Protocol);
    CPPUNIT_ASSERT(aURL.User.isEmpty());
    CPPUNIT_ASSERT(aURL.Password.isEmpty());
    CPPUNIT_ASSERT_EQUAL(u"www.example.com"_ustr, aURL.Server);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(8080), aURL.Port);
    CPPUNIT_ASSERT_EQUAL(u"/foo/"_ustr, aURL.Path);
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, aURL.Name);
    CPPUNIT_ASSERT_EQUAL(u"q=baz"_ustr, aURL.Arguments);
    CPPUNIT_ASSERT_EQUAL(u"F"_ustr, aURL.Mark);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
