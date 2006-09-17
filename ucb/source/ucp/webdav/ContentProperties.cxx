/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContentProperties.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:04:00 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif
#ifndef _DAVRESOURCE_HXX_
#include "DAVResource.hxx"
#endif
#ifndef _DAVPROPERTIES_HXX_
#include "DAVProperties.hxx"
#endif
#ifndef _WEBDAV_DATETIME_HELPER_HXX
#include "DateTimeHelper.hxx"
#endif
#ifndef _WEBDAV_UCP_PROVIDER_HXX
#include "webdavprovider.hxx"
#endif

#ifndef _WEBDAV_UCP_CONTENTPROPERTIES_HXX
#include "ContentProperties.hxx"
#endif

using namespace com::sun::star;
using namespace webdav_ucp;

/*
=============================================================================

                            Property Mapping

=============================================================================
HTTP (entity header)    WebDAV (property)   UCB (property)
=============================================================================

Allow
Content-Encoding
Content-Language        getcontentlanguage
Content-Length          getcontentlength    Size
Content-Location
Content-MD5
Content-Range
Content-Type            getcontenttype      MediaType
Expires
Last-Modified           getlastmodified     DateModified
                        creationdate        DateCreated
                        resourcetype        IsFolder,IsDocument,ContentType
                        displayname
ETag (actually          getetag
a response header )
                        lockdiscovery
                        supportedlock
                        source
                                            Title (always taken from URI)

=============================================================================

Important: HTTP headers will not be mapped to DAV properties; only to UCB
           properties. (Content-Length,Content-Type,Last-Modified)
*/

//=========================================================================
//=========================================================================
//
// ContentProperties Implementation.
//
//=========================================================================
//=========================================================================

// static member!
uno::Any ContentProperties::m_aEmptyAny;

