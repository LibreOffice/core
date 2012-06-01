/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include <com/sun/star/util/DateTime.hpp>
#include "NeonUri.hxx"
#include "DAVResource.hxx"
#include "DAVProperties.hxx"
#include "DateTimeHelper.hxx"
#include "webdavprovider.hxx"
#include "ContentProperties.hxx"

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
    OSL_ENSURE( !rResource.uri.isEmpty(),
                "ContentProperties ctor - Empty resource URI!" );

    // Title
    try
    {
        NeonUri aURI( rResource.uri );
        m_aEscapedTitle = aURI.GetPathBaseName();

        (*m_xProps)[ rtl::OUString("Title") ]
            = PropertyValue(
                uno::makeAny( aURI.GetPathBaseNameUnescaped() ), true );
    }
    catch ( DAVException const & )
    {
        (*m_xProps)[ rtl::OUString("Title") ]
            = PropertyValue(
                uno::makeAny(
                    rtl::OUString(
                         "*** unknown ***"  ) ),
                true );
    }

    std::vector< DAVPropertyValue >::const_iterator it
        = rResource.properties.begin();
    std::vector< DAVPropertyValue >::const_iterator end
        = rResource.properties.end();

    while ( it != end )
    {
        addProperty( (*it) );
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
    (*m_xProps)[ rtl::OUString("Title") ]
        = PropertyValue( uno::makeAny( rTitle ), true );
    (*m_xProps)[ rtl::OUString("IsFolder") ]
        = PropertyValue( uno::makeAny( bFolder ), true );
    (*m_xProps)[ rtl::OUString("IsDocument") ]
        = PropertyValue( uno::makeAny( sal_Bool( !bFolder ) ), true );
}

//=========================================================================
ContentProperties::ContentProperties( const rtl::OUString & rTitle )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( sal_False )
{
    (*m_xProps)[ rtl::OUString("Title") ]
        = PropertyValue( uno::makeAny( rTitle ), true );
}

//=========================================================================
ContentProperties::ContentProperties()
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( sal_False )
{
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
    if ( get( rName ) )
        return true;
    else
        return false;
}

//=========================================================================
const uno::Any & ContentProperties::getValue(
                                    const rtl::OUString & rName ) const
{
    const PropertyValue * pProp = get( rName );
    if ( pProp )
        return pProp->value();
    else
        return m_aEmptyAny;
}

//=========================================================================
const PropertyValue * ContentProperties::get(
                                    const rtl::OUString & rName ) const
{
    PropertyValueMap::const_iterator it = m_xProps->find( rName );
    const PropertyValueMap::const_iterator end = m_xProps->end();

    if ( it == end )
    {
        it  = m_xProps->begin();
        while ( it != end )
        {
            if ( (*it).first.equalsIgnoreAsciiCase( rName ) )
                return &(*it).second;

            ++it;
        }
        return 0;
    }
    else
        return &(*it).second;
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

        if ( rProp.Name == "Title" )
        {
            // Title is always obtained from resource's URI.
            continue;
        }
        else if ( rProp.Name == "DateCreated" || rProp.Name == DAVProperties::CREATIONDATE )
        {
            if ( !bCreationDate )
            {
                    propertyNames.push_back( DAVProperties::CREATIONDATE );
                bCreationDate = sal_True;
            }
        }
        else if ( rProp.Name == "DateModified" || rProp.Name == DAVProperties::GETLASTMODIFIED )
        {
            if ( !bLastModified )
            {
                    propertyNames.push_back(
                    DAVProperties::GETLASTMODIFIED );
                bLastModified = sal_True;
            }
        }
        else if ( rProp.Name == "MediaType" || rProp.Name == DAVProperties::GETCONTENTTYPE )
        {
            if ( !bContentType )
            {
                    propertyNames.push_back(
                        DAVProperties::GETCONTENTTYPE );
                bContentType = sal_True;
            }
        }
        else if ( rProp.Name == "Size" || rProp.Name == DAVProperties::GETCONTENTLENGTH )
        {
            if ( !bContentLength )
            {
                    propertyNames.push_back(
                    DAVProperties::GETCONTENTLENGTH );
                bContentLength = sal_True;
            }
        }
        else if ( rProp.Name == "ContentType" || rProp.Name == "IsDocument" || rProp.Name == "IsFolder" || rProp.Name == DAVProperties::RESOURCETYPE )
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

        if ( rProp.Name == "DateModified" )
        {
            propertyNames.push_back(
                rtl::OUString("Last-Modified") );
        }
        else if ( rProp.Name == "MediaType" )
        {
            propertyNames.push_back(
                rtl::OUString("Content-Type") );
        }
        else if ( rProp.Name == "Size" )
        {
            propertyNames.push_back(
                rtl::OUString("Content-Length") );
        }
        else
        {
            if ( bIncludeUnmatched )
                propertyNames.push_back( rProp.Name );
        }
    }
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

    return ( rNamesNotContained.empty() );
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
            const PropertyValue * pProp = rContentProps.get( rName );
            if ( pProp )
            {
                // Add it.
                addProperty( rName, pProp->value(), pProp->isCaseSensitive() );
            }
            else
            {
                addProperty( rName, uno::Any(), false );
            }
        }
        ++it;
    }
}

