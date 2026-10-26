/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/XDiagram.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/UndoContextNotClosedException.hpp>
#include <com/sun/star/document/EmptyUndoStackException.hpp>
#include <com/sun/star/document/UndoFailedException.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/document/XUndoAction.hpp>
#include <com/sun/star/document/XUndoManagerListener.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/NotLockedException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <salhelper/thread.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <condition_variable>
#include <mutex>
#include <stack>

using namespace ::cpo::uno;
using namespace css::accessibility;
using namespace css::awt;
using namespace css::beans;
using namespace css::chart;
using namespace css::chart2;
using namespace css::container;
using namespace css::document;
using namespace css::drawing;
using namespace css::frame;
using namespace css::embed;
using namespace css::lang;
using namespace css::script;
using namespace css::sheet;
using namespace css::table;
using namespace css::task;
using namespace css::text;
using namespace css::util;
using namespace css::view;

#include "DocumentTest.hxx"
#include "DocumentTestBase.hxx"
#include "WriterDocumentTest.hxx"
#include "CalcDocumentTest.hxx"
#include "DrawingOrPresentationDocumentTest.hxx"
#include "DrawDocumentTest.hxx"
#include "ImpressDocumentTest.hxx"
#include "ChartDocumentTest.hxx"

DocumentTest::~DocumentTest() {}

namespace
{
class UndoListener;
class CallbackComponentFactory;

class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/sfx2/qa/complex/data/"_ustr)
    {
    }
    virtual void setUp() override;
    virtual void tearDown() override;

    void impl_checkUndo();
    Reference<XUndoManager> getUndoManager();
    void impl_setupBrokenBasicScript();
    Reference<XPropertySet> impl_setupButton();
    void impl_assignScript(const Reference<XPropertySet>& i_controlModel,
                           const OUString& i_interfaceName, const OUString& i_interfaceMethod,
                           const OUString& i_scriptURI);
    Reference<XMultiServiceFactory> getORB();
    void impl_clickButton(const Reference<XPropertySet>& i_buttonModel);
    void impl_testSingleModification(const Reference<XUndoManager>& i_undoManager);
    void impl_testMultipleModifications(const Reference<XUndoManager>& i_undoManager);
    void impl_testCustomUndoActions(const Reference<XUndoManager>& i_undoManager);
    void impl_testLocking(const Reference<XUndoManager>& i_undoManager);
    void impl_testNestedContexts(const Reference<XUndoManager>& i_undoManager);
    void impl_testErrorHandling(const Reference<XUndoManager>& i_undoManager);
    void impl_testContextHandling(const Reference<XUndoManager>& i_undoManager);
    void impl_testStackHandling(const Reference<XUndoManager>& i_undoManager);
    void impl_testClearance(const Reference<XUndoManager>& i_undoManager);
    void impl_testHiddenContexts(const Reference<XUndoManager>& i_undoManager);

    std::unique_ptr<DocumentTest> m_currentTestCase;
    rtl::Reference<UndoListener> m_undoListener;
    rtl::Reference<CallbackComponentFactory> m_callbackFactory;
    bool m_callbackCalled = false;
    bool m_closeAfterCallback = false;
    std::mutex m_callbackCondition;
    std::condition_variable m_callbackConditionCondition;
};

class UndoListener : public cppu::WeakImplHelper<XUndoManagerListener>
{
public:
    virtual void undoActionAdded(const UndoManagerEvent& i_event) override
    {
        CPPUNIT_ASSERT_MESSAGE("|undoActionAdded| called after document was disposed",
                               !m_isDisposed);

        ++m_undoActionsAdded;
        m_mostRecentlyAddedAction = i_event.UndoActionTitle;
    }

    virtual void actionUndone(const UndoManagerEvent& i_event) override
    {
        CPPUNIT_ASSERT_MESSAGE("|actionUndone| called after document was disposed", !m_isDisposed);

        ++m_undoCount;
        m_mostRecentlyUndone = i_event.UndoActionTitle;
    }

    virtual void actionRedone(const UndoManagerEvent&) override
    {
        CPPUNIT_ASSERT_MESSAGE("|actionRedone| called after document was disposed", !m_isDisposed);

        ++m_redoCount;
    }

    virtual void allActionsCleared(const css::lang::EventObject&) override
    {
        CPPUNIT_ASSERT_MESSAGE("|allActionsCleared| called after document was disposed",
                               !m_isDisposed);

        m_wasCleared = true;
    }

    virtual void redoActionsCleared(const css::lang::EventObject&) override
    {
        CPPUNIT_ASSERT_MESSAGE("|redoActionsCleared| called after document was disposed",
                               !m_isDisposed);

        m_redoWasCleared = true;
    }

    virtual void resetAll(const css::lang::EventObject&) override
    {
        CPPUNIT_ASSERT_MESSAGE("|resetAll| called after document was disposed", !m_isDisposed);

        m_managerWasReset = true;
        m_activeUndoContexts.clear();
    }

    virtual void enteredContext(const UndoManagerEvent& i_event) override
    {
        CPPUNIT_ASSERT_MESSAGE("|enteredContext| called after document was disposed",
                               !m_isDisposed);

        m_activeUndoContexts.push_back(i_event.UndoActionTitle);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "different opinions on the context nesting level (after entering)",
            sal_Int32(m_activeUndoContexts.size()), i_event.UndoContextDepth);
    }

    virtual void enteredHiddenContext(const UndoManagerEvent& i_event) override
    {
        CPPUNIT_ASSERT_MESSAGE("|enteredHiddenContext| called after document was disposed",
                               !m_isDisposed);

        m_activeUndoContexts.push_back(i_event.UndoActionTitle);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "different opinions on the context nesting level (after entering hidden)",
            sal_Int32(m_activeUndoContexts.size()), i_event.UndoContextDepth);
    }

    virtual void leftContext(const UndoManagerEvent& i_event) override
    {
        CPPUNIT_ASSERT_MESSAGE("|leftContext| called after document was disposed", !m_isDisposed);

        OUString undo = m_activeUndoContexts.back();
        m_activeUndoContexts.pop_back();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("nested undo context descriptions do not match", undo,
                                     i_event.UndoActionTitle);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "different opinions on the context nesting level (after leaving)",
            sal_Int32(m_activeUndoContexts.size()), i_event.UndoContextDepth);
        m_leftContext = true;
        impl_notifyContextDepth();
    }

    virtual void leftHiddenContext(const UndoManagerEvent& i_event) override
    {
        CPPUNIT_ASSERT_MESSAGE("|leftHiddenContext| called after document was disposed",
                               !m_isDisposed);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "|leftHiddenContext| is not expected to notify an action title", sal_Int32(0),
            i_event.UndoActionTitle.getLength());

        m_activeUndoContexts.pop_back();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "different opinions on the context nesting level (after leaving)",
            sal_Int32(m_activeUndoContexts.size()), i_event.UndoContextDepth);
        m_leftHiddenContext = true;
        impl_notifyContextDepth();
    }

    virtual void cancelledContext(const UndoManagerEvent& i_event) override
    {
        CPPUNIT_ASSERT_MESSAGE("|cancelledContext| called after document was disposed",
                               !m_isDisposed);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("|cancelledContext| is not expected to notify an action title",
                                     sal_Int32(0), i_event.UndoActionTitle.getLength());

        m_activeUndoContexts.pop_back();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "different opinions on the context nesting level (after cancelling)",
            sal_Int32(m_activeUndoContexts.size()), i_event.UndoContextDepth);
        m_cancelledContext = true;
        impl_notifyContextDepth();
    }

    virtual void disposing(const css::lang::EventObject&) override { m_isDisposed = true; }

    virtual void waitForAllContextsClosed(int i_milliSeconds)
    {
        std::unique_lock l(m_allContextsClosedCondition);
        if (m_activeUndoContexts.empty())
            return;
        m_allContextsClosedConditionVariable.wait_for(l, std::chrono::milliseconds(i_milliSeconds),
                                                      [this]
                                                      { return m_activeUndoContexts.empty(); });
    }

    OUString getCurrentUndoContextTitle() { return m_activeUndoContexts.back(); }
    sal_Int32 getCurrentUndoContextDepth() { return m_activeUndoContexts.size(); }
    bool isDisposed() { return m_isDisposed; }
    int getUndoActionCount() { return m_undoCount; }
    int getRedoActionCount() { return m_redoCount; }

    int getUndoActionsAdded() { return m_undoActionsAdded; }
    OUString getMostRecentlyAddedActionTitle() { return m_mostRecentlyAddedAction; }
    OUString getMostRecentlyUndoneTitle() { return m_mostRecentlyUndone; }
    bool wasContextLeft() { return m_leftContext; }
    bool wasHiddenContextLeft() { return m_leftHiddenContext; }
    bool hasContextBeenCancelled() { return m_cancelledContext; }
    bool wereStacksCleared() { return m_wasCleared; }
    bool wasRedoStackCleared() { return m_redoWasCleared; }
    bool wasManagerReset() { return m_managerWasReset; }

    void reset()
    {
        m_undoActionsAdded = m_undoCount = m_redoCount = 0;
        m_activeUndoContexts.clear();
        m_mostRecentlyAddedAction = m_mostRecentlyUndone = u""_ustr;
        // m_isDisposed is not cleared, intentionally
        m_leftContext = m_leftHiddenContext = m_cancelledContext = m_wasCleared = m_redoWasCleared
            = m_managerWasReset = false;
    }

