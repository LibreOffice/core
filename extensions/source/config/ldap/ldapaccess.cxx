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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "ldapaccess.hxx"

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>


namespace extensions { namespace config { namespace ldap {

oslModule       LdapConnection::s_Ldap_Module = NULL;
t_ldap_unbind_s          LdapConnection::s_p_unbind_s = NULL;
t_ldap_simple_bind_s     LdapConnection::s_p_simple_bind_s = NULL;
t_ldap_set_option        LdapConnection::s_p_set_option = NULL;
t_ldap_err2string        LdapConnection::s_p_err2string = NULL;
t_ldap_init              LdapConnection::s_p_init = NULL;
t_ldap_msgfree           LdapConnection::s_p_msgfree = NULL;
t_ldap_get_dn            LdapConnection::s_p_get_dn = NULL;
t_ldap_first_entry       LdapConnection::s_p_first_entry = NULL;
t_ldap_first_attribute   LdapConnection::s_p_first_attribute = NULL;
t_ldap_next_attribute    LdapConnection::s_p_next_attribute = NULL;
t_ldap_search_s          LdapConnection::s_p_search_s = NULL;
t_ldap_value_free        LdapConnection::s_p_value_free = NULL;
t_ldap_get_values        LdapConnection::s_p_get_values = NULL;
t_ldap_memfree           LdapConnection::s_p_memfree = NULL;
//------------------------------------------------------------------------------
typedef int LdapErrCode;
//------------------------------------------------------------------------------
struct LdapMessageHolder
{
    LdapMessageHolder() : msg(0) {}
    ~LdapMessageHolder()
    {
        if (msg)
            (*LdapConnection::s_p_msgfree)(msg);
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
        (*s_p_unbind_s)(mConnection) ;
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
    message.appendAscii((*LdapConnection::s_p_err2string)(aRetCode)).appendAscii(" (") ;
    sal_Char *stub = NULL ;

#ifndef LDAP_OPT_SIZELIMIT // for use with OpenLDAP
    (*s_p_get_lderrno)(aConnection, NULL, &stub) ;
#endif
    if (stub != NULL)
    {
        message.appendAscii(stub) ;
        // It would seem the message returned is actually
        // not a copy of a string but rather some static
        // string itself. At any rate freeing it seems to
        // cause some undue problems at least on Windows.
        // This call is thus disabled for the moment.
        //(*s_p_memfree)(stub) ;
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
        (*s_p_set_option)(mConnection,
                        LDAP_OPT_PROTOCOL_VERSION,
                        &version);

#ifdef LDAP_X_OPT_CONNECT_TIMEOUT // OpenLDAP doesn't support this and the func
        /* timeout is specified in milliseconds -> 4 seconds*/
        int timeout = 4000;
        (*s_p_set_option)( mConnection,
                        LDAP_X_OPT_CONNECT_TIMEOUT,
                        &timeout );
#endif

        // Do the bind
        LdapErrCode retCode = (*s_p_simple_bind_s)(mConnection,
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

    mConnection = (*s_p_init)(mLdapDefinition.mServer,
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
 void LdapConnection::getUserProfile(
     const rtl::OUString& aUser, LdapData * data)
    throw (lang::IllegalArgumentException,
            ldap::LdapConnectionException, ldap::LdapGenericException)
{
    OSL_ASSERT(data != 0);
    if (!isValid()) { connectSimple(); }

    rtl::OString aUserDn =findUserDn( rtl::OUStringToOString(aUser, RTL_TEXTENCODING_ASCII_US));

    LdapMessageHolder result;
    LdapErrCode retCode = (*s_p_search_s)(mConnection,
                                      aUserDn,
                                      LDAP_SCOPE_BASE,
                                      "(objectclass=*)",
                                      0,
                                      0, // Attributes + values
                                      &result.msg) ;

    checkLdapReturnCode("getUserProfile", retCode,mConnection) ;

    void * ptr;
    char * attr = (*s_p_first_attribute)(mConnection, result.msg, &ptr);
    while (attr != 0) {
        char ** values = (*s_p_get_values)(mConnection, result.msg, attr);
        if (values != 0) {
            data->insert(
                LdapData::value_type(
                    rtl::OStringToOUString(attr, RTL_TEXTENCODING_ASCII_US),
                    rtl::OStringToOUString(*values, RTL_TEXTENCODING_UTF8)));
            (*s_p_value_free)(values);
        }
        attr = (*s_p_next_attribute)(mConnection, result.msg, ptr);
    }
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
    LdapErrCode retCode = (*s_p_search_s)(mConnection,
                                      mLdapDefinition.mBaseDN,
                                      LDAP_SCOPE_SUBTREE,
                                      filter.makeStringAndClear(), attributes, 0, &result.msg) ;

    checkLdapReturnCode("FindUserDn", retCode,mConnection) ;
    rtl::OString userDn ;
    LDAPMessage *entry = (*s_p_first_entry)(mConnection, result.msg) ;

    if (entry != NULL)
    {
        sal_Char *charsDn = (*s_p_get_dn)(mConnection, entry) ;

        userDn = charsDn ;
        (*s_p_memfree)(charsDn) ;
    }
    else
    {
        OSL_FAIL( "LdapConnection::findUserDn-could not get DN for User ");
    }

    return userDn ;
}

extern "C" { static void SAL_CALL thisModule() {} }
void LdapConnection::loadModule()
{
    if ( !s_Ldap_Module )
    {
#if defined(WNT)
#       define LIBLDAP "nsldap32v50.dll"
#else
#   ifdef WITH_OPENLDAP
#       define xstr(s) str(s)
#       define str(s) #s
#       define LIBLDAP "libldap-" xstr(LDAP_VENDOR_VERSION_MAJOR) "." xstr(LDAP_VENDOR_VERSION_MINOR) ".so." xstr(LDAP_VENDOR_VERSION_MAJOR)
#   else
#       define LIBLDAP "libldap50.so"
#   endif
#endif
        const ::rtl::OUString sModuleName(RTL_CONSTASCII_USTRINGPARAM(LIBLDAP));

        // load the dbtools library
        s_Ldap_Module = osl_loadModuleRelative(&thisModule, sModuleName.pData, 0);
        if ( s_Ldap_Module != NULL )
        {
            s_p_unbind_s = (t_ldap_unbind_s)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_unbind_s")).pData));
            s_p_simple_bind_s = (t_ldap_simple_bind_s)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_simple_bind_s")).pData));
            s_p_set_option = (t_ldap_set_option)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_set_option")).pData));
            s_p_err2string = (t_ldap_err2string)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_err2string")).pData));
            s_p_init = (t_ldap_init)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_init")).pData));
            s_p_msgfree = (t_ldap_msgfree)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_msgfree")).pData));
            s_p_get_dn = (t_ldap_get_dn)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_get_dn")).pData));
            s_p_first_entry = (t_ldap_first_entry)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_first_entry")).pData));
            s_p_first_attribute = (t_ldap_first_attribute)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_first_attribute")).pData));
            s_p_next_attribute = (t_ldap_next_attribute)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_next_attribute")).pData));
            s_p_search_s = (t_ldap_search_s)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_search_s")).pData));
            s_p_value_free = (t_ldap_value_free)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_value_free")).pData));
            s_p_get_values = (t_ldap_get_values)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_get_values")).pData));
            s_p_memfree = (t_ldap_memfree)(osl_getFunctionSymbol(s_Ldap_Module, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ldap_memfree")).pData));
        }
    }
}

//------------------------------------------------------------------------------
} } } // extensions.config.ldap

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
