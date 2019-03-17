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

#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/NoMasterException.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/XInteractionHandler2.hpp>
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#include <com/sun/star/ucb/URLAuthenticationRequest.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication2.hpp>
#include <officecfg/Office/Common.hxx>

#include "passwordcontainer.hxx"

using namespace com::sun::star;

namespace {


bool fillContinuation(
    bool bUseSystemCredentials,
    const ucb::AuthenticationRequest & rRequest,
    const task::UrlRecord & aRec,
    const uno::Reference< ucb::XInteractionSupplyAuthentication > &
        xSupplyAuthentication,
    const uno::Reference< ucb::XInteractionSupplyAuthentication2 > &
        xSupplyAuthentication2,
    bool bCanUseSystemCredentials,
    bool bCheckForEqualPasswords )
{
    if ( bUseSystemCredentials )
    {
        // "use system creds" record found.
        // Wants client that we use it?
        if ( xSupplyAuthentication2.is() && bCanUseSystemCredentials )
        {
            xSupplyAuthentication2->setUseSystemCredentials( true );
            return true;
        }
        return false;
    }
    else if (aRec.UserList.getLength() != 0)
    {
        if (aRec.UserList[0].Passwords.getLength() == 0)
        {
            // Password sequence can be empty, for instance if master
            // password was not given (e.g. master pw dialog canceled)
            // pw container does not throw NoMasterException in this case.
            // bug???
            return false;
        }

        // "user/pass" record found.
        if (!bCheckForEqualPasswords || !rRequest.HasPassword
            || rRequest.Password != aRec.UserList[0].Passwords[0]) // failed login attempt?
        {
            if (xSupplyAuthentication->canSetUserName())
                xSupplyAuthentication->
                    setUserName(aRec.UserList[0].UserName);

            if (xSupplyAuthentication->canSetPassword())
                xSupplyAuthentication->
                    setPassword(aRec.UserList[0].Passwords[0]);
            if (aRec.UserList[0].Passwords.getLength() > 1)
            {
                if (rRequest.HasRealm)
                {
                    if (xSupplyAuthentication->canSetRealm())
                        xSupplyAuthentication->
                            setRealm(aRec.UserList[0].Passwords[1]);
                }
                else if (xSupplyAuthentication->canSetAccount())
                    xSupplyAuthentication->
                        setAccount(aRec.UserList[0].Passwords[1]);
            }

            if ( xSupplyAuthentication2.is() && bCanUseSystemCredentials )
                xSupplyAuthentication2->setUseSystemCredentials( false );

            return true;
        }
    }
    return false;
}

} // namespace

namespace uui {


PasswordContainerHelper::PasswordContainerHelper(
    uno::Reference< uno::XComponentContext > const & xContext ):
    m_xPasswordContainer(task::PasswordContainer::create(xContext))
{}


bool PasswordContainerHelper::handleAuthenticationRequest(
    ucb::AuthenticationRequest const & rRequest,
    uno::Reference< ucb::XInteractionSupplyAuthentication > const &
        xSupplyAuthentication,
    OUString const & rURL,
    uno::Reference< task::XInteractionHandler2 > const & xIH )
{
    uno::Reference< task::XInteractionHandler > xIH1(xIH, uno::UNO_QUERY);

    // Is continuation even a XInteractionSupplyAuthentication2, which
    // is derived from XInteractionSupplyAuthentication?
    uno::Reference< ucb::XInteractionSupplyAuthentication2 >
        xSupplyAuthentication2(xSupplyAuthentication, uno::UNO_QUERY);

    bool bCanUseSystemCredentials = false;
    if (xSupplyAuthentication2.is())
    {
        sal_Bool bDefaultUseSystemCredentials;
        bCanUseSystemCredentials
            =  xSupplyAuthentication2->canUseSystemCredentials(
                bDefaultUseSystemCredentials );
    }

    if ( bCanUseSystemCredentials )
    {
        // Does the configuration mandate that we try system credentials first?
        bool bUseSystemCredentials = ::officecfg::Office::Common::Passwords::TrySystemCredentialsFirst::get();
        if (!bUseSystemCredentials)
        {
            // Runtime / Persistent info avail for current auth request?
            OUString aResult = m_xPasswordContainer->findUrl(
                rURL.isEmpty() ? rRequest.ServerName : rURL);
            bUseSystemCredentials = !aResult.isEmpty();
        }
        if ( bUseSystemCredentials )
        {
            if ( fillContinuation( true,
                                   rRequest,
                                   task::UrlRecord(),
                                   xSupplyAuthentication,
                                   xSupplyAuthentication2,
                                   bCanUseSystemCredentials,
                                   false ) )
            {
                return true;
            }
        }
    }

    // m_xPasswordContainer works with userName passwdSequences pairs:
    if (rRequest.HasUserName && rRequest.HasPassword)
    {
        try
        {
            if (rRequest.UserName.isEmpty())
            {
                task::UrlRecord aRec;
                if ( !rURL.isEmpty() )
                    aRec = m_xPasswordContainer->find(rURL, xIH1);

                if ( aRec.UserList.getLength() == 0 )
                {
                    // compat: try server name.
                    aRec = m_xPasswordContainer->find(rRequest.ServerName, xIH1);
                }

                if ( fillContinuation( false,
                                       rRequest,
                                       aRec,
                                       xSupplyAuthentication,
                                       xSupplyAuthentication2,
                                       bCanUseSystemCredentials,
                                       false ) )
                {
                    return true;
                }
            }
            else
            {
                task::UrlRecord aRec;
                if ( !rURL.isEmpty() )
                    aRec = m_xPasswordContainer->findForName(
                        rURL, rRequest.UserName, xIH1);

                if ( aRec.UserList.getLength() == 0 )
                {
                    // compat: try server name.
                    aRec = m_xPasswordContainer->findForName(
                        rRequest.ServerName, rRequest.UserName, xIH1);
                }

                if ( fillContinuation( false,
                                       rRequest,
                                       aRec,
                                       xSupplyAuthentication,
                                       xSupplyAuthentication2,
                                       bCanUseSystemCredentials,
                                       true ) )
                {
                    return true;
                }
            }
        }
        catch (task::NoMasterException const &)
        {} // user did not enter master password
    }
    return false;
}


bool PasswordContainerHelper::addRecord(
    OUString const & rURL,
    OUString const & rUsername,
    uno::Sequence< OUString > const & rPasswords,
    uno::Reference< task::XInteractionHandler2 > const & xIH,
    bool bPersist )
{
    uno::Reference< task::XInteractionHandler > xIH1(xIH, uno::UNO_QUERY);

    try
    {
        if ( !rUsername.isEmpty() )
        {
            OSL_ENSURE( m_xPasswordContainer.is(),
                        "Got no XPasswordContainer!" );
            if ( !m_xPasswordContainer.is() )
                return false;

            if ( bPersist )
            {
                // If persistent storing of passwords is not yet
                // allowed, enable it.
                if ( !m_xPasswordContainer->isPersistentStoringAllowed() )
                    m_xPasswordContainer->allowPersistentStoring( true );

                m_xPasswordContainer->addPersistent( rURL,
                                                     rUsername,
                                                     rPasswords,
                                                     xIH1 );
            }
            else
                m_xPasswordContainer->add( rURL,
                                           rUsername,
                                           rPasswords,
                                           xIH1 );
        }
        else
        {
            m_xPasswordContainer->addUrl( rURL, bPersist );
        }
    }
    catch ( task::NoMasterException const & )
    {
        // user did not enter master password
        return false;
    }
    return true;
}


PasswordContainerInteractionHandler::PasswordContainerInteractionHandler(
    const uno::Reference< uno::XComponentContext >& xContext )
: m_aPwContainerHelper( xContext )
{
}


// virtual
PasswordContainerInteractionHandler::~PasswordContainerInteractionHandler()
{
}


// XServiceInfo methods.


// virtual
OUString SAL_CALL
PasswordContainerInteractionHandler::getImplementationName()
{
    return getImplementationName_Static();
}


// virtual
sal_Bool SAL_CALL
PasswordContainerInteractionHandler::supportsService(
        const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


// virtual
uno::Sequence< OUString > SAL_CALL
PasswordContainerInteractionHandler::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}


// static
OUString
PasswordContainerInteractionHandler::getImplementationName_Static()
{
    return OUString( "com.sun.star.comp.uui.PasswordContainerInteractionHandler" );
}


// static
uno::Sequence< OUString >
PasswordContainerInteractionHandler::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS { "com.sun.star.task.PasswordContainerInteractionHandler" };
    return aSNS;
}