ContentProperties::ContentProperties( const DAVResource& rResource )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( false )
{
    OSL_ENSURE( rResource.uri.getLength(),
                "ContentProperties ctor - Empty resource URI!" );

    // Title
      NeonUri aURI( rResource.uri );
    m_aEscapedTitle = aURI.GetPathBaseName();

    (*m_xProps)[ rtl::OUString::createFromAscii( "Title" ) ]
                = uno::makeAny( aURI.GetPathBaseNameUnescaped() );

    std::vector< beans::PropertyValue >::const_iterator it
        = rResource.properties.begin();
      std::vector< beans::PropertyValue >::const_iterator end
        = rResource.properties.end();

      while ( it != end )
      {
        beans::PropertyValue aProp = (*it);

        if ( aProp.Name.equals( DAVProperties::CREATIONDATE ) )
        {
            // Map DAV:creationdate to UCP:DateCreated
            rtl::OUString aValue;
            aProp.Value >>= aValue;
            util::DateTime aDate;
            DateTimeHelper::convert( aValue, aDate );

            (*m_xProps)[ rtl::OUString::createFromAscii( "DateCreated" ) ]
                = uno::makeAny( aDate );
        }
//      else if ( aProp.Name.equals( DAVProperties::DISPLAYNAME ) )
//      {
//      }
//      else if ( aProp.Name.equals( DAVProperties::GETCONTENTLANGUAGE ) )
//      {
//      }
        else if ( aProp.Name.equals( DAVProperties::GETCONTENTLENGTH ) )
        {
            // Map DAV:getcontentlength to UCP:Size
            rtl::OUString aValue;
            aProp.Value >>= aValue;

            (*m_xProps)[ rtl::OUString::createFromAscii( "Size" ) ]
                = uno::makeAny( aValue.toInt64() );
        }
        else if ( aProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Content-Length" ) ) )
        {
            // Do NOT map Content-Lenght entity header to DAV:getcontentlength!
            // Only DAV resources have this property.

            // Map Content-Length entity header to UCP:Size
            rtl::OUString aValue;
            aProp.Value >>= aValue;

            (*m_xProps)[ rtl::OUString::createFromAscii( "Size" ) ]
                = uno::makeAny( aValue.toInt64() );
        }
        else if ( aProp.Name.equals( DAVProperties::GETCONTENTTYPE ) )
        {
            // Map DAV:getcontenttype to UCP:MediaType (1:1)
            (*m_xProps)[ rtl::OUString::createFromAscii( "MediaType" ) ]
                = aProp.Value;
        }
        else if ( aProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Content-Type" ) ) )
        {
            // Do NOT map Content-Type entity header to DAV:getcontenttype!
            // Only DAV resources have this property.

            // Map DAV:getcontenttype to UCP:MediaType (1:1)
            (*m_xProps)[ rtl::OUString::createFromAscii( "MediaType" ) ]
                = aProp.Value;
        }
//      else if ( aProp.Name.equals( DAVProperties::GETETAG ) )
//      {
//      }
        else if ( aProp.Name.equals( DAVProperties::GETLASTMODIFIED ) )
        {
            // Map the DAV:getlastmodified entity header to UCP:DateModified
            rtl::OUString aValue;
            aProp.Value >>= aValue;
            util::DateTime aDate;
            DateTimeHelper::convert( aValue, aDate );

            (*m_xProps)[ rtl::OUString::createFromAscii( "DateModified" ) ]
                = uno::makeAny( aDate );
        }
        else if ( aProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Last-Modified" ) ) )
        {
            // Do not map Last-Modified entity header to DAV:getlastmodified!
            // Only DAV resources have this property.

            // Map the Last-Modified entity header to UCP:DateModified
            rtl::OUString aValue;
            aProp.Value >>= aValue;
            util::DateTime aDate;
            DateTimeHelper::convert( aValue, aDate );

            (*m_xProps)[ rtl::OUString::createFromAscii( "DateModified" ) ]
                = uno::makeAny( aDate );
        }
//      else if ( aProp.Name.equals( DAVProperties::LOCKDISCOVERY ) )
//      {
//      }
        else if ( aProp.Name.equals( DAVProperties::RESOURCETYPE ) )
        {
            rtl::OUString aValue;
            aProp.Value >>= aValue;

            // Map DAV:resourceype to UCP:IsFolder, UCP:IsDocument, UCP:ContentType
            sal_Bool bFolder =
                aValue.equalsIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "collection" ) );

            (*m_xProps)[ rtl::OUString::createFromAscii( "IsFolder" ) ]
                = uno::makeAny( bFolder );
            (*m_xProps)[ rtl::OUString::createFromAscii( "IsDocument" ) ]
                = uno::makeAny( sal_Bool( !bFolder ) );
            (*m_xProps)[ rtl::OUString::createFromAscii( "ContentType" ) ]
                    = uno::makeAny( bFolder
                        ? rtl::OUString::createFromAscii(
                            WEBDAV_COLLECTION_TYPE )
                        : rtl::OUString::createFromAscii(
                            WEBDAV_CONTENT_TYPE ) );
        }
//      else if ( aProp.Name.equals( DAVProperties::SOURCE ) )
//      {
//      }
//      else if ( aProp.Name.equals( DAVProperties::SUPPORTEDLOCK ) )
//      {
//      }

        // Save property.
        (*m_xProps)[ aProp.Name ] = aProp.Value;

        ++it;
      }

    if ( rResource.uri.getStr()[ rResource.uri.getLength() - 1 ]
        == sal_Unicode( '/' ) )
        m_bTrailingSlash = sal_True;
}

//=========================================================================
ContentProperties::ContentProperties(
                        const rtl::OUString & rTitle, sal_Bool bFolder )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( sal_False )
{
    (*m_xProps)[ rtl::OUString::createFromAscii( "Title" ) ]
        = uno::makeAny( rTitle );
    (*m_xProps)[ rtl::OUString::createFromAscii( "IsFolder" ) ]
        = uno::makeAny( bFolder );
    (*m_xProps)[ rtl::OUString::createFromAscii( "IsDocument" ) ]
        = uno::makeAny( sal_Bool( !bFolder ) );
}

//=========================================================================
ContentProperties::ContentProperties( const rtl::OUString & rTitle )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( sal_False )
{
    (*m_xProps)[ rtl::OUString::createFromAscii( "Title" ) ]
        = uno::makeAny( rTitle );
}

//=========================================================================
ContentProperties::ContentProperties( const ContentProperties & rOther )
: m_aEscapedTitle( rOther.m_aEscapedTitle ),
  m_xProps( rOther.m_xProps.get()
            ? new PropertyValueMap( *rOther.m_xProps )
            : new PropertyValueMap ),
  m_bTrailingSlash( rOther.m_bTrailingSlash )
{
}

//=========================================================================
bool ContentProperties::contains( const rtl::OUString & rName ) const
{
    return ( m_xProps->find( rName ) != m_xProps->end() );
}

//=========================================================================
const uno::Any & ContentProperties::getValue(
                                    const rtl::OUString & rName ) const
{
    PropertyValueMap::const_iterator it = m_xProps->find( rName );
    if ( it != m_xProps->end() )
        return (*it).second;

    return m_aEmptyAny;
}

//=========================================================================
// static
void ContentProperties::UCBNamesToDAVNames(
                            const uno::Sequence< beans::Property > & rProps,
                            std::vector< rtl::OUString > & propertyNames,
                            bool bIncludeUnmatched /* = true */ )
{
    //////////////////////////////////////////////////////////////
    // Assemble list of DAV properties to obtain from server.
    // Append DAV properties needed to obtain requested UCB props.
    //////////////////////////////////////////////////////////////

    //       DAV              UCB
    // creationdate     <- DateCreated
    // getlastmodified  <- DateModified
    // getcontenttype   <- MediaType
    // getcontentlength <- Size
    // resourcetype     <- IsFolder, IsDocument, ContentType
    // (taken from URI) <- Title

    sal_Bool bCreationDate  = sal_False;
    sal_Bool bLastModified  = sal_False;
    sal_Bool bContentType   = sal_False;
    sal_Bool bContentLength = sal_False;
    sal_Bool bResourceType  = sal_False;

    sal_Int32 nCount = rProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::Property & rProp = rProps[ n ];

        if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            // Title is always obtained from resource's URI.
            continue;
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) )
                  ||
                  ( rProp.Name == DAVProperties::CREATIONDATE ) )
        {
            if ( !bCreationDate )
            {
                   propertyNames.push_back( DAVProperties::CREATIONDATE );
                bCreationDate = sal_True;
            }
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DateModified" ) )
                  ||
                  ( rProp.Name == DAVProperties::GETLASTMODIFIED ) )
        {
            if ( !bLastModified )
            {
                   propertyNames.push_back(
                    DAVProperties::GETLASTMODIFIED );
                bLastModified = sal_True;
            }
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "MediaType" ) )
                  ||
                  ( rProp.Name == DAVProperties::GETCONTENTTYPE ) )
        {
            if ( !bContentType )
            {
                   propertyNames.push_back(
                        DAVProperties::GETCONTENTTYPE );
                bContentType = sal_True;
            }
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Size" ) )
                  ||
                  ( rProp.Name == DAVProperties::GETCONTENTLENGTH ) )
        {
            if ( !bContentLength )
            {
                   propertyNames.push_back(
                    DAVProperties::GETCONTENTLENGTH );
                bContentLength = sal_True;
            }
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "ContentType" ) )
                  ||
                  rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) )
                  ||
                  rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) )
                  ||
                  ( rProp.Name == DAVProperties::RESOURCETYPE ) )
        {
            if ( !bResourceType )
            {
                   propertyNames.push_back( DAVProperties::RESOURCETYPE );
                bResourceType = sal_True;
            }
        }
        else
        {
            if ( bIncludeUnmatched )
                propertyNames.push_back( rProp.Name );
        }
    }
}

