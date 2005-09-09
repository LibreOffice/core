/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdoc_passwordrequest.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:01:26 $
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

#ifndef INCLUDED_TDOC_PASSWORDREQUEST_HXX
#define INCLUDED_TDOC_PASSWORDREQUEST_HXX

#include "com/sun/star/task/PasswordRequestMode.hpp"
#include "com/sun/star/task/XInteractionPassword.hpp"

#include "ucbhelper/interactionrequest.hxx"

namespace tdoc_ucp {

    /*
        @usage:

        uno::Reference< ucb::XCommandEnvironment > Environment = ...;

        if ( Environment.is() )
        {
            uno::Reference< task::XInteractionHandler > xIH
                = Environment->getInteractionHandler();
            if ( xIH.is() )
            {
                rtl::Reference< DocumentPasswordRequest > xRequest
                    = new DocumentPasswordRequest(
                        task::PasswordRequestMode_PASSWORD_ENTER,
                        m_xIdentifier->getContentIdentifier() );
                xIH->handle( xRequest.get() );

                rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                    = xRequest->getSelection();

                if ( xSelection.is() )
                {
                    // Handler handled the request.
                    uno::Reference< task::XInteractionAbort > xAbort(
                        xSelection.get(), uno::UNO_QUERY );
                    if ( xAbort.is() )
                    {
                        // @@@
                    }

                    uno::Reference< task::XInteractionRetry > xRetry(
                        xSelection.get(), uno::UNO_QUERY );
                    if ( xRetry.is() )
                    {
                        // @@@
                    }

                    uno::Reference< task::XInteractionPassword > xPassword(
                        xSelection.get(), uno::UNO_QUERY );
                    if ( xPassword.is() )
                    {
                        rtl::OUString aPassword = xPassword->getPassword();

                        // @@@
                    }
                }
            }
        }

    */

    class DocumentPasswordRequest : public ucbhelper::InteractionRequest
    {
    public:
        DocumentPasswordRequest(
            com::sun::star::task::PasswordRequestMode eMode,
            const rtl::OUString & rDocumentName );
    };

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_PASSWORDREQUEST_HXX */

