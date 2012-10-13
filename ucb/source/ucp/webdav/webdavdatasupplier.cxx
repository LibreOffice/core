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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include "webdavdatasupplier.hxx"
#include "webdavcontent.hxx"
#include "ContentProperties.hxx"
#ifndef _WEBDAV_SESSION_HXX
#include "DAVSession.hxx"
#endif
#include "SerfUri.hxx"

using namespace com::sun::star;
using namespace http_dav_ucp;

namespace http_dav_ucp
{

//=========================================================================
//
// struct ResultListEntry.
//
//=========================================================================

struct ResultListEntry
{
    rtl::OUString                             aId;
    uno::Reference< ucb::XContentIdentifier > xId;
    uno::Reference< ucb::XContent >           xContent;
    uno::Reference< sdbc::XRow >              xRow;
    const ContentProperties*                  pData;

    ResultListEntry( const ContentProperties* pEntry ) : pData( pEntry ) {};
    ~ResultListEntry() { delete pData; }
};

//=========================================================================
//
// ResultList.
//
//=========================================================================

typedef std::vector< ResultListEntry* > ResultList;

//=========================================================================
//
// struct DataSupplier_Impl.
//
//=========================================================================

struct DataSupplier_Impl
{
    osl::Mutex                                   m_aMutex;
    ResultList                                   m_aResults;
    rtl::Reference< Content >                    m_xContent;
    uno::Reference< lang::XMultiServiceFactory > m_xSMgr;
    sal_Int32                                    m_nOpenMode;
    sal_Bool                                     m_bCountFinal;
    sal_Bool                                     m_bThrowException;

    DataSupplier_Impl(
                const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
                const rtl::Reference< Content >& rContent,
                sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xSMgr( rxSMgr ), m_nOpenMode( nOpenMode ),
      m_bCountFinal( sal_False ), m_bThrowException( sal_False ) {}
    ~DataSupplier_Impl();
};

//=========================================================================
DataSupplier_Impl::~DataSupplier_Impl()
{
    ResultList::const_iterator it  = m_aResults.begin();
    ResultList::const_iterator end = m_aResults.end();

    while ( it != end )
    {
        delete (*it);
        ++it;
    }
}

}

//=========================================================================
//=========================================================================
//
// DataSupplier Implementation.
//
//=========================================================================
//=========================================================================

DataSupplier::DataSupplier(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            const rtl::Reference< Content >& rContent,
            sal_Int32 nOpenMode )
: m_pImpl( new DataSupplier_Impl( rxSMgr, rContent, nOpenMode ) )
{
}

//=========================================================================
// virtual
DataSupplier::~DataSupplier()
{
    delete m_pImpl;
}

//=========================================================================
// virtual
rtl::OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        rtl::OUString aId = m_pImpl->m_aResults[ nIndex ]->aId;
        if ( aId.getLength() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( nIndex ) )
    {
        rtl::OUString aId = m_pImpl->m_xContent->getResourceAccess().getURL();

        const ContentProperties& props
                            = *( m_pImpl->m_aResults[ nIndex ]->pData );

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += rtl::OUString::createFromAscii( "/" );

        aId += props.getEscapedTitle();

        if ( props.isTrailingSlash() )
            aId += rtl::OUString::createFromAscii( "/" );

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return rtl::OUString();
}

//=========================================================================
// virtual
uno::Reference< ucb::XContentIdentifier >
DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = m_pImpl->m_aResults[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    rtl::OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() )
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = new ::ucbhelper::ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return uno::Reference< ucb::XContentIdentifier >();
}

//=========================================================================
// virtual
uno::Reference< ucb::XContent >
DataSupplier::queryContent( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        uno::Reference< ucb::XContent > xContent
            = m_pImpl->m_aResults[ nIndex ]->xContent;
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
            m_pImpl->m_aResults[ nIndex ]->xContent = xContent;
            return xContent;

        }
        catch ( ucb::IllegalIdentifierException& )
        {
        }
    }
    return uno::Reference< ucb::XContent >();
}

//=========================================================================
// virtual
sal_Bool DataSupplier::getResult( sal_uInt32 nIndex )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_aResults.size() > nIndex )
    {
        // Result already present.
        return sal_True;
    }

    // Obtain values...
    if ( getData() )
    {
        if ( m_pImpl->m_aResults.size() > nIndex )
        {
            // Result already present.
            return sal_True;
        }
    }

    return sal_False;
}

//=========================================================================
// virtual
sal_uInt32 DataSupplier::totalCount()
{
  // Obtain values...
  getData();

  return m_pImpl->m_aResults.size();
}

//=========================================================================
// virtual
sal_uInt32 DataSupplier::currentCount()
{
    return m_pImpl->m_aResults.size();
}

