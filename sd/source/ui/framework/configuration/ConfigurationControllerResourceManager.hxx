/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConfigurationControllerResourceManager.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_FRAMEWORK_RESOURCE_MANAGER_HXX
#define SD_FRAMEWORK_RESOURCE_MANAGER_HXX

#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XResource.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

namespace css = ::com::sun::star;

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
        const ::boost::shared_ptr<ResourceFactoryManager>& rpResourceFactoryContainer,
        const ::boost::shared_ptr<ConfigurationControllerBroadcaster>& rpBroadcaster);

    ~ConfigurationControllerResourceManager (void);

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

    ::boost::shared_ptr<ResourceFactoryManager> mpResourceFactoryContainer;

    /** This broadcaster is used to notify the activation and deactivation
        of resources.
    */
    ::boost::shared_ptr<ConfigurationControllerBroadcaster> mpBroadcaster;

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