//=========================================================================
void ContentProperties::addProperty( const DAVPropertyValue & rProp )
{
    addProperty( rProp.Name, rProp.Value, rProp.IsCaseSensitive );
}

//=========================================================================
void ContentProperties::addProperty( const rtl::OUString & rName,
                                     const com::sun::star::uno::Any & rValue,
                                     bool bIsCaseSensitive )
{
    if ( rName.equals( DAVProperties::CREATIONDATE ) )
    {
        // Map DAV:creationdate to UCP:DateCreated
        rtl::OUString aValue;
        rValue >>= aValue;
        util::DateTime aDate;
        DateTimeHelper::convert( aValue, aDate );

        (*m_xProps)[ rtl::OUString("DateCreated") ]
            = PropertyValue( uno::makeAny( aDate ), true );
    }
    //  else if ( rName.equals( DAVProperties::DISPLAYNAME ) )
    //  {
    //  }
    //  else if ( rName.equals( DAVProperties::GETCONTENTLANGUAGE ) )
    //  {
    //  }
    else if ( rName.equals( DAVProperties::GETCONTENTLENGTH ) )
    {
        // Map DAV:getcontentlength to UCP:Size
        rtl::OUString aValue;
        rValue >>= aValue;

        (*m_xProps)[ rtl::OUString("Size") ]
            = PropertyValue( uno::makeAny( aValue.toInt64() ), true );
    }
    else if ( rName == "Content-Length" )
    {
        // Do NOT map Content-Lenght entity header to DAV:getcontentlength!
        // Only DAV resources have this property.

        // Map Content-Length entity header to UCP:Size
        rtl::OUString aValue;
        rValue >>= aValue;

        (*m_xProps)[ rtl::OUString("Size") ]
            = PropertyValue( uno::makeAny( aValue.toInt64() ), true );
    }
    else if ( rName.equals( DAVProperties::GETCONTENTTYPE ) )
    {
        // Map DAV:getcontenttype to UCP:MediaType (1:1)
        (*m_xProps)[ rtl::OUString("MediaType") ]
            = PropertyValue( rValue, true );
    }
    else if ( rName == "Content-Type" )
    {
        // Do NOT map Content-Type entity header to DAV:getcontenttype!
        // Only DAV resources have this property.

        // Map DAV:getcontenttype to UCP:MediaType (1:1)
        (*m_xProps)[ rtl::OUString("MediaType") ]
            = PropertyValue( rValue, true );
    }
    //  else if ( rName.equals( DAVProperties::GETETAG ) )
    //  {
    //  }
    else if ( rName.equals( DAVProperties::GETLASTMODIFIED ) )
    {
        // Map the DAV:getlastmodified entity header to UCP:DateModified
        rtl::OUString aValue;
        rValue >>= aValue;
        util::DateTime aDate;
        DateTimeHelper::convert( aValue, aDate );

        (*m_xProps)[ rtl::OUString("DateModified") ]
            = PropertyValue( uno::makeAny( aDate ), true );
    }
    else if ( rName == "Last-Modified" )
    {
        // Do not map Last-Modified entity header to DAV:getlastmodified!
        // Only DAV resources have this property.

        // Map the Last-Modified entity header to UCP:DateModified
        rtl::OUString aValue;
        rValue >>= aValue;
        util::DateTime aDate;
        DateTimeHelper::convert( aValue, aDate );

        (*m_xProps)[ rtl::OUString("DateModified") ]
            = PropertyValue( uno::makeAny( aDate ), true );
    }
    //  else if ( rName.equals( DAVProperties::LOCKDISCOVERY ) )
    //  {
    //  }
    else if ( rName.equals( DAVProperties::RESOURCETYPE ) )
    {
        rtl::OUString aValue;
        rValue >>= aValue;

        // Map DAV:resourceype to UCP:IsFolder, UCP:IsDocument, UCP:ContentType
        sal_Bool bFolder =
            aValue.equalsIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "collection" ) );

        (*m_xProps)[ rtl::OUString("IsFolder") ]
            = PropertyValue( uno::makeAny( bFolder ), true );
        (*m_xProps)[ rtl::OUString("IsDocument") ]
            = PropertyValue( uno::makeAny( sal_Bool( !bFolder ) ), true );
        (*m_xProps)[ rtl::OUString("ContentType") ]
            = PropertyValue( uno::makeAny( bFolder
                ? rtl::OUString( WEBDAV_COLLECTION_TYPE )
                : rtl::OUString( WEBDAV_CONTENT_TYPE ) ), true );
    }
    //  else if ( rName.equals( DAVProperties::SOURCE ) )
    //  {
    //  }
    //  else if ( rName.equals( DAVProperties::SUPPORTEDLOCK ) )
    //  {
    //  }

    // Save property.
    (*m_xProps)[ rName ] = PropertyValue( rValue, bIsCaseSensitive );
}

