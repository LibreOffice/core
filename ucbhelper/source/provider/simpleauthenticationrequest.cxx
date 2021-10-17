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

#include <com/sun/star/ucb/URLAuthenticationRequest.hpp>
#include <ucbhelper/simpleauthenticationrequest.hxx>

using namespace com::sun::star;
using namespace ucbhelper;


SimpleAuthenticationRequest::SimpleAuthenticationRequest(
                                      const OUString & rURL,
                                      const OUString & rServerName,
                                      const OUString & rRealm,
                                      const OUString & rUserName,
                                      const OUString & rPassword,
                                      bool bAllowUseSystemCredentials,
                                      bool bAllowSessionStoring )
{

    // Fill request...
    ucb::URLAuthenticationRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification = task::InteractionClassification_ERROR;
    aRequest.ServerName     = rServerName;
//    aRequest.Diagnostic     = // OUString
    aRequest.HasRealm       = !rRealm.isEmpty();
    if ( aRequest.HasRealm )
        aRequest.Realm = rRealm;
    aRequest.HasUserName    = true;
    aRequest.UserName       = rUserName;
    aRequest.HasPassword    = true;
    aRequest.Password       = rPassword;
    aRequest.HasAccount     = false;
    aRequest.URL = rURL;

    initialize(aRequest,
       false,
       true,
       true,
       aRequest.HasAccount,
       bAllowUseSystemCredentials,
       bAllowSessionStoring );
}


SimpleAuthenticationRequest::SimpleAuthenticationRequest(
                                      const OUString & rURL,
                                      const OUString & rServerName,
                                      EntityType eRealmType,
                                      const OUString & rRealm,
                                      EntityType eUserNameType,
                                      const OUString & rUserName,
                                      EntityType ePasswordType,
                                      const OUString & rPassword)
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
    aRequest.HasAccount     = false;
    aRequest.URL = rURL;

    initialize(aRequest,
       eRealmType == ENTITY_MODIFY,
       eUserNameType == ENTITY_MODIFY,
       ePasswordType == ENTITY_MODIFY,
       false,
       false,
       true );
}


void SimpleAuthenticationRequest::initialize(
      const ucb::URLAuthenticationRequest & rRequest,
      bool bCanSetRealm,
      bool bCanSetUserName,
      bool bCanSetPassword,
      bool bCanSetAccount,
      bool bAllowUseSystemCredentials,
      bool bAllowSessionStoring )
{
    setRequest( uno::makeAny( rRequest ) );

    // Fill continuations...
    unsigned int nSize = 2;

    if( bAllowSessionStoring )
        nSize++;

    uno::Sequence< ucb::RememberAuthentication > aRememberModes( nSize );
    auto it = aRememberModes.getArray();
    *it++ = ucb::RememberAuthentication_NO;

    if( bAllowSessionStoring )
        *it++ = ucb::RememberAuthentication_SESSION;

    *it = ucb::RememberAuthentication_PERSISTENT;

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
                bAllowUseSystemCredentials // bCanUseSystemCredentials,
            );

    setContinuations(
        { new InteractionAbort(this), new InteractionRetry(this), m_xAuthSupplier.get() });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