private:
    void impl_notifyContextDepth()
    {
        bool bNotify;
        {
            std::unique_lock l(m_allContextsClosedCondition);
            bNotify = m_activeUndoContexts.empty();
        }
        if (bNotify)
            m_allContextsClosedConditionVariable.notify_all();
    }

    int m_undoActionsAdded = 0;
    int m_undoCount = 0;
    int m_redoCount = 0;
    bool m_isDisposed = false;
    bool m_leftContext = false;
    bool m_leftHiddenContext = false;
    bool m_cancelledContext = false;
    bool m_wasCleared = false;
    bool m_redoWasCleared = false;
    bool m_managerWasReset = false;
    std::vector<OUString> m_activeUndoContexts;
    OUString m_mostRecentlyAddedAction;
    OUString m_mostRecentlyUndone;
    std::mutex m_allContextsClosedCondition;
    std::condition_variable m_allContextsClosedConditionVariable;
};

OUString getCallbackUndoContextTitle() { return u"Some Unfinished Undo Context"_ustr; }

OUString getCallbackComponentServiceName() { return u"org.openoffice.complex.sfx2.Callback"_ustr; }

class CallbackComponent : public cppu::WeakImplHelper<XJob>
{
public:
    CallbackComponent(Test& test)
        : mrTest(test)
    {
    }

    virtual Any execute(const Sequence<NamedValue>&) override
    {
        // this method is called from within the Basic script which is to check whether the OOo framework
        // properly cleans up unfinished Undo contexts. It is called immediately after the context has been
        // entered, so verify the expected Undo manager state.
        CPPUNIT_ASSERT_EQUAL(getCallbackUndoContextTitle(),
                             mrTest.m_undoListener->getCurrentUndoContextTitle());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), mrTest.m_undoListener->getCurrentUndoContextDepth());

        {
            std::unique_lock l(mrTest.m_callbackCondition);
            mrTest.m_callbackCalled = true;
        }
        mrTest.m_callbackConditionCondition.notify_all();
        return Any(mrTest.m_closeAfterCallback ? u"close"_ustr : u""_ustr);
    }

    virtual Sequence<sal_Int8> getImplementationId() override { return {}; }

private:
    Test& mrTest;
};

/**
 * a factory for a callback component which, at OOo runtime, is inserted into OOo's "component repository"
 */
class CallbackComponentFactory
    : public cppu::WeakImplHelper<XSingleComponentFactory, XServiceInfo, XComponent>
{
public:
    CallbackComponentFactory(Test& test)
        : mrTest(test)
    {
    }

    virtual Reference<XInterface>
    createInstanceWithContext(const Reference<XComponentContext>&) override
    {
        return static_cast<cppu::OWeakObject*>(new CallbackComponent(mrTest));
    }

    virtual Reference<XInterface>
    createInstanceWithArgumentsAndContext(const Sequence<Any>&,
                                          const Reference<XComponentContext>& i_context) override
    {
        return createInstanceWithContext(i_context);
    }

    virtual OUString getImplementationName() override
    {
        return u"org.openoffice.complex.sfx2.CallbackComponent"_ustr;
    }

    virtual bool supportsService(const OUString& i_serviceName) override
    {
        return i_serviceName == getCallbackComponentServiceName();
    }

    virtual Sequence<OUString> getSupportedServiceNames() override
    {
        return { getCallbackComponentServiceName() };
    }

    virtual void dispose() override
    {
        css::lang::EventObject event(static_cast<cppu::OWeakObject*>(this));

        std::vector<Reference<css::lang::XEventListener>> eventListenersCopy(m_eventListeners);
        for (const auto& i : eventListenersCopy)
            i->disposing(event);
    }

    virtual void addEventListener(const Reference<css::lang::XEventListener>& i_listener) override
    {
        if (i_listener != nullptr)
            m_eventListeners.push_back(i_listener);
    }

    virtual void
    removeEventListener(const Reference<css::lang::XEventListener>& i_listener) override
    {
        m_eventListeners.erase(
            std::remove(m_eventListeners.begin(), m_eventListeners.end(), i_listener),
            m_eventListeners.end());
    }

private:
    Test& mrTest;
    std::vector<Reference<css::lang::XEventListener>> m_eventListeners;
};

class CountingUndoAction : public cppu::WeakImplHelper<XUndoAction>
{
public:
    CountingUndoAction(sal_Int32 i_expectedOrder, std::mutex& rLock,
                       sal_Int32& i_actionsUndoneCounter)
        : m_expectedOrder(i_expectedOrder)
        , m_rLock(rLock)
        , m_rActionsUndoneCounter(i_actionsUndoneCounter)
    {
    }

    virtual OUString getTitle() override { return u"Counting Undo Action"_ustr; }

    virtual void undo() override
    {
        std::unique_lock l(m_rLock);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Undo action called out of order", m_expectedOrder,
                                     m_rActionsUndoneCounter);
        ++m_rActionsUndoneCounter;
    }

    virtual void redo() override
    {
        CPPUNIT_FAIL("CountingUndoAction.redo is not expected to be called in this test.");
    }
    sal_Int32 m_expectedOrder;
    std::mutex& m_rLock;
    sal_Int32& m_rActionsUndoneCounter;
};

class CustomUndoAction : public cppu::WeakImplHelper<XUndoAction, XComponent>
{
public:
    CustomUndoAction() { m_title = u"Custom Undo Action"_ustr; }

    CustomUndoAction(const OUString& i_title) { m_title = i_title; }

    virtual OUString getTitle() override { return m_title; }

    virtual void undo() override { m_undoCalled = true; }

    virtual void redo() override {}

    virtual void dispose() override { m_disposed = true; }

