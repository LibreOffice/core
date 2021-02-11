/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xactivationbroadcaster.hxx>

#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/sheet/ActivationEvent.hpp>
#include <com/sun/star/sheet/XActivationBroadcaster.hpp>
#include <com/sun/star/sheet/XActivationEventListener.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
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
class MockedActivationEventListener : public ::cppu::WeakImplHelper<sheet::XActivationEventListener>
{
public:
    MockedActivationEventListener()
        : mbListenerCalled(false)
    {
    }
    bool mbListenerCalled;
    virtual void SAL_CALL
    activeSpreadsheetChanged(const sheet::ActivationEvent& /* xEvent */) override
    {
        mbListenerCalled = true;
    }
    virtual void SAL_CALL disposing(const lang::EventObject& /* xEventObj */) override {}
};
}

void XActivationBroadcaster::testAddRemoveActivationEventListener()
{
    uno::Reference<sheet::XActivationBroadcaster> xAB(init(), UNO_QUERY_THROW);
    xAB->addActivationEventListener(nullptr);

    rtl::Reference<MockedActivationEventListener> xListener = new MockedActivationEventListener();
    xAB->addActivationEventListener(uno::Reference<sheet::XActivationEventListener>(xListener));

    uno::Reference<sheet::XSpreadsheetView> xView(xAB, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet1(xView->getActiveSheet(), UNO_SET_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet2(getXSpreadsheet(1), UNO_QUERY_THROW);

    xView->setActiveSheet(xSheet2);

    CPPUNIT_ASSERT_MESSAGE("Listener wasn't called", xListener->mbListenerCalled);

    xAB->removeActivationEventListener(uno::Reference<sheet::XActivationEventListener>(xListener));
    xView->setActiveSheet(xSheet1);
    CPPUNIT_ASSERT_MESSAGE("Listener still called after removal", xListener->mbListenerCalled);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
