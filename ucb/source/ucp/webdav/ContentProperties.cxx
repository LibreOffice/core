/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/diagnose.h>
#include <com/sun/star/util/DateTime.hpp>
#include "SerfUri.hxx"
#include "DAVResource.hxx"
#include "DAVProperties.hxx"
#include "DateTimeHelper.hxx"
#include "webdavprovider.hxx"
#include "ContentProperties.hxx"

using namespace com::sun::star;
using namespace http_dav_ucp;

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




// ContentProperties Implementation.




// static member!
uno::Any ContentProperties::m_aEmptyAny;

ContentProperties::ContentProperties( const DAVResource& rResource )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( false )
{
    SAL_WARN_IF( !rResource.uri.getLength(), "ucb.ucp.webdav",
                "ContentProperties ctor - Empty resource URI!" );

    // Title
    try
    {
        SerfUri aURI( rResource.uri );
        m_aEscapedTitle = aURI.GetPathBaseName();

        (*m_xProps)[ OUString( "Title" ) ]
            = PropertyValue(
                uno::makeAny( aURI.GetPathBaseNameUnescaped() ), true );
    }
    catch ( DAVException const & )
    {
        (*m_xProps)[ OUString( "Title" ) ]
            = PropertyValue(
                uno::makeAny(
                    OUString( "*** unknown ***" ) ),
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

    if ( rResource.uri.endsWith("/") )
        m_bTrailingSlash = true;
}


ContentProperties::ContentProperties(
                        const OUString & rTitle, bool bFolder )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( false )
{
    (*m_xProps)[ OUString( "Title" ) ]
        = PropertyValue( uno::makeAny( rTitle ), true );
    (*m_xProps)[ OUString( "IsFolder" ) ]
        = PropertyValue( uno::makeAny( bFolder ), true );
    (*m_xProps)[ OUString( "IsDocument" ) ]
        = PropertyValue( uno::makeAny( bool( !bFolder ) ), true );
}


ContentProperties::ContentProperties( const OUString & rTitle )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( false )
{
    (*m_xProps)[ OUString( "Title" ) ]
        = PropertyValue( uno::makeAny( rTitle ), true );
}


ContentProperties::ContentProperties()
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( false )
{
}


ContentProperties::ContentProperties( const ContentProperties & rOther )
: m_aEscapedTitle( rOther.m_aEscapedTitle ),
  m_xProps( rOther.m_xProps.get()
            ? new PropertyValueMap( *rOther.m_xProps )
            : new PropertyValueMap ),
  m_bTrailingSlash( rOther.m_bTrailingSlash )
{
}


bool ContentProperties::contains( const OUString & rName ) const
{
    if ( get( rName ) )
        return true;
    else
        return false;
}


const uno::Any & ContentProperties::getValue(
                                    const OUString & rName ) const
{
    const PropertyValue * pProp = get( rName );
    if ( pProp )
        return pProp->value();
    else
        return m_aEmptyAny;
}


const PropertyValue * ContentProperties::get(
                                    const OUString & rName ) const
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


// static
void ContentProperties::UCBNamesToDAVNames(
                            const uno::Sequence< beans::Property > & rProps,
                            std::vector< OUString > & propertyNames,
                            bool bIncludeUnmatched /* = true */ )
{

    // Assemble list of DAV properties to obtain from server.
    // Append DAV properties needed to obtain requested UCB props.


    //       DAV              UCB
    // creationdate     <- DateCreated
    // getlastmodified  <- DateModified
    // getcontenttype   <- MediaType
    // getcontentlength <- Size
    // resourcetype     <- IsFolder, IsDocument, ContentType
    // (taken from URI) <- Title

    bool bCreationDate  = false;
    bool bLastModified  = false;
    bool bContentType   = false;
    bool bContentLength = false;
    bool bResourceType  = false;

    sal_Int32 nCount = rProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::Property & rProp = rProps[ n ];

        if ( rProp.Name == "Title" )
        {
            // Title is always obtained from resource's URI.
            continue;
        }
        else if ( rProp.Name == "DateCreated" ||
                  ( rProp.Name == DAVProperties::CREATIONDATE ) )
        {
            if ( !bCreationDate )
            {
                    propertyNames.push_back( DAVProperties::CREATIONDATE );
                bCreationDate = true;
            }
        }
        else if ( rProp.Name == "DateModified" ||
                  ( rProp.Name == DAVProperties::GETLASTMODIFIED ) )
        {
            if ( !bLastModified )
            {
                    propertyNames.push_back(
                    DAVProperties::GETLASTMODIFIED );
                bLastModified = true;
            }
        }
        else if ( rProp.Name == "MediaType" ||
                  ( rProp.Name == DAVProperties::GETCONTENTTYPE ) )
        {
            if ( !bContentType )
            {
                    propertyNames.push_back(
                        DAVProperties::GETCONTENTTYPE );
                bContentType = true;
            }
        }
        else if ( rProp.Name == "Size" ||
                  ( rProp.Name == DAVProperties::GETCONTENTLENGTH ) )
        {
            if ( !bContentLength )
            {
                    propertyNames.push_back(
                    DAVProperties::GETCONTENTLENGTH );
                bContentLength = true;
            }
        }
        else if ( rProp.Name == "ContentType" ||
                  rProp.Name == "IsDocument" ||
                  rProp.Name == "IsFolder" ||
                  ( rProp.Name == DAVProperties::RESOURCETYPE ) )
        {
            if ( !bResourceType )
            {
                    propertyNames.push_back( DAVProperties::RESOURCETYPE );
                bResourceType = true;
            }
        }
        else
        {
            if ( bIncludeUnmatched )
                propertyNames.push_back( rProp.Name );
        }
    }
}


// static
void ContentProperties::UCBNamesToHTTPNames(
                            const uno::Sequence< beans::Property > & rProps,
                            std::vector< OUString > & propertyNames,
                            bool bIncludeUnmatched /* = true */ )
{

    // Assemble list of HTTP header names to obtain from server.
    // Append HTTP headers needed to obtain requested UCB props.


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
            propertyNames.push_back( OUString( "Last-Modified" ) );
        }
        else if ( rProp.Name == "MediaType" )
        {
            propertyNames.push_back( OUString( "Content-Type" ) );
        }
        else if ( rProp.Name == "Size" )
        {
            propertyNames.push_back( OUString( "Content-Length" ) );
        }
        else
        {
            if ( bIncludeUnmatched )
                propertyNames.push_back( rProp.Name );
        }
    }
}


