/*************************************************************************
 *
 *  $RCSfile: simpleauthenticationrequest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sb $ $Date: 2001-07-13 12:49:57 $
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

#ifndef _COM_SUN_STAR_UCB_AUTHENTICATIONREQUEST_HPP_
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#endif

#ifndef _UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX
#include <ucbhelper/simpleauthenticationrequest.hxx>
#endif

using namespace com::sun::star;
using namespace ucbhelper;

//=========================================================================
SimpleAuthenticationRequest::SimpleAuthenticationRequest(
                                      const rtl::OUString & rServerName,
                                      const rtl::OUString & rRealm,
                                      const rtl::OUString & rUserName,
                                      const rtl::OUString & rPassword,
                                      const rtl::OUString & rAccount )
{
    // Fill request...
    ucb::AuthenticationRequest aRequest;
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

    setRequest( uno::makeAny( aRequest ) );

    // Fill continuations...
    uno::Sequence< ucb::RememberAuthentication > aRememberModes( 1 );
    aRememberModes[ 0 ] = ucb::RememberAuthentication_NO;

    m_xAuthSupplier
        = new InteractionSupplyAuthentication(
                this,
                sal_False, // bCanSetRealm
                sal_True,  // bCanSetUserName
                sal_True,  // bCanSetPassword
                aRequest.HasAccount, // bCanSetAccount
                aRememberModes, // rRememberPasswordModes
                ucb::RememberAuthentication_NO, // eDefaultRememberPasswordMode
                aRememberModes, // rRememberAccountModes
                ucb::RememberAuthentication_NO // eDefaultRememberAccountMode
            );

    uno::Sequence<
        uno::Reference< task::XInteractionContinuation > > aContinuations( 3 );
    aContinuations[ 0 ] = new InteractionAbort( this );
    aContinuations[ 1 ] = new InteractionRetry( this );
    aContinuations[ 2 ] = m_xAuthSupplier.get();

    setContinuations( aContinuations );
}

//=========================================================================
SimpleAuthenticationRequest::SimpleAuthenticationRequest(
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
    ucb::AuthenticationRequest aRequest;
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

    setRequest( uno::makeAny( aRequest ) );

    // Fill continuations...
    uno::Sequence< ucb::RememberAuthentication > aRememberModes( 1 );
    aRememberModes[ 0 ] = ucb::RememberAuthentication_NO;

    m_xAuthSupplier
        = new InteractionSupplyAuthentication(
                this,
                eRealmType == ENTITY_MODIFY, // bCanSetRealm
                eUserNameType == ENTITY_MODIFY,  // bCanSetUserName
                ePasswordType == ENTITY_MODIFY,  // bCanSetPassword
                eAccountType == ENTITY_MODIFY, // bCanSetAccount
                aRememberModes, // rRememberPasswordModes
                ucb::RememberAuthentication_NO, // eDefaultRememberPasswordMode
                aRememberModes, // rRememberAccountModes
                ucb::RememberAuthentication_NO // eDefaultRememberAccountMode
            );

    uno::Sequence<
        uno::Reference< task::XInteractionContinuation > > aContinuations( 3 );
    aContinuations[ 0 ] = new InteractionAbort( this );
    aContinuations[ 1 ] = new InteractionRetry( this );
    aContinuations[ 2 ] = m_xAuthSupplier.get();

    setContinuations( aContinuations );
}
