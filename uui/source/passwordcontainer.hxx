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

#ifndef INCLUDED_UUI_PASSWORDCONTAINER_HXX
#define INCLUDED_UUI_PASSWORDCONTAINER_HXX

#include "cppuhelper/implbase2.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/task/XPasswordContainer.hpp"

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

// ============================================================================

/** Passwordcontainer UNO service (com.sun.star.task.PasswordContainer) helper.
 */
class PasswordContainerHelper
{
public:
    PasswordContainerHelper(
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext > const &
                xContext );

    // ------------------------------------------------------------------------

    /** This member function tries to handle an authentication interaction
        request by looking up credentials for the given URL in the password
        container service.

        In case of success the given interaction continuation
        (XInteractionSupplyAuthentication) is filled with the credentials found
        in the password container.

        Please note the the continuation gets not "selected" by this
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
        com::sun::star::ucb::AuthenticationRequest const & rRequest,
        com::sun::star::uno::Reference<
            com::sun::star::ucb::XInteractionSupplyAuthentication > const &
                xSupplyAuthentication,
        rtl::OUString const & rURL,
        com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionHandler > const & xIH )
    SAL_THROW( (com::sun::star::uno::RuntimeException) );

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
    bool addRecord( rtl::OUString const & rURL,
                    rtl::OUString const & rUsername,
                    com::sun::star::uno::Sequence< rtl::OUString > const &
                    rPasswords,
                    com::sun::star::uno::Reference<
                        com::sun::star::task::XInteractionHandler > const & xIH,
                    bool bPersist )
        SAL_THROW( (com::sun::star::uno::RuntimeException) );

    // ------------------------------------------------------------------------

private:
    com::sun::star::uno::Reference<
        com::sun::star::task::XPasswordContainer > m_xPasswordContainer;
};

// ============================================================================

class PasswordContainerInteractionHandler :
        public cppu::WeakImplHelper2< com::sun::star::lang::XServiceInfo,
                                      com::sun::star::task::XInteractionHandler >
{
public:
    PasswordContainerInteractionHandler(
        const com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext >& xContext );
    virtual ~PasswordContainerInteractionHandler();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw ( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    supportsService( const ::rtl::OUString& ServiceName )
        throw ( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( com::sun::star::uno::RuntimeException );

    // XInteractionHandler
    virtual void SAL_CALL
    handle( const ::com::sun::star::uno::Reference<
                ::com::sun::star::task::XInteractionRequest >& Request )
        throw (::com::sun::star::uno::RuntimeException);

    // Non-UNO interfaces
    static rtl::OUString
    getImplementationName_Static();

    static com::sun::star::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_Static();

    static com::sun::star::uno::Reference<
            com::sun::star::lang::XSingleServiceFactory >
    createServiceFactory( const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > & rxServiceMgr );

private:
    //com::sun::star::uno::Reference<
    //    com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    PasswordContainerHelper m_aPwContainerHelper;
};

} // namespace uui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
