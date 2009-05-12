/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ldapaccess.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_extensions.hxx"

#include "ldapaccess.hxx"

#include "ldapuserprof.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>


namespace extensions { namespace config { namespace ldap {


//------------------------------------------------------------------------------
typedef int LdapErrCode;
//------------------------------------------------------------------------------
struct LdapMessageHolder
{
    LdapMessageHolder() : msg(0) {}
    ~LdapMessageHolder() { if (msg) ldap_msgfree(msg); }

    LDAPMessage * msg;

private:
    LdapMessageHolder(LdapMessageHolder const&);
    void operator=(LdapMessageHolder const&);
};
//------------------------------------------------------------------------------
LdapConnection::~LdapConnection()
{
    if (isValid()) disconnect();
}
//------------------------------------------------------------------------------

void LdapConnection::disconnect()
{
    if (mConnection != NULL)
    {
        ldap_unbind_s(mConnection) ;
        mConnection = NULL;
    }
}
//------------------------------------------------------------------------------

static void checkLdapReturnCode(const sal_Char *aOperation,
                                LdapErrCode aRetCode,
                                LDAP * /*aConnection*/)
{
    if (aRetCode == LDAP_SUCCESS) { return ; }

    static const sal_Char *kNoSpecificMessage = "No additional information" ;
    rtl::OUStringBuffer message ;

    if (aOperation != NULL)
    {
        message.appendAscii(aOperation).appendAscii(": ") ;
    }
    message.appendAscii(ldap_err2string(aRetCode)).appendAscii(" (") ;
    sal_Char *stub = NULL ;

#ifndef LDAP_OPT_SIZELIMIT // for use with OpenLDAP
    ldap_get_lderrno(aConnection, NULL, &stub) ;
#endif
    if (stub != NULL)
    {
        message.appendAscii(stub) ;
        // It would seem the message returned is actually
        // not a copy of a string but rather some static
        // string itself. At any rate freeing it seems to
        // cause some undue problems at least on Windows.
        // This call is thus disabled for the moment.
        //ldap_memfree(stub) ;
    }
    else { message.appendAscii(kNoSpecificMessage) ; }
    message.appendAscii(")") ;
    throw ldap::LdapGenericException(message.makeStringAndClear(),
                                     NULL, aRetCode) ;
}
//------------------------------------------------------------------------------
void  LdapConnection::connectSimple(const LdapDefinition& aDefinition)
   throw (ldap::LdapConnectionException, ldap::LdapGenericException)
{
    OSL_ENSURE(!isValid(), "Recoonecting an LDAP connection that is already established");
    if (isValid()) disconnect();

    mLdapDefinition = aDefinition;
    connectSimple();
}
//------------------------------------------------------------------------------
void  LdapConnection::connectSimple()
   throw (ldap::LdapConnectionException, ldap::LdapGenericException)
{
    if (!isValid())
    {
        // Connect to the server
        initConnection() ;
        // Set Protocol V3
        int version = LDAP_VERSION3;
        ldap_set_option(mConnection,
                        LDAP_OPT_PROTOCOL_VERSION,
                        &version);

#ifdef LDAP_X_OPT_CONNECT_TIMEOUT // OpenLDAP doesn't support this and the func
        /* timeout is specified in milliseconds -> 4 seconds*/
        int timeout = 4000;
        ldap_set_option( mConnection,
                        LDAP_X_OPT_CONNECT_TIMEOUT,
                        &timeout );
#endif

        // Do the bind
        LdapErrCode retCode = ldap_simple_bind_s(mConnection,
                                               mLdapDefinition.mAnonUser ,
                                               mLdapDefinition.mAnonCredentials) ;

        checkLdapReturnCode("SimpleBind", retCode, mConnection) ;
    }
}
//------------------------------------------------------------------------------
void LdapConnection::initConnection()
    throw (ldap::LdapConnectionException)
{
    if (mLdapDefinition.mServer.getLength() == 0)
    {
        rtl::OUStringBuffer message ;

        message.appendAscii("Cannot initialise connection to LDAP: No server specified.") ;
        throw ldap::LdapConnectionException(message.makeStringAndClear(), NULL) ;
    }

    if (mLdapDefinition.mPort == 0) mLdapDefinition.mPort = LDAP_PORT;

    mConnection = ldap_init(mLdapDefinition.mServer,
                            mLdapDefinition.mPort) ;
    if (mConnection == NULL)
    {
        rtl::OUStringBuffer message ;

        message.appendAscii("Cannot initialise connection to LDAP server ") ;
        message.appendAscii(mLdapDefinition.mServer) ;
        message.appendAscii(":") ;
        message.append(mLdapDefinition.mPort) ;
        throw ldap::LdapConnectionException(message.makeStringAndClear(),
                                            NULL) ;
    }
}
//------------------------------------------------------------------------------
 void LdapConnection::getUserProfile(const rtl::OUString& aUser,
                                     const LdapUserProfileMap& aUserProfileMap,
                                     LdapUserProfile& aUserProfile)
    throw (lang::IllegalArgumentException,
            ldap::LdapConnectionException, ldap::LdapGenericException)
 {
    if (!isValid()) { connectSimple(); }

    rtl::OString aUserDn =findUserDn( rtl::OUStringToOString(aUser, RTL_TEXTENCODING_ASCII_US));

    LdapMessageHolder result;
    LdapErrCode retCode = ldap_search_s(mConnection,
                                      aUserDn,
                                      LDAP_SCOPE_BASE,
                                      "(objectclass=*)",
                                      const_cast<sal_Char **>(aUserProfileMap.getLdapAttributes()),
                                      0, // Attributes + values
                                      &result.msg) ;

    checkLdapReturnCode("getUserProfile", retCode,mConnection) ;


    aUserProfileMap.ldapToUserProfile(mConnection,
                                        result.msg,
                                      aUserProfile) ;

 }
//------------------------------------------------------------------------------
 rtl::OString LdapConnection::findUserDn(const rtl::OString& aUser)
    throw (lang::IllegalArgumentException,
            ldap::LdapConnectionException, ldap::LdapGenericException)
{
    if (!isValid()) { connectSimple(); }

    if (aUser.getLength() == 0)
    {
        throw lang::IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
            ("LdapConnection::findUserDn -User id is empty")),
                NULL, 0) ;
    }



