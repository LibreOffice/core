/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationControllerBroadcaster.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:46:07 $
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

#ifndef SD_FRAMEWORK_CONFIGURATION_CONTROLLER_BROADCASTER_HXX
#define SD_FRAMEWORK_CONFIGURATION_CONTROLLER_BROADCASTER_HXX

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCHANGELISTENER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_CONFIGURATIONCHANGEEVENT_HPP_
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>
#endif

#include <comphelper/stl_types.hxx>
#include <vector>
#include <hash_map>

namespace sd { namespace framework {

/** This class manages the set of XConfigurationChangeListeners and
    calls them when the ConfigurationController wants to broadcast an
    event.

    For every registered combination of listener and event type a user data
    object is stored.  This user data object is then given to the listener
    whenever it is called for an event.  With this the listener can use
    a switch statement to handle different event types.
*/
class ConfigurationControllerBroadcaster
{
public:
    /** The given controller is used as origin of thrown exceptions.
    */
    ConfigurationControllerBroadcaster (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationController>& rxController);

    /** Add a listener for one type of event.  When one listener is
        interested in more than one event type this method has to be called
        once for every event type.  Alternatively it can register as
        universal listener that will be called for all event types.
        @param rxListener
            A valid reference to a listener.
        @param rsEventType
            The type of event that the listener will be called for.  The
            empty string is a special value in that the listener will be
            called for all event types.
        @param rUserData
            This object is passed to the listener whenever it is called for
            the specified event type.  For different event types different
            user data objects can be provided.
        @throws IllegalArgumentException
            when an empty listener reference is given.
    */
    void AddListener(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationChangeListener>& rxListener,
        const ::rtl::OUString& rsEventType,
        const ::com::sun::star::uno::Any& rUserData);

    /** Remove all references to the given listener.  When one listener has
        been registered for more than one type of event then it is removed
        for all of them.
        @param rxListener
            A valid reference to a listener.
        @throws IllegalArgumentException
            when an empty listener reference is given.
    */
    void RemoveListener(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationChangeListener>& rxListener);

    /** Broadcast the given event to all listeners that have been registered
        for its type of event as well as all universal listeners.

        When calling a listener results in a DisposedException being thrown
        the listener is unregistered automatically.
    */
    void NotifyListeners (
        const ::com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent);


    /** Call all listeners and inform them that the
        ConfigurationController is being disposed.  When this method returns
        the list of registered listeners is empty.  Further calls to
        RemoveListener() are not necessary but do not result in an error.
    */
    void DisposeAndClear (void);

private:
    ::com::sun::star::uno::Reference<
        com::sun::star::drawing::framework::XConfigurationController> mxConfigurationController;
    class ListenerDescriptor {public:
        ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationChangeListener> mxListener;
        ::com::sun::star::uno::Any maUserData;
    };
    typedef ::std::vector<ListenerDescriptor> ListenerList;
    typedef ::std::hash_map
        <rtl::OUString,
         ListenerList,
         ::comphelper::UStringHash,
         ::comphelper::UStringEqual> ListenerMap;
    ListenerMap maListenerMap;

    /** Broadcast the given event to all listeners in the given list.

        When calling a listener results in a DisposedException being thrown
        the listener is unregistered automatically.
    */
    void NotifyListeners (
        const ListenerList& rList,
        const ::com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent);
};




} } // end of namespace sd::framework

#endif
