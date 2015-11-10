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

#ifndef INCLUDED_EXTENSIONS_SOURCE_CONFIG_LDAP_LDAPACCESS_HXX
#define INCLUDED_EXTENSIONS_SOURCE_CONFIG_LDAP_LDAPACCESS_HXX

#include "sal/config.h"

#include <map>

#ifdef WNT
#include <windows.h>
#include <winldap.h>
#else // !defined WNT
#include <ldap.h>
#endif // WNT

#include <com/sun/star/ldap/LdapGenericException.hpp>

#include <com/sun/star/ldap/LdapConnectionException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <osl/module.h>

namespace extensions { namespace config { namespace ldap {

namespace uno = css::uno ;
namespace lang = css::lang ;
namespace ldap = css::ldap ;

struct LdapUserProfile;


/** Struct containing the information on LDAP connection */
struct LdapDefinition
{
    /** LDAP server name */
    OUString mServer ;
    /** LDAP server port number */
    sal_Int32 mPort ;
    /** Repository base DN */
    OUString mBaseDN ;
    /** DN to use for "anonymous" connection */
    OUString mAnonUser ;
    /** Credentials to use for "anonymous" connection */
    OUString mAnonCredentials ;
    /** User Entity Object Class */
    OUString mUserObjectClass;
    /** User Entity Unique Attribute */
    OUString mUserUniqueAttr;

    LdapDefinition()
        : mPort(0)
    {
    }
};

typedef std::map< OUString, OUString > LdapData; // key/value pairs

/** Class encapulating all LDAP functionality */
class LdapConnection
{
    friend struct LdapMessageHolder;
public:

    /** Default constructor */
    LdapConnection() : mConnection(nullptr),mLdapDefinition() {}
    /** Destructor, releases the connection */
    ~LdapConnection() ;
    /** Make connection to LDAP server */
    void  connectSimple(const LdapDefinition& aDefinition)
        throw (ldap::LdapConnectionException,
                ldap::LdapGenericException);

    /**
        Gets LdapUserProfile from LDAP repository for specified user
        @param aUser    name of logged on user
        @param aUserProfileMap  Map containing LDAP->00o mapping
       @param aUserProfile     struct for holding OOo values

         @throws css::ldap::LdapGenericException
                  if an LDAP error occurs.
    */
    void getUserProfile(const OUString& aUser, LdapData * data)
         throw (lang::IllegalArgumentException,
                 ldap::LdapConnectionException,
                 ldap::LdapGenericException);

    /** finds DN of user
        @return  DN of User
    */
    OUString findUserDn(const OUString& aUser)
        throw (lang::IllegalArgumentException,
                ldap::LdapConnectionException,
                ldap::LdapGenericException);

private:

    void initConnection()
         throw (ldap::LdapConnectionException);
    void disconnect();
    /**
      Indicates whether the connection is in a valid state.
      @return   sal_True if connection is valid, sal_False otherwise
      */
    bool isValid() const { return mConnection != nullptr ; }

    void  connectSimple()
        throw (ldap::LdapConnectionException,
                ldap::LdapGenericException);

    /** LDAP connection object */
    LDAP* mConnection ;
    LdapDefinition mLdapDefinition;
} ;


}} }

#endif // EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
