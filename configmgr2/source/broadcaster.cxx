/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/XPropertiesChangeListener.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XVetoableChangeListener.hpp"
#include "com/sun/star/container/XContainerListener.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XEventListener.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/XChangesListener.hpp"
#include "osl/diagnose.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "broadcaster.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

void Broadcaster::addDisposeNotification(
    css::uno::Reference< css::lang::XEventListener > const & listener,
    css::lang::EventObject const & event)
{
    disposeNotifications_.push_back(DisposeNotification(listener, event));
}

void Broadcaster::addContainerNotification(
    css::uno::Reference< css::container::XContainerListener > const & listener,
    css::container::ContainerEvent const & event)
{
    containerNotifications_.push_back(ContainerNotification(listener, event));
}

void Broadcaster::addPropertyChangeNotification(
    css::uno::Reference< css::beans::XPropertyChangeListener > const & listener,
    css::beans::PropertyChangeEvent const & event)
{
    propertyChangeNotifications_.push_back(
        PropertyChangeNotification(listener, event));
}

void Broadcaster::addVetoableChangeNotification(
    css::uno::Reference< css::beans::XVetoableChangeListener > const & listener,
    css::beans::PropertyChangeEvent const & event)
{
    vetoableChangeNotifications_.push_back(
        VetoableChangeNotification(listener, event));
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
    bool exception = false;
    for (DisposeNotifications::iterator i(disposeNotifications_.begin());
         i != disposeNotifications_.end(); ++i) {
        try {
            i->listener->disposing(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception &) {
            exception = true;
        }
    }
    for (ContainerNotifications::iterator i(containerNotifications_.begin());
         i != containerNotifications_.end(); ++i)
    {
        try {
            i->listener->elementReplaced(i->event);
                //TODO: elementInserted/Removed/Replaced
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception &) {
            exception = true;
        }
    }
    for (PropertyChangeNotifications::iterator i(
             propertyChangeNotifications_.begin());
         i != propertyChangeNotifications_.end(); ++i)
    {
        try {
            i->listener->propertyChange(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception &) {
            exception = true;
        }
    }
    for (VetoableChangeNotifications::iterator i(
             vetoableChangeNotifications_.begin());
         i != vetoableChangeNotifications_.end(); ++i)
    {
        try {
            i->listener->vetoableChange(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::beans::PropertyVetoException &) {
            //TODO
        } catch (css::uno::Exception &) {
            exception = true;
        }
    }
    for (PropertiesChangeNotifications::iterator i(
             propertiesChangeNotifications_.begin());
         i != propertiesChangeNotifications_.end(); ++i)
    {
        try {
            i->listener->propertiesChange(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception &) {
            exception = true;
        }
    }
    for (ChangesNotifications::iterator i(changesNotifications_.begin());
         i != changesNotifications_.end(); ++i) {
        try {
            i->listener->changesOccurred(i->event);
        } catch (css::lang::DisposedException &) {
        } catch (css::uno::Exception &) {
            exception = true;
        }
    }
    if (exception) { //TODO
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr exceptions during listener notification")),
            css::uno::Reference< css::uno::XInterface >());
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

Broadcaster::VetoableChangeNotification::VetoableChangeNotification(
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
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