    virtual void addEventListener(const Reference<css::lang::XEventListener>&) override
    {
        CPPUNIT_FAIL("addEventListener is not expected to be called in the course of this test");
    }

    virtual void removeEventListener(const Reference<css::lang::XEventListener>&) override
    {
        CPPUNIT_FAIL("removeEventListener is not expected to be called in the course of this test");
    }

    bool undoCalled() { return m_undoCalled; }
    bool disposed() { return m_disposed; }

private:
    OUString m_title;
    bool m_undoCalled = false;
    bool m_disposed = false;
};

constexpr sal_Int16 FAIL_UNDO = 1;
constexpr sal_Int16 FAIL_REDO = 2;

class FailingUndoAction : public cppu::WeakImplHelper<XUndoAction>
{
public:
    FailingUndoAction(sal_Int16 i_failWhich) { m_failWhich = i_failWhich; }

    virtual OUString getTitle() override { return u"failing undo"_ustr; }

    virtual void undo() override
    {
        if (m_failWhich != FAIL_REDO)
            impl_throw();
    }

    virtual void redo() override
    {
        if (m_failWhich != FAIL_UNDO)
            impl_throw();
    }

private:
    static void impl_throw() { throw UndoFailedException(); }

    sal_Int16 m_failWhich;
};

void Test::setUp()
{
    UnoApiTest::setUp();

    m_currentTestCase = nullptr;
    m_undoListener = nullptr;

    // at our service factory, insert a new factory for our CallbackComponent
    // this will allow the Basic code in our test documents to call back into this test case
    // here, by just instantiating this service
    Reference<XSet> globalFactory(comphelper::getProcessComponentContext()->getServiceManager(), UNO_QUERY);
    m_callbackFactory = new CallbackComponentFactory(*this);
    globalFactory->insert(Any(Reference<XSingleComponentFactory>(m_callbackFactory)));
}

void Test::tearDown()
{
    m_currentTestCase = nullptr;
    m_callbackFactory->dispose();
    m_callbackFactory = nullptr;
    UnoApiTest::tearDown();
}

CPPUNIT_TEST_FIXTURE(Test, checkWriterUndo)
{
    loadFromURL(u"private:factory/swriter"_ustr);
    m_currentTestCase = std::make_unique<WriterDocumentTest>(mxComponent);
    impl_checkUndo();
}

//FIXME fails fdo#35663   @Test
//CPPUNIT_TEST_FIXTURE(Test, checkCalcUndo)
//{
//    loadFromURL(u"private:factory/scalc"_ustr);
//    m_currentTestCase = std::make_unique<CalcDocumentTest>(mxComponent);
//    impl_checkUndo();
//}

CPPUNIT_TEST_FIXTURE(Test, checkDrawUndo)
{
    loadFromURL(u"private:factory/sdraw"_ustr);
    m_currentTestCase = std::make_unique<DrawDocumentTest>(mxComponent);
    impl_checkUndo();
}

CPPUNIT_TEST_FIXTURE(Test, checkImpressUndo)
{
    loadFromURL(u"private:factory/simpress"_ustr);
    m_currentTestCase = std::make_unique<ImpressDocumentTest>(mxComponent);
    impl_checkUndo();
}

// This currently fails with css::embed::WrongStateException down in the embedded code
// trying to retrieve some menu configuration data ???
//CPPUNIT_TEST_FIXTURE(Test, checkChartUndo)
//{
//    loadFromURL(u"private:factory/swriter"_ustr);
//    m_currentTestCase = std::make_unique<ChartDocumentTest>(mxComponent);
//    impl_checkUndo();
//}

CPPUNIT_TEST_FIXTURE(Test, checkBrokenScripts)
{
    loadFromURL(u"private:factory/scalc"_ustr);
    m_undoListener = new UndoListener;
    getUndoManager()->addUndoManagerListener(m_undoListener);

    impl_setupBrokenBasicScript();
    OUString scriptURI
        = u"vnd.sun.star.script:default.callbacks.brokenScript?language=Basic&location=document"_ustr;

    // scenario 1: Pressing a button which is bound to execute the script
    // (This is one of the many cases where SfxObjectShell::CallXScript is invoked)

    // set up the button
    Reference<XPropertySet> buttonModel = impl_setupButton();
    buttonModel->setPropertyValue(u"Label"_ustr, Any(u"exec broken script"_ustr));
    impl_assignScript(buttonModel, u"XActionListener"_ustr, u"actionPerformed"_ustr, scriptURI);

    // switch the doc's view to form alive mode (so the button will actually work)
    dispatchCommand(mxComponent, u".uno:SwitchControlDesignMode"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // click the button
    m_callbackCalled = false;
    impl_clickButton(buttonModel);
    // the macro is executed asynchronously by the button, so wait at most 2 seconds for the callback to be
    // triggered
    {
        // we need to yield or the delayed event will not get processed
        for (int i=0; i<20; i++)
        {
            Scheduler::ProcessEventsToIdle();
            if (m_callbackCalled)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (m_callbackCalled)
                break;
        }
    }
    // check the callback has actually been called
    CPPUNIT_ASSERT_MESSAGE(
        "clicking the test button did not work as expected - basic script not called",
        m_callbackCalled);

    // again, since the script is executed asynchronously, we might arrive here while its execution
    // is not completely finished. Give OOo another (at most) 2 seconds to finish it.
    m_undoListener->waitForAllContextsClosed(20000);
    // assure that the Undo Context Depth of the doc is still "0": The Basic script entered such a
    // context, and didn't close it (thus it is broken), but the application framework should have
    // auto-closed the context after the macro finished.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo context was not auto-closed as expected", sal_Int32(0),
                                 m_undoListener->getCurrentUndoContextDepth());

    // scenario 2: dispatching the script URL. Technically, this is equivalent to configuring the
    // script into a menu or toolbar, and selecting the respective menu/toolbar item
    m_callbackCalled = false;
    dispatchCommand(mxComponent, scriptURI, {});
    CPPUNIT_ASSERT_MESSAGE(
        "dispatching the Script URL did not work as expected - basic script not called",
        m_callbackCalled);
    // same as above: The script didn't close the context, but the OOo framework should have
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo context was not auto-closed as expected", sal_Int32(0),
                                 m_undoListener->getCurrentUndoContextDepth());

    // scenario 3: assigning the script to some document event, and triggering this event
    Reference<XEventsSupplier> eventSupplier(mxComponent, UNO_QUERY);
    Reference<XNameReplace> events(eventSupplier->getEvents());
    Sequence<NamedValue> scriptDescriptor{ { u"EventType"_ustr, Any(u"Script"_ustr) },
                                           { u"Script"_ustr, Any(scriptURI) } };
    events->replaceByName(u"OnViewCreated"_ustr, Any(scriptDescriptor));

    // note: this may be prevented from working by setting
    // Office::Common::Security::Scripting::AllowedDocumentEventURLs
    // (checked in SfxEvents_Impl::isScriptURLAllowed())
    m_callbackCalled = false;
    dispatchCommand(mxComponent, u".uno:NewWindow"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("triggering an event did not work as expected - basic script not called",
                           m_callbackCalled);
    // same as above: The script didn't close the context, but the OOo framework should have
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo context was not auto-closed as expected", sal_Int32(0),
                                 m_undoListener->getCurrentUndoContextDepth());

    // scenario 4: let the script enter an Undo context, but not close it, as usual.
    // Additionally, let the script close the document - the OOo framework code which cares for
    // auto-closing of Undo contexts should survive this, ideally ...
    m_closeAfterCallback = true;
    m_callbackCalled = false;
    dispatchCommand(mxComponent, scriptURI, {});
    CPPUNIT_ASSERT(m_callbackCalled);
    CPPUNIT_ASSERT_MESSAGE("The Basic script should have closed the document.",
                           m_undoListener->isDisposed());
}

