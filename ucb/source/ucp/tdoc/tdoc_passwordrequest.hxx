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

#ifndef INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_PASSWORDREQUEST_HXX
#define INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_PASSWORDREQUEST_HXX

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
                        OUString aPassword = xPassword->getPassword();

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
            css::task::PasswordRequestMode eMode,
            const OUString & rDocumentName );
    };

} // namespace tdoc_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_PASSWORDREQUEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