    rtl::OStringBuffer filter( "(&(objectclass=" );

    filter.append( mLdapDefinition.mUserObjectClass ).append(")(") ;
    filter.append( mLdapDefinition.mUserUniqueAttr ).append("=").append(aUser).append("))") ;

    LdapMessageHolder result;
    sal_Char * attributes [2];
    attributes[0]= const_cast<sal_Char *>(LDAP_NO_ATTRS);
    attributes[1]= NULL;
    LdapErrCode retCode = ldap_search_s(mConnection,
                                      mLdapDefinition.mBaseDN,
                                      LDAP_SCOPE_SUBTREE,
                                      filter.makeStringAndClear(), attributes, 0, &result.msg) ;

    checkLdapReturnCode("FindUserDn", retCode,mConnection) ;
    rtl::OString userDn ;
    LDAPMessage *entry = ldap_first_entry(mConnection, result.msg) ;

    if (entry != NULL)
    {
        sal_Char *charsDn = ldap_get_dn(mConnection, entry) ;

        userDn = charsDn ;
        ldap_memfree(charsDn) ;
    }
    else
    {
        OSL_ENSURE( false, "LdapConnection::findUserDn-could not get DN for User ");
    }

    return userDn ;
}
//------------------------------------------------------------------------------
rtl::OString LdapConnection::getSingleAttribute(
    const rtl::OString& aDn,
    const rtl::OString& aAttribute)
    throw (ldap::LdapConnectionException, ldap::LdapGenericException)
{
    if (!isValid()) { connectSimple(); }
    const sal_Char *attributes [2] ;
    rtl::OString value ;

    attributes [0] = aAttribute ;
    attributes [1] = 0 ;
    LdapMessageHolder result ;
    LdapErrCode retCode = ldap_search_s(mConnection,
                                      aDn,
                                      LDAP_SCOPE_BASE,
                                      "(objectclass=*)",
                                      const_cast<sal_Char **>(attributes),
                                      0, // Attributes + values
                                      &result.msg) ;

    if (retCode == LDAP_NO_SUCH_OBJECT)
    {
        return value ;
    }
    checkLdapReturnCode("GetSingleAttribute", retCode, mConnection) ;
    LDAPMessage *entry = ldap_first_entry(mConnection, result.msg) ;

    if (entry != NULL)
    {
        sal_Char **values = ldap_get_values(mConnection, entry,
                                            aAttribute) ;

        if (values != NULL)
        {
            if (*values != NULL) { value = *values ; }
            ldap_value_free(values) ;
        }
    }
    return value ;
}

//------------------------------------------------------------------------------
} } } // extensions.config.ldap

