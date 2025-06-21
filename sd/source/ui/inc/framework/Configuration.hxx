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

#pragma once

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/framework/AnchorBindingMode.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <comphelper/compbase.hxx>
#include <rtl/ref.hxx>
#include <memory>

namespace sd::framework {
class ConfigurationController;

typedef comphelper::WeakComponentImplHelper <
    css::container::XNamed
    > ConfigurationInterfaceBase;

/** A configuration describes the resources of an application like panes,
    views, and tool bars and their relationships that are currently active
    or are requested to be activated. Resources are specified by URLs rather
    than references so that not only the current configuration but also a
    requested configuration can be represented.

    A resource URL describes the type of a resource, not its actual
    instance. For resources, like panes, that are unique with respect to an
    application frame, that does not mean much of a difference. For other
    resources like views, that may have more than one instance per
    application frame, this is different. To identify them unambiguously a
    second URL, one of a unique resource, is necessary. This second URL is
    called the anchor of the first. The two types of resources are called
    unique and linked respectively.

    Direct manipulation of a configuration object is not advised with the
    exception of the configuration controller and objects that implement the
    XConfigurationChangeOperation interface.
*/
class Configuration final
    : public ConfigurationInterfaceBase
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
    Configuration (const rtl::Reference<ConfigurationController>& rxBroadcaster,
        bool bBroadcastRequestEvents);
    virtual ~Configuration() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;


    /** Add a resource to the configuration.
        <p>This method should be used only by objects that implement the
        XConfigurationRequest interface or by the configuration
        controller.</p>
        @param xResourceId
            The resource to add to the configuration.  When the specified
            resource is already part of the configuration then this call is
            silently ignored.
        @throws IllegalArgumentException
            When an empty resource id is given then an
            IllegalArgumentException is thrown.
    */
    void addResource (
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId);

    /** Remove a resource from the configuration.
        <p>This method should be used only by objects that implement the
        XConfigurationRequest interface or by the configuration
        controller.</p>
        @param xResourceId
            The resource to remove from the configuration. When the
            specified resource is not part of the configuration then this
            call is silently ignored.
        @throws IllegalArgumentException
            When an empty resource id is given then an
            IllegalArgumentException is thrown.
    */
    void removeResource(
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId);

    /** Returns the list of resources that are bound directly and/or
        indirectly to the given anchor.  A URL filter can reduce the set of
        returned resource ids.
        @param xAnchorId
            This anchor typically is either a pane or an empty
            XResourceId object.  An
            empty reference is treated like an XResourceId object.
        @param sTargetURLPrefix
            When a non-empty string is given then resource ids are returned
            only when their resource URL matches this prefix, i.e. when it
            begins with this prefix or is equal to it.  Characters with
            special meaning to URLs are not interpreted.  In the typical
            usage the prefix specifies the type of a resource.  A typical
            value is "private:resource/floater/", which is the prefix for
            pane URLs.  In a recursive search, only resource ids at the top
            level are matched against this prefix.
            <p>Use an empty string to prevent filtering out resource ids.</p>
        @param eSearchMode
            This flag defines whether to return only resources that are
            directly bound to the given anchor or a recursive search is to
            be made.  Note that for the recursive search and an empty anchor
            all resource ids are returned that belong to the configuration.
        @return
            The set of returned resource ids may be empty when there are no
            resource ids that match all conditions.  The resources in the
            sequence are ordered with respect to the
            XResourceId::compareTo() method.
    */
    css::uno::Sequence< css::uno::Reference<
        css::drawing::framework::XResourceId> > getResources (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId,
        std::u16string_view rsResourceURLPrefix,
        css::drawing::framework::AnchorBindingMode eMode);

    /** <p>Returns whether the specified resource is part of the
        configuration.</p>
        This is independent of whether the resource does really exist and is
        active, i.e. has a visible representation in the GUI.
        @param xResourceId
            The id of a resource.  May be empty (empty reference or empty
            XResourceId object) in which case `FALSE` is
            returned.
        @return
            Returns `TRUE` when the resource is part of the configuration
            and `FALSE` when it is not.
    */
    bool hasResource (
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId);

    rtl::Reference<Configuration> createClone();

    // XNamed

    /** Return a human readable string representation.  This is used for
        debugging purposes.
    */
    virtual OUString SAL_CALL getName() override;

    /** This call is ignored because the XNamed interface is (mis)used to
        give access to a human readable name for debugging purposes.
    */
    virtual void SAL_CALL setName (const OUString& rName) override;

private:
    class ResourceContainer;
    /** The resource container holds the URLs of unique resource and of
        resource linked to unique resources.
    */
    std::unique_ptr<ResourceContainer> mpResourceContainer;

    /** The broadcaster used for notifying listeners of requests for
        configuration changes.
    */
    rtl::Reference<ConfigurationController> mxBroadcaster;

    bool mbBroadcastRequestEvents;

    /** This private variant of the constructor is used for cloning a
        Configuration object.
        @param rResourceContainer
            The new Configuration is created with a copy of the elements in
            this container.
    */
    Configuration (const rtl::Reference<ConfigurationController>& rxBroadcaster,
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

        @throws css::lang::DisposedException
    */
    void ThrowIfDisposed() const;
};

/** Return whether the two given configurations contain the same resource
    ids.  The order of resource ids is ignored.  Empty references are
    treated like empty configurations.
*/
bool AreConfigurationsEquivalent (
        const rtl::Reference<Configuration>& rxConfiguration1,
        const rtl::Reference<Configuration>& rxConfiguration2);

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