constexpr int actionsPerThread = 10;

class CheckSerializationThread : public salhelper::Thread
{
public:
    CheckSerializationThread(const Reference<XUndoManager>& undoManager)
        : salhelper::Thread("CheckSerializationThread")
        , m_undoManager(undoManager)
    {
    }

    virtual void execute() override
    {
        for (int j = 0; j < actionsPerThread; ++j)
        {
            try
            {
                m_undoManager->undo();
            }
            catch (Exception)
            {
                CPPUNIT_FAIL("Those dummy actions are not expected to fail.");
            }
        }
    }

private:
    Reference<XUndoManager> m_undoManager;
};

CPPUNIT_TEST_FIXTURE(Test, checkSerialization)
{
    loadFromURL(u"private:factory/scalc"_ustr);
    Reference<XUndoManager> undoManager = getUndoManager();

    const sal_Int32 threadCount = 10;
    const sal_Int32 actionCount = threadCount * actionsPerThread;

    // add some actions to the UndoManager, each knowing its position on the stack
    std::mutex lock;
    sal_Int32 actionsUndone = 0;
    for (int i = actionCount; i > 0;)
        undoManager->addUndoAction(new CountingUndoAction(--i, lock, actionsUndone));

    // some concurrent threads which undo the actions
    std::vector<rtl::Reference<CheckSerializationThread>> threads(threadCount);
    for (sal_Int32 i = 0; i < threadCount; ++i)
        threads[i] = new CheckSerializationThread(undoManager);

    {
        // drop the solar mutex so the threads can execute
        SolarMutexReleaser releaser;

        // start the threads
        for (sal_Int32 i = 0; i < threadCount; ++i)
            threads[i]->launch();

        // wait for them to be finished
        for (sal_Int32 i = 0; i < threadCount; ++i)
            threads[i]->join();
    }

    // ensure all actions have been undone
    CPPUNIT_ASSERT_EQUAL_MESSAGE("not all actions have been undone", actionCount, actionsUndone);
}

/**
 * @return returns the undo manager belonging to a given document
 */
Reference<XUndoManager> Test::getUndoManager()
{
    Reference<XUndoManagerSupplier> suppUndo(mxComponent, UNO_QUERY);
    Reference<XUndoManager> undoManager = suppUndo->getUndoManager();
    CPPUNIT_ASSERT(undoManager->getParent() == mxComponent);
    return undoManager;
}

void Test::impl_setupBrokenBasicScript()
{
    try
    {
        Reference<XEmbeddedScripts> embeddedScripts(mxComponent, UNO_QUERY);
        Reference<XLibraryContainer> basicLibs = embeddedScripts->getBasicLibraries();
        Reference<XNameContainer> basicLib = basicLibs->createLibrary(u"default"_ustr);

        OUString brokenScriptCode = "Option Explicit\n"
                                    "\n"
                                    "Sub brokenScript\n"
                                    "    Dim callback as Object\n"
                                    "    ThisComponent.UndoManager.enterUndoContext( \""
                                    + getCallbackUndoContextTitle()
                                    + "\" )\n"
                                      "\n"
                                      "    callback = createUnoService( \""
                                    + getCallbackComponentServiceName()
                                    + "\" )\n"
                                      "    Dim emptyArgs() as new com.sun.star.beans.NamedValue\n"
                                      "    Dim result as String\n"
                                      "    result = callback.execute( emptyArgs() )\n"
                                      "    If result = \"close\" Then\n"
                                      "        ThisComponent.close( TRUE )\n"
                                      "    End If\n"
                                      "End Sub\n"
                                      "\n";

        basicLib->insertByName(u"callbacks"_ustr, Any(brokenScriptCode));
    }
    catch (Exception)
    {
        CPPUNIT_FAIL("caught an exception while setting up the script");
    }
}

Reference<XPropertySet> Test::impl_setupButton()
{
    // let the document create a shape
    Reference<XMultiServiceFactory> docAsFactory(mxComponent, UNO_QUERY);
    Reference<XControlShape> xShape(
        docAsFactory->createInstance(u"com.sun.star.drawing.ControlShape"_ustr), UNO_QUERY);

    // position and size of the shape
    xShape->setSize(css::awt::Size(28 * 100, 10 * 100));
    xShape->setPosition(css::awt::Point(10 * 100, 10 * 100));

    // create the form component (the model of a form control)
    Reference<XControlModel> controlModel(
        getORB()->createInstance(u"com.sun.star.form.component.CommandButton"_ustr), UNO_QUERY);

    // knitt both
    xShape->setControl(controlModel);

    // add the shape to the shapes collection of the document
    Reference<XSpreadsheetDocument> spreadsheetDoc(mxComponent, UNO_QUERY);
    Reference<XIndexAccess> sheets(spreadsheetDoc->getSheets(), UNO_QUERY);
    Reference<XDrawPageSupplier> suppDrawPage(sheets->getByIndex(0), UNO_QUERY);
    Reference<XDrawPage> insertIntoPage = suppDrawPage->getDrawPage();

    Reference<XShapes> sheetShapes(insertIntoPage);
    sheetShapes->add(xShape);

    return Reference<XPropertySet>(controlModel, UNO_QUERY);
}

void Test::impl_assignScript(const Reference<XPropertySet>& i_controlModel,
                             const OUString& i_interfaceName, const OUString& i_interfaceMethod,
                             const OUString& i_scriptURI)
{
    Reference<XChild> modelAsChild(i_controlModel, UNO_QUERY);
    Reference<XIndexContainer> parentForm(modelAsChild->getParent(), UNO_QUERY);

    Reference<XEventAttacherManager> manager(parentForm, UNO_QUERY);

    int containerPosition = -1;
    for (int i = 0; i < parentForm->getCount(); ++i)
    {
        Reference<XPropertySet> child(parentForm->getByIndex(i), UNO_QUERY);
        if (child == i_controlModel)
        {
            containerPosition = i;
            break;
        }
    }
    CPPUNIT_ASSERT_MESSAGE("could not find the given control model within its parent",
                           containerPosition != -1);
    manager->registerScriptEvent(containerPosition,
                                 ScriptEventDescriptor(i_interfaceName, i_interfaceMethod, u""_ustr,
                                                       u"Script"_ustr, i_scriptURI));
}

void Test::impl_clickButton(const Reference<XPropertySet>& i_buttonModel)
{
    Reference<XModel> xDocModel(mxComponent, UNO_QUERY);
    Reference<XController> xController = xDocModel->getCurrentController();
    Reference<XControlAccess> controlAccess(xController, UNO_QUERY);
    Reference<XControl> control
        = controlAccess->getControl(Reference<XControlModel>(i_buttonModel, UNO_QUERY));
    Reference<XAccessible> accessible(control, UNO_QUERY);
    Reference<XAccessibleAction> controlActions(accessible->getAccessibleContext(), UNO_QUERY);
    for (int i = 0; i < controlActions->getAccessibleActionCount(); ++i)
    {
        if (controlActions->getAccessibleActionDescription(i) == "press")
        {
            controlActions->doAccessibleAction(i);
            return;
        }
    }
    CPPUNIT_FAIL("did not find the accessible action named 'press'");
}

