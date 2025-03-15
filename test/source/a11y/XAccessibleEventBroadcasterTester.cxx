/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <iostream>

#include <cppunit/TestAssert.h>

#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/EventObject.hpp>

#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include <test/a11y/AccessibilityTools.hxx>
#include <test/a11y/XAccessibleEventBroadcasterTester.hxx>

using namespace css;

namespace
{
class EvListener : public cppu::WeakImplHelper<accessibility::XAccessibleEventListener>
{
public:
    bool mbGotEvent;

    EvListener()
        : mbGotEvent(false)
    {
    }

    // XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject&) override {}

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent(const accessibility::AccessibleEventObject& aEvent) override
    {
        std::cout << "Listener got event: " << AccessibilityTools::debugString(aEvent) << std::endl;
        uno::Reference<accessibility::XAccessible> xOld(aEvent.OldValue, uno::UNO_QUERY);
        if (xOld.is())
            std::cout << "Old: " << AccessibilityTools::debugString(xOld) << std::endl;

        uno::Reference<accessibility::XAccessible> xNew(aEvent.NewValue, uno::UNO_QUERY);
        if (xNew.is())
            std::cout << "New: " << AccessibilityTools::debugString(xNew) << std::endl;

        mbGotEvent = true;
    }
};
}

XAccessibleEventBroadcasterTester::XAccessibleEventBroadcasterTester(
    const uno::Reference<accessibility::XAccessibleEventBroadcaster>& xBroadcaster,
    const uno::Reference<awt::XWindow>& xWindow)
    : mxBroadcaster(xBroadcaster)
    , mxWindow(xWindow)
{
}

bool XAccessibleEventBroadcasterTester::isTransient(
    const uno::Reference<accessibility::XAccessibleEventBroadcaster>& xBroadcaster)
{
    uno::Reference<accessibility::XAccessibleContext> xCtx(xBroadcaster, uno::UNO_QUERY_THROW);
    return isTransient(xCtx);
}

bool XAccessibleEventBroadcasterTester::isTransient(
    const uno::Reference<accessibility::XAccessibleContext>& xCtx)
{
    return ((xCtx->getAccessibleStateSet() & accessibility::AccessibleStateType::TRANSIENT)
            && (xCtx->getAccessibleParent()->getAccessibleContext()->getAccessibleStateSet()
                & accessibility::AccessibleStateType::MANAGES_DESCENDANTS));
}

/**
 * @brief Generates an event on @c mxBroadcaster.
 *
 * This method indirectly alters the state of @c mxBroadcaster so that an
 * accessible event will be fired for it.  It can be called as many times as
 * needed and triggers an event each time.
 */
void XAccessibleEventBroadcasterTester::fireEvent()
{
    awt::Rectangle newPosSize = mxWindow->getPosSize();
    newPosSize.Width = newPosSize.Width - 20;
    newPosSize.Height = newPosSize.Height - 20;
    newPosSize.X = newPosSize.X + 20;
    newPosSize.Y = newPosSize.Y + 20;
    mxWindow->setPosSize(newPosSize.X, newPosSize.Y, newPosSize.Width, newPosSize.Height,
                         awt::PosSize::POSSIZE);
}

/**
 * @brief Adds a listener and fires events by mean of object relation.
 *
 * Asserts that the listener was properly called.
 */
void XAccessibleEventBroadcasterTester::testAddEventListener()
{
    rtl::Reference<EvListener> xListener(new EvListener);
    mxBroadcaster->addAccessibleEventListener(xListener);
    bool transient = isTransient(mxBroadcaster);

    std::cout << "firing event" << std::endl;
    fireEvent();

    AccessibilityTools::Await([&xListener]() { return xListener->mbGotEvent; });

    if (!transient)
        CPPUNIT_ASSERT_MESSAGE("listener wasn't called", xListener->mbGotEvent);
    else
        CPPUNIT_ASSERT_MESSAGE("Object is Transient, listener isn't expected to be called",
                               !xListener->mbGotEvent);

    mxBroadcaster->removeAccessibleEventListener(xListener);
}

/**
 * @brief Similar to @c testAddEventListener() but also removes the listener
 *
 * Adds an event listener just like @c testAddEventListener(), and then removes it and verifies an
 * event doesn't trigger the supposedly removed listener.
 *
 * @see testAddEventListener()
 */
void XAccessibleEventBroadcasterTester::testRemoveEventListener()
{
    /* there is nothing we can really test for transient objects */
    if (isTransient(mxBroadcaster))
    {
        std::cerr << "could not test removing listener on transient object " << mxBroadcaster
                  << std::endl;
        return;
    }

    rtl::Reference<EvListener> xListener(new EvListener);
    mxBroadcaster->addAccessibleEventListener(xListener);

    std::cout << "firing event (with listener)" << std::endl;
    fireEvent();

    AccessibilityTools::Await([&xListener]() { return xListener->mbGotEvent; });

    CPPUNIT_ASSERT_MESSAGE("listener wasn't called", xListener->mbGotEvent);

    /* reset listener, remove it and try again */
    xListener->mbGotEvent = false;

    std::cout << "removing listener" << std::endl;
    mxBroadcaster->removeAccessibleEventListener(xListener);

    std::cout << "firing event (without listener)" << std::endl;
    fireEvent();

    AccessibilityTools::Wait(500);

    CPPUNIT_ASSERT_MESSAGE("removed listener was called", !xListener->mbGotEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
