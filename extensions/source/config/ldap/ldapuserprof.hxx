/*************************************************************************
 *
 *  $RCSfile: ldapuserprof.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 14:38:29 $
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

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROF_HXX_
#define EXTENSIONS_CONFIG_LDAP_LDAPUSERPROF_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif // _RTL_USTRING_HXX_

#ifndef MAP_INCLUDED
#define MAP_INCLUDED
#include <map>
#endif // MAP_INCLUDED

#ifndef VECTOR_INCLUDED
#define VECTOR_INCLUDED
#include <vector>
#endif // VECTOR_INCLUDED

#ifndef SET_INCLUDED
#define SET_INCLUDED
#include <set>
#endif // SET_INCLUDED

#ifndef LDAP_INCLUDED
#define LDAP_INCLUDED
#include <mozilla/ldap/ldap.h>
#endif // LDAP_INCLUDED


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
} ;

/** Provider of UserProfile mapping services */
class LdapUserProfileMap {
    public :
        /** Default constructor, doesn't do much. */
        LdapUserProfileMap(void) : mAttributes(NULL) {}
        /** Destructor */
        ~LdapUserProfileMap(void) ;

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

    protected :

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
