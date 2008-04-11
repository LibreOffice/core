/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ldapuserprof.cxx,v $
 * $Revision: 1.6 $
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
#include "ldapuserprof.hxx"

namespace extensions { namespace config { namespace ldap {
//==============================================================================

//------------------------------------------------------------------------------

/**
  Finds the next line in a buffer and returns it, along with a
  modified version of the buffer with the line removed.

  @param aString    string to extract the next line from
  @param aLine      next line
  @return   sal_True if a line has been extracted, sal_False otherwise
  */
static sal_Bool getNextLine(rtl::OString& aString,
                            rtl::OString& aLine)
{
    aString = aString.trim() ;
    const sal_Char *currentChar = aString ;
    const sal_Char *endChar = currentChar + aString.getLength() ;
    sal_Int32 lineThreshold = 0 ;

    while (currentChar < endChar &&
            *currentChar != '\r' && *currentChar != '\n') { ++ currentChar ; }
    lineThreshold = currentChar - static_cast<const sal_Char *>(aString) ;
    if (lineThreshold == 0) { return sal_False ; }
    aLine = aString.copy(0, lineThreshold) ;
    aString = aString.copy(lineThreshold) ;
    return sal_True ;
}
//------------------------------------------------------------------------------

LdapUserProfileMap::~LdapUserProfileMap(void)
{
    // No need to delete the contents of the mAttributes array,
    // since they refer to rtl::OStrings stored in the mLdapAttributes
    // array.
    if (mAttributes != NULL)
    {
        delete [] mAttributes ;
    }
}
//------------------------------------------------------------------------------

void LdapUserProfileMap::source(const rtl::OString& aMap)
{
    if (mAttributes != NULL)
    {
        delete [] mAttributes ; mAttributes = NULL ;
        mMapping.clear() ;
    }
    rtl::OString currentLine ;
    rtl::OString buffer = aMap ;
    std::set<rtl::OString> attributes ;
    rtl::OString prefix ;

    // First, parse the buffer to find all the mapping definitions.
    // While we're at it, we collect the list of unique LDAP attributes
    // involved in the mapping.
    while (getNextLine(buffer, currentLine))
    {
        addNewMapping(currentLine, attributes, prefix) ;
    }
    // Now we use the list of attributes to build mAttributes
    mAttributes = new const sal_Char * [attributes.size() + 1] ;
    std::set<rtl::OString>::const_iterator attribute ;
    sal_Int32 i = 0 ;

    for (attribute = attributes.begin() ;
            attribute != attributes.end() ; ++ attribute)
    {
        mAttributes [i ++] = static_cast<const sal_Char *>(*attribute) ;
    }
    mAttributes [i] = NULL ;
}
//------------------------------------------------------------------------------

void LdapUserProfileMap::ldapToUserProfile(LDAP *aConnection,
                                           LDAPMessage *aEntry,
                                           LdapUserProfile& aProfile) const
{
    if (aEntry == NULL) { return ; }
    // Ensure return value has proper size
    aProfile.mProfile.resize(mMapping.size()) ;
    sal_Char **values = NULL ;

    for (sal_uInt32 i = 0 ; i < mMapping.size() ; ++ i)
    {
        aProfile.mProfile [i].mAttribute = rtl::OStringToOUString(
                                               mMapping [i].mProfileElement,
                                               RTL_TEXTENCODING_ASCII_US);
        rtl::OUString debugStr = aProfile.mProfile [i].mAttribute;

        for (sal_uInt32 j = 0 ;
                j < mMapping [i].mLdapAttributes.size() ; ++ j)
        {
            values = ldap_get_values(aConnection, aEntry,
                                     mMapping [i].mLdapAttributes [j]) ;

            if (values != NULL)
            {
                aProfile.mProfile[i].mValue = rtl::OStringToOUString(
                    *values, RTL_TEXTENCODING_UTF8);
                ldap_value_free(values);
                break;
            }
        }
    }
}
//------------------------------------------------------------------------------
void LdapUserProfileMap::addNewMapping(const rtl::OString& aLine,
                                    std::set<rtl::OString>& aLdapAttributes,
                                    rtl::OString& aPrefix)
{
    if (aLine.getStr() [0] == '#') { return ; }
    sal_Int32 prefixLength = aPrefix.getLength() ;

    if (prefixLength == 0)
    {
        sal_Int32 firstSlash = aLine.indexOf('/') ;

        if (firstSlash == -1) { return ; }
        sal_Int32 secondSlash = aLine.indexOf('/', firstSlash + 1) ;

        if (secondSlash == -1){ return; }


        mComponentName =
            rtl::OUString::createFromAscii(aLine.copy(0, firstSlash)) ;
        mGroupName =
            rtl::OUString::createFromAscii(aLine.copy(firstSlash + 1,
                                                secondSlash - firstSlash - 1)) ;
        aPrefix = aLine.copy(0, secondSlash + 1) ;
        prefixLength = secondSlash + 1 ;

    }
    else if (aLine.compareTo(aPrefix, prefixLength) != 0)
    {
        return ;
    }
    mMapping.push_back(Mapping()) ;
    if (!mMapping.back().parse(aLine.copy(prefixLength)))
    {
        mMapping.pop_back() ;
    }
    else
    {
        const std::vector<rtl::OString>& attributes =
                                        mMapping.back().mLdapAttributes ;
        std::vector<rtl::OString>::const_iterator ldapAttribute ;

        for (ldapAttribute = attributes.begin() ;
                ldapAttribute != attributes.end() ; ++ ldapAttribute)
        {
            aLdapAttributes.insert(*ldapAttribute) ;
        }
    }
}
//------------------------------------------------------------------------------

static sal_Char kMappingSeparator = '=' ;
static sal_Char kLdapMapSeparator = ',' ;

sal_Bool LdapUserProfileMap::Mapping::parse(const rtl::OString& aLine)
{
    sal_Int32 index = aLine.indexOf(kMappingSeparator) ;

    if (index == -1)
    {
        // Imparsable line
        return sal_False ;
    }
    sal_Int32 oldIndex = index + 1 ;

    mProfileElement = aLine.copy(0, index).trim() ;
    mLdapAttributes.clear() ;
    index = aLine.indexOf(kLdapMapSeparator, oldIndex) ;
    while (index != -1)
    {
        mLdapAttributes.push_back(
                                aLine.copy(oldIndex, index - oldIndex).trim()) ;
        oldIndex = index + 1 ;
        index = aLine.indexOf(kLdapMapSeparator, oldIndex) ;
    }
    rtl::OString endOfLine = aLine.copy(oldIndex).trim() ;

    if (endOfLine.getLength() > 0)
    {
        mLdapAttributes.push_back(endOfLine) ;
    }
    return sal_True ;
}
//------------------------------------------------------------------------------

} } } // extensiond.config.ldap