void Test::impl_checkUndo()
{
    m_currentTestCase->initializeDocument();
    m_currentTestCase->verifyInitialDocumentState();

    Reference<XUndoManager> undoManager = getUndoManager();
    undoManager->clear();
    CPPUNIT_ASSERT_MESSAGE(
        "clearing the Undo manager should result in the impossibility to undo anything",
        !undoManager->isUndoPossible());
    CPPUNIT_ASSERT_MESSAGE(
        "clearing the Undo manager should result in the impossibility to redo anything",
        !undoManager->isRedoPossible());

    m_undoListener = new UndoListener;
    undoManager->addUndoManagerListener(m_undoListener);

    impl_testSingleModification(undoManager);
    impl_testMultipleModifications(undoManager);
    impl_testCustomUndoActions(undoManager);
    impl_testLocking(undoManager);
    impl_testNestedContexts(undoManager);
    impl_testErrorHandling(undoManager);
    impl_testContextHandling(undoManager);
    impl_testStackHandling(undoManager);
    impl_testClearance(undoManager);
    impl_testHiddenContexts(undoManager);

    // close the document, ensure the Undo manager listener gets notified
    m_currentTestCase->closeDocument();
    m_currentTestCase = nullptr;
    CPPUNIT_ASSERT_MESSAGE(
        "document is closed, but the UndoManagerListener has not been notified of the disposal",
        m_undoListener->isDisposed());
}

void Test::impl_testSingleModification(const Reference<XUndoManager>& i_undoManager)
{
    m_currentTestCase->doSingleModification();
    m_currentTestCase->verifySingleModificationDocumentState();

    // undo the modification, ensure the listener got the proper notifications
    CPPUNIT_ASSERT_EQUAL_MESSAGE("We did not yet do an undo!", 0,
                                 m_undoListener->getUndoActionCount());
    i_undoManager->undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A simple undo does not result in the proper Undo count.", 1,
                                 m_undoListener->getUndoActionCount());

    // verify the document is in its initial state, again
    m_currentTestCase->verifyInitialDocumentState();

    // redo the modification, ensure the listener got the proper notifications
    CPPUNIT_ASSERT_EQUAL_MESSAGE("not yet made a redo!", 0, m_undoListener->getRedoActionCount());
    i_undoManager->redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("made a redo, but got no notification of it!", 1,
                                 m_undoListener->getRedoActionCount());
    // ensure the document is in the proper state, again
    m_currentTestCase->verifySingleModificationDocumentState();

    // now do an Undo via the UI (aka the dispatch API), and see if this works, and notifies the listener as
    // expected
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    m_currentTestCase->verifyInitialDocumentState();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("UI-Undo does not notify the listener", 2,
                                 m_undoListener->getUndoActionCount());
}

void Test::impl_testMultipleModifications(const Reference<XUndoManager>& i_undoManager)
{
    m_undoListener->reset();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected initial undo context depth", sal_Int32(0),
                                 m_undoListener->getCurrentUndoContextDepth());
    i_undoManager->enterUndoContext(u"Batch Changes"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected undo context depth after entering a context", sal_Int32(1),
                                 m_undoListener->getCurrentUndoContextDepth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("entering an Undo context has not been notified properly",
                                 u"Batch Changes"_ustr,
                                 m_undoListener->getCurrentUndoContextTitle());

    int modifications = m_currentTestCase->doMultipleModifications();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "unexpected number of undo actions while doing batch changes to the document",
        modifications, m_undoListener->getUndoActionsAdded());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("seems the document operations touched the undo context depth", sal_Int32(1),
                                 m_undoListener->getCurrentUndoContextDepth());

    i_undoManager->leaveUndoContext();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected undo context depth after leaving the last context", sal_Int32(0),
                                 m_undoListener->getCurrentUndoContextDepth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "no Undo done, yet - still the listener has been notified of an Undo action", 0,
        m_undoListener->getUndoActionCount());

    i_undoManager->undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Just did an undo - the listener should have been notified", 1,
                                 m_undoListener->getUndoActionCount());
    m_currentTestCase->verifyInitialDocumentState();
}

void Test::impl_testCustomUndoActions(const Reference<XUndoManager>& i_undoManager)
{
    i_undoManager->clear();
    m_undoListener->reset();
    CPPUNIT_ASSERT_MESSAGE("undo stack not empty after clearing the undo manager",
                           !i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT_MESSAGE("redo stack not empty after clearing the undo manager",
                           !i_undoManager->isRedoPossible());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(">0 descriptions for an empty undo stack?", Sequence<OUString>{},
                                 i_undoManager->getAllUndoActionTitles());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(">0 descriptions for an empty redo stack?", Sequence<OUString>{},
                                 i_undoManager->getAllRedoActionTitles());

    // add two actions, one directly, one within a context
    rtl::Reference<CustomUndoAction> action1 = new CustomUndoAction(u"UndoAction1"_ustr);
    i_undoManager->addUndoAction(action1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Adding an undo action not observed by the listener", 1,
                                 m_undoListener->getUndoActionsAdded());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Adding an undo action did not notify the proper title",
                                 action1->getTitle(),
                                 m_undoListener->getMostRecentlyAddedActionTitle());
    OUString contextTitle = u"Undo Context"_ustr;
    i_undoManager->enterUndoContext(contextTitle);
    rtl::Reference<CustomUndoAction> action2 = new CustomUndoAction(u"UndoAction2"_ustr);
    i_undoManager->addUndoAction(action2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Adding an undo action not observed by the listener", 2,
                                 m_undoListener->getUndoActionsAdded());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Adding an undo action did not notify the proper title",
                                 action2->getTitle(),
                                 m_undoListener->getMostRecentlyAddedActionTitle());
    i_undoManager->leaveUndoContext();

    // see if the manager has proper descriptions
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected Redo descriptions after adding two actions",
                                 Sequence<OUString>{}, i_undoManager->getAllRedoActionTitles());
    Sequence<OUString> redoDescriptions{ contextTitle, action1->getTitle() };
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected Undo descriptions after adding two actions",
                                 redoDescriptions, i_undoManager->getAllUndoActionTitles());

    // undo one action
    i_undoManager->undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("improper action title notified during programmatic Undo",
                                 contextTitle, m_undoListener->getMostRecentlyUndoneTitle());
    CPPUNIT_ASSERT_MESSAGE("nested custom undo action has not been undone as expected",
                           action2->undoCalled());
    CPPUNIT_ASSERT_MESSAGE("nested custom undo action has not been undone as expected",
                           !action1->undoCalled());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "unexpected Redo descriptions after undoing a nested custom action",
        Sequence<OUString>{ contextTitle }, i_undoManager->getAllRedoActionTitles());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "unexpected Undo descriptions after undoing a nested custom action",
        Sequence<OUString>{ action1->getTitle() }, i_undoManager->getAllUndoActionTitles());

    // undo the second action, via UI dispatches
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL_MESSAGE("improper action title notified during UI Undo",
                                 action1->getTitle(), m_undoListener->getMostRecentlyUndoneTitle());
    CPPUNIT_ASSERT_MESSAGE("nested custom undo action has not been undone as expected",
                           action1->undoCalled());
    Sequence<OUString> redoDescriptions2{ action1->getTitle(), contextTitle };
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "unexpected Redo descriptions after undoing the second custom action", redoDescriptions2,
        i_undoManager->getAllRedoActionTitles());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "unexpected Undo descriptions after undoing the second custom action", Sequence<OUString>{},
        i_undoManager->getAllUndoActionTitles());

    // check the actions are disposed when the stacks are cleared
    i_undoManager->clear();
    CPPUNIT_ASSERT(action1->disposed());
    CPPUNIT_ASSERT(action2->disposed());
}