//=========================================================================
//=========================================================================
//
// CachableContentProperties Implementation.
//
//=========================================================================
//=========================================================================

namespace
{
    bool isCachable( rtl::OUString const & rName,
                     bool isCaseSensitive )
    {
        static rtl::OUString aNonCachableProps [] =
        {
            DAVProperties::LOCKDISCOVERY,

            DAVProperties::GETETAG,
            rtl::OUString(  "ETag"  ),

            rtl::OUString(  "DateModified"  ),
            rtl::OUString(  "Last-Modified"  ),
            DAVProperties::GETLASTMODIFIED,

            rtl::OUString(  "Size"  ),
            rtl::OUString(  "Content-Length"  ),
            DAVProperties::GETCONTENTLENGTH,

            rtl::OUString(  "Date"  )
        };

        for ( sal_uInt32 n = 0;
              n <  ( sizeof( aNonCachableProps )
                     / sizeof( aNonCachableProps[ 0 ] ) );
              ++n )
        {
            if ( isCaseSensitive )
            {
                if ( rName.equals( aNonCachableProps[ n ] ) )
                    return false;
            }
            else
                if ( rName.equalsIgnoreAsciiCase( aNonCachableProps[ n ] ) )
                    return false;
        }
        return true;
    }

} // namespace

//=========================================================================
CachableContentProperties::CachableContentProperties(
    const ContentProperties & rProps )
{
    addProperties( rProps );
}

//=========================================================================
void CachableContentProperties::addProperties(
    const ContentProperties & rProps )
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const std::auto_ptr< PropertyValueMap > & props = rProps.getProperties();
    SAL_WNODEPRECATED_DECLARATIONS_POP

    PropertyValueMap::const_iterator it = props->begin();
    const PropertyValueMap::const_iterator end = props->end();

    while ( it != end )
    {
        if ( isCachable( (*it).first, (*it).second.isCaseSensitive() ) )
            m_aProps.addProperty( (*it).first,
                                  (*it).second.value(),
                                  (*it).second.isCaseSensitive() );

        ++it;
    }
}

//=========================================================================
void CachableContentProperties::addProperties(
    const std::vector< DAVPropertyValue > & rProps )
{
    std::vector< DAVPropertyValue >::const_iterator it  = rProps.begin();
    const std::vector< DAVPropertyValue >::const_iterator end = rProps.end();

    while ( it != end )
    {
        if ( isCachable( (*it).Name, (*it).IsCaseSensitive ) )
            m_aProps.addProperty( (*it) );

        ++it;
     }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
