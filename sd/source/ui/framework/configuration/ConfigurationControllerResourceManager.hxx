/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationControllerResourceManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:29:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
