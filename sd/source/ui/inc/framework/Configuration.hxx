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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_CONFIGURATION_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_CONFIGURATION_HXX

#include "MutexOwner.hxx"
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XConfigurationControllerBroadcaster.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <cppuhelper/compbase.hxx>

#include <memory>

namespace {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XConfiguration,
    css::container::XNamed,
    css::lang::XServiceInfo
    > ConfigurationInterfaceBase;

} // end of anonymous namespace.

namespace sd { namespace framework {

/** A configuration describes the resources of an application like panes,
    views, and tool bars and their relationships that are currently active
    or are requested to be activated. Resources are specified by URLs rather
    than references so that not only the current configuration but also a
    requested configuration can be represented.

    A resource URL describes the type of a resource, not its actual
    instance. For resources, like panes, that are unique with respect to an
    application frame, that does not mean much of a difference. For other
    resources like views, that may have more than one instance per
    application frame, this is different. To identify them unambigously a
    second URL, one of a unique resource, is necessary. This second URL is
    called the anchor of the first. The two types of resources are called
    unique and linked respectively.

    Direct manipulation of a configuration object is not advised with the
    exception of the configuration controller and objects that implement the
    XConfigurationChangeOperation interface.
*/
class Configuration
    : private sd::MutexOwner,
      public ConfigurationInterfaceBase
{
public:
    /** Create a new configuration with a broadcaster that is used to send
        events about requested configuration changes.
        @param rxBroadcaster
            This broadcaster is typically the same as the one used by the
            ConfigurationController.
        @param bBroadcastRequestEvents
            When this is <TRUE/> then modifications to the configuration
            trigger the broadcasting of "ResourceActivationRequestEvent" and
            "ResourceDeactivationRequestEvent".  When this flag is <FALSE/>
            then events with type "ResourceActivationEvent" and
            "ResourceDeactivationEvent" are broadcasted.
    */
    Configuration (const css::uno::Reference<css::drawing::framework::XConfigurationControllerBroadcaster>& rxBroadcaster,
        bool bBroadcastRequestEvents);
    virtual ~Configuration();

    virtual void SAL_CALL disposing() override;

    // XConfiguration

    virtual void SAL_CALL addResource (
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeResource(
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< css::uno::Reference<
        css::drawing::framework::XResourceId> > SAL_CALL getResources (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId,
        const OUString& rsResourceURLPrefix,
        css::drawing::framework::AnchorBindingMode eMode)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasResource (
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId)
        throw (css::uno::RuntimeException, std::exception) override;

    // XCloneable

    virtual css::uno::Reference<css::util::XCloneable>
        SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    // XNamed

    /** Return a human readable string representation.  This is used for
        debugging purposes.
    */
    virtual OUString SAL_CALL getName()
        throw (css::uno::RuntimeException, std::exception) override;

    /** This call is ignored because the XNamed interface is (mis)used to
        give access to a human readable name for debugging purposes.
    */
    virtual void SAL_CALL setName (const OUString& rName)
        throw (css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    class ResourceContainer;
    /** The resource container holds the URLs of unique resource and of
        resource linked to unique resources.
    */
    std::unique_ptr<ResourceContainer> mpResourceContainer;

    /** The broadcaster used for notifying listeners of requests for
        configuration changes.
    */
    css::uno::Reference<css::drawing::framework::XConfigurationControllerBroadcaster>
        mxBroadcaster;

    bool mbBroadcastRequestEvents;

    /** This private variant of the constructor is used for cloning a
        Configuration object.
        @param rResourceContainer
            The new Configuration is created with a copy of the elements in
            this container.
    */
    Configuration (const css::uno::Reference<css::drawing::framework::XConfigurationControllerBroadcaster>& rxBroadcaster,
        bool bBroadcastRequestEvents,
        const ResourceContainer& rResourceContainer);

    /** Send an event to all interested listeners that a resource has been
        added or removed.  The event is sent to the listeners via the
        ConfigurationController.
        @param rxResourceId
            The resource that is added to or removed from the configuration.
        @param bActivation
            This specifies whether an activation or deactivation is
            broadcasted.  The mbBroadcastRequestEvents member is also taken
            into account when the actual event type field is determined.
    */
    void PostEvent (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        const bool bActivation);

    /** When the called object has already been disposed this method throws
        an exception and does not return.
    */
    void ThrowIfDisposed() const
        throw (css::lang::DisposedException);
};

/** Return whether the two given configurations contain the same resource
    ids.  The order of resource ids is ignored.  Empty references are
    treated like empty configurations.
*/
bool AreConfigurationsEquivalent (
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration1,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration2);

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
