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

#ifndef INCLUDED_COMPHELPER_ACCESSIBLEEVENTNOTIFIER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLEEVENTNOTIFIER_HXX

#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>

#include <comphelper/comphelperdllapi.h>


namespace comphelper
{



    //= AccessibleEventNotifier

    class COMPHELPER_DLLPUBLIC AccessibleEventNotifier
    {
    // typedefs
    public:
        typedef sal_uInt32  TClientId;

    protected:
        AccessibleEventNotifier( );     // never implemented
        ~AccessibleEventNotifier( );    // never implemented

    private:
        AccessibleEventNotifier( const AccessibleEventNotifier& ) = delete;
        AccessibleEventNotifier& operator=( const AccessibleEventNotifier& ) = delete;

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
                    );

        /** registers a listener for the given client

            @param _nClient
                the id of the client for which a listener should be registered
            @return
                the number of event listeners currently registered for this client
        */
        static sal_Int32 addEventListener(
                        const TClientId _nClient,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& _rxListener
                    );

        /** revokes a listener for the given client

            @param _nClient
                the id of the client for which a listener should be revoked
            @return
                the number of event listeners currently registered for this client
        */
        static sal_Int32 removeEventListener(
                        const TClientId _nClient,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& _rxListener
                    );

        /** adds an event, which is to be braodcasted, to the queue

            @param _nClient
                the id of the client which needs to broadcast the event
        */
        static void addEvent(
                        const TClientId _nClient,
                        const ::com::sun::star::accessibility::AccessibleEventObject& _rEvent
                    );

    };


}   // namespace comphelper

#endif // INCLUDED_COMPHELPER_ACCESSIBLEEVENTNOTIFIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
