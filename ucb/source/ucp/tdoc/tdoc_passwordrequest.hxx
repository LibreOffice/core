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