// XInteractionHandler2 methods.


// virtual
void SAL_CALL
PasswordContainerInteractionHandler::handle(
        const uno::Reference< task::XInteractionRequest >& rRequest )
{
    handleInteractionRequest( rRequest );
}

// virtual
sal_Bool SAL_CALL
PasswordContainerInteractionHandler::handleInteractionRequest(
        const uno::Reference< task::XInteractionRequest >& rRequest )
{
    if ( !rRequest.is() )
        return false;

    uno::Any aAnyRequest( rRequest->getRequest() );

    ucb::AuthenticationRequest aAuthenticationRequest;
    if ( !( aAnyRequest >>= aAuthenticationRequest ) )
        return false;

    OUString aURL;
    ucb::URLAuthenticationRequest aURLAuthenticationRequest;
    if ( aAnyRequest >>= aURLAuthenticationRequest )
        aURL = aURLAuthenticationRequest.URL;

    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
        rContinuations = rRequest->getContinuations();

    uno::Reference< ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;

    for ( sal_Int32 i = 0; i < rContinuations.getLength(); ++i )
    {
        xSupplyAuthentication.set( rContinuations[i], uno::UNO_QUERY );
        if( xSupplyAuthentication.is() )
            break;
    }

    if ( !xSupplyAuthentication.is() )
        return false;

    // Try to obtain credentials from password container.
    if ( m_aPwContainerHelper.
             handleAuthenticationRequest( aAuthenticationRequest,
                                          xSupplyAuthentication,
                                          aURL,
                                          // @@@ FIXME: this not able to
                                          // handle master pw request!
                                          // master pw request is never
                                          // solvable without UI!
                                          this ) )
    {
        // successfully handled
        xSupplyAuthentication->select();
        return true;
    }
    return false;
}


// Service factory implementation.

/// @throws uno::Exception
static uno::Reference< uno::XInterface >
PasswordContainerInteractionHandler_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory> & rSMgr )
{
    lang::XServiceInfo* pX
        = new PasswordContainerInteractionHandler(comphelper::getComponentContext(rSMgr));
    return uno::Reference< uno::XInterface >::query( pX );
}


// static
uno::Reference< lang::XSingleServiceFactory >
PasswordContainerInteractionHandler::createServiceFactory(
    const uno::Reference< lang::XMultiServiceFactory >& rxServiceMgr )
{
    return cppu::createOneInstanceFactory(
            rxServiceMgr,
            PasswordContainerInteractionHandler::getImplementationName_Static(),
            PasswordContainerInteractionHandler_CreateInstance,
            PasswordContainerInteractionHandler::getSupportedServiceNames_Static() );
}

} // namespace uui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