void Test::impl_testLocking(const Reference<XUndoManager>& i_undoManager)
{
    i_undoManager->reset();
    m_undoListener->reset();

    // implicit Undo actions, triggered by changes to the document
    CPPUNIT_ASSERT_MESSAGE("unexpected initial locking state", !i_undoManager->isLocked());
    i_undoManager->lock();
    CPPUNIT_ASSERT_MESSAGE("just locked the manager, why does it lie?", i_undoManager->isLocked());
    m_currentTestCase->doSingleModification();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "when the Undo manager is locked, no implicit additions should happen", 0,
        m_undoListener->getUndoActionsAdded());
    CPPUNIT_ASSERT_MESSAGE(
        "Undo manager gets unlocked as a side effect of performing a simple operation",
        i_undoManager->isLocked());
    i_undoManager->unlock();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "unlock is not expected to add collected actions - they should be discarded", 0,
        m_undoListener->getUndoActionsAdded());
    CPPUNIT_ASSERT_MESSAGE("just unlocked the manager, why does it lie?",
                           !i_undoManager->isLocked());

    // explicit Undo actions
    i_undoManager->lock();
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->unlock();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "explicit Undo actions are expected to be ignored when the manager is locked", 0,
        m_undoListener->getUndoActionsAdded());

    // Undo contexts while being locked
    i_undoManager->lock();
    i_undoManager->enterUndoContext(u"Dummy Context"_ustr);
    i_undoManager->enterHiddenUndoContext();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "entering Undo contexts should be ignored when the manager is locked", sal_Int32(0),
        m_undoListener->getCurrentUndoContextDepth());
    i_undoManager->leaveUndoContext();
    i_undoManager->leaveUndoContext();
    i_undoManager->unlock();

    // |unlock| error handling
    CPPUNIT_ASSERT_MESSAGE("internal error: manager should not be locked at this point in time",
                           !i_undoManager->isLocked());
    bool caughtExpected = false;
    try
    {
        i_undoManager->unlock();
    }
    catch (NotLockedException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE("unlocking the manager when it is not locked should throw",
                           caughtExpected);
}

void Test::impl_testContextHandling(const Reference<XUndoManager>& i_undoManager)
{
    // part I: non-empty contexts
    i_undoManager->reset();
    m_undoListener->reset();

    // put one action on the undo and one on the redo stack, as precondition for the following tests
    Reference<XUndoAction> undoAction1 = new CustomUndoAction(u"Undo Action 1"_ustr);
    i_undoManager->addUndoAction(undoAction1);
    Reference<XUndoAction> undoAction2 = new CustomUndoAction(u"Undo Action 2"_ustr);
    i_undoManager->addUndoAction(undoAction2);
    i_undoManager->undo();
    CPPUNIT_ASSERT_MESSAGE("precondition for context handling tests not met (1)",
                           i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT_MESSAGE("precondition for context handling tests not met (2)",
                           i_undoManager->isRedoPossible());
    CPPUNIT_ASSERT_EQUAL(Sequence<OUString>{ undoAction1->getTitle() },
                         i_undoManager->getAllUndoActionTitles());
    CPPUNIT_ASSERT_EQUAL(Sequence<OUString>{ undoAction2->getTitle() },
                         i_undoManager->getAllRedoActionTitles());

    Sequence<OUString> expectedRedoActionComments{ undoAction2->getTitle() };
    CPPUNIT_ASSERT_EQUAL(expectedRedoActionComments, i_undoManager->getAllRedoActionTitles());

    // enter a context
    i_undoManager->enterUndoContext(u"Undo Context"_ustr);
    // this should not (yet) touch the redo stack
    CPPUNIT_ASSERT_EQUAL(expectedRedoActionComments, i_undoManager->getAllRedoActionTitles());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected undo context depth after entering a context",
                                 sal_Int32(1), m_undoListener->getCurrentUndoContextDepth());
    // add a single action
    Reference<XUndoAction> undoAction3 = new CustomUndoAction(u"Undo Action 3"_ustr);
    i_undoManager->addUndoAction(undoAction3);
    // still, the redo stack should be untouched - added at a lower level does not affect it at all
    CPPUNIT_ASSERT_EQUAL(expectedRedoActionComments, i_undoManager->getAllRedoActionTitles());

    // while the context is open, its title should already contribute to the stack, ...
    CPPUNIT_ASSERT_EQUAL(u"Undo Context"_ustr, i_undoManager->getCurrentUndoActionTitle());
    // ... getAllUndo/RedoActionTitles should operate on the top level, not on the level defined by the open
    // context,  ...
    Sequence<OUString> undoActionTitles{ u"Undo Context"_ustr, undoAction1->getTitle() };
    CPPUNIT_ASSERT_EQUAL(undoActionTitles, i_undoManager->getAllUndoActionTitles());
    // ... but Undo and Redo should be impossible as long as the context is open
    CPPUNIT_ASSERT(!i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(!i_undoManager->isRedoPossible());

    // leave the context, check the listener has been notified properly, and the notified context depth is correct
    i_undoManager->leaveUndoContext();
    CPPUNIT_ASSERT(m_undoListener->wasContextLeft());
    CPPUNIT_ASSERT(!m_undoListener->wasHiddenContextLeft());
    CPPUNIT_ASSERT(!m_undoListener->hasContextBeenCancelled());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected undo context depth leaving a non-empty context", sal_Int32(0),
                                 m_undoListener->getCurrentUndoContextDepth());
    // leaving a non-empty context should have cleared the redo stack
    CPPUNIT_ASSERT_EQUAL(Sequence<OUString>{}, i_undoManager->getAllRedoActionTitles());
    CPPUNIT_ASSERT(m_undoListener->wasRedoStackCleared());

    // part II: empty contexts
    i_undoManager->reset();
    m_undoListener->reset();

    // enter a context, leave it immediately without adding an action to it
    i_undoManager->enterUndoContext(u"Undo Context"_ustr);
    i_undoManager->leaveUndoContext();
    CPPUNIT_ASSERT(!m_undoListener->wasContextLeft());
    CPPUNIT_ASSERT(!m_undoListener->wasHiddenContextLeft());
    CPPUNIT_ASSERT(m_undoListener->hasContextBeenCancelled());
    CPPUNIT_ASSERT_MESSAGE(
        "leaving an empty context should silently remove it, and not contribute to the stack",
        !i_undoManager->isUndoPossible());
}

void Test::impl_testNestedContexts(const Reference<XUndoManager>& i_undoManager)
{
    i_undoManager->reset();
    m_undoListener->reset();
    i_undoManager->enterUndoContext(u"context 1"_ustr);
    i_undoManager->enterUndoContext(u"context 1.1"_ustr);
    rtl::Reference<CustomUndoAction> action1 = new CustomUndoAction(u"action 1.1.1"_ustr);
    i_undoManager->addUndoAction(action1);
    i_undoManager->enterUndoContext(u"context 1.1.2"_ustr);
    rtl::Reference<CustomUndoAction> action2 = new CustomUndoAction(u"action 1.1.2.1"_ustr);
    i_undoManager->addUndoAction(action2);
    i_undoManager->leaveUndoContext();
    rtl::Reference<CustomUndoAction> action3 = new CustomUndoAction(u"action 1.1.3"_ustr);
    i_undoManager->addUndoAction(action3);
    i_undoManager->leaveUndoContext();
    i_undoManager->leaveUndoContext();
    rtl::Reference<CustomUndoAction> action4 = new CustomUndoAction(u"action 1.2"_ustr);
    i_undoManager->addUndoAction(action4);

    i_undoManager->undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undoing a single action notifies a wrong title",
                                 action4->getTitle(), m_undoListener->getMostRecentlyUndoneTitle());
    CPPUNIT_ASSERT_MESSAGE("custom Undo not called", action4->undoCalled());
    CPPUNIT_ASSERT_MESSAGE("too many custom Undos called", !action1->undoCalled());
    CPPUNIT_ASSERT_MESSAGE("too many custom Undos called", !action2->undoCalled());
    CPPUNIT_ASSERT_MESSAGE("too many custom Undos called", !action3->undoCalled());
    i_undoManager->undo();
    CPPUNIT_ASSERT_MESSAGE("nested actions not properly undone", action1->undoCalled());
    CPPUNIT_ASSERT_MESSAGE("nested actions not properly undone", action2->undoCalled());
    CPPUNIT_ASSERT_MESSAGE("nested actions not properly undone", action3->undoCalled());
}

