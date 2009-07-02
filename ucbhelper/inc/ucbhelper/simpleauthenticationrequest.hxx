/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: simpleauthenticationrequest.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX
#define _UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper {

/**
  * This class implements a simple authentication interaction request.
  * Instances can be passed directly to XInteractionHandler::handle(...). Each
  * instance contains an AuthenticationRequest and three interaction
  * continuations: "Abort", "Retry" and "SupplyAuthentication". The parameters
  * for the AuthenticationRequest and the InteractionSupplyAuthentication
  * objects are partly taken from contructors parameters and partly defaulted
  * as follows:
  *
  * Read-only values : servername, realm
  * Read-write values: username, password, account
  * All remember-authentication values: RememberAuthentication_NO
  *
  * @see com::sun::star::ucb::AuthenticationRequest
  * @see com::sun::star::ucb::RememberAuthentication
  * @see InteractionAbort
  * @see InteractionRetry
  * @see InteractionSupplyAuthentication
  */
class UCBHELPER_DLLPUBLIC SimpleAuthenticationRequest : public ucbhelper::InteractionRequest
{
    rtl::Reference<
        ucbhelper::InteractionSupplyAuthentication > m_xAuthSupplier;

private:
    void initialize( ::com::sun::star::ucb::AuthenticationRequest aRequest,
                const sal_Bool &  bCanSetRealm,
                const sal_Bool &  bCanSetUserName,
                const sal_Bool &  bCanSetPassword,
                const sal_Bool &  bCanSetAccount,
                 const sal_Bool &  bAllowPersistentStoring );

public:
    /** Specification whether some entity (realm, username, password, account)
        is either not applicable at all, has a fixed value, or is modifiable.
     */
    enum EntityType
    {
        ENTITY_NA,
        ENTITY_FIXED,
        ENTITY_MODIFY
    };

    /**
      * Constructor.
      *
      * @param rServerName contains a server name.
      * @param rRealm contains a realm, if applicable.
      * @param rUserName contains a username, if available (for instance from
      *        a previous try).
      * @param rPassword contains a password, if available (for instance from
      *        a previous try).
      * @param rAccount contains an account, if applicable.
      */
    SimpleAuthenticationRequest( const rtl::OUString & rServerName,
                                 const rtl::OUString & rRealm,
                                 const rtl::OUString & rUserName,
                                 const rtl::OUString & rPassword,
                                 const rtl::OUString & rAccount
                                    = rtl::OUString() );

    /**
      * Constructor.
      *
      * @param rServerName contains a server name.
      * @param rRealm contains a realm, if applicable.
      * @param rUserName contains a username, if available (for instance from
      *        a previous try).
      * @param rPassword contains a password, if available (for instance from
      *        a previous try).
      * @param rAccount contains an account, if applicable.
      * @param bAllowPersistentStoring specifies if the credentials should stored in the passowrd container persistently
      */
    SimpleAuthenticationRequest( const rtl::OUString & rServerName,
                                 const rtl::OUString & rRealm,
                                 const rtl::OUString & rUserName,
                                 const rtl::OUString & rPassword,
                                 const rtl::OUString & rAccount,
                                 const sal_Bool & bAllowPersistentStoring);


    /**
      * Constructor.
      *
      * @param rServerName contains a server name.
      * @param eRealmType specifies whether a realm is applicable and
               modifiable.
      * @param rRealm contains a realm, if applicable.
      * @param eUserNameType specifies whether a username is applicable and
               modifiable.
      * @param rUserName contains a username, if available (for instance from
      *        a previous try).
      * @param ePasswordType specifies whether a password is applicable and
               modifiable.
      * @param rPassword contains a password, if available (for instance from
      *        a previous try).
      * @param eAccountType specifies whether an account is applicable and
               modifiable.
      * @param rAccount contains an account, if applicable.
      */
    SimpleAuthenticationRequest( const rtl::OUString & rServerName,
                                 EntityType eRealmType,
                                 const rtl::OUString & rRealm,
                                 EntityType eUserNameType,
                                 const rtl::OUString & rUserName,
                                 EntityType ePasswordType,
                                 const rtl::OUString & rPassword,
                                 EntityType eAccountType = ENTITY_NA,
                                 const rtl::OUString & rAccount
                                    = rtl::OUString() );

     /**
      * Constructor.
      *
      * @param rServerName contains a server name.
      * @param eRealmType specifies whether a realm is applicable and
               modifiable.
      * @param rRealm contains a realm, if applicable.
      * @param eUserNameType specifies whether a username is applicable and
               modifiable.
      * @param rUserName contains a username, if available (for instance from
      *        a previous try).
      * @param ePasswordType specifies whether a password is applicable and
               modifiable.
      * @param rPassword contains a password, if available (for instance from
      *        a previous try).
      * @param eAccountType specifies whether an account is applicable and
               modifiable.
      * @param rAccount contains an account, if applicable.
      * @param bAllowPersistentStoring specifies if the credentials should stored in the passowrd container persistently
      */
    SimpleAuthenticationRequest( const rtl::OUString & rServerName,
                                 EntityType eRealmType,
                                 const rtl::OUString & rRealm,
                                 EntityType eUserNameType,
                                 const rtl::OUString & rUserName,
                                 EntityType ePasswordType,
                                 const rtl::OUString & rPassword,
                                 EntityType eAccountType,
                                 const rtl::OUString & rAccount,
                                 const sal_Bool & bAllowPersistentStoring);

    /**
      * This method returns the supplier for the missing authentication data,
      * that, for instance can be used to query the password supplied by the
      * interaction handler.
      *
      * @return the supplier for the missing authentication data.
      */
    const rtl::Reference< ucbhelper::InteractionSupplyAuthentication > &
    getAuthenticationSupplier() const { return m_xAuthSupplier; }
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX */
