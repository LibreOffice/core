/*************************************************************************
 *
 *  $RCSfile: ContentProperties.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-29 09:00:12 $
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
      ContentProperties( const DAVResource& rResource );

    // Mini props for transient contents.
      ContentProperties( const rtl::OUString & rTitle, sal_Bool bFolder );

    // Micro props for non-existing contents.
    ContentProperties( const rtl::OUString & rTitle );

    inline ~ContentProperties();

    // Maps the UCB property names contained in rProps with their DAV property
    // counterparts, if possible. All unmappable properties will be included
    // unchanged in resulting vector unless bIncludeUnmatched is set to false.
    // The vector filles by this method can directly be handed over to
    // DAVResourceAccess::PROPFIND. The result from PROPFIND
    // (vector< DAVResource >) can be used to create a ContentProperties
    // instance which can map DAV properties back to UCB properties.
    static void UCBNamesToDAVNames( const com::sun::star::uno::Sequence<
                                        com::sun::star::beans::Property > &
                                            rProps,
                                    std::vector< rtl::OUString > & resources,
                                    bool bIncludeUnmatched = true );

    // Maps the UCB property names contained in rProps with their HTTP header
    // counterparts, if possible. All unmappable properties will be included
    // unchanged in resulting vector unless bIncludeUnmatched is set to false.
    // The vector filles by this method can directly be handed over to
    // DAVResourceAccess::HEAD. The result from HEAD (vector< DAVResource >)
    // can be used to create a ContentProperties instance which can map header
    // names back to UCB properties.
    static void UCBNamesToHTTPNames( const com::sun::star::uno::Sequence<
                                        com::sun::star::beans::Property > &
                                            rProps,
                                    std::vector< rtl::OUString > & resources,
                                    bool bIncludeUnmatched = true );

    sal_Bool isTrailingSlash() const { return bTrailingSlash; }

    /////////////////////////////////////////////////////////////////////////
    // UCB properties
    /////////////////////////////////////////////////////////////////////////
    sal_Bool queryTitle( rtl::OUString & rTitle ) const
    {
        rTitle = aTitle;
        return true;
    }

    sal_Bool queryEscapedTitle( rtl::OUString & rTitle ) const
    {
        rTitle = aEscapedTitle;
        return true;
    }

    sal_Bool queryIsFolder( sal_Bool & rbFolder ) const
    {
        if ( pIsFolder ) rbFolder = *pIsFolder;
        return !!pIsFolder;
    }

    sal_Bool queryIsDocument( sal_Bool & rbDocument ) const
    {
        if ( pIsDocument ) rbDocument = *pIsDocument;
        return !!pIsDocument;
    }

    sal_Bool querySize( sal_Int64 & rSize ) const
    {
        if ( pSize ) rSize = *pSize;
        return !!pSize;
    }

    sal_Bool queryDateCreated( ::com::sun::star::util::DateTime & rDate ) const
    {
        if ( pDateCreated ) rDate = *pDateCreated;
        return !!pDateCreated;
    }

    sal_Bool queryDateModified( ::com::sun::star::util::DateTime & rDate ) const
    {
        if ( pDateModified ) rDate = *pDateModified;
        return !!pDateModified;
    }

    sal_Bool queryMediaType( rtl::OUString & rType ) const
    {
        if ( pMediaType ) rType = *pMediaType;
        return !!pMediaType;
    }

    /////////////////////////////////////////////////////////////////////////
    // DAV properties
    /////////////////////////////////////////////////////////////////////////

    sal_Bool queryDAVCreationDate( rtl::OUString & rDate ) const
    {
        if ( pcreationdate ) rDate = *pcreationdate;
        return !!pcreationdate;
    }

    sal_Bool queryDAVDisplayName( rtl::OUString & rName ) const
    {
        if ( pdisplayname ) rName = *pdisplayname;
        return !!pdisplayname;
    }

    sal_Bool queryDAVContentType( rtl::OUString & rType ) const
    {
        if ( pgetcontenttype ) rType = *pgetcontenttype;
        return !!pgetcontenttype;
    }

    sal_Bool queryDAVContentLanguage( rtl::OUString & rLang ) const
    {
        if ( pgetcontentlanguage ) rLang = *pgetcontentlanguage;
        return !!pgetcontentlanguage;
    }

    sal_Bool queryDAVContentLength( rtl::OUString & rLen ) const
    {
        if ( pgetcontentlength ) rLen = *pgetcontentlength;
        return !!pgetcontentlength;
    }

    sal_Bool queryDAVETag( rtl::OUString & rTag ) const
    {
        if ( pgetetag ) rTag = *pgetetag;
        return !!pgetetag;
    }

    sal_Bool queryDAVLastModified( rtl::OUString & rDate ) const
    {
        if ( pgetlastmodified ) rDate = *pgetlastmodified;
        return !!pgetlastmodified;
    }

    sal_Bool queryDAVLockDiscovery(
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::ucb::Lock >& rLocks ) const
    {
        if ( plockdiscovery ) rLocks = *plockdiscovery;
        return !!plockdiscovery;
    }

    sal_Bool queryDAVResourceType( rtl::OUString & rType ) const
    {
        if ( presourcetype ) rType = *presourcetype;
        return !!presourcetype;
    }

    sal_Bool queryDAVSource(
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::ucb::Link >& rLinks ) const
    {
        if ( psource ) rLinks = *psource;
        return !!psource;
    }

    sal_Bool queryDAVSupportedLock(
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::ucb::LockEntry >& rEntries ) const
    {
        if ( psource ) rEntries = *psupportedlock;
        return !!psupportedlock;
    }

    /////////////////////////////////////////////////////////////////////////
    // Other properties (i.e. HTTP entity headers, DAV dead properties)
    /////////////////////////////////////////////////////////////////////////

    const PropertyValueMap * getOtherProperties() const { return pOtherProps; }

private:
    // Mandatory UCB props.
    ::rtl::OUString aTitle;           // Title
    ::rtl::OUString aEscapedTitle;    // escaped Title
    sal_Bool bTrailingSlash;

    sal_Bool * pIsFolder;
    sal_Bool * pIsDocument;

    // Optional UCB props.
    sal_Int64 *                        pSize;         // Size <- getcontentlength
    ::com::sun::star::util::DateTime * pDateCreated;  // DateCreated <- creationdate
    ::com::sun::star::util::DateTime * pDateModified; // DateModified <- getlastmodified

    ::rtl::OUString * pMediaType;

    // DAV props.
    ::rtl::OUString * pgetcontenttype;
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
};

inline ContentProperties::~ContentProperties()
{
    delete pIsFolder;
    delete pIsDocument;
      delete pSize;
      delete pDateCreated;
      delete pDateModified;
    delete pMediaType;
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
