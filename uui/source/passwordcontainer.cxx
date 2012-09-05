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

#include "comphelper/componentcontext.hxx"
#include "cppuhelper/factory.hxx"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/task/PasswordContainer.hpp"
#include "com/sun/star/task/NoMasterException.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/task/XMasterPasswordHandling.hpp"
#include "com/sun/star/task/XPasswordContainer.hpp"
#include "com/sun/star/task/XUrlContainer.hpp"
#include "com/sun/star/ucb/AuthenticationRequest.hpp"
#include "com/sun/star/ucb/URLAuthenticationRequest.hpp"
#include "com/sun/star/ucb/XInteractionSupplyAuthentication.hpp"
#include "com/sun/star/ucb/XInteractionSupplyAuthentication2.hpp"

#include "passwordcontainer.hxx"

using namespace com::sun::star;

namespace {

//=========================================================================
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
            xSupplyAuthentication2->setUseSystemCredentials( sal_True );
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
                    setUserName(aRec.UserList[0].UserName.getStr());

            if (xSupplyAuthentication->canSetPassword())
                xSupplyAuthentication->
                    setPassword(aRec.UserList[0].Passwords[0].getStr());
            if (aRec.UserList[0].Passwords.getLength() > 1)
            {
                if (rRequest.HasRealm)
                {
                    if (xSupplyAuthentication->canSetRealm())
                        xSupplyAuthentication->
                            setRealm(aRec.UserList[0].Passwords[1].
                                getStr());
                }
                else if (xSupplyAuthentication->canSetAccount())
                    xSupplyAuthentication->
                        setAccount(aRec.UserList[0].Passwords[1].
                            getStr());
            }

            if ( xSupplyAuthentication2.is() && bCanUseSystemCredentials )
                xSupplyAuthentication2->setUseSystemCredentials( sal_False );

            return true;
        }
    }
    return false;
}

} // namespace

namespace uui {

//=========================================================================
PasswordContainerHelper::PasswordContainerHelper(
    uno::Reference< uno::XComponentContext > const & xContext )
{
    OSL_ENSURE(xContext.is(), "no service factory given!");
    if (xContext.is())
        try
        {
            m_xPasswordContainer
                = uno::Reference< task::XPasswordContainer >(
                      task::PasswordContainer::create(xContext),
                      uno::UNO_QUERY);
        }
        catch (uno::Exception const &)
        {}
    OSL_ENSURE(m_xPasswordContainer.is(),
               "unable to instanciate password container service");
}

//=========================================================================
bool PasswordContainerHelper::handleAuthenticationRequest(
    ucb::AuthenticationRequest const & rRequest,
    uno::Reference< ucb::XInteractionSupplyAuthentication > const &
        xSupplyAuthentication,
    rtl::OUString const & rURL,
    uno::Reference< task::XInteractionHandler > const & xIH )
        SAL_THROW((uno::RuntimeException))
{
    // Is continuation even a XInteractionSupplyAuthentication2, which
    // is derived from XInteractionSupplyAuthentication?
    uno::Reference< ucb::XInteractionSupplyAuthentication2 >
        xSupplyAuthentication2(xSupplyAuthentication, uno::UNO_QUERY);

    sal_Bool bCanUseSystemCredentials = sal_False;
    if (xSupplyAuthentication2.is())
    {
        sal_Bool bDefaultUseSystemCredentials;
        bCanUseSystemCredentials
            =  xSupplyAuthentication2->canUseSystemCredentials(
                bDefaultUseSystemCredentials );
    }

    uno::Reference< task::XPasswordContainer > xContainer(
        m_xPasswordContainer );
    uno::Reference< task::XUrlContainer > xUrlContainer(
        m_xPasswordContainer, uno::UNO_QUERY );
    OSL_ENSURE( xUrlContainer.is(), "Got no XUrlContainer!" );

    if ( !xContainer.is() || !xUrlContainer.is() )
        return false;

    if ( bCanUseSystemCredentials )
    {
        // Runtime / Persistent info avail for current auth request?

        rtl::OUString aResult = xUrlContainer->findUrl(
            rURL.isEmpty() ? rRequest.ServerName : rURL );
        if ( !aResult.isEmpty() )
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

    // xContainer works with userName passwdSequences pairs:
    if (rRequest.HasUserName && rRequest.HasPassword)
    {
        try
        {
            if (rRequest.UserName.isEmpty())
            {
                task::UrlRecord aRec;
                if ( !rURL.isEmpty() )
                    aRec = xContainer->find(rURL, xIH);

                if ( aRec.UserList.getLength() == 0 )
                {
                    // compat: try server name.
                    aRec = xContainer->find(rRequest.ServerName, xIH);
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
                    aRec = xContainer->findForName(
                        rURL, rRequest.UserName, xIH);

                if ( aRec.UserList.getLength() == 0 )
                {
                    // compat: try server name.
                    aRec = xContainer->findForName(
                        rRequest.ServerName, rRequest.UserName, xIH);
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

//=========================================================================
bool PasswordContainerHelper::addRecord(
    rtl::OUString const & rURL,
    rtl::OUString const & rUsername,
    uno::Sequence< rtl::OUString > const & rPasswords,
    uno::Reference< task::XInteractionHandler > const & xIH,
    bool bPersist )
        SAL_THROW((uno::RuntimeException))
{
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
                uno::Reference< task::XMasterPasswordHandling > xMPH(
                    m_xPasswordContainer, uno::UNO_QUERY_THROW );

                // If persistent storing of passwords is not yet
                // allowed, enable it.
                if ( !xMPH->isPersistentStoringAllowed() )
                    xMPH->allowPersistentStoring( sal_True );

                m_xPasswordContainer->addPersistent( rURL,
                                                     rUsername,
                                                     rPasswords,
                                                     xIH );
            }
            else
                m_xPasswordContainer->add( rURL,
                                           rUsername,
                                           rPasswords,
                                           xIH );
        }
        else
        {
            uno::Reference< task::XUrlContainer >
                xContainer( m_xPasswordContainer, uno::UNO_QUERY );
            OSL_ENSURE( xContainer.is(), "Got no XUrlContainer!" );
            if ( !xContainer.is() )
                return false;

            xContainer->addUrl( rURL, bPersist );
        }
    }
    catch ( task::NoMasterException const & )
    {
        // user did not enter master password
        return false;
    }
    return true;
}

//=========================================================================
//=========================================================================
//=========================================================================

PasswordContainerInteractionHandler::PasswordContainerInteractionHandler(
    const uno::Reference< uno::XComponentContext >& xContext )
: m_aPwContainerHelper( xContext )
{
}

//=========================================================================
// virtual
PasswordContainerInteractionHandler::~PasswordContainerInteractionHandler()
{
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
::rtl::OUString SAL_CALL
PasswordContainerInteractionHandler::getImplementationName()
    throw ( uno::RuntimeException )
{
    return getImplementationName_Static();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL
PasswordContainerInteractionHandler::supportsService(
        const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSNL = getSupportedServiceNames();
    const rtl::OUString * pArray = aSNL.getConstArray();
    for ( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if ( pArray[ i ] == ServiceName )
            return sal_True;
    }
    return sal_False;
}

//=========================================================================
// virtual
uno::Sequence< ::rtl::OUString > SAL_CALL
PasswordContainerInteractionHandler::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

//=========================================================================
// static
rtl::OUString
PasswordContainerInteractionHandler::getImplementationName_Static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.uui.PasswordContainerInteractionHandler" ) );
}

//=========================================================================
// static
uno::Sequence< rtl::OUString >
PasswordContainerInteractionHandler::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
        = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.task.PasswordContainerInteractionHandler" ) );
    return aSNS;
}

//=========================================================================
//
// XInteractionHandler methods.
//
//=========================================================================

// virtual
void SAL_CALL
PasswordContainerInteractionHandler::handle(
        const uno::Reference< task::XInteractionRequest >& rRequest )
    throw ( uno::RuntimeException )
{
    if ( !rRequest.is() )
        return;

    uno::Any aAnyRequest( rRequest->getRequest() );

    ucb::AuthenticationRequest aAuthenticationRequest;
    if ( !( aAnyRequest >>= aAuthenticationRequest ) )
        return;

    rtl::OUString aURL;
    ucb::URLAuthenticationRequest aURLAuthenticationRequest;
    if ( aAnyRequest >>= aURLAuthenticationRequest )
        aURL = aURLAuthenticationRequest.URL;

    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
        rContinuations = rRequest->getContinuations();

    uno::Reference< ucb::XInteractionSupplyAuthentication >
        xSupplyAuthentication;

    for ( sal_Int32 i = 0; i < rContinuations.getLength(); ++i )
    {
        xSupplyAuthentication
            = uno::Reference< ucb::XInteractionSupplyAuthentication >(
                rContinuations[i], uno::UNO_QUERY );
        if( xSupplyAuthentication.is() )
            break;
    }

    if ( !xSupplyAuthentication.is() )
        return;

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
    }
}

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

static uno::Reference< uno::XInterface > SAL_CALL
PasswordContainerInteractionHandler_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory> & rSMgr )
    throw( uno::Exception )
{
    lang::XServiceInfo * pX = static_cast< lang::XServiceInfo * >(
        new PasswordContainerInteractionHandler( comphelper::ComponentContext(rSMgr).getUNOContext() ) );
    return uno::Reference< uno::XInterface >::query( pX );
}

//=========================================================================
// static
uno::Reference< lang::XSingleServiceFactory >
PasswordContainerInteractionHandler::createServiceFactory(
    const uno::Reference< lang::XMultiServiceFactory >& rxServiceMgr )
{
    return uno::Reference< lang::XSingleServiceFactory >(
        cppu::createOneInstanceFactory(
            rxServiceMgr,
            PasswordContainerInteractionHandler::getImplementationName_Static(),
            PasswordContainerInteractionHandler_CreateInstance,
            PasswordContainerInteractionHandler::getSupportedServiceNames_Static() ) );
}

} // namespace uui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
