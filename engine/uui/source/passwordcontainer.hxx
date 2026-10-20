/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>
#include <cpo/uno/XComponentContext.hpp>

namespace com {
    namespace sun {
        namespace star {
            namespace ucb {
                class AuthenticationRequest;
                class XInteractionSupplyAuthentication;
} } } }

namespace uui {


/** Passwordcontainer UNO service (com.sun.star.task.PasswordContainer) helper.
 */
class PasswordContainerHelper
{
public:
    explicit PasswordContainerHelper(
        cpo::uno::Reference< cpo::uno::XComponentContext > const & xContext );


    /** This member function tries to handle an authentication interaction
        request by looking up credentials for the given URL in the password
        container service.

        In case of success the given interaction continuation
        (XInteractionSupplyAuthentication) is filled with the credentials found
        in the password container.

        Please note the continuation gets not "selected" by this
        implementation. "Selecting" the continuation is up to the caller (e.g.
        an implementation of XInteractionHandler::handle) of this function.

        @param rRequest
            The authentication request.

        @param xSupplyAuthentication
            The "supply authentication" interaction continuation.

        @param rURL
            The URL to lookup credentials for.

        @param xIH
            The interaction handler to use, for example if a master password is
            needed to access the password container.

        @return
            True, if the authentication request was handled successfully.
            False, otherwise.
     */
    bool handleAuthenticationRequest(
        css::ucb::AuthenticationRequest const & rRequest,
        cpo::uno::Reference< css::ucb::XInteractionSupplyAuthentication > const & xSupplyAuthentication,
        OUString const & rURL,
        cpo::uno::Reference< css::task::XInteractionHandler2 > const & xIH );

private:
    cpo::uno::Reference< css::task::XPasswordContainer2 > m_xPasswordContainer;
};


class PasswordContainerInteractionHandler :
        public cppu::WeakImplHelper< css::lang::XServiceInfo,
                                     css::task::XInteractionHandler2 >
{
public:
    explicit PasswordContainerInteractionHandler(
        const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext );
    virtual ~PasswordContainerInteractionHandler() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;

    virtual bool
    supportsService( const OUString& ServiceName ) override;

    virtual cpo::uno::Sequence< OUString >
    getSupportedServiceNames() override;

    // XInteractionHandler2
    virtual void
    handle( const cpo::uno::Reference< css::task::XInteractionRequest >& Request ) override;

    virtual bool
    handleInteractionRequest( const cpo::uno::Reference< css::task::XInteractionRequest >& Request ) override;

private:
    PasswordContainerHelper m_aPwContainerHelper;
};

} // namespace uui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
