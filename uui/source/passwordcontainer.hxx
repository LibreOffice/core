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

#ifndef INCLUDED_UUI_SOURCE_PASSWORDCONTAINER_HXX
#define INCLUDED_UUI_SOURCE_PASSWORDCONTAINER_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>

namespace com {
    namespace sun {
        namespace star {
            namespace lang {
                class XMultiServiceFactory;
            }
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
        css::uno::Reference< css::uno::XComponentContext > const & xContext );



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
        css::uno::Reference< css::ucb::XInteractionSupplyAuthentication > const & xSupplyAuthentication,
        OUString const & rURL,
        css::uno::Reference< css::task::XInteractionHandler2 > const & xIH );

    /** This member function adds credentials for the given URL to the password
        container.

        @param rURL
            The URL the credentials are valid for. rURL must not be empty.

        @param rUsername
            The user name.

        @param rPasswords
            This list of passwords.

        @param xIH
            The interaction handler to use, for example if a master password is
            needed to access the password container.

        @param bPersist
            True, the record will get stored persistently; restored upon
            password container initialization
            False, the record will be stored until password container instance
            gets destroyed.

        @return
            True, if the record was added successfully.
            False, otherwise.

    */
    bool addRecord( OUString const & rURL,
                    OUString const & rUsername,
                    css::uno::Sequence< OUString > const & rPasswords,
                    css::uno::Reference< css::task::XInteractionHandler2 > const & xIH,
                    bool bPersist );



private:
    css::uno::Reference< css::task::XPasswordContainer2 > m_xPasswordContainer;
};



class PasswordContainerInteractionHandler :
        public cppu::WeakImplHelper< css::lang::XServiceInfo,
                                     css::task::XInteractionHandler2 >
{
public:
    explicit PasswordContainerInteractionHandler(
        const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~PasswordContainerInteractionHandler();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    supportsService( const OUString& ServiceName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XInteractionHandler2
    virtual void SAL_CALL
    handle( const css::uno::Reference< css::task::XInteractionRequest >& Request )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
    handleInteractionRequest( const css::uno::Reference< css::task::XInteractionRequest >& Request )
        throw (css::uno::RuntimeException, std::exception) override;

    // Non-UNO interfaces
    static OUString
    getImplementationName_Static();

    static css::uno::Sequence< OUString >
    getSupportedServiceNames_Static();

    static css::uno::Reference< css::lang::XSingleServiceFactory >
    createServiceFactory( const css::uno::Reference< css::lang::XMultiServiceFactory > & rxServiceMgr );

private:
    PasswordContainerHelper m_aPwContainerHelper;
};

} // namespace uui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
