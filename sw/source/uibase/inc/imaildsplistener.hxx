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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_IMAILDSPLISTENER_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_IMAILDSPLISTENER_HXX

#include <com/sun/star/mail/XMailMessage.hpp>
#include <salhelper/simplereferenceobject.hxx>

class MailDispatcher;

/**
    MailDispatcher listener interface.
    Clients may implement and register instances of the
    mail dispatcher interface in order to get notifications
    about the MailDispatcher status.

    @see MailDispatcher
*/
class IMailDispatcherListener : public salhelper::SimpleReferenceObject
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
    virtual void mailDelivered(::rtl::Reference<MailDispatcher> xMailDispatcher, css::uno::Reference< css::mail::XMailMessage> xMailMessage) = 0;

    /**
        Called for every mail message whose delivery
        failed.
    */
    virtual void mailDeliveryError(::rtl::Reference<MailDispatcher> xMailDispatcher, css::uno::Reference< css::mail::XMailMessage> xMailMessage, const OUString& sErrorMessage) = 0;
};

#endif // INCLUDED_IMAILDISPATCHERLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
