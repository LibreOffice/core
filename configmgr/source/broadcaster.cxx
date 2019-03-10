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

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/string.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

#include "broadcaster.hxx"

namespace configmgr {

namespace {

void appendMessage(
    OUStringBuffer & buffer, css::uno::Exception const & exception)
{
    buffer.append("; ");
    buffer.append(exception.Message);
}

}

void Broadcaster::addDisposeNotification(
    css::uno::Reference< css::lang::XEventListener > const & listener,
    css::lang::EventObject const & event)
{
    disposeNotifications_.emplace_back(listener, event);
}

void Broadcaster::addContainerElementReplacedNotification(
    css::uno::Reference< css::container::XContainerListener > const & listener,
    css::container::ContainerEvent const & event)
{
    containerElementReplacedNotifications_.emplace_back(listener, event);
}

void Broadcaster::addContainerElementInsertedNotification(
    css::uno::Reference< css::container::XContainerListener > const & listener,
    css::container::ContainerEvent const & event)
{
    containerElementInsertedNotifications_.emplace_back(listener, event);
}

void Broadcaster::addContainerElementRemovedNotification(
    css::uno::Reference< css::container::XContainerListener > const & listener,
    css::container::ContainerEvent const & event)
{
    containerElementRemovedNotifications_.emplace_back(listener, event);
}

void Broadcaster::addPropertyChangeNotification(
    css::uno::Reference< css::beans::XPropertyChangeListener > const & listener,
    css::beans::PropertyChangeEvent const & event)
{
    propertyChangeNotifications_.emplace_back(listener, event);
}

void Broadcaster::addPropertiesChangeNotification(
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        listener,
    css::uno::Sequence< css::beans::PropertyChangeEvent > const & event)
{
    propertiesChangeNotifications_.emplace_back(listener, event);
}

void Broadcaster::addChangesNotification(
    css::uno::Reference< css::util::XChangesListener > const & listener,
    css::util::ChangesEvent const & event)
{
    changesNotifications_.emplace_back(listener, event);
}

void Broadcaster::send() {
    css::uno::Any exception;
    OUStringBuffer messages;
    for (auto& rNotification : disposeNotifications_) {
        try {
            rNotification.listener->disposing(rNotification.event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (auto& rNotification : containerElementInsertedNotifications_)
    {
        try {
            rNotification.listener->elementInserted(rNotification.event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (auto& rNotification : containerElementRemovedNotifications_)
    {
        try {
            rNotification.listener->elementRemoved(rNotification.event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (auto& rNotification : containerElementReplacedNotifications_)
    {
        try {
            rNotification.listener->elementReplaced(rNotification.event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (auto& rNotification : propertyChangeNotifications_)
    {
        try {
            rNotification.listener->propertyChange(rNotification.event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (auto& rNotification : propertiesChangeNotifications_)
    {
        try {
            rNotification.listener->propertiesChange(rNotification.event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (auto& rNotification : changesNotifications_) {
        try {
            rNotification.listener->changesOccurred(rNotification.event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    if (exception.hasValue()) {
        throw css::lang::WrappedTargetRuntimeException(
            ("configmgr exceptions during listener notification" +
             messages.makeStringAndClear()),
            css::uno::Reference< css::uno::XInterface >(),
            exception);
    }
}

Broadcaster::DisposeNotification::DisposeNotification(
    css::uno::Reference< css::lang::XEventListener > const & theListener,
    css::lang::EventObject const & theEvent):
    listener(theListener), event(theEvent)
{
    assert(theListener.is());
}

Broadcaster::ContainerNotification::ContainerNotification(
    css::uno::Reference< css::container::XContainerListener > const &
        theListener,
    css::container::ContainerEvent const & theEvent):
    listener(theListener), event(theEvent)
{
    assert(theListener.is());
}

Broadcaster::PropertyChangeNotification::PropertyChangeNotification(
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        theListener,
    css::beans::PropertyChangeEvent const & theEvent):
    listener(theListener), event(theEvent)
{
    assert(theListener.is());
}

Broadcaster::PropertiesChangeNotification::PropertiesChangeNotification(
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        theListener,
    css::uno::Sequence< css::beans::PropertyChangeEvent > const & theEvent):
    listener(theListener), event(theEvent)
{
    assert(theListener.is());
}

Broadcaster::ChangesNotification::ChangesNotification(
    css::uno::Reference< css::util::XChangesListener > const & theListener,
    css::util::ChangesEvent const & theEvent):
    listener(theListener), event(theEvent)
{
    assert(theListener.is());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
