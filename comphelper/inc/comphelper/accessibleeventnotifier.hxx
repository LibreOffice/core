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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
