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

#ifndef INCLUDED_COMPHELPER_SIMPLEFILEACCESSINTERACTION_HXX
#define INCLUDED_COMPHELPER_SIMPLEFILEACCESSINTERACTION_HXX

#include <ucbhelper/interceptedinteraction.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>

#include <comphelper/comphelperdllapi.h>


namespace comphelper{
    /** An interaction handler wrapper for simple file access.

        This wrapper is to be used together with XSimpleFileAccess when
        you want to avoid the error messages displayed when accessing a file (the messages activated
        by ucphelper::cancelCommandExecution).

        This wrapper is especially useful when you need to access a Web/DAV connection
        enabling https certificate validation and optionally enabling the authentication
        dialog that may be needed in these operations.

        @param xHandler
               Default handler, always needed.
               It will be used for Certificate Validation dialog.

        @param xAuthenticationHandler
               Optional authentication handler, used in Web/DAV access
               when the server requests credentials to be accessed.
     */
class COMPHELPER_DLLPUBLIC SimpleFileAccessInteraction : public ::ucbhelper::InterceptedInteraction
{
private:
    /// Will handle com::sun::star::ucb::InteractiveIOException and derived classes
    static const sal_Int32 HANDLE_INTERACTIVEIOEXCEPTION = 0;
    /// Will handle com::sun::star::ucb::UnsupportedDataSinkException
    static const sal_Int32 HANDLE_UNSUPPORTEDDATASINKEXCEPTION = 1;
    /// Will handle com::sun::star::ucb::InteractiveNetworkException
    static const sal_Int32 HANDLE_INTERACTIVENETWORKEXCEPTION = 2;
    /// Will handle com::sun::star::ucb::CertificateValidationRequest
    static const sal_Int32 HANDLE_CERTIFICATEREQUEST = 3;
    /// Will handle com::sun::star::ucb::AuthenticationRequest
    static const sal_Int32 HANDLE_AUTHENTICATIONREQUEST = 4;

public:
    SimpleFileAccessInteraction(const css::uno::Reference< css::task::XInteractionHandler >& xHandler,
                                const css::uno::Reference< css::task::XInteractionHandler >& xAuthenticationHandler = nullptr );

private:
    css::uno::Reference< css::task::XInteractionHandler > m_xAuthenticationHandler;

    virtual ucbhelper::InterceptedInteraction::EInterceptionState intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest& aRequest,
        const css::uno::Reference< css::task::XInteractionRequest >& xRequest) override;

};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
