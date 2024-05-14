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

#include <sal/config.h>
#include <sal/log.hxx>

#include <utility>

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include "webdavdatasupplier.hxx"
#include "webdavcontent.hxx"
#include "DAVProperties.hxx"
#include "CurlUri.hxx"
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/ResultSetException.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace com::sun::star;
using namespace http_dav_ucp;

namespace http_dav_ucp
{

namespace {

auto DumpResources(std::vector<DAVResource> const& rResources) -> OUString
{
    OUStringBuffer buf;
    for (auto const& rResource : rResources)
    {
        buf.append("resource URL: <" + rResource.uri);
        try {
            CurlUri const uri(rResource.uri);
            buf.append("> parsed URL: <"
                + DecodeURI(uri.GetRelativeReference())
                + "> ");
        } catch (...) {
            // parsing uri could fail
            buf.append("> parsing URL failed! ");
        }
        buf.append("properties: ");
        for (auto const& it : rResource.properties)
        {
            buf.append("\"" + it.Name + "\" ");
        }
        buf.append("\n");
    }
    buf.stripEnd('\n'); // the last newline is superfluous, remove it
    return buf.makeStringAndClear();
}

}

}


// DataSupplier Implementation.


DataSupplier::DataSupplier(
            const uno::Reference< uno::XComponentContext >& rxContext,
            const rtl::Reference< Content >& rContent,
            sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xContext( rxContext ), m_nOpenMode( nOpenMode ),
      m_bCountFinal( false ), m_bThrowException( false )
{
}


// virtual
DataSupplier::~DataSupplier()
{}


// virtual
OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if (nIndex < m_Results.size())
    {
        OUString aId = m_Results[ nIndex ]->aId;
        if ( aId.getLength() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( nIndex ) )
    {
        OUString aId = m_xContent->getResourceAccess().getURL();

        const ContentProperties& props(*(m_Results[ nIndex ]->pData));

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += "/";

        aId += props.getEscapedTitle();

        if ( props.isTrailingSlash() )
            aId += "/";

        m_Results[ nIndex ]->aId = aId;
        return aId;
    }
    return OUString();
}


// virtual
uno::Reference< ucb::XContentIdentifier >
DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if (nIndex < m_Results.size())
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = m_Results[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() )
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = new ::ucbhelper::ContentIdentifier( aId );
        m_Results[ nIndex ]->xId = xId;
        return xId;
    }
    return uno::Reference< ucb::XContentIdentifier >();
}


// virtual
uno::Reference< ucb::XContent >
DataSupplier::queryContent( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if (nIndex < m_Results.size())
    {
        uno::Reference< ucb::XContent > xContent
            = m_Results[ nIndex ]->xContent;
        if ( xContent.is() )
        {
            // Already cached.
            return xContent;
        }
    }

    uno::Reference< ucb::XContentIdentifier > xId
        = queryContentIdentifier( nIndex );
    if ( xId.is() )
    {
        try
        {
            uno::Reference< ucb::XContent > xContent
                = m_xContent->getProvider()->queryContent( xId );
            m_Results[ nIndex ]->xContent = xContent;
            return xContent;

        }
        catch ( ucb::IllegalIdentifierException& )
        {
        }
    }
    return uno::Reference< ucb::XContent >();
}


// virtual
bool DataSupplier::getResult( sal_uInt32 nIndex )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    if (nIndex < m_Results.size())
    {
        // Result already present.
        return true;
    }

    // Obtain values...
    if ( getData() )
    {
        if (nIndex < m_Results.size())
        {
            // Result already present.
            return true;
        }
    }

    return false;
}


// virtual
sal_uInt32 DataSupplier::totalCount()
{
  // Obtain values...
  getData();

  return m_Results.size();
}


// virtual
sal_uInt32 DataSupplier::currentCount()
{
    return m_Results.size();
}


// virtual
bool DataSupplier::isCountFinal()
{
    return m_bCountFinal;
}


