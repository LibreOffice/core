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

#include <memory>
#include <com/sun/star/util/DateTime.hpp>
#include "CurlUri.hxx"
#include "DAVResource.hxx"
#include "DAVProperties.hxx"
#include "DateTimeHelper.hxx"
#include "webdavprovider.hxx"
#include "ContentProperties.hxx"
#include <o3tl/string_view.hxx>

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
    assert(!rResource.uri.isEmpty() &&
                "ContentProperties ctor - Empty resource URI!");

    // Title
    try
    {
        CurlUri const aURI( rResource.uri );
        m_aEscapedTitle = aURI.GetPathBaseName();

        (*m_xProps)[ u"Title"_ustr ]
            = PropertyValue(
                uno::Any( aURI.GetPathBaseNameUnescaped() ), true );
    }
    catch ( DAVException const & )
    {
        (*m_xProps)[ u"Title"_ustr ]
            = PropertyValue(
                uno::Any(
                    u"*** unknown ***"_ustr ),
                true );
    }

    for ( const auto& rProp : rResource.properties )
    {
        addProperty( rProp );
    }

    if ( rResource.uri.endsWith("/") )
        m_bTrailingSlash = true;
}


ContentProperties::ContentProperties(
                        const OUString & rTitle, bool bFolder )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( false )
{
    (*m_xProps)[ u"Title"_ustr ]
        = PropertyValue( uno::Any( rTitle ), true );
    (*m_xProps)[ u"IsFolder"_ustr ]
        = PropertyValue( uno::Any( bFolder ), true );
    (*m_xProps)[ u"IsDocument"_ustr ]
        = PropertyValue( uno::Any( bool( !bFolder ) ), true );
}


ContentProperties::ContentProperties( const OUString & rTitle )
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( false )
{
    (*m_xProps)[ u"Title"_ustr ]
        = PropertyValue( uno::Any( rTitle ), true );
}


ContentProperties::ContentProperties()
: m_xProps( new PropertyValueMap ),
  m_bTrailingSlash( false )
{
}


ContentProperties::ContentProperties( const ContentProperties & rOther )
: m_aEscapedTitle( rOther.m_aEscapedTitle ),
  m_xProps( rOther.m_xProps
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
        it = std::find_if(m_xProps->cbegin(), end,
            [&rName](const PropertyValueMap::value_type& rEntry) {
                return rEntry.first.equalsIgnoreAsciiCase( rName );
            });
        if ( it != end )
            return &(*it).second;

        return nullptr;
    }
    else
        return &(*it).second;
}


// static
void ContentProperties::UCBNamesToDAVNames(
                            const uno::Sequence< beans::Property > & rProps,
                            std::vector< OUString > & propertyNames )
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
            propertyNames.push_back( rProp.Name );
        }
    }
}


// static
void ContentProperties::UCBNamesToHTTPNames(
                            const uno::Sequence< beans::Property > & rProps,
                            std::vector< OUString > & propertyNames )
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
            propertyNames.emplace_back("Last-Modified");
        }
        else if ( rProp.Name == "MediaType" )
        {
            propertyNames.emplace_back("Content-Type");
        }
        else if ( rProp.Name == "Size" )
        {
            propertyNames.emplace_back("Content-Length");
        }
        else
        {
            propertyNames.emplace_back(rProp.Name);
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

    return rNamesNotContained.empty();
}


