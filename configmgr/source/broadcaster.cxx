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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/beans/XPropertiesChangeListener.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/container/XContainerListener.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/XChangesListener.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "osl/diagnose.hxx"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "broadcaster.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

void appendMessage(
    rtl::OUStringBuffer & buffer, css::uno::Exception const & exception)
{
    buffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("; "));
    buffer.append(exception.Message);
}

}

void Broadcaster::addDisposeNotification(
    css::uno::Reference< css::lang::XEventListener > const & listener,
    css::lang::EventObject const & event)
{
    disposeNotifications_.push_back(DisposeNotification(listener, event));
}

void Broadcaster::addContainerElementReplacedNotification(
    css::uno::Reference< css::container::XContainerListener > const & listener,
    css::container::ContainerEvent const & event)
{
    containerElementReplacedNotifications_.push_back(
        ContainerNotification(listener, event));
}

void Broadcaster::addContainerElementInsertedNotification(
    css::uno::Reference< css::container::XContainerListener > const & listener,
    css::container::ContainerEvent const & event)
{
    containerElementInsertedNotifications_.push_back(
        ContainerNotification(listener, event));
}

void Broadcaster::addContainerElementRemovedNotification(
    css::uno::Reference< css::container::XContainerListener > const & listener,
    css::container::ContainerEvent const & event)
{
    containerElementRemovedNotifications_.push_back(
        ContainerNotification(listener, event));
}

void Broadcaster::addPropertyChangeNotification(
    css::uno::Reference< css::beans::XPropertyChangeListener > const & listener,
    css::beans::PropertyChangeEvent const & event)
{
    propertyChangeNotifications_.push_back(
        PropertyChangeNotification(listener, event));
}

void Broadcaster::addPropertiesChangeNotification(
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        listener,
    css::uno::Sequence< css::beans::PropertyChangeEvent > const & event)
{
    propertiesChangeNotifications_.push_back(
        PropertiesChangeNotification(listener, event));
}

void Broadcaster::addChangesNotification(
    css::uno::Reference< css::util::XChangesListener > const & listener,
    css::util::ChangesEvent const & event)
{
    changesNotifications_.push_back(ChangesNotification(listener, event));
}

void Broadcaster::send() {
    css::uno::Any exception;
    rtl::OUStringBuffer messages;
    for (DisposeNotifications::iterator i(disposeNotifications_.begin());
         i != disposeNotifications_.end(); ++i) {
        try {
            i->listener->disposing(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (ContainerNotifications::iterator i(
             containerElementInsertedNotifications_.begin());
         i != containerElementInsertedNotifications_.end(); ++i)
    {
        try {
            i->listener->elementInserted(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (ContainerNotifications::iterator i(
             containerElementRemovedNotifications_.begin());
         i != containerElementRemovedNotifications_.end(); ++i)
    {
        try {
            i->listener->elementRemoved(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (ContainerNotifications::iterator i(
             containerElementReplacedNotifications_.begin());
         i != containerElementReplacedNotifications_.end(); ++i)
    {
        try {
            i->listener->elementReplaced(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (PropertyChangeNotifications::iterator i(
             propertyChangeNotifications_.begin());
         i != propertyChangeNotifications_.end(); ++i)
    {
        try {
            i->listener->propertyChange(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (PropertiesChangeNotifications::iterator i(
             propertiesChangeNotifications_.begin());
         i != propertiesChangeNotifications_.end(); ++i)
    {
        try {
            i->listener->propertiesChange(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    for (ChangesNotifications::iterator i(changesNotifications_.begin());
         i != changesNotifications_.end(); ++i) {
        try {
            i->listener->changesOccurred(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception & e) {
            exception = cppu::getCaughtException();
            appendMessage(messages, e);
        }
    }
    if (exception.hasValue()) {
        throw css::lang::WrappedTargetRuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr exceptions during listener notification")) +
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
    OSL_ASSERT(theListener.is());
}

Broadcaster::ContainerNotification::ContainerNotification(
    css::uno::Reference< css::container::XContainerListener > const &
        theListener,
    css::container::ContainerEvent const & theEvent):
    listener(theListener), event(theEvent)
{
    OSL_ASSERT(theListener.is());
}

Broadcaster::PropertyChangeNotification::PropertyChangeNotification(
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        theListener,
    css::beans::PropertyChangeEvent const & theEvent):
    listener(theListener), event(theEvent)
{
    OSL_ASSERT(theListener.is());
}

Broadcaster::PropertiesChangeNotification::PropertiesChangeNotification(
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        theListener,
    css::uno::Sequence< css::beans::PropertyChangeEvent > const & theEvent):
    listener(theListener), event(theEvent)
{
    OSL_ASSERT(theListener.is());
}

Broadcaster::ChangesNotification::ChangesNotification(
    css::uno::Reference< css::util::XChangesListener > const & theListener,
    css::util::ChangesEvent const & theEvent):
    listener(theListener), event(theEvent)
{
    OSL_ASSERT(theListener.is());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
