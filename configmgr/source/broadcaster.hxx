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

class Broadcaster {
public:
    Broadcaster() {}

    void addDisposeNotification(
        css::uno::Reference< css::lang::XEventListener > const & listener,
        css::lang::EventObject const & event);

    void addContainerElementInsertedNotification(
        css::uno::Reference< css::container::XContainerListener > const & listener,
        css::container::ContainerEvent const & event);

    void addContainerElementRemovedNotification(
        css::uno::Reference< css::container::XContainerListener > const & listener,
        css::container::ContainerEvent const & event);

    void addContainerElementReplacedNotification(
        css::uno::Reference< css::container::XContainerListener > const & listener,
        css::container::ContainerEvent const & event);

    void addPropertyChangeNotification(
        css::uno::Reference< css::beans::XPropertyChangeListener > const & listener,
        css::beans::PropertyChangeEvent const & event);

    void addPropertiesChangeNotification(
        css::uno::Reference< css::beans::XPropertiesChangeListener > const & listener,
        css::uno::Sequence< css::beans::PropertyChangeEvent > const & event);

    void addChangesNotification(
        css::uno::Reference< css::util::XChangesListener > const & listener,
        css::util::ChangesEvent const & event);

    void send();

private:
    Broadcaster(const Broadcaster&) = delete;
    Broadcaster& operator=(const Broadcaster&) = delete;

    struct DisposeNotification {
        css::uno::Reference< css::lang::XEventListener >  listener;
        css::lang::EventObject                            event;

        DisposeNotification(
            css::uno::Reference< css::lang::XEventListener > const & theListener,
            css::lang::EventObject const & theEvent);
    };

    struct ContainerNotification {
        css::uno::Reference< css::container::XContainerListener > listener;
        css::container::ContainerEvent event;

        ContainerNotification(
            css::uno::Reference< css::container::XContainerListener > const & theListener,
            css::container::ContainerEvent const & theEvent);
    };

    struct PropertyChangeNotification {
        css::uno::Reference< css::beans::XPropertyChangeListener > listener;
        css::beans::PropertyChangeEvent                            event;

        PropertyChangeNotification(
            css::uno::Reference< css::beans::XPropertyChangeListener > const & theListener,
            css::beans::PropertyChangeEvent const & theEvent);
    };

    struct PropertiesChangeNotification {
        css::uno::Reference< css::beans::XPropertiesChangeListener > listener;
        css::uno::Sequence< css::beans::PropertyChangeEvent >        event;

        PropertiesChangeNotification(
            css::uno::Reference< css::beans::XPropertiesChangeListener > const & theListener,
            css::uno::Sequence< css::beans::PropertyChangeEvent > const & theEvent);
    };

    struct ChangesNotification {
        css::uno::Reference< css::util::XChangesListener > listener;
        css::util::ChangesEvent                            event;

        ChangesNotification(
            css::uno::Reference< css::util::XChangesListener > const & theListener,
            css::util::ChangesEvent const & theEvent);
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
