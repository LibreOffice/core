/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
