/*************************************************************************
 *
 *  $RCSfile: simpleauthenticationrequest.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sb $ $Date: 2001-07-13 12:46:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX
#define _UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif

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
class SimpleAuthenticationRequest : public ucbhelper::InteractionRequest
{
    rtl::Reference<
        ucbhelper::InteractionSupplyAuthentication > m_xAuthSupplier;

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
