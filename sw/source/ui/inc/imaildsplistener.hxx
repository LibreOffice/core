/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imaildsplistener.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:20:27 $
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

#ifndef INCLUDED_IMAILDSPLISTENER_HXX
#define INCLUDED_IMAILDSPLISTENER_HXX

#ifndef _COM_SUN_STAR_MAIL_XMAILMESSAGE_HPP_
#include "com/sun/star/mail/XMailMessage.hpp"
#endif

#ifndef _SALHELPER_REFOBJ_HXX_
#include <salhelper/refobj.hxx>
#endif


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
