/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/util/xrefreshable.hxx>

#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/util/XRefreshListener.hpp>
#include <com/sun/star/util/XRefreshable.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
namespace
{
class MockedRefreshListener : public ::cppu::WeakImplHelper<util::XRefreshListener>
{
public:
    MockedRefreshListener()
        : m_bListenerCalled(false)
    {
    }

    bool m_bListenerCalled;
    virtual void SAL_CALL refreshed(const lang::EventObject& /* xEvent */) override
    {
        m_bListenerCalled = true;
    }
    virtual void SAL_CALL disposing(const lang::EventObject& /* xEventObj */) override {}
};
}

void XRefreshable::testRefreshListener()
{
    uno::Reference<util::XRefreshable> xRefreshable(init(), uno::UNO_QUERY_THROW);

    rtl::Reference<MockedRefreshListener> xListener = new MockedRefreshListener();
    xRefreshable->addRefreshListener(uno::Reference<util::XRefreshListener>(xListener));

    xRefreshable->refresh();
    CPPUNIT_ASSERT(xListener->m_bListenerCalled);

    xListener->m_bListenerCalled = false;
    xRefreshable->removeRefreshListener(uno::Reference<util::XRefreshListener>(xListener));
    xRefreshable->refresh();
    CPPUNIT_ASSERT(!xListener->m_bListenerCalled);
}

} // namespace apitest
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
