/*************************************************************************
 *
 *  $RCSfile: ContentProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2001-09-06 10:37:56 $
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

#ifndef _WEBDAV_UCP_CONTENTPROPERTIES_HXX
#define _WEBDAV_UCP_CONTENTPROPERTIES_HXX

#include <vector>
#include <hash_map>

#ifndef _COM_SUN_STAR_UCB_LINK_HPP_
#include <com/sun/star/ucb/Link.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_LOCK_HPP_
#include <com/sun/star/ucb/Lock.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_LOCKENTRY_HPP_
#include <com/sun/star/ucb/LockEntry.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
} } } }

namespace webdav_ucp
{

//=========================================================================

struct equalString
{
  bool operator()( const rtl::OUString& s1, const rtl::OUString& s2 ) const
  {
        return !!( s1 == s2 );
  }
};

struct hashString
{
    size_t operator()( const rtl::OUString & rName ) const
    {
        return rName.hashCode();
    }
};

//=========================================================================
//
// PropertyValueMap.
//
//=========================================================================

typedef std::hash_map
<
    rtl::OUString,
    ::com::sun::star::uno::Any,
    hashString,
    equalString
>
PropertyValueMap;

struct DAVResource;

class ContentProperties
{
public:
    // Mandatory UCB props.
      ::rtl::OUString aTitle;           // Title
      ::rtl::OUString aEscapedTitle;    // escaped Title
    sal_Bool * pIsFolder;
    sal_Bool * pIsDocument;

    // Optional UCB props.
      sal_Int64 *                          pSize;         // Size <- getcontentlength
      ::com::sun::star::util::DateTime * pDateCreated;  // DateCreated <- creationdate
      ::com::sun::star::util::DateTime * pDateModified; // DateModified <- getlastmodified

      ::rtl::OUString * pgetcontenttype;   // MediaType == getcontenttype

    // DAV props.
      ::rtl::OUString * pcreationdate;
      ::rtl::OUString * pdisplayname;
      ::rtl::OUString * pgetcontentlanguage;
      ::rtl::OUString * pgetcontentlength;
      ::rtl::OUString * pgetetag;
      ::rtl::OUString * pgetlastmodified;
      ::com::sun::star::uno::Sequence<
        ::com::sun::star::ucb::Lock > * plockdiscovery;
      ::rtl::OUString * presourcetype;
      ::com::sun::star::uno::Sequence<
          ::com::sun::star::ucb::Link > * psource;
      ::com::sun::star::uno::Sequence<
        ::com::sun::star::ucb::LockEntry > * psupportedlock;

    PropertyValueMap * pOtherProps;

public:
      ContentProperties( const DAVResource& rResource );

    // Mini props for transient contents.
      ContentProperties( const rtl::OUString & rTitle, sal_Bool bFolder );

    // Micro props for non-existing contents.
    ContentProperties( const rtl::OUString & rTitle );

    inline ~ContentProperties();

    // Substitutes UCB property names by DAV property names. The list
    // generated can directly be handed over to DAVResourceAccess::PROPFIND.
    // The result from PROPFIND can be used to create a ContentProperties
    // instance which can map DAV properties back to UCB properties.
    static void UCBNamesToDAVNames( const com::sun::star::uno::Sequence<
                                        com::sun::star::beans::Property > &
                                            rProps,
                                    std::vector< rtl::OUString > & resources );
};

inline ContentProperties::~ContentProperties()
{
    delete pIsFolder;
    delete pIsDocument;
      delete pSize;
      delete pDateCreated;
      delete pDateModified;
      delete pgetcontenttype;
      delete pcreationdate;
      delete pdisplayname;
      delete pgetcontentlanguage;
      delete pgetcontentlength;
      delete pgetetag;
      delete pgetlastmodified;
    delete plockdiscovery;
      delete presourcetype;
    delete psource;
    delete psupportedlock;
    delete pOtherProps;
}

};

#endif /* !_WEBDAV_UCP_CONTENTPROPERTIES_HXX */
