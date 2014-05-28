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

#ifndef INCLUDED_CONFIGMGR_SOURCE_BROADCASTER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_BROADCASTER_HXX

#include <sal/config.h>

#include <vector>

#include <boost/noncopyable.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/ChangesEvent.hpp>

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
