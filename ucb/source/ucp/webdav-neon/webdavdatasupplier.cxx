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

#include <sal/log.hxx>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/ResultSetException.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <memory>
#include <vector>
#include "webdavdatasupplier.hxx"
#include "webdavcontent.hxx"
#include "ContentProperties.hxx"
#include "DAVSession.hxx"
#include "NeonUri.hxx"

using namespace com::sun::star;
using namespace webdav_ucp;

namespace webdav_ucp
{


// struct ResultListEntry.


struct ResultListEntry
{
    OUString                             aId;
    uno::Reference< ucb::XContentIdentifier > xId;
    uno::Reference< ucb::XContent >           xContent;
    uno::Reference< sdbc::XRow >              xRow;
    std::shared_ptr<ContentProperties> const pData;

    explicit ResultListEntry(std::shared_ptr<ContentProperties> const& pEntry)
        : pData(pEntry)
    {}
};


// ResultList.


typedef std::vector<std::unique_ptr<ResultListEntry>> ResultList;


// struct DataSupplier_Impl.


struct DataSupplier_Impl
{
    osl::Mutex                                   m_aMutex;
    ResultList                                   m_Results;
    rtl::Reference< Content >                    m_xContent;
    uno::Reference< uno::XComponentContext >     m_xContext;
    sal_Int32 const                              m_nOpenMode;
    bool                                     m_bCountFinal;
    bool                                     m_bThrowException;

    DataSupplier_Impl(
                const uno::Reference< uno::XComponentContext >& rxContext,
                const rtl::Reference< Content >& rContent,
                sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xContext( rxContext ), m_nOpenMode( nOpenMode ),
      m_bCountFinal( false ), m_bThrowException( false ) {}
};

}


// DataSupplier Implementation.


DataSupplier::DataSupplier(
            const uno::Reference< uno::XComponentContext >& rxContext,
            const rtl::Reference< Content >& rContent,
            sal_Int32 nOpenMode )
: m_pImpl( new DataSupplier_Impl( rxContext, rContent, nOpenMode ) )
{
}


// virtual
DataSupplier::~DataSupplier()
{
}


// virtual
OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if (nIndex < m_pImpl->m_Results.size())
    {
        OUString aId = m_pImpl->m_Results[ nIndex ]->aId;
        if ( !aId.isEmpty() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( nIndex ) )
    {
        OUString aId = m_pImpl->m_xContent->getResourceAccess().getURL();

        const ContentProperties& props(*(m_pImpl->m_Results[ nIndex ]->pData));

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += "/";

        aId += props.getEscapedTitle();

        if ( props.isTrailingSlash() )
            aId += "/";

        m_pImpl->m_Results[ nIndex ]->aId = aId;
        return aId;
    }
    return OUString();
}


// virtual
uno::Reference< ucb::XContentIdentifier >
DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if (nIndex < m_pImpl->m_Results.size())
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = m_pImpl->m_Results[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    OUString aId = queryContentIdentifierString( nIndex );
    if ( !aId.isEmpty() )
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = new ::ucbhelper::ContentIdentifier( aId );
        m_pImpl->m_Results[ nIndex ]->xId = xId;
        return xId;
    }
    return uno::Reference< ucb::XContentIdentifier >();
}


// virtual
uno::Reference< ucb::XContent >
DataSupplier::queryContent( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if (nIndex < m_pImpl->m_Results.size())
    {
        uno::Reference< ucb::XContent > xContent
            = m_pImpl->m_Results[ nIndex ]->xContent;
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
                = m_pImpl->m_xContent->getProvider()->queryContent( xId );
            m_pImpl->m_Results[ nIndex ]->xContent = xContent;
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
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if (nIndex < m_pImpl->m_Results.size())
    {
        // Result already present.
        return true;
    }

    // Obtain values...
    if ( getData() )
    {
        if (nIndex < m_pImpl->m_Results.size())
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

  return m_pImpl->m_Results.size();
}


// virtual
sal_uInt32 DataSupplier::currentCount()
{
    return m_pImpl->m_Results.size();
}


// virtual
bool DataSupplier::isCountFinal()
{
    return m_pImpl->m_bCountFinal;
}


// virtual
uno::Reference< sdbc::XRow > DataSupplier::queryPropertyValues(
                                                    sal_uInt32 nIndex  )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if (nIndex < m_pImpl->m_Results.size())
    {
        uno::Reference< sdbc::XRow > xRow = m_pImpl->m_Results[ nIndex ]->xRow;
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
                m_pImpl->m_xContext,
                getResultSet()->getProperties(),
                *(m_pImpl->m_Results[ nIndex ]->pData),
                rtl::Reference< ::ucbhelper::ContentProviderImplHelper >(
                    m_pImpl->m_xContent->getProvider().get() ),
                queryContentIdentifierString( nIndex ) );
        m_pImpl->m_Results[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}


// virtual
void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if (nIndex < m_pImpl->m_Results.size())
        m_pImpl->m_Results[ nIndex ]->xRow.clear();
}


// virtual
void DataSupplier::close()
{
}


// virtual
void DataSupplier::validate()
{
    if ( m_pImpl->m_bThrowException )
        throw ucb::ResultSetException();
}


bool DataSupplier::getData()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_bCountFinal )
    {
        std::vector< OUString > propertyNames;
        ContentProperties::UCBNamesToDAVNames(
                        getResultSet()->getProperties(), propertyNames );

        // Append "resourcetype", if not already present. It's value is
        // needed to get a valid ContentProperties::pIsFolder value, which
        // is needed for OpenMode handling.

        bool isNoResourceType = std::none_of(propertyNames.begin(), propertyNames.end(),
            [](const OUString& rName) { return rName == DAVProperties::RESOURCETYPE; });

        if ( isNoResourceType )
            propertyNames.push_back( DAVProperties::RESOURCETYPE );

        std::vector< DAVResource > resources;
        try
        {
            // propfind depth 1, get property values for parent AND for each
            // child
            m_pImpl->m_xContent->getResourceAccess()
                .PROPFIND( DAVONE,
                           propertyNames,
                           resources,
                           getResultSet()->getEnvironment() );
#if defined SAL_LOG_INFO
            {
                //print the resource for every URI returned
                for ( const auto& rResource : resources )
                {
                    NeonUri aCurrURI( rResource.uri );
                    OUString aCurrPath = aCurrURI.GetPath();
                    aCurrPath = NeonUri::unescape( aCurrPath );
                    SAL_INFO( "ucb.ucp.webdav", "getData() - resource URL: <" << rResource.uri << ">, unescaped to: <" << aCurrPath << "> )" );
                    for ( const auto& rProp : rResource.properties )
                    {
                        SAL_INFO( "ucb.ucp.webdav", "PROPFIND - property name: " << rProp.Name );
                    }
                }
            }
#endif
        }
        catch ( DAVException & )
        {
            SAL_WARN( "ucb.ucp.webdav", "Running PROPFIND: DAVException" );
            m_pImpl->m_bThrowException = true;
        }

        if ( !m_pImpl->m_bThrowException )
        {
            try
            {
                NeonUri aURI(
                    m_pImpl->m_xContent->getResourceAccess().getURL() );
                OUString aPath = aURI.GetPath();

                if ( aPath.endsWith("/") )
                    aPath = aPath.copy( 0, aPath.getLength() - 1 );

                aPath = NeonUri::unescape( aPath );
                bool bFoundParent = false;

                for (DAVResource & rRes : resources)
                {
                    // Filter parent, which is contained somewhere(!) in
                    // the vector.
                    if ( !bFoundParent )
                    {
                        try
                        {
                            NeonUri aCurrURI( rRes.uri );
                            OUString aCurrPath = aCurrURI.GetPath();
                            if ( aCurrPath.endsWith("/") )
                                aCurrPath
                                    = aCurrPath.copy(
                                        0,
                                        aCurrPath.getLength() - 1 );

                            aCurrPath = NeonUri::unescape( aCurrPath );
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

                    std::shared_ptr<ContentProperties> const
                        pContentProperties(new ContentProperties(rRes));

                    // Check resource against open mode.
                    switch ( m_pImpl->m_nOpenMode )
                    {
                    case ucb::OpenMode::FOLDERS:
                        {
                            bool bFolder = false;

                            const uno::Any & rValue
                                = pContentProperties->getValue(
                                    "IsFolder" );
                            rValue >>= bFolder;

                            if ( !bFolder )
                                continue;

                            break;
                        }

                    case ucb::OpenMode::DOCUMENTS:
                        {
                            bool bDocument = false;

                            const uno::Any & rValue
                                = pContentProperties->getValue(
                                    "IsDocument" );
                            rValue >>= bDocument;

                            if ( !bDocument )
                                continue;

                            break;
                        }

                    case ucb::OpenMode::ALL:
                    default:
                        break;
                    }

                    m_pImpl->m_Results.push_back(
                        std::make_unique<ResultListEntry>(pContentProperties));
                }
            }
            catch ( DAVException const & )
            {
            }
        }

        m_pImpl->m_bCountFinal = true;

        // Callback possible, because listeners may be informed!
        aGuard.clear();
        getResultSet()->rowCountFinal();
    }
    return !m_pImpl->m_bThrowException;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
