/*************************************************************************
 *
 *  $RCSfile: ContentProperties.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:27:18 $
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

#include <memory>
#include <vector>
#include <hash_map>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace webdav_ucp
{

struct DAVResource;

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

    ContentProperties( const ContentProperties & rOther );

    bool contains( const rtl::OUString & rName ) const;

    const com::sun::star::uno::Any &
    getValue( const rtl::OUString & rName ) const;

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

    // Returns a list of HTTP header names that can be mapped to UCB property
    // names.
    static void getMappableHTTPHeaders(
         std::vector< rtl::OUString > & rHeaderNames );

    // return true, if all properties contained in rProps are contained in
    // this ContentProperties instance. Otherwiese, false will be returned.
    // rNamesNotContained contain the missing names.
    bool containsAllNames(
                    const com::sun::star::uno::Sequence<
                        com::sun::star::beans::Property >& rProps,
                    std::vector< rtl::OUString > & rNamesNotContained ) const;

    // adds all properties described by rProps that are actually contained in
    // rContentProps to this instance. In case of duplicates the value
    // already contained in this will left unchanged.
    void addProperties( const std::vector< rtl::OUString > & rProps,
                        const ContentProperties & rContentProps );

    // overwrites probably existing entry.
    void addProperty( const rtl::OUString & rName,
                      const com::sun::star::uno::Any & rValue );

    bool isTrailingSlash() const { return m_bTrailingSlash; }

    const rtl::OUString & getEscapedTitle() const { return m_aEscapedTitle; }

    // Not good to expose implementation details, but this is actually an
    // internal class.
    const std::auto_ptr< PropertyValueMap > & getProperties() const
    { return m_xProps; }

private:
    ::rtl::OUString m_aEscapedTitle;  // escaped Title
    std::auto_ptr< PropertyValueMap > m_xProps;
    bool m_bTrailingSlash;

    static com::sun::star::uno::Any m_aEmptyAny;

    ContentProperties & operator=( const ContentProperties & ); // n.i.
};

};

#endif /* !_WEBDAV_UCP_CONTENTPROPERTIES_HXX */