//=========================================================================
// virtual
sal_Bool DataSupplier::isCountFinal()
{
    return m_pImpl->m_bCountFinal;
}

//=========================================================================
// virtual
uno::Reference< sdbc::XRow > DataSupplier::queryPropertyValues(
                                                    sal_uInt32 nIndex  )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        uno::Reference< sdbc::XRow > xRow = m_pImpl->m_aResults[ nIndex ]->xRow;
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
                m_pImpl->m_xSMgr,
                getResultSet()->getProperties(),
                *(m_pImpl->m_aResults[ nIndex ]->pData),
                rtl::Reference< ::ucbhelper::ContentProviderImplHelper >(
                    m_pImpl->m_xContent->getProvider().get() ),
                queryContentIdentifierString( nIndex ) );
        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}

//=========================================================================
// virtual
void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
        m_pImpl->m_aResults[ nIndex ]->xRow = uno::Reference< sdbc::XRow >();
}

//=========================================================================
// virtual
void DataSupplier::close()
{
}

//=========================================================================
// virtual
void DataSupplier::validate()
    throw( ucb::ResultSetException )
{
    if ( m_pImpl->m_bThrowException )
        throw ucb::ResultSetException();
}

//=========================================================================
sal_Bool DataSupplier::getData()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_bCountFinal )
    {
        std::vector< rtl::OUString > propertyNames;
        ContentProperties::UCBNamesToDAVNames(
                        getResultSet()->getProperties(), propertyNames );

        // Append "resourcetype", if not already present. It's value is
        // needed to get a valid ContentProperties::pIsFolder value, which
        // is needed for OpenMode handling.

        std::vector< rtl::OUString >::const_iterator it
            = propertyNames.begin();
        std::vector< rtl::OUString >::const_iterator end
            = propertyNames.end();

        while ( it != end )
        {
            if ( (*it).equals( DAVProperties::RESOURCETYPE ) )
                break;

            ++it;
        }

        if ( it == end )
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
        }
        catch ( DAVException & )
        {
//          OSL_ENSURE( sal_False, "PROPFIND : DAVException" );
            m_pImpl->m_bThrowException = sal_True;
        }

        if ( !m_pImpl->m_bThrowException )
        {
            try
            {
                SerfUri aURI(
                    m_pImpl->m_xContent->getResourceAccess().getURL() );
                rtl::OUString aPath = aURI.GetPath();

                if ( aPath.getStr()[ aPath.getLength() - 1 ]
                     == sal_Unicode( '/' ) )
                    aPath = aPath.copy( 0, aPath.getLength() - 1 );

                aPath = SerfUri::unescape( aPath );
                bool bFoundParent = false;

                for ( sal_uInt32 n = 0; n < resources.size(); ++n )
                {
                    const DAVResource & rRes = resources[ n ];

                    // Filter parent, which is contained somewhere(!) in
                    // the vector.
                    if ( !bFoundParent )
                    {
                        try
                        {
                            SerfUri aCurrURI( rRes.uri );
                            rtl::OUString aCurrPath = aCurrURI.GetPath();
                            if ( aCurrPath.getStr()[
                                     aCurrPath.getLength() - 1 ]
                                 == sal_Unicode( '/' ) )
                                aCurrPath
                                    = aCurrPath.copy(
                                        0,
                                        aCurrPath.getLength() - 1 );

                            aCurrPath = SerfUri::unescape( aCurrPath );
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

                    ContentProperties* pContentProperties
                        = new ContentProperties( rRes );

                    // Check resource against open mode.
                    switch ( m_pImpl->m_nOpenMode )
                    {
                    case ucb::OpenMode::FOLDERS:
                        {
                            sal_Bool bFolder = sal_False;

                            const uno::Any & rValue
                                = pContentProperties->getValue(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM(
                                            "IsFolder" ) ) );
                            rValue >>= bFolder;

                            if ( !bFolder )
                                continue;

                            break;
                        }

                    case ucb::OpenMode::DOCUMENTS:
                        {
                            sal_Bool bDocument = sal_False;

                            const uno::Any & rValue
                                = pContentProperties->getValue(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM(
                                            "IsDocument" ) ) );
                            rValue >>= bDocument;

                            if ( !bDocument )
                                continue;

                            break;
                        }

                    case ucb::OpenMode::ALL:
                    default:
                        break;
                    }

                    m_pImpl->m_aResults.push_back(
                        new ResultListEntry( pContentProperties ) );
                }
            }
            catch ( DAVException const & )
            {
            }
        }

        m_pImpl->m_bCountFinal = sal_True;

        // Callback possible, because listeners may be informed!
        aGuard.clear();
        getResultSet()->rowCountFinal();
    }
    return !m_pImpl->m_bThrowException;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
