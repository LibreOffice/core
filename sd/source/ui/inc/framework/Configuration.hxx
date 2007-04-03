/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Configuration.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:10:06 $
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

#ifndef SD_FRAMEWORK_CONFIGURATION_CONFIGURATION_HXX
#define SD_FRAMEWORK_CONFIGURATION_CONFIGURATION_HXX

#include "MutexOwner.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATION_HPP_
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLERBROADCASTER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationControllerBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

#include <boost/scoped_ptr.hpp>

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    ::com::sun::star::drawing::framework::XConfiguration,
    ::com::sun::star::container::XNamed
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
    Configuration (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfigurationControllerBroadcaster>& rxBroadcaster,
        bool bBroadcastRequestEvents);
    virtual ~Configuration (void);

    virtual void SAL_CALL disposing (void);


    // XConfiguration

    virtual void SAL_CALL addResource (
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>&
            rxResourceId)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeResource(
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>&
            rxResourceId)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
        com::sun::star::drawing::framework::XResourceId> > SAL_CALL getResources (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxAnchorId,
        const ::rtl::OUString& rsResourceURLPrefix,
        ::com::sun::star::drawing::framework::AnchorBindingMode eMode)
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasResource (
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>&
            rxResourceId)
        throw (::com::sun::star::uno::RuntimeException);


    // XCloneable

    virtual ::com::sun::star::uno::Reference<com::sun::star::util::XCloneable>
        SAL_CALL createClone (void)
        throw (::com::sun::star::uno::RuntimeException);


    // XNamed

    /** Return a human readable string representation.  This is used for
        debugging purposes.
    */
    virtual ::rtl::OUString SAL_CALL getName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** This call is ignored because the XNamed interface is (mis)used to
        give access to a human readable name for debugging purposes.
    */
    virtual void SAL_CALL setName (const ::rtl::OUString& rName)
        throw (::com::sun::star::uno::RuntimeException);

private:
    class ResourceContainer;
    /** The resource container holds the URLs of unique resource and of
        resource linked to unique resources.
    */
    ::boost::scoped_ptr<ResourceContainer> mpResourceContainer;

    /** The broadcaster used for notifying listeners of requests for
        configuration changes.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfigurationControllerBroadcaster>
        mxBroadcaster;

    bool mbBroadcastRequestEvents;

    /** This private variant of the constructor is used for cloning a
        Configuration object.
        @param rResourceContainer
            The new Configuration is created with a copy of the elements in
            this container.
    */
    Configuration (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfigurationControllerBroadcaster>& rxBroadcaster,
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
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>&
            rxResourceId,
        const bool bActivation);

    /** When the called object has already been disposed this method throws
        an exception and does not return.
    */
    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);
};


/** Return whether the two given configurations contain the same resource
    ids.  The order of resource ids is ignored.  Empty references are
    treated like empty configurations.
*/
bool AreConfigurationsEquivalent (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfiguration>& rxConfiguration1,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfiguration>& rxConfiguration2);

} } // end of namespace sd::framework

#endif
