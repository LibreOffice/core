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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_CONFIGURATIONCONTROLLERRESOURCEMANAGER_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_CONFIGURATIONCONTROLLERRESOURCEMANAGER_HXX

#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XResource.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <osl/mutex.hxx>

#include <boost/noncopyable.hpp>
#include <map>
#include <memory>
#include <vector>

namespace sd { namespace framework {

class ConfigurationControllerBroadcaster;
class ResourceFactoryManager;

/** Manage the set of active resources.  Activate and deactivate resources.
*/
class ConfigurationControllerResourceManager
    : ::boost::noncopyable
{
public:
    /** For every active resource both the resource itself as well as its
        creating factory are remembered, so that on deactivation, the
        resource can be deactivated by this factory.
    */
    class ResourceDescriptor
    {
    public:
        css::uno::Reference<css::drawing::framework::XResource> mxResource;
        css::uno::Reference<css::drawing::framework::XResourceFactory> mxResourceFactory;
    };

    /** A new ResourceManager object is created with the resource factory
        container for creating resources and the event broadcaster for
        notifying ConfigurationChangeListeners of activated or deactivated
        resources.
    */
    ConfigurationControllerResourceManager (
        const std::shared_ptr<ResourceFactoryManager>& rpResourceFactoryContainer,
        const std::shared_ptr<ConfigurationControllerBroadcaster>& rpBroadcaster);

    ~ConfigurationControllerResourceManager();

    /** Activate all the resources that are specified by resource ids in
        rResources.  The resource ids of activated resources are added to
        the given configuration.  Activated resources are notified to all
        interested ConfigurationChangeListeners.
    */
    void ActivateResources (
        const ::std::vector<
            css::uno::Reference<css::drawing::framework::XResourceId> >& rResources,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration);

    /** Deactivate all the resources that are specified by resource ids in
        rResources.  The resource ids of deactivated resources are removed
        from the given configuration.  Activated resources are notified to all
        interested ConfigurationChangeListeners.
    */
    void DeactivateResources (
        const ::std::vector<
            css::uno::Reference<css::drawing::framework::XResourceId> >& rResources,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration);

    /** Return the descriptor for the specified resource.
        @return
            When there is no active resource for the given resource id then
            an empty descriptor is returned.
    */
    ResourceDescriptor GetResource (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId);

private:
    osl::Mutex maMutex;

    class ResourceComparator
    {
    public:
        bool operator() (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxId1,
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxId2) const;
    };

    typedef ::std::map<
        css::uno::Reference<css::drawing::framework::XResourceId>,
        ResourceDescriptor,
        ResourceComparator> ResourceMap;
    ResourceMap maResourceMap;

    std::shared_ptr<ResourceFactoryManager> mpResourceFactoryContainer;

    /** This broadcaster is used to notify the activation and deactivation
        of resources.
    */
    std::shared_ptr<ConfigurationControllerBroadcaster> mpBroadcaster;

    void ActivateResource (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration);

    void DeactivateResource (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration);

    void AddResource (
        const css::uno::Reference<css::drawing::framework::XResource>& rxResource,
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxFactory);

    ResourceDescriptor RemoveResource (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
