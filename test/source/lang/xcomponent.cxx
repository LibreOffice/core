/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/implbase.hxx>
#include <test/lang/xcomponent.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;
namespace
{
struct TestEventListener final : ::cppu::WeakImplHelper<lang::XEventListener>
{
    bool m_hasDisposingCalled;
    TestEventListener()
        : m_hasDisposingCalled(false)
    {
    }
    virtual void SAL_CALL disposing(const lang::EventObject&) override
    {
        m_hasDisposingCalled = true;
    }
};
}

namespace apitest
{
void XComponent::testAddEventListener()
{
    Reference<lang::XComponent> xComponent(init(), uno::UNO_QUERY_THROW);
    auto pListenerAdded(new TestEventListener());
    Reference<lang::XEventListener> xListenerAdded(pListenerAdded);
    xComponent->addEventListener(xListenerAdded);
    xComponent->dispose();
    CPPUNIT_ASSERT_EQUAL(true, pListenerAdded->m_hasDisposingCalled);
}

void XComponent::testRemoveEventListener()
{
    Reference<lang::XComponent> xComponent(init(), uno::UNO_QUERY_THROW);
    auto pListenerAddedAndRemoved(new TestEventListener());
    Reference<lang::XEventListener> xListenerAddedAndRemoved(pListenerAddedAndRemoved);
    xComponent->addEventListener(xListenerAddedAndRemoved);
    xComponent->removeEventListener(xListenerAddedAndRemoved);
    xComponent->dispose();
    CPPUNIT_ASSERT_EQUAL(false, pListenerAddedAndRemoved->m_hasDisposingCalled);
}

void XComponent::testDisposedByDesktopTerminate()
{
    Reference<lang::XComponent> xComponent(init(), uno::UNO_QUERY_THROW);
    auto pListenerAdded(new TestEventListener());
    Reference<lang::XEventListener> xListenerAdded(pListenerAdded);
    xComponent->addEventListener(xListenerAdded);
    triggerDesktopTerminate();
    CPPUNIT_ASSERT_EQUAL(true, pListenerAdded->m_hasDisposingCalled);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