void Test::impl_testErrorHandling(const Reference<XUndoManager>& i_undoManager)
{
    i_undoManager->reset();
    m_undoListener->reset();

    // try retrieving the comments for the current Undo/Redo - this should fail
    bool caughtExpected = false;
    try
    {
        i_undoManager->getCurrentUndoActionTitle();
    }
    catch (EmptyUndoStackException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE(
        "trying the title of the current Undo action is expected to fail for an empty stack",
        caughtExpected);

    caughtExpected = false;
    try
    {
        i_undoManager->getCurrentRedoActionTitle();
    }
    catch (EmptyUndoStackException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE(
        "trying the title of the current Redo action is expected to fail for an empty stack",
        caughtExpected);

    caughtExpected = false;
    try
    {
        i_undoManager->undo();
    }
    catch (EmptyUndoStackException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE("undo should throw if no Undo action is on the stack", caughtExpected);

    caughtExpected = false;
    try
    {
        i_undoManager->redo();
    }
    catch (EmptyUndoStackException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE("redo should throw if no Redo action is on the stack", caughtExpected);

    caughtExpected = false;
    try
    {
        i_undoManager->leaveUndoContext();
    }
    catch (InvalidStateException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE("leaveUndoContext should throw if no context is currently open",
                           caughtExpected);

    caughtExpected = false;
    try
    {
        i_undoManager->addUndoAction(nullptr);
    }
    catch (IllegalArgumentException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE("adding a NULL action should be rejected", caughtExpected);

    i_undoManager->reset();
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->undo();
    i_undoManager->enterUndoContext(u"Undo Context"_ustr);
    // those methods should fail when a context is open:
    CPPUNIT_ASSERT_THROW_MESSAGE("should be rejected when there is an open context",
                                 i_undoManager->undo(), UndoContextNotClosedException);
    CPPUNIT_ASSERT_THROW_MESSAGE("should be rejected when there is an open context",
                                 i_undoManager->redo(), UndoContextNotClosedException);
    CPPUNIT_ASSERT_THROW_MESSAGE("should be rejected when there is an open context",
                                 i_undoManager->clear(), UndoContextNotClosedException);
    CPPUNIT_ASSERT_THROW_MESSAGE("should be rejected when there is an open context",
                                 i_undoManager->clearRedo(), UndoContextNotClosedException);
    i_undoManager->leaveUndoContext();

    // try Undo actions which fail in their Undo/Redo
    for (int i = 0; i < 4; ++i)
    {
        const bool undo = (i < 2);
        const bool doByAPI = (i % 2) == 0;

        i_undoManager->reset();
        i_undoManager->addUndoAction(new CustomUndoAction());
        i_undoManager->addUndoAction(new FailingUndoAction(undo ? FAIL_UNDO : FAIL_REDO));
        i_undoManager->addUndoAction(new CustomUndoAction());
        i_undoManager->undo();
        if (!undo)
            i_undoManager->undo();
        // assert preconditions for the below test
        CPPUNIT_ASSERT(i_undoManager->isUndoPossible());
        CPPUNIT_ASSERT(i_undoManager->isRedoPossible());

        bool caughtUndoFailed = false;
        try
        {
            if (undo)
                if (doByAPI)
                    i_undoManager->undo();
                else
                    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
            else if (doByAPI)
                i_undoManager->redo();
            else
                dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
        }
        catch (UndoFailedException)
        {
            caughtUndoFailed = true;
        }
        if (doByAPI)
            CPPUNIT_ASSERT_MESSAGE("Exceptions in XUndoAction.undo should be propagated at the API",
                                   caughtUndoFailed);
        else
            CPPUNIT_ASSERT_MESSAGE("Undo/Redo by UI should not let escape Exceptions",
                                   !caughtUndoFailed);
        if (undo)
        {
            CPPUNIT_ASSERT_MESSAGE("a failing Undo should clear the Undo stack",
                                   !i_undoManager->isUndoPossible());
            CPPUNIT_ASSERT_MESSAGE("a failing Undo should /not/ clear the Redo stack",
                                   i_undoManager->isRedoPossible());
        }
        else
        {
            CPPUNIT_ASSERT_MESSAGE("a failing Redo should /not/ clear the Undo stack",
                                   i_undoManager->isUndoPossible());
            CPPUNIT_ASSERT_MESSAGE("a failing Redo should clear the Redo stack",
                                   !i_undoManager->isRedoPossible());
        }
    }
}

void Test::impl_testStackHandling(const Reference<XUndoManager>& i_undoManager)
{
    i_undoManager->reset();
    m_undoListener->reset();

    CPPUNIT_ASSERT(!i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(!i_undoManager->isRedoPossible());

    i_undoManager->addUndoAction(new CustomUndoAction());
    CPPUNIT_ASSERT(i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(!i_undoManager->isRedoPossible());
    i_undoManager->addUndoAction(new CustomUndoAction());
    CPPUNIT_ASSERT(i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(!i_undoManager->isRedoPossible());
    i_undoManager->undo();
    CPPUNIT_ASSERT(i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(i_undoManager->isRedoPossible());
    i_undoManager->undo();
    CPPUNIT_ASSERT(!i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(i_undoManager->isRedoPossible());
    i_undoManager->addUndoAction(new CustomUndoAction());
    CPPUNIT_ASSERT(i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT_MESSAGE("adding a new action should have cleared the Redo stack",
                           !i_undoManager->isRedoPossible());
}

void Test::impl_testClearance(const Reference<XUndoManager>& i_undoManager)
{
    i_undoManager->reset();
    m_undoListener->reset();

    // add an action, clear the stack, verify the listener has been called
    i_undoManager->addUndoAction(new CustomUndoAction());
    CPPUNIT_ASSERT_MESSAGE("clearance listener unexpectedly called",
                           !m_undoListener->wereStacksCleared());
    CPPUNIT_ASSERT_MESSAGE("redo-clearance listener unexpectedly called",
                           !m_undoListener->wasRedoStackCleared());
    i_undoManager->clear();
    CPPUNIT_ASSERT_MESSAGE("clearance listener not called as expected",
                           m_undoListener->wereStacksCleared());
    CPPUNIT_ASSERT_MESSAGE("redo-clearance listener unexpectedly called (2)",
                           !m_undoListener->wasRedoStackCleared());

    // ensure the listener is also called if the stack is actually empty at the moment of the call
    m_undoListener->reset();
    CPPUNIT_ASSERT(!i_undoManager->isUndoPossible());
    i_undoManager->clear();
    CPPUNIT_ASSERT_MESSAGE(
        "clearance listener is also expected to be called if the stack was empty before",
        m_undoListener->wereStacksCleared());

    // ensure the proper listeners are called for clearRedo
    m_undoListener->reset();
    i_undoManager->clearRedo();
    CPPUNIT_ASSERT(!m_undoListener->wereStacksCleared());
    CPPUNIT_ASSERT(m_undoListener->wasRedoStackCleared());

    // ensure the redo listener is also called upon implicit redo stack clearance
    m_undoListener->reset();
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->undo();
    CPPUNIT_ASSERT(i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(i_undoManager->isRedoPossible());
    i_undoManager->addUndoAction(new CustomUndoAction());
    CPPUNIT_ASSERT(!i_undoManager->isRedoPossible());
    CPPUNIT_ASSERT_MESSAGE("implicit clearance of the Redo stack does not notify listeners",
                           m_undoListener->wasRedoStackCleared());

    // test resetting the manager
    m_undoListener->reset();
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->undo();
    CPPUNIT_ASSERT(i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(i_undoManager->isRedoPossible());
    i_undoManager->reset();
    CPPUNIT_ASSERT(!i_undoManager->isUndoPossible());
    CPPUNIT_ASSERT(!i_undoManager->isRedoPossible());
    CPPUNIT_ASSERT_MESSAGE("|reset| does not properly notify", m_undoListener->wasManagerReset());

    // resetting the manager, with open undo contexts
    m_undoListener->reset();
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->enterUndoContext(u"Undo Context"_ustr);
    i_undoManager->addUndoAction(new CustomUndoAction());
    i_undoManager->enterHiddenUndoContext();
    i_undoManager->reset();
    CPPUNIT_ASSERT_MESSAGE("|reset| while contexts are open does not properly notify",
                           m_undoListener->wasManagerReset());
    // verify the manager really has the proper context depth now
    i_undoManager->enterUndoContext(u"Undo Context"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("seems that |reset| did not really close the open contexts", sal_Int32(1),
                                 m_undoListener->getCurrentUndoContextDepth());
}

void Test::impl_testHiddenContexts(const Reference<XUndoManager>& i_undoManager)
{
    i_undoManager->reset();
    m_undoListener->reset();
    CPPUNIT_ASSERT_MESSAGE("precondition for testing hidden undo contexts not met",
                           !i_undoManager->isUndoPossible());

    // entering a hidden context should be rejected if the stack is empty
    bool caughtExpected = false;
    try
    {
        i_undoManager->enterHiddenUndoContext();
    }
    catch (EmptyUndoStackException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE("entering hidden contexts should be denied on an empty stack",
                           caughtExpected);

    // but it should be allowed if the context is not empty
    rtl::Reference<CustomUndoAction> undoAction0 = new CustomUndoAction(u"Step 0"_ustr);
    i_undoManager->addUndoAction(undoAction0);
    rtl::Reference<CustomUndoAction> undoAction1 = new CustomUndoAction(u"Step 1"_ustr);
    i_undoManager->addUndoAction(undoAction1);
    i_undoManager->enterHiddenUndoContext();
    rtl::Reference<CustomUndoAction> hiddenUndoAction
        = new CustomUndoAction(u"hidden context action"_ustr);
    i_undoManager->addUndoAction(hiddenUndoAction);
    i_undoManager->leaveUndoContext();
    CPPUNIT_ASSERT_MESSAGE("leaving a hidden should not call |leftUndocontext|",
                           !m_undoListener->wasContextLeft());
    CPPUNIT_ASSERT_MESSAGE("leaving a hidden does not call |leftHiddenUndocontext|",
                           m_undoListener->wasHiddenContextLeft());
    CPPUNIT_ASSERT_MESSAGE("leaving a non-empty hidden context claims to have cancelled it",
                           !m_undoListener->hasContextBeenCancelled());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("leaving a hidden context is not properly notified", sal_Int32(0),
                                 m_undoListener->getCurrentUndoContextDepth());
    Sequence<OUString> undoTitles{ undoAction1->getTitle(), undoAction0->getTitle() };
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected Undo stack after leaving a hidden context", undoTitles,
                                 i_undoManager->getAllUndoActionTitles());

    // and then calling |undo| once should not only undo everything in the hidden context, but also
    // the previous action - but not more
    i_undoManager->undo();
    CPPUNIT_ASSERT_MESSAGE(
        "Undo after leaving a hidden context does not actually undo the context actions",
        hiddenUndoAction->undoCalled());
    CPPUNIT_ASSERT_MESSAGE(
        "Undo after leaving a hidden context does not undo the predecessor action",
        undoAction1->undoCalled());
    CPPUNIT_ASSERT_MESSAGE("Undo after leaving a hidden context undoes too much",
                           !undoAction0->undoCalled());

    // leaving an empty hidden context should call the proper notification method
    m_undoListener->reset();
    i_undoManager->enterHiddenUndoContext();
    i_undoManager->leaveUndoContext();
    CPPUNIT_ASSERT(!m_undoListener->wasContextLeft());
    CPPUNIT_ASSERT(!m_undoListener->wasHiddenContextLeft());
    CPPUNIT_ASSERT(m_undoListener->hasContextBeenCancelled());

    // nesting hidden and normal contexts
    m_undoListener->reset();
    i_undoManager->reset();
    rtl::Reference<CustomUndoAction> action0 = new CustomUndoAction(u"action 0"_ustr);
    i_undoManager->addUndoAction(action0);
    i_undoManager->enterUndoContext(u"context 1"_ustr);
    rtl::Reference<CustomUndoAction> action1 = new CustomUndoAction(u"action 1"_ustr);
    i_undoManager->addUndoAction(action1);
    i_undoManager->enterHiddenUndoContext();
    rtl::Reference<CustomUndoAction> action2 = new CustomUndoAction(u"action 2"_ustr);
    i_undoManager->addUndoAction(action2);
    i_undoManager->enterUndoContext(u"context 2"_ustr);
    // is entering a hidden context rejected even at the nesting level > 0 (the above test was for nesting level == 0)?
    caughtExpected = false;
    try
    {
        i_undoManager->enterHiddenUndoContext();
    }
    catch (EmptyUndoStackException)
    {
        caughtExpected = true;
    }
    CPPUNIT_ASSERT_MESSAGE(
        "at a nesting level > 0, denied hidden contexts does not work as expected", caughtExpected);
    rtl::Reference<CustomUndoAction> action3 = new CustomUndoAction(u"action 3"_ustr);
    i_undoManager->addUndoAction(action3);
    i_undoManager->enterHiddenUndoContext();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("mixed hidden/normal context do are not properly notified", sal_Int32(4),
                                 m_undoListener->getCurrentUndoContextDepth());
    i_undoManager->leaveUndoContext();
    CPPUNIT_ASSERT_MESSAGE("the left context was empty - why wasn't 'cancelled' notified?",
                           m_undoListener->hasContextBeenCancelled());
    CPPUNIT_ASSERT(!m_undoListener->wasContextLeft());
    CPPUNIT_ASSERT(!m_undoListener->wasHiddenContextLeft());
    i_undoManager->leaveUndoContext();
    i_undoManager->leaveUndoContext();
    i_undoManager->leaveUndoContext();
    i_undoManager->undo();
    CPPUNIT_ASSERT_MESSAGE("one action too much has been undone", !action0->undoCalled());
    CPPUNIT_ASSERT(action1->undoCalled());
    CPPUNIT_ASSERT(action2->undoCalled());
    CPPUNIT_ASSERT(action3->undoCalled());
}

Reference<XMultiServiceFactory> Test::getORB()
{
    Reference<XMultiServiceFactory> xMSF1(
        comphelper::getProcessComponentContext()->getServiceManager(), UNO_QUERY);
    return xMSF1;
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
