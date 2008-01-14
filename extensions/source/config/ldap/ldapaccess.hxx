/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ldapaccess.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:41:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_
#define EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_

#include "wrapldapinclude.hxx"

#ifndef _COM_SUN_STAR_LDAP_LDAPGENERICEXCEPTION_HPP_
#include <com/sun/star/ldap/LdapGenericException.hpp>
#endif // _COM_SUN_STAR_LDAP_LDAPGENERICEXCEPTION_HPP_

#ifndef _COM_SUN_STAR_LDAP_LDAP_CONNECTIONEXCEPTION_HPP_
#include <com/sun/star/ldap/LdapConnectionException.hpp>
#endif // _COM_SUN_STAR_LDAP_LDAP_CONNECTIONEXCEPTION_HPP_

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif // _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_

namespace extensions { namespace config { namespace ldap {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace ldap = css::ldap ;
//------------------------------------------------------------------------------
// LdapUserProfile classes
struct LdapUserProfile;
class LdapUserProfileMap;

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
    /** Mapping File */
    rtl::OString mMapping;
 } ;

/** Class encapulating all LDAP functionality */
class LdapConnection
{
public:

    /** Default constructor */
    LdapConnection(void) : mConnection(NULL),mLdapDefinition() {}
    /** Destructor, releases the connection */
    ~LdapConnection(void) ;
    /** Make connection to LDAP server */
    void  connectSimple(const LdapDefinition& aDefinition)
        throw (ldap::LdapConnectionException,
                ldap::LdapGenericException);

    /** query connection status */
    bool isConnected() const { return isValid(); }

    /**
        Gets LdapUserProfile from LDAP repository for specified user
        @param aUser    name of logged on user
        @param aUserProfileMap  Map containing LDAP->00o mapping
        @param aUserProfile     struct for holding OOo values

         @throws com::sun::star::ldap::LdapGenericException
                  if an LDAP error occurs.
    */
    void getUserProfile(const rtl::OUString& aUser,
                        const LdapUserProfileMap& aUserProfileMap,
                        LdapUserProfile& aUserProfile)
         throw (lang::IllegalArgumentException,
                 ldap::LdapConnectionException,
                 ldap::LdapGenericException);
    /**
          Retrieves a single attribute from a single entry.
          @param aDn            entry DN
          @param aAttribute     attribute name

          @throws com::sun::star::ldap::LdapGenericException
                  if an LDAP error occurs.
     */
     rtl::OString getSingleAttribute(const rtl::OString& aDn,
                                     const rtl::OString& aAttribute)
        throw (ldap::LdapConnectionException,
                ldap::LdapGenericException);

    /** finds DN of user
        @return  DN of User
    */
    rtl::OString findUserDn(const rtl::OString& aUser)
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
    bool isValid(void) const { return mConnection != NULL ; }

    void  connectSimple()
        throw (ldap::LdapConnectionException,
                ldap::LdapGenericException);

    /** LDAP connection object */
    LDAP* mConnection ;
    LdapDefinition mLdapDefinition;
} ;
//------------------------------------------------------------------------------
}} }

#endif // EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILE_HXX_
