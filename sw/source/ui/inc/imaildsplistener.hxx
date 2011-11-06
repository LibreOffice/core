/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_IMAILDSPLISTENER_HXX
#define INCLUDED_IMAILDSPLISTENER_HXX

#include "com/sun/star/mail/XMailMessage.hpp"
#include <salhelper/refobj.hxx>


class MailDispatcher;

/**
    MailDispatcher listener interface.
    Clients may implement and register instances of the
    mail dispatcher interface in order to get notifications
    about the MailDispatcher status.

    @see MailDispatcher
*/
class IMailDispatcherListener : public ::salhelper::ReferenceObject
{
public:
    /**
        Called when the MailDispatcher is started.
    */
    virtual void started(::rtl::Reference<MailDispatcher> xMailDispatcher) = 0;

    /**
        Called when the MailDispatcher is stopped.
    */
    virtual void stopped(::rtl::Reference<MailDispatcher> xMailDispatcher) = 0;

    /**
        Called when there are no more mail messages
        to deliver.
    */
    virtual void idle(::rtl::Reference<MailDispatcher> xMailDispatcher) = 0;

    /**
        Called for every mail message that has been
        successfully delivered.
    */
    virtual void mailDelivered(::rtl::Reference<MailDispatcher> xMailDispatcher, ::com::sun::star::uno::Reference< ::com::sun::star::mail::XMailMessage> xMailMessage) = 0;

    /**
        Called for every mail message whose delivery
        failed.
    */
    virtual void mailDeliveryError(::rtl::Reference<MailDispatcher> xMailDispatcher, ::com::sun::star::uno::Reference< ::com::sun::star::mail::XMailMessage> xMailMessage, const rtl::OUString& sErrorMessage) = 0;
};

#endif // INCLUDED_IMAILDISPATCHERLISTENER_HXX