bool ContentProperties::containsAllNames(
                    const uno::Sequence< beans::Property >& rProps,
                    std::vector< OUString > & rNamesNotContained ) const
{
    rNamesNotContained.clear();

    sal_Int32 nCount = rProps.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const OUString & rName = rProps[ n ].Name;
        if ( !contains( rName ) )
        {
            // Not found.
            rNamesNotContained.push_back( rName );
        }
    }

    return ( rNamesNotContained.size() == 0 );
}


void ContentProperties::addProperties(
                                const std::vector< OUString > & rProps,
                                const ContentProperties & rContentProps )
{
    std::vector< OUString >::const_iterator it  = rProps.begin();
    std::vector< OUString >::const_iterator end = rProps.end();

    while ( it != end )
    {
        const OUString & rName = (*it);

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


void ContentProperties::addProperties( const ContentProperties & rProps )
{
    PropertyValueMap::const_iterator it = rProps.m_xProps->begin();
    const PropertyValueMap::const_iterator end = rProps.m_xProps->end();

    while ( it != end )
    {
        addProperty(
            (*it).first, (*it).second.value(), (*it).second.isCaseSensitive() );
        ++it;
    }
}


void ContentProperties::addProperties(
    const std::vector< DAVPropertyValue > & rProps )
{
    std::vector< DAVPropertyValue >::const_iterator it  = rProps.begin();
    const std::vector< DAVPropertyValue >::const_iterator end = rProps.end();

    while ( it != end )
    {
        addProperty( (*it) );
        ++it;
    }
}


void ContentProperties::addProperty( const DAVPropertyValue & rProp )
{
    addProperty( rProp.Name, rProp.Value, rProp.IsCaseSensitive );
}


void ContentProperties::addProperty( const OUString & rName,
                                     const css::uno::Any & rValue,
                                     bool bIsCaseSensitive )
{
    if ( rName == DAVProperties::CREATIONDATE )
    {
        // Map DAV:creationdate to UCP:DateCreated
        OUString aValue;
        rValue >>= aValue;
        util::DateTime aDate;
        DateTimeHelper::convert( aValue, aDate );

        (*m_xProps)[ OUString( "DateCreated" ) ]
            = PropertyValue( uno::makeAny( aDate ), true );
    }
    //  else if ( rName.equals( DAVProperties::DISPLAYNAME ) )
    //  {
    //  }
    //  else if ( rName.equals( DAVProperties::GETCONTENTLANGUAGE ) )
    //  {
    //  }
    else if ( rName == DAVProperties::GETCONTENTLENGTH )
    {
        // Map DAV:getcontentlength to UCP:Size
        OUString aValue;
        rValue >>= aValue;

        (*m_xProps)[ OUString( "Size" ) ]
            = PropertyValue( uno::makeAny( aValue.toInt64() ), true );
    }
    else if ( rName == "Content-Length" )
    {
        // Do NOT map Content-Length entity header to DAV:getcontentlength!
        // Only DAV resources have this property.

        // Map Content-Length entity header to UCP:Size
        OUString aValue;
        rValue >>= aValue;

        (*m_xProps)[ OUString( "Size" ) ]
            = PropertyValue( uno::makeAny( aValue.toInt64() ), true );
    }
    else if ( rName == DAVProperties::GETCONTENTTYPE )
    {
        // Map DAV:getcontenttype to UCP:MediaType (1:1)
        (*m_xProps)[ OUString( "MediaType" ) ]
            = PropertyValue( rValue, true );
    }
    else if ( rName == "Content-Type" )
    {
        // Do NOT map Content-Type entity header to DAV:getcontenttype!
        // Only DAV resources have this property.

        // Map DAV:getcontenttype to UCP:MediaType (1:1)
        (*m_xProps)[ OUString( "MediaType" ) ]
            = PropertyValue( rValue, true );
    }
    //  else if ( rName.equals( DAVProperties::GETETAG ) )
    //  {
    //  }
    else if ( rName == DAVProperties::GETLASTMODIFIED )
    {
        // Map the DAV:getlastmodified entity header to UCP:DateModified
        OUString aValue;
        rValue >>= aValue;
        util::DateTime aDate;
        DateTimeHelper::convert( aValue, aDate );

        (*m_xProps)[ OUString( "DateModified" ) ]
            = PropertyValue( uno::makeAny( aDate ), true );
    }
    else if ( rName == "Last-Modified" )
    {
        // Do not map Last-Modified entity header to DAV:getlastmodified!
        // Only DAV resources have this property.

        // Map the Last-Modified entity header to UCP:DateModified
        OUString aValue;
        rValue >>= aValue;
        util::DateTime aDate;
        DateTimeHelper::convert( aValue, aDate );

        (*m_xProps)[ OUString( "DateModified" ) ]
            = PropertyValue( uno::makeAny( aDate ), true );
    }
    //  else if ( rName.equals( DAVProperties::LOCKDISCOVERY ) )
    //  {
    //  }
    else if ( rName == DAVProperties::RESOURCETYPE )
    {
        OUString aValue;
        rValue >>= aValue;

        // Map DAV:resourceype to UCP:IsFolder, UCP:IsDocument, UCP:ContentType
        bool bFolder =
            aValue.equalsIgnoreAsciiCase( "collection" );

        (*m_xProps)[ OUString( "IsFolder" ) ]
            = PropertyValue( uno::makeAny( bFolder ), true );
        (*m_xProps)[ OUString( "IsDocument" ) ]
            = PropertyValue( uno::makeAny( bool( !bFolder ) ), true );
        (*m_xProps)[ OUString( "ContentType" ) ]
            = PropertyValue( uno::makeAny( bFolder
                ? OUString( WEBDAV_COLLECTION_TYPE )
                : OUString( WEBDAV_CONTENT_TYPE ) ), true );
    }
    //  else if ( rName.equals( DAVProperties::SUPPORTEDLOCK ) )
    //  {
    //  }

    // Save property.
    (*m_xProps)[ rName ] = PropertyValue( rValue, bIsCaseSensitive );
}




// CachableContentProperties Implementation.




namespace
{
    bool isCachable( OUString const & rName,
                     bool isCaseSensitive )
    {
        static const OUString aNonCachableProps [] =
        {
            DAVProperties::LOCKDISCOVERY,

            DAVProperties::GETETAG,
            OUString( "ETag" ),

            OUString( "DateModified" ),
            OUString( "Last-Modified" ),
            DAVProperties::GETLASTMODIFIED,

            OUString( "Size" ),
            OUString( "Content-Length" ),
            DAVProperties::GETCONTENTLENGTH,

            OUString( "Date" )
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


CachableContentProperties::CachableContentProperties(
    const ContentProperties & rProps )
{
    addProperties( rProps );
}


void CachableContentProperties::addProperties(
    const ContentProperties & rProps )
{
    const std::unique_ptr< PropertyValueMap > & props = rProps.getProperties();

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