// virtual
uno::Reference< sdbc::XRow > DataSupplier::queryPropertyValues(
                                                    sal_uInt32 nIndex  )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if (nIndex < m_Results.size())
    {
        uno::Reference< sdbc::XRow > xRow = m_Results[ nIndex ]->xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( getResult( nIndex ) )
    {
        uno::Reference< sdbc::XRow > xRow
            = Content::getPropertyValues(
                m_xContext,
                getResultSet()->getProperties(),
                *(m_Results[ nIndex ]->pData),
                m_xContent->getProvider(),
                queryContentIdentifierString( nIndex ) );
        m_Results[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}


// virtual
void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if (nIndex < m_Results.size())
        m_Results[ nIndex ]->xRow.clear();
}


// virtual
void DataSupplier::close()
{
}


// virtual
void DataSupplier::validate()
{
    if ( m_bThrowException )
        throw ucb::ResultSetException();
}

bool DataSupplier::getData()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_bCountFinal )
    {
        std::vector< OUString > propertyNames;
        ContentProperties::UCBNamesToDAVNames(
                        getResultSet()->getProperties(), propertyNames );

        // Append "resourcetype", if not already present. It's value is
        // needed to get a valid ContentProperties::pIsFolder value, which
        // is needed for OpenMode handling.

        bool isNoResourceType = std::none_of(propertyNames.begin(), propertyNames.end(),
            [](const OUString& rPropName) { return rPropName.equals(DAVProperties::RESOURCETYPE); });

        if ( isNoResourceType )
            propertyNames.push_back( DAVProperties::RESOURCETYPE );

        std::vector< DAVResource > resources;
        try
        {
            // propfind depth 1, get property values for parent AND for each
            // child
            m_xContent->getResourceAccess()
                .PROPFIND( DAVONE,
                           propertyNames,
                           resources,
                           getResultSet()->getEnvironment() );
            SAL_INFO("ucb.ucp.webdav", "getData() - " << DumpResources(resources));
        }
        catch ( DAVException & )
        {
            TOOLS_WARN_EXCEPTION( "ucb.ucp.webdav", "PROPFIND : DAVException" );
            m_bThrowException = true;
        }

        if ( !m_bThrowException )
        {
            try
            {
                CurlUri const aURI(
                    m_xContent->getResourceAccess().getURL() );
                OUString aPath = aURI.GetPath();

                if ( aPath.endsWith("/") )
                    aPath = aPath.copy( 0, aPath.getLength() - 1 );

                aPath = DecodeURI(aPath);
                bool bFoundParent = false;

                for ( size_t n = 0; n < resources.size(); ++n )
                {
                    const DAVResource & rRes = resources[ n ];

                    // Filter parent, which is contained somewhere(!) in
                    // the vector.
                    if ( !bFoundParent )
                    {
                        try
                        {
                            CurlUri const aCurrURI( rRes.uri );
                            OUString aCurrPath = aCurrURI.GetPath();
                            if ( aCurrPath.endsWith("/") )
                                aCurrPath
                                    = aCurrPath.copy(
                                        0,
                                        aCurrPath.getLength() - 1 );

                            aCurrPath = DecodeURI(aCurrPath);
                            if ( aPath == aCurrPath )
                            {
                                bFoundParent = true;
                                continue;
                            }
                        }
                        catch ( DAVException const & )
                        {
                            // do nothing, ignore error. continue.
                        }
                    }

                    std::unique_ptr<ContentProperties> pContentProperties
                        = std::make_unique<ContentProperties>( rRes );

                    // Check resource against open mode.
                    switch ( m_nOpenMode )
                    {
                    case ucb::OpenMode::FOLDERS:
                        {
                            bool bFolder = false;

                            const uno::Any & rValue
                                = pContentProperties->getValue( u"IsFolder"_ustr );
                            rValue >>= bFolder;

                            if ( !bFolder )
                                continue;

                            break;
                        }

                    case ucb::OpenMode::DOCUMENTS:
                        {
                            bool bDocument = false;

                            const uno::Any & rValue
                                = pContentProperties->getValue( u"IsDocument"_ustr );
                            rValue >>= bDocument;

                            if ( !bDocument )
                                continue;

                            break;
                        }

                    case ucb::OpenMode::ALL:
                    default:
                        break;
                    }

                    m_Results.push_back(
                        std::make_unique<ResultListEntry>(std::move(pContentProperties)));
                }
            }
            catch ( DAVException const & )
            {
            }
        }

        m_bCountFinal = true;

        // Callback possible, because listeners may be informed!
        aGuard.clear();
        getResultSet()->rowCountFinal();
    }
    return !m_bThrowException;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
