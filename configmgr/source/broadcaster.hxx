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

#ifndef INCLUDED_CONFIGMGR_SOURCE_BROADCASTER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_BROADCASTER_HXX

#include "sal/config.h"

#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/PropertyChangeEvent.hpp"
#include "com/sun/star/container/ContainerEvent.hpp"
#include "com/sun/star/lang/EventObject.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/util/ChangesEvent.hpp"

namespace com { namespace sun { namespace star {
    namespace beans {
        class XPropertiesChangeListener;
        class XPropertyChangeListener;
    }
    namespace container { class XContainerListener; }
    namespace lang { class XEventListener; }
    namespace util { class XChangesListener; }
} } }

namespace configmgr {

class Access;

class Broadcaster: private boost::noncopyable {
public:
    void addDisposeNotification(
        com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >
            const & listener,
        com::sun::star::lang::EventObject const & event);

    void addContainerElementInsertedNotification(
        com::sun::star::uno::Reference<
            com::sun::star::container::XContainerListener > const & listener,
        com::sun::star::container::ContainerEvent const & event);

    void addContainerElementRemovedNotification(
        com::sun::star::uno::Reference<
            com::sun::star::container::XContainerListener > const & listener,
        com::sun::star::container::ContainerEvent const & event);

    void addContainerElementReplacedNotification(
        com::sun::star::uno::Reference<
            com::sun::star::container::XContainerListener > const & listener,
        com::sun::star::container::ContainerEvent const & event);

    void addPropertyChangeNotification(
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertyChangeListener > const & listener,
        com::sun::star::beans::PropertyChangeEvent const & event);

    void addPropertiesChangeNotification(
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener > const & listener,
        com::sun::star::uno::Sequence<
            com::sun::star::beans::PropertyChangeEvent > const & event);

    void addChangesNotification(
        com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
            const & listener,
        com::sun::star::util::ChangesEvent const & event);

    void send();

private:
    struct DisposeNotification {
        com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >
            listener;
        com::sun::star::lang::EventObject event;

        DisposeNotification(
            com::sun::star::uno::Reference<
                com::sun::star::lang::XEventListener > const & theListener,
            com::sun::star::lang::EventObject const & theEvent);
    };

    struct ContainerNotification {
        com::sun::star::uno::Reference<
            com::sun::star::container::XContainerListener > listener;
        com::sun::star::container::ContainerEvent event;

        ContainerNotification(
            com::sun::star::uno::Reference<
                com::sun::star::container::XContainerListener > const &
                    theListener,
            com::sun::star::container::ContainerEvent const & theEvent);
    };

    struct PropertyChangeNotification {
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertyChangeListener > listener;
        com::sun::star::beans::PropertyChangeEvent event;

        PropertyChangeNotification(
            com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertyChangeListener > const &
                theListener,
            com::sun::star::beans::PropertyChangeEvent const & theEvent);
    };

    struct PropertiesChangeNotification {
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertiesChangeListener > listener;
        com::sun::star::uno::Sequence<
            com::sun::star::beans::PropertyChangeEvent > event;

        PropertiesChangeNotification(
            com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertiesChangeListener > const &
                theListener,
            com::sun::star::uno::Sequence<
                com::sun::star::beans::PropertyChangeEvent > const & theEvent);
    };

    struct ChangesNotification {
        com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
            listener;
        com::sun::star::util::ChangesEvent event;

        ChangesNotification(
            com::sun::star::uno::Reference<
                com::sun::star::util::XChangesListener > const & theListener,
            com::sun::star::util::ChangesEvent const & theEvent);
    };

    typedef std::vector< DisposeNotification > DisposeNotifications;

    typedef std::vector< ContainerNotification > ContainerNotifications;

    typedef std::vector< PropertyChangeNotification >
        PropertyChangeNotifications;

    typedef std::vector< PropertiesChangeNotification >
        PropertiesChangeNotifications;

    typedef std::vector< ChangesNotification > ChangesNotifications;

    DisposeNotifications disposeNotifications_;
    ContainerNotifications containerElementInsertedNotifications_;
    ContainerNotifications containerElementRemovedNotifications_;
    ContainerNotifications containerElementReplacedNotifications_;
    PropertyChangeNotifications propertyChangeNotifications_;
    PropertiesChangeNotifications propertiesChangeNotifications_;
    ChangesNotifications changesNotifications_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
