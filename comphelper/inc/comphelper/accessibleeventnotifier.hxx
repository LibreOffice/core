/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibleeventnotifier.hxx,v $
 * $Revision: 1.8 $
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

#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#define COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER

#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <osl/thread.hxx>
#include <osl/conditn.hxx>
#include <cppuhelper/interfacecontainer.h>
#include "comphelper/comphelperdllapi.h"

#include <map>
#include <list>

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= AccessibleEventNotifier
    //=====================================================================
    class COMPHELPER_DLLPUBLIC AccessibleEventNotifier
    {
    // typedefs
    public:
        typedef sal_uInt32  TClientId;

        typedef ::std::pair< TClientId, ::com::sun::star::accessibility::AccessibleEventObject >
                                                                                    ClientEvent;

        typedef ::cppu::OInterfaceContainerHelper                                   EventListeners;
        typedef ::std::map< TClientId, EventListeners*, ::std::less< TClientId > >  ClientMap;

    protected:
        AccessibleEventNotifier( );     // never implemented
        ~AccessibleEventNotifier( );    // never implemented

    private:
        COMPHELPER_DLLPRIVATE AccessibleEventNotifier( const AccessibleEventNotifier& );                // never implemented!
        COMPHELPER_DLLPRIVATE AccessibleEventNotifier& operator=( const AccessibleEventNotifier& ); // never implemented!

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
        COMPHELPER_DLLPRIVATE static    TClientId   generateId();

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
        COMPHELPER_DLLPRIVATE static    sal_Bool    implLookupClient( const TClientId _nClient, ClientMap::iterator& _rPos );
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER


