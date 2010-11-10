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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"
#include <com/sun/star/task/XMasterPasswordHandling.hpp>
#include <com/sun/star/ucb/URLAuthenticationRequest.hpp>
#include <ucbhelper/simpleauthenticationrequest.hxx>

using namespace com::sun::star;
using namespace ucbhelper;

//=========================================================================
SimpleAuthenticationRequest::SimpleAuthenticationRequest(
                                      const rtl::OUString & rURL,
                                      const rtl::OUString & rServerName,
                                      const rtl::OUString & rRealm,
                                      const rtl::OUString & rUserName,
                                      const rtl::OUString & rPassword,
                                      const rtl::OUString & rAccount )
{
    // Fill request...
    ucb::URLAuthenticationRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification = task::InteractionClassification_ERROR;
    aRequest.ServerName     = rServerName;
//    aRequest.Diagnostic     = // OUString
    aRequest.HasRealm       = ( rRealm.getLength() > 0 );
    if ( aRequest.HasRealm )
        aRequest.Realm = rRealm;
    aRequest.HasUserName    = sal_True;
    aRequest.UserName       = rUserName;
    aRequest.HasPassword    = sal_True;
    aRequest.Password       = rPassword;
    aRequest.HasAccount     = ( rAccount.getLength() > 0 );
    if ( aRequest.HasAccount )
        aRequest.Account = rAccount;
    aRequest.URL = rURL;

    initialize(aRequest,
       sal_False,
       sal_True,
       sal_True,
       aRequest.HasAccount,
       sal_True,
       sal_False );
}
//=========================================================================
SimpleAuthenticationRequest::SimpleAuthenticationRequest(
                                      const rtl::OUString & rURL,
                                      const rtl::OUString & rServerName,
                                      const rtl::OUString & rRealm,
                                      const rtl::OUString & rUserName,
                                      const rtl::OUString & rPassword,
                                      const rtl::OUString & rAccount,
                                      sal_Bool bAllowPersistentStoring,
                                      sal_Bool bAllowUseSystemCredentials )
{

    // Fill request...
    ucb::URLAuthenticationRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification = task::InteractionClassification_ERROR;
    aRequest.ServerName     = rServerName;
//    aRequest.Diagnostic     = // OUString
    aRequest.HasRealm       = ( rRealm.getLength() > 0 );
    if ( aRequest.HasRealm )
        aRequest.Realm = rRealm;
    aRequest.HasUserName    = sal_True;
    aRequest.UserName       = rUserName;
    aRequest.HasPassword    = sal_True;
    aRequest.Password       = rPassword;
    aRequest.HasAccount     = ( rAccount.getLength() > 0 );
    if ( aRequest.HasAccount )
        aRequest.Account = rAccount;
    aRequest.URL = rURL;

    initialize(aRequest,
       sal_False,
       sal_True,
       sal_True,
       aRequest.HasAccount,
       bAllowPersistentStoring,
       bAllowUseSystemCredentials );
}

//=========================================================================
SimpleAuthenticationRequest::SimpleAuthenticationRequest(
                                      const rtl::OUString & rURL,
                                      const rtl::OUString & rServerName,
                                      EntityType eRealmType,
                                      const rtl::OUString & rRealm,
                                      EntityType eUserNameType,
                                      const rtl::OUString & rUserName,
                                      EntityType ePasswordType,
                                      const rtl::OUString & rPassword,
                                      EntityType eAccountType,
                                      const rtl::OUString & rAccount )
{
    // Fill request...
    ucb::URLAuthenticationRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification = task::InteractionClassification_ERROR;
    aRequest.ServerName     = rServerName;
//    aRequest.Diagnostic     = // OUString
    aRequest.HasRealm       = eRealmType != ENTITY_NA;
    if ( aRequest.HasRealm )
        aRequest.Realm = rRealm;
    aRequest.HasUserName    = eUserNameType != ENTITY_NA;
    if ( aRequest.HasUserName )
        aRequest.UserName = rUserName;
    aRequest.HasPassword    = ePasswordType != ENTITY_NA;
    if ( aRequest.HasPassword )
        aRequest.Password = rPassword;
    aRequest.HasAccount     = eAccountType != ENTITY_NA;
    if ( aRequest.HasAccount )
        aRequest.Account = rAccount;
    aRequest.URL = rURL;

    initialize(aRequest,
       eRealmType == ENTITY_MODIFY,
       eUserNameType == ENTITY_MODIFY,
       ePasswordType == ENTITY_MODIFY,
       eAccountType == ENTITY_MODIFY,
       sal_True,
       sal_False );
}

//=========================================================================
SimpleAuthenticationRequest::SimpleAuthenticationRequest(
                                      const rtl::OUString & rURL,
                                      const rtl::OUString & rServerName,
                                      EntityType eRealmType,
                                      const rtl::OUString & rRealm,
                                      EntityType eUserNameType,
                                      const rtl::OUString & rUserName,
                                      EntityType ePasswordType,
                                      const rtl::OUString & rPassword,
                                      EntityType eAccountType,
                                      const rtl::OUString & rAccount,
                                      sal_Bool bAllowPersistentStoring,
                                      sal_Bool bAllowUseSystemCredentials )
{
    // Fill request...
    ucb::URLAuthenticationRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification = task::InteractionClassification_ERROR;
    aRequest.ServerName     = rServerName;
//    aRequest.Diagnostic     = // OUString
    aRequest.HasRealm       = eRealmType != ENTITY_NA;
    if ( aRequest.HasRealm )
        aRequest.Realm = rRealm;
    aRequest.HasUserName    = eUserNameType != ENTITY_NA;
    if ( aRequest.HasUserName )
        aRequest.UserName = rUserName;
    aRequest.HasPassword    = ePasswordType != ENTITY_NA;
    if ( aRequest.HasPassword )
        aRequest.Password = rPassword;
    aRequest.HasAccount     = eAccountType != ENTITY_NA;
    if ( aRequest.HasAccount )
        aRequest.Account = rAccount;
    aRequest.URL = rURL;

    initialize(aRequest,
       eRealmType == ENTITY_MODIFY,
       eUserNameType == ENTITY_MODIFY,
       ePasswordType == ENTITY_MODIFY,
       eAccountType == ENTITY_MODIFY,
       bAllowPersistentStoring,
       bAllowUseSystemCredentials );
}

//=========================================================================
void SimpleAuthenticationRequest::initialize(
      const ucb::URLAuthenticationRequest & rRequest,
      sal_Bool bCanSetRealm,
      sal_Bool bCanSetUserName,
      sal_Bool bCanSetPassword,
      sal_Bool bCanSetAccount,
      sal_Bool bAllowPersistentStoring,
      sal_Bool bAllowUseSystemCredentials )
{
    setRequest( uno::makeAny( rRequest ) );

    // Fill continuations...
    uno::Sequence< ucb::RememberAuthentication > aRememberModes(
        bAllowPersistentStoring ? 3 : 2 );
    aRememberModes[ 0 ] = ucb::RememberAuthentication_NO;
    aRememberModes[ 1 ] = ucb::RememberAuthentication_SESSION;
    if (bAllowPersistentStoring)
        aRememberModes[ 2 ] = ucb::RememberAuthentication_PERSISTENT;

    m_xAuthSupplier
        = new InteractionSupplyAuthentication(
                this,
                bCanSetRealm,
                bCanSetUserName,
                bCanSetPassword,
                bCanSetAccount,
                aRememberModes, // rRememberPasswordModes
                ucb::RememberAuthentication_SESSION, // eDefaultRememberPasswordMode
                aRememberModes, // rRememberAccountModes
                ucb::RememberAuthentication_SESSION, // eDefaultRememberAccountMode
                bAllowUseSystemCredentials, // bCanUseSystemCredentials,
                false // bDefaultUseSystemCredentials
            );

    uno::Sequence<
        uno::Reference< task::XInteractionContinuation > > aContinuations( 3 );
    aContinuations[ 0 ] = new InteractionAbort( this );
    aContinuations[ 1 ] = new InteractionRetry( this );
    aContinuations[ 2 ] = m_xAuthSupplier.get();

    setContinuations( aContinuations );
}