//=========================================================================
// static
void ContentProperties::UCBNamesToHTTPNames(
                            const uno::Sequence< beans::Property > & rProps,
                            std::vector< rtl::OUString > & propertyNames,
                            bool bIncludeUnmatched /* = true */ )
{
    //////////////////////////////////////////////////////////////
    // Assemble list of HTTP header names to obtain from server.
    // Append HTTP headers needed to obtain requested UCB props.
    //////////////////////////////////////////////////////////////

    //       HTTP              UCB
    // Last-Modified  <- DateModified
    // Content-Type   <- MediaType
    // Content-Length <- Size

    sal_Int32 nCount = rProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::Property & rProp = rProps[ n ];

        if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) )
        {
            propertyNames.push_back(
                rtl::OUString::createFromAscii( "Last-Modified" ) );
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
        {
            propertyNames.push_back(
                rtl::OUString::createFromAscii( "Content-Type" ) );
        }
        else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
        {
            propertyNames.push_back(
                rtl::OUString::createFromAscii( "Content-Length" ) );
        }
        else
        {
            if ( bIncludeUnmatched )
                propertyNames.push_back( rProp.Name );
        }
    }
}

//=========================================================================
// static
void ContentProperties::getMappableHTTPHeaders(
                            std::vector< rtl::OUString > & rHeaderNames )
{
    // -> DateModified
    rHeaderNames.push_back(
        rtl::OUString::createFromAscii( "Last-Modified" ) );

    // -> MediaType
    rHeaderNames.push_back(
        rtl::OUString::createFromAscii( "Content-Type" ) );

    // -> Size
    rHeaderNames.push_back(
        rtl::OUString::createFromAscii( "Content-Length" ) );

    // -> BaseURI
    rHeaderNames.push_back(
        rtl::OUString::createFromAscii( "Content-Location" ) );
}

//=========================================================================
bool ContentProperties::containsAllNames(
                    const uno::Sequence< beans::Property >& rProps,
                    std::vector< rtl::OUString > & rNamesNotContained ) const
{
    rNamesNotContained.clear();

    sal_Int32 nCount = rProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const rtl::OUString & rName = rProps[ n ].Name;
        if ( !contains( rName ) )
        {
            // Not found.
            rNamesNotContained.push_back( rName );
        }
    }

    return ( rNamesNotContained.size() == 0 );
}

//=========================================================================
void ContentProperties::addProperties(
                                const std::vector< rtl::OUString > & rProps,
                                const ContentProperties & rContentProps )
{
    std::vector< rtl::OUString >::const_iterator it  = rProps.begin();
    std::vector< rtl::OUString >::const_iterator end = rProps.end();

    while ( it != end )
    {
        const rtl::OUString & rName = (*it);

        if ( !contains( rName ) ) // ignore duplicates
        {
            const uno::Any & rValue = rContentProps.getValue( rName );
            if ( rValue.hasValue() )
            {
                // Add it.
                (*m_xProps)[ rName ] = rValue;
            }
        }

        ++it;
    }
}

//=========================================================================
void ContentProperties::addProperty( const rtl::OUString & rName,
                                     const com::sun::star::uno::Any & rValue )
{
    (*m_xProps)[ rName ] = rValue;
}

