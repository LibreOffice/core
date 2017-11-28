/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_
#define EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_

#include "sal/config.h"

#include <map>

#include "wrapldapinclude.hxx"
#include <com/sun/star/ldap/LdapGenericException.hpp>

#include <com/sun/star/ldap/LdapConnectionException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <osl/module.h>

namespace extensions { namespace config { namespace ldap {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace ldap = css::ldap ;
//------------------------------------------------------------------------------
// LdapUserProfile classes
struct LdapUserProfile;
class LdapUserProfileMap;

typedef LDAP_API(int) (LDAP_CALL *t_ldap_unbind_s)( LDAP *ld );
typedef LDAP_API(int) (LDAP_CALL *t_ldap_simple_bind_s)( LDAP *ld, const char *who, const char *passwd );
typedef LDAP_API(int) (LDAP_CALL *t_ldap_set_option)( LDAP *ld, int option, const void *optdata );
typedef LDAP_API(char *) (LDAP_CALL *t_ldap_err2string)( int err );
typedef LDAP_API(LDAP *) (LDAP_CALL *t_ldap_init)( const char *defhost, int defport );
typedef LDAP_API(int) (LDAP_CALL *t_ldap_msgfree)( LDAPMessage *lm );
typedef LDAP_API(char *) (LDAP_CALL *t_ldap_get_dn)( LDAP *ld, LDAPMessage *entry );
typedef LDAP_API(LDAPMessage *) (LDAP_CALL *t_ldap_first_entry)( LDAP *ld,  LDAPMessage *chain );
typedef LDAP_API(char *) (LDAP_CALL *t_ldap_first_attribute)( LDAP *ld, LDAPMessage *entry, void **ptr );
typedef LDAP_API(char *) (LDAP_CALL *t_ldap_next_attribute)( LDAP *ld, LDAPMessage *entry, void *ptr );
typedef LDAP_API(int) (LDAP_CALL *t_ldap_search_s)( LDAP *ld, const char *base, int scope,  const char *filter, char **attrs, int attrsonly, LDAPMessage **res );
typedef LDAP_API(void) (LDAP_CALL *t_ldap_value_free)( char **vals );
typedef LDAP_API(char **) (LDAP_CALL *t_ldap_get_values)( LDAP *ld, LDAPMessage *entry, const char *target );
typedef LDAP_API(void) (LDAP_CALL *t_ldap_memfree)( void *p );
//------------------------------------------------------------------------------
/** Struct containing the information on LDAP connection */
struct LdapDefinition
{
    /** LDAP server name */
    rtl::OString mServer ;
    /** LDAP server port number */
    sal_Int32 mPort ;
    /** Repository base DN */
    rtl::OString mBaseDN ;
    /** DN to use for "anonymous" connection */
    rtl::OString mAnonUser ;
    /** Credentials to use for "anonymous" connection */
    rtl::OString mAnonCredentials ;
    /** User Entity Object Class */
    rtl::OString mUserObjectClass;
    /** User Entity Unique Attribute */
    rtl::OString mUserUniqueAttr;
 } ;

typedef std::map< rtl::OUString, rtl::OUString > LdapData; // key/value pairs

/** Class encapsulating all LDAP functionality */
class LdapConnection
{
    friend struct LdapMessageHolder;
public:

    /** Default constructor */
    LdapConnection(void) : mConnection(NULL),mLdapDefinition() {}
    /** Destructor, releases the connection */
    ~LdapConnection(void) ;
    /** Make connection to LDAP server */
    void  connectSimple(const LdapDefinition& aDefinition)
        throw (ldap::LdapConnectionException,
                ldap::LdapGenericException);

    /**
        Gets LdapUserProfile from LDAP repository for specified user
        @param aUser    name of logged on user
        @param aUserProfileMap  Map containing LDAP->00o mapping
       @param aUserProfile     struct for holding OOo values

         @throws com::sun::star::ldap::LdapGenericException
                  if an LDAP error occurs.
    */
    void getUserProfile(const rtl::OUString& aUser, LdapData * data)
         throw (lang::IllegalArgumentException,
                 ldap::LdapConnectionException,
                 ldap::LdapGenericException);

    /** finds DN of user
        @return  DN of User
    */
    rtl::OString findUserDn(const rtl::OString& aUser)
        throw (lang::IllegalArgumentException,
                ldap::LdapConnectionException,
                ldap::LdapGenericException);

    void loadModule();

    static t_ldap_err2string        s_p_err2string;
private:

    void initConnection()
         throw (ldap::LdapConnectionException);
    void disconnect();
    /**
      Indicates whether the connection is in a valid state.
      @return   sal_True if connection is valid, sal_False otherwise
      */
    bool isValid(void) const { return mConnection != NULL ; }

    void  connectSimple()
        throw (ldap::LdapConnectionException,
                ldap::LdapGenericException);

    /** LDAP connection object */
    LDAP* mConnection ;
    LdapDefinition mLdapDefinition;

    static oslModule                s_Ldap_Module;
    static t_ldap_value_free        s_p_value_free;
    static t_ldap_get_values        s_p_get_values;
    static t_ldap_unbind_s          s_p_unbind_s;
    static t_ldap_simple_bind_s     s_p_simple_bind_s;
    static t_ldap_set_option        s_p_set_option;
    static t_ldap_init              s_p_init;
    static t_ldap_msgfree           s_p_msgfree;
    static t_ldap_get_dn            s_p_get_dn;
    static t_ldap_first_entry       s_p_first_entry;
    static t_ldap_first_attribute   s_p_first_attribute;
    static t_ldap_next_attribute    s_p_next_attribute;
    static t_ldap_search_s          s_p_search_s;

    static t_ldap_memfree           s_p_memfree;

} ;
//------------------------------------------------------------------------------
}} }

#endif // EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILE_HXX_
