/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <salhelper/thread.hxx>
#include <vector>

using namespace ::cpo::uno;
using namespace css::document;
using namespace css::lang;
using namespace css::util;

namespace
{

enum class CloseEventType
{
    OnUnload,
    NotifyClosing,
    Disposing
};

class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/sfx2/qa/complex/data/"_ustr)
    {
    }

    void impl_setupDocCloseTest();
    void impl_tearDownDocCloseTest();

    std::mutex maMutex;
    std::vector<CloseEventType> m_observedCloseEvents;
};

class DocumentEventListener : public cppu::WeakImplHelper<XDocumentEventListener>
{
public:
    DocumentEventListener(Test& rTest)
        : m_rTest(rTest)
    {
    }

    virtual void documentEventOccured(const DocumentEvent& i_documentEvent) override
    {
        if (i_documentEvent.EventName == "OnUnload")
        {
            std::unique_lock l(m_rTest.maMutex);
            m_rTest.m_observedCloseEvents.push_back(CloseEventType::OnUnload);
        }
    }

    virtual void disposing(const EventObject&) override
    {
        // not interested in
    }

private:
    Test& m_rTest;
};

class CloseListener : public cppu::WeakImplHelper<XCloseListener>
{
public:
    CloseListener(Test& rTest)
        : m_rTest(rTest)
    {
    }

    virtual void queryClosing(const EventObject&, bool) override
    {
        // not interested in
    }

    virtual void notifyClosing(const EventObject&) override
    {
        std::unique_lock l(m_rTest.maMutex);
        m_rTest.m_observedCloseEvents.push_back(CloseEventType::NotifyClosing);
    }

    virtual void disposing(const EventObject&) override
    {
        // not interested in
    }

private:
    Test& m_rTest;
};

class DocDisposeListener : public cppu::WeakImplHelper<XEventListener>
{
public:
    DocDisposeListener(Test& rTest)
        : m_rTest(rTest)
    {
    }

    virtual void disposing(const EventObject&) override
    {
        std::unique_lock l(m_rTest.maMutex);
        m_rTest.m_observedCloseEvents.push_back(CloseEventType::Disposing);
    }

private:
    Test& m_rTest;
};

/**
 * sets up the environment for a test which checks the behavior upon closing a doc
 */
void Test::impl_setupDocCloseTest()
{
    m_observedCloseEvents.clear();

    Reference<XDocumentEventBroadcaster> docEventBroadcaster(mxComponent, UNO_QUERY);
    docEventBroadcaster->addDocumentEventListener(new DocumentEventListener(*this));

    Reference<XCloseable> docCloseable(mxComponent, UNO_QUERY);
    docCloseable->addCloseListener(new CloseListener(*this));

    mxComponent->addEventListener(new DocDisposeListener(*this));
}

std::ostream& operator<<(std::ostream& rStream, const std::vector<CloseEventType>&)
{
    return rStream;
};

/**
 * sets up the environment for a test which checks the behavior upon closing a doc
 */
void Test::impl_tearDownDocCloseTest()
{
    std::vector<CloseEventType> aExpected{ CloseEventType::OnUnload, CloseEventType::NotifyClosing,
                                           CloseEventType::Disposing };
    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong order of events when closing a doc", aExpected,
                                 m_observedCloseEvents);
}

CPPUNIT_TEST_FIXTURE(Test, testCloseWinEvents)
{
    loadFromURL(u"private:factory/swriter"_ustr);
    impl_setupDocCloseTest();
    dispatchCommand(mxComponent, u".uno:CloseWin"_ustr, {});
    impl_tearDownDocCloseTest();
}

} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
