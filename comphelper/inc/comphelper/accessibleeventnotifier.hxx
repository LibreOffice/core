/*************************************************************************
 *
 *  $RCSfile: accessibleeventnotifier.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:25:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#define COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTOBJECT_HPP_
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif

#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#include <map>
#include <list>

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= AccessibleEventNotifier
    //=====================================================================
    class AccessibleEventNotifier
    {
    // typedefs
    public:
        typedef sal_uInt32  TClientId;

    private:
        typedef ::std::pair< TClientId, ::com::sun::star::accessibility::AccessibleEventObject >
                                                                                    ClientEvent;

        typedef ::cppu::OInterfaceContainerHelper                                   EventListeners;
        typedef ::std::map< TClientId, EventListeners*, ::std::less< TClientId > >  ClientMap;

    // members
    private:
        // client administration
        static ::osl::Mutex     s_aMutex;
        static ClientMap        s_aClients;

    protected:
        AccessibleEventNotifier( );     // never implemented
        ~AccessibleEventNotifier( );    // never implemented

    private:
        AccessibleEventNotifier( const AccessibleEventNotifier& );              // never implemented!
        AccessibleEventNotifier& operator=( const AccessibleEventNotifier& );   // never implemented!

    public:
        /** registers a client of this class, means a broadcaster of AccessibleEvents

            <p>No precaution is taken to care for disposal of this component. When the component
            dies, it <b>must</b> call <member>revokeClient</member> or <member>revokeClientNotifyDisposing</member>
            explicitly itself.</p>
        */
        static  TClientId   registerClient( );

        /** revokes a broadcaster of AccessibleEvents

            <p>Note that no disposing event is fired when you use this method, the client is simply revoked.
            You can for instance revoke a client if the last listener for it is revoked, but the client
            itself is not disposed.<br/>
            When the client is disposed, you should prefer <member>revokeClientNotifyDisposing</member></p>

            <p>Any possibly pending events for this client are removed from the queue.</p>

            @seealso revokeClientNotifyDisposing
        */
        static  void        revokeClient( const TClientId _nClient );

        /** revokes a client, with additionally notifying a disposing event to all listeners registered for
            this client

            <p>Any other possibly pending events for this client are removed from the queue</p>

            @param _nClient
                the id of the client which should be revoked
            @param _rxEventSource
                the source to be notified together with the <member scope="com.sun.star.lang">XComponent::disposing</member>
                call.
        */
        static  void        revokeClientNotifyDisposing(
                        const TClientId _nClient,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxEventSource
                    ) SAL_THROW( ( ) );

        /** registers a listener for the given client

            @param _nClient
                the id of the client for which a listener should be registered
            @return
                the number of event listeners currently registered for this client
        */
        static sal_Int32 addEventListener(
                        const TClientId _nClient,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& _rxListener
                    ) SAL_THROW( ( ) );

        /** revokes a listener for the given client

            @param _nClient
                the id of the client for which a listener should be revoked
            @return
                the number of event listeners currently registered for this client
        */
        static sal_Int32 removeEventListener(
                        const TClientId _nClient,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& _rxListener
                    ) SAL_THROW( ( ) );

        /** retrieves the set of listeners registered for a given client
        */
        static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
                        getEventListeners( const TClientId _nClient ) SAL_THROW( ( ) );

        /** adds an event, which is to be braodcasted, to the queue

            @param _nClient
                the id of the client which needs to broadcast the event
        */
        static void addEvent(
                        const TClientId _nClient,
                        const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent
                    ) SAL_THROW( ( ) );

    private:
        /// generates a new client id
        static  TClientId   generateId();

        /** looks up a client in our client map, asserts if it cannot find it or no event thread is present

            @precond
                to be called with our mutex locked

            @param _nClient
                the id of the client to loopup
            @param _rPos
                out-parameter for the position of the client in the client map

            @return
                <TRUE/> if and only if the client could be found and <arg>_rPos</arg> has been filled with
                it's position
        */
        static  sal_Bool    implLookupClient( const TClientId _nClient, ClientMap::iterator& _rPos );
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER


