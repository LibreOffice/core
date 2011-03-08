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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