void ContentProperties::addProperties(
                                const std::vector< OUString > & rProps,
                                const ContentProperties & rContentProps )
{
    for ( const OUString & rName : rProps )
    {
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

        (*m_xProps)[ u"DateCreated"_ustr ]
            = PropertyValue( uno::Any( aDate ), true );
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

        (*m_xProps)[ u"Size"_ustr ]
            = PropertyValue( uno::Any( aValue.toInt64() ), true );
    }
    else if ( rName.equalsIgnoreAsciiCase( "Content-Length" ) )
    {
        // Do NOT map Content-Length entity header to DAV:getcontentlength!
        // Only DAV resources have this property.

        // Map Content-Length entity header to UCP:Size
        OUString aValue;
        rValue >>= aValue;

        (*m_xProps)[ u"Size"_ustr ]
            = PropertyValue( uno::Any( aValue.toInt64() ), true );
    }
    else if ( rName == DAVProperties::GETCONTENTTYPE )
    {
        // Map DAV:getcontenttype to UCP:MediaType (1:1)
        (*m_xProps)[ u"MediaType"_ustr ]
            = PropertyValue( rValue, true );
    }
    else if ( rName.equalsIgnoreAsciiCase( "Content-Type" ) )
    {
        // Do NOT map Content-Type entity header to DAV:getcontenttype!
        // Only DAV resources have this property.

        // Map DAV:getcontenttype to UCP:MediaType (1:1)
        (*m_xProps)[ u"MediaType"_ustr ]
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

        (*m_xProps)[ u"DateModified"_ustr ]
            = PropertyValue( uno::Any( aDate ), true );
    }
    else if ( rName.equalsIgnoreAsciiCase( "Last-Modified" ) )
    {
        // Do not map Last-Modified entity header to DAV:getlastmodified!
        // Only DAV resources have this property.

        // Map the Last-Modified entity header to UCP:DateModified
        OUString aValue;
        rValue >>= aValue;
        util::DateTime aDate;
        DateTimeHelper::convert( aValue, aDate );

        (*m_xProps)[ u"DateModified"_ustr ]
            = PropertyValue( uno::Any( aDate ), true );
    }
    //  else if ( rName.equals( DAVProperties::LOCKDISCOVERY ) )
    //  {
    //  }
    else if ( rName == DAVProperties::RESOURCETYPE )
    {
        OUString aValue;
        rValue >>= aValue;

        // Map DAV:resourcetype to UCP:IsFolder, UCP:IsDocument, UCP:ContentType
        bool bFolder =
            aValue.equalsIgnoreAsciiCase( "collection" );

        (*m_xProps)[ u"IsFolder"_ustr ]
            = PropertyValue( uno::Any( bFolder ), true );
        (*m_xProps)[ u"IsDocument"_ustr ]
            = PropertyValue( uno::Any( bool( !bFolder ) ), true );
        (*m_xProps)[ u"ContentType"_ustr ]
            = PropertyValue( uno::Any( bFolder
                ? WEBDAV_COLLECTION_TYPE
                : WEBDAV_CONTENT_TYPE ), true );
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
    bool isCachable( std::u16string_view rName, bool isCaseSensitive )
    {
        const OUString aNonCachableProps [] =
        {
            DAVProperties::LOCKDISCOVERY,

            DAVProperties::GETETAG,
            u"ETag"_ustr,

            u"DateModified"_ustr,
            u"Last-Modified"_ustr,
            DAVProperties::GETLASTMODIFIED,

            u"Size"_ustr,
            u"Content-Length"_ustr,
            DAVProperties::GETCONTENTLENGTH,

            u"Date"_ustr
        };

        for ( sal_uInt32 n = 0;
              n <  ( sizeof( aNonCachableProps )
                     / sizeof( aNonCachableProps[ 0 ] ) );
              ++n )
        {
            if ( isCaseSensitive )
            {
                if ( rName == aNonCachableProps[ n ] )
                    return false;
            }
            else
                if ( o3tl::equalsIgnoreAsciiCase( rName, aNonCachableProps[ n ] ) )
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

    for ( const auto& rProp : *props )
    {
        if ( isCachable( rProp.first, rProp.second.isCaseSensitive() ) )
            m_aProps.addProperty( rProp.first,
                                  rProp.second.value(),
                                  rProp.second.isCaseSensitive() );
    }
}


void CachableContentProperties::addProperties(
    const std::vector< DAVPropertyValue > & rProps )
{
    for ( const auto& rProp : rProps )
    {
        if ( isCachable( rProp.Name, rProp.IsCaseSensitive ) )
            m_aProps.addProperty( rProp );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
