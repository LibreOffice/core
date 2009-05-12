/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ldapuserprof.hxx,v $
 * $Revision: 1.7 $
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

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROF_HXX_
#define EXTENSIONS_CONFIG_LDAP_LDAPUSERPROF_HXX_

#include "wrapldapinclude.hxx"
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#ifndef VECTOR_INCLUDED
#define VECTOR_INCLUDED
#include <vector>
#endif // VECTOR_INCLUDED

#ifndef SET_INCLUDED
#define SET_INCLUDED
#include <set>
#endif // SET_INCLUDED


namespace extensions { namespace config { namespace ldap {

/** Struct containing the data associated to a UserProfile */
struct LdapUserProfile {
    /** Struct defining an entry in the profile */
    struct ProfileEntry {
        rtl::OUString mAttribute ;
        rtl::OUString mValue ;
    } ;
    /** List of attribute/value pairs */
    std::vector<ProfileEntry> mProfile ;

    typedef std::vector<ProfileEntry>::const_iterator Iterator;
} ;

/** Provider of UserProfile mapping services */
class LdapUserProfileMap
{
    public :
        /** Default constructor, doesn't do much. */
        LdapUserProfileMap() : mAttributes(NULL) {}
        /** Destructor */
        ~LdapUserProfileMap() ;

        /**
          Fills the map using a string description of the mapping.
          Each possible UserProfile attribute is mapped to a list
          of LDAP attributes, in order of priority.
          The entry is formatted as:
          <UP Attribute>=<LDAP Attr1>,...,<LDAP AttrN>

          @param aMap   string description of the mapping
          */
        void source(const rtl::OString& aMap) ;
        /**
          Returns an array of all the LDAP attribute names that may
          be relevant for UserProfile mapping.

          @return   array of LDAP attribute names
          */
        const sal_Char **getLdapAttributes(void) const { return mAttributes ; }
          /**
          Returns the component name that holds the data
          when presented to the outside.

          @return component name
          */
        const rtl::OUString& getComponentName(void) const {
            return mComponentName ;
        }
        /**
          Returns the group name that holds the data when
          presented to the outside world.

          @return   group name
          */
        const rtl::OUString& getGroupName(void) const {
            return mGroupName ;
        }
        /**
          Struct defining the mapping between UserProfile attributes
          and LDAP ones. It also serves as a crude "schema" of the
          UserProfile component, as the list of profile elements
          is the list of properties in the component and the presence
          or absence of LDAP mapping indicates whether the property
          should be made writable or kept read-only.
          */
        struct Mapping {
            /** Name of the UserProfile element */
            rtl::OString mProfileElement ;
            /** Corresponding LDAP attributes in order of priority */
            std::vector<rtl::OString> mLdapAttributes ;

            /** Utility to fill the mapping from a <Prof>=<Ldap1>;...;<LdapN>
                description */
            sal_Bool parse(const rtl::OString& aDescription) ;
        } ;


        /**
          Maps an LDAP entry to a UserProfile.

          @param aConnection    LDAP connection to the repository
          @param aEntry         entry containing the data
          @param aProfile       user profile struct to fill up
          */
        void ldapToUserProfile(LDAP *aConnection,
                               LDAPMessage *aEntry,
                               LdapUserProfile& aProfile) const ;

    private :
        /** Contains the mapping entries */
        std::vector<Mapping> mMapping ;
        /** Contains the LDAP attributes used in a NULL terminated array */
        const sal_Char **mAttributes ;
        /** User profile component name */
        rtl::OUString mComponentName ;
        /** User profile storage group name */
        rtl::OUString mGroupName ;

        /**
          Adds a new mapping entry to the list, and collects the
          LDAP attributes involved in the mapping.

          @param aMappingDescription    string describing the mapping
          @param aLdapAttributes        collection of LDAP attributes
          @param aPrefix                common prefix to the attributes
                                        containing the component and group
                                        name, used for validation
         */
        void addNewMapping(const rtl::OString& aMappingDescription,
                           std::set<rtl::OString>& aLdapAttributes,
                           rtl::OString& aPrefix) ;
} ;

} } } // extensions.config.ldap

#endif // EXTENSIONS_CONFIG_LDAP_LDAPUSERPROF_HXX_
