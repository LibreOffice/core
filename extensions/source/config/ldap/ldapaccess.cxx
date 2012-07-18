/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "ldapaccess.hxx"

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>


namespace extensions { namespace config { namespace ldap {

//------------------------------------------------------------------------------
typedef int LdapErrCode;
//------------------------------------------------------------------------------
struct LdapMessageHolder
{
    LdapMessageHolder() : msg(0) {}
    ~LdapMessageHolder()
    {
        if (msg)
            ldap_msgfree(msg);
    }

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
#ifdef WNT
        ldap_set_optionW( mConnection,
                        LDAP_X_OPT_CONNECT_TIMEOUT,
                        &timeout );
#else
        ldap_set_option( mConnection,
                        LDAP_X_OPT_CONNECT_TIMEOUT,
                        &timeout );
#endif
#endif

        // Do the bind
#ifdef WNT
        LdapErrCode retCode = ldap_simple_bind_sW(mConnection,
                                               (PWCHAR) mLdapDefinition.mAnonUser.getStr(),
                                               (PWCHAR) mLdapDefinition.mAnonCredentials.getStr() );
#else
        LdapErrCode retCode = ldap_simple_bind_s(mConnection,
                                               rtl::OUStringToOString( mLdapDefinition.mAnonUser, RTL_TEXTENCODING_UTF8 ).getStr(),
                                               rtl::OUStringToOString( mLdapDefinition.mAnonCredentials, RTL_TEXTENCODING_UTF8 ).getStr()) ;
#endif

        checkLdapReturnCode("SimpleBind", retCode, mConnection) ;
    }
}
//------------------------------------------------------------------------------
void LdapConnection::initConnection()
    throw (ldap::LdapConnectionException)
{
    if (mLdapDefinition.mServer.isEmpty())
    {
        rtl::OUStringBuffer message ;

        message.appendAscii("Cannot initialise connection to LDAP: No server specified.") ;
        throw ldap::LdapConnectionException(message.makeStringAndClear(), NULL) ;
    }

    if (mLdapDefinition.mPort == 0) mLdapDefinition.mPort = LDAP_PORT;

#ifdef WNT
    mConnection = ldap_initW((PWCHAR) mLdapDefinition.mServer.getStr(),
                            mLdapDefinition.mPort) ;
#else
    mConnection = ldap_init(rtl::OUStringToOString( mLdapDefinition.mServer, RTL_TEXTENCODING_UTF8 ).getStr(),
                            mLdapDefinition.mPort) ;
#endif
    if (mConnection == NULL)
    {
        rtl::OUStringBuffer message ;

        message.appendAscii("Cannot initialise connection to LDAP server ") ;
        message.append(mLdapDefinition.mServer) ;
        message.appendAscii(":") ;
        message.append(mLdapDefinition.mPort) ;
        throw ldap::LdapConnectionException(message.makeStringAndClear(),
                                            NULL) ;
    }
}
//------------------------------------------------------------------------------
 void LdapConnection::getUserProfile(
     const rtl::OUString& aUser, LdapData * data)
    throw (lang::IllegalArgumentException,
            ldap::LdapConnectionException, ldap::LdapGenericException)
{
    OSL_ASSERT(data != 0);
    if (!isValid()) { connectSimple(); }

    rtl::OUString aUserDn =findUserDn( aUser );

    LdapMessageHolder result;
#ifdef WNT
    LdapErrCode retCode = ldap_search_sW(mConnection,
                                      (PWCHAR) aUserDn.getStr(),
                                      LDAP_SCOPE_BASE,
                                      L"(objectclass=*)",
                                      0,
                                      0, // Attributes + values
                                      &result.msg) ;
#else
    LdapErrCode retCode = ldap_search_s(mConnection,
                                      rtl::OUStringToOString( aUserDn, RTL_TEXTENCODING_UTF8 ).getStr(),
                                      LDAP_SCOPE_BASE,
                                      "(objectclass=*)",
                                      0,
                                      0, // Attributes + values
                                      &result.msg) ;
#endif
    checkLdapReturnCode("getUserProfile", retCode,mConnection) ;

    BerElement * ptr;
#ifdef WNT
    PWCHAR attr = ldap_first_attributeW(mConnection, result.msg, &ptr);
    while (attr != 0) {
        PWCHAR * values = ldap_get_valuesW(mConnection, result.msg, attr);
        if (values != 0) {
            data->insert(
                LdapData::value_type( attr, *values ));
            ldap_value_freeW(values);
        }
        attr = ldap_next_attributeW(mConnection, result.msg, ptr);
#else
    char * attr = ldap_first_attribute(mConnection, result.msg, &ptr);
    while (attr != 0) {
        char ** values = ldap_get_values(mConnection, result.msg, attr);
        if (values != 0) {
            data->insert(
                LdapData::value_type(
                    rtl::OStringToOUString(attr, RTL_TEXTENCODING_ASCII_US),
                    rtl::OStringToOUString(*values, RTL_TEXTENCODING_UTF8)));
            ldap_value_free(values);
        }
        attr = ldap_next_attribute(mConnection, result.msg, ptr);
#endif
    }
}
//------------------------------------------------------------------------------
 rtl::OUString LdapConnection::findUserDn(const rtl::OUString& aUser)
    throw (lang::IllegalArgumentException,
            ldap::LdapConnectionException, ldap::LdapGenericException)
{
    if (!isValid()) { connectSimple(); }

    if (aUser.isEmpty())
    {
        throw lang::IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
            ("LdapConnection::findUserDn -User id is empty")),
                NULL, 0) ;
    }



    rtl::OUStringBuffer filter( "(&(objectclass=" );

    filter.append( mLdapDefinition.mUserObjectClass ).append(")(") ;
    filter.append( mLdapDefinition.mUserUniqueAttr ).append("=").append(aUser).append("))") ;

    LdapMessageHolder result;
#ifdef WNT
    PWCHAR attributes [2] = { L"1.1", NULL };
    LdapErrCode retCode = ldap_search_sW(mConnection,
                                      (PWCHAR) mLdapDefinition.mBaseDN.getStr(),
                                      LDAP_SCOPE_SUBTREE,
                                      (PWCHAR) filter.makeStringAndClear().getStr(), attributes, 0, &result.msg) ;
#else
    sal_Char * attributes [2] = { const_cast<sal_Char *>(LDAP_NO_ATTRS), NULL };
    LdapErrCode retCode = ldap_search_s(mConnection,
                                      rtl::OUStringToOString( mLdapDefinition.mBaseDN, RTL_TEXTENCODING_UTF8 ).getStr(),
                                      LDAP_SCOPE_SUBTREE,
                                      rtl::OUStringToOString( filter.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ).getStr(), attributes, 0, &result.msg) ;
#endif
    checkLdapReturnCode("FindUserDn", retCode,mConnection) ;
    rtl::OUString userDn ;
    LDAPMessage *entry = ldap_first_entry(mConnection, result.msg) ;

    if (entry != NULL)
    {
#ifdef WNT
        PWCHAR charsDn = ldap_get_dnW(mConnection, entry) ;

        userDn = charsDn;
        ldap_memfreeW(charsDn) ;
#else
        sal_Char *charsDn = ldap_get_dn(mConnection, entry) ;

        userDn = rtl::OStringToOUString( charsDn, RTL_TEXTENCODING_UTF8 );
        ldap_memfree(charsDn) ;
#endif
    }
    else
    {
        OSL_FAIL( "LdapConnection::findUserDn-could not get DN for User ");
    }

    return userDn ;
}

//------------------------------------------------------------------------------

} } } // extensions.config.ldap

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
