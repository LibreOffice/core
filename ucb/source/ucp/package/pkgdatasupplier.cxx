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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <osl/diagnose.h>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/ResultSetException.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <utility>
#include "pkgdatasupplier.hxx"
#include "pkgcontent.hxx"
#include "pkgprovider.hxx"

#include "../inc/urihelper.hxx"

using namespace com::sun::star;
using namespace package_ucp;

// DataSupplier Implementation.


DataSupplier::DataSupplier(
                uno::Reference< uno::XComponentContext > xContext,
                const rtl::Reference< Content >& rContent )
: m_xContent( rContent ), m_xContext(std::move( xContext )),
  m_xFolderEnum( rContent->getIterator() ),
  m_bCountFinal( !m_xFolderEnum.is() ), m_bThrowException( m_bCountFinal )
{
}


// virtual
DataSupplier::~DataSupplier()
{
}


// virtual
OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );
    return queryContentIdentifierStringImpl(aGuard, nIndex);
}

OUString DataSupplier::queryContentIdentifierStringImpl( std::unique_lock<std::mutex>& rGuard, sal_uInt32 nIndex )
{
    if ( nIndex < m_aResults.size() )
    {
        OUString aId = m_aResults[ nIndex ].aURL;
        if ( !aId.isEmpty() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResultImpl( rGuard, nIndex ) )
    {
        // Note: getResult fills m_aResults[ nIndex ].aURL.
        return m_aResults[ nIndex ].aURL;
    }
    return OUString();
}


// virtual
uno::Reference< ucb::XContentIdentifier >
DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );
    return queryContentIdentifierImpl(aGuard, nIndex);
}

uno::Reference< ucb::XContentIdentifier >
DataSupplier::queryContentIdentifierImpl( std::unique_lock<std::mutex>& rGuard, sal_uInt32 nIndex )
{
    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< ucb::XContentIdentifier >& xId
                                = m_aResults[ nIndex ].xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    OUString aId = queryContentIdentifierStringImpl( rGuard, nIndex );
    if ( !aId.isEmpty() )
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = new ::ucbhelper::ContentIdentifier( aId );
        m_aResults[ nIndex ].xId = xId;
        return xId;
    }
    return uno::Reference< ucb::XContentIdentifier >();
}


// virtual
uno::Reference< ucb::XContent > DataSupplier::queryContent(
                                                        sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );

    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< ucb::XContent >& xContent
                                = m_aResults[ nIndex ].xContent;
        if ( xContent.is() )
        {
            // Already cached.
            return xContent;
        }
    }

    uno::Reference< ucb::XContentIdentifier > xId
        = queryContentIdentifierImpl( aGuard, nIndex );
    if ( xId.is() )
    {
        try
        {
            uno::Reference< ucb::XContent > xContent
                = m_xContent->getProvider()->queryContent( xId );
            m_aResults[ nIndex ].xContent = xContent;
            return xContent;

        }
        catch ( ucb::IllegalIdentifierException const & )
        {
        }
    }
    return uno::Reference< ucb::XContent >();
}


// virtual
bool DataSupplier::getResult( sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );
    return getResultImpl(aGuard, nIndex);
}

bool DataSupplier::getResultImpl( std::unique_lock<std::mutex>& rGuard, sal_uInt32 nIndex )
{
    if ( m_aResults.size() > nIndex )
    {
        // Result already present.
        return true;
    }

    // Result not (yet) present.

    if ( m_bCountFinal )
        return false;

    // Try to obtain result...

    sal_uInt32 nOldCount = m_aResults.size();
    bool bFound = false;
    sal_uInt32 nPos = nOldCount;

    while ( m_xFolderEnum->hasMoreElements() )
    {
        try
        {
            uno::Reference< container::XNamed > xNamed;
            m_xFolderEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                OSL_FAIL( "DataSupplier::getResult - Got no XNamed!" );
                break;
            }

            OUString aName = xNamed->getName();

            if ( aName.isEmpty() )
            {
                OSL_FAIL( "DataSupplier::getResult - Empty name!" );
                break;
            }

            // Assemble URL for child.
            OUString aURL = assembleChildURL( aName );

            m_aResults.push_back( ResultListEntry( aURL ) );

            if ( nPos == nIndex )
            {
                // Result obtained.
                bFound = true;
                break;
            }

            nPos++;
        }
        catch ( container::NoSuchElementException const & )
        {
            m_bThrowException = true;
            break;
        }
        catch ( lang::WrappedTargetException const & )
        {
            m_bThrowException = true;
            break;
        }
    }

    if ( !bFound )
        m_bCountFinal = true;

    rtl::Reference< ::ucbhelper::ResultSet > xResultSet = getResultSet();
    if ( xResultSet.is() )
    {
        // Callbacks follow!
        rGuard.unlock();

        if ( nOldCount < m_aResults.size() )
            xResultSet->rowCountChanged(
                                    nOldCount, m_aResults.size() );

        if ( m_bCountFinal )
            xResultSet->rowCountFinal();

        rGuard.lock();
    }

    return bFound;
}


// virtual
sal_uInt32 DataSupplier::totalCount()
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_bCountFinal )
        return m_aResults.size();

    sal_uInt32 nOldCount = m_aResults.size();

    while ( m_xFolderEnum->hasMoreElements() )
    {
        try
        {
            uno::Reference< container::XNamed > xNamed;
            m_xFolderEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                OSL_FAIL( "DataSupplier::getResult - Got no XNamed!" );
                break;
            }

            OUString aName = xNamed->getName();

            if ( aName.isEmpty() )
            {
                OSL_FAIL( "DataSupplier::getResult - Empty name!" );
                break;
            }

            // Assemble URL for child.
            OUString aURL = assembleChildURL( aName );

            m_aResults.push_back( ResultListEntry( aURL ) );
        }
        catch ( container::NoSuchElementException const & )
        {
            m_bThrowException = true;
            break;
        }
        catch ( lang::WrappedTargetException const & )
        {
            m_bThrowException = true;
            break;
        }
    }

    m_bCountFinal = true;

    rtl::Reference< ::ucbhelper::ResultSet > xResultSet = getResultSet();
    if ( xResultSet.is() )
    {
        // Callbacks follow!
        aGuard.unlock();

        if ( nOldCount < m_aResults.size() )
            xResultSet->rowCountChanged(
                                    nOldCount, m_aResults.size() );

        xResultSet->rowCountFinal();
    }

    return m_aResults.size();
}


// virtual
sal_uInt32 DataSupplier::currentCount()
{
    return m_aResults.size();
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
    std::unique_lock aGuard( m_aMutex );

    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< sdbc::XRow >& xRow = m_aResults[ nIndex ].xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( getResultImpl( aGuard, nIndex ) )
    {
        uno::Reference< sdbc::XRow > xRow = Content::getPropertyValues(
                        m_xContext,
                        getResultSet()->getProperties(),
                        static_cast< ContentProvider * >(
                            m_xContent->getProvider().get() ),
                        queryContentIdentifierStringImpl( aGuard, nIndex ) );
        m_aResults[ nIndex ].xRow = xRow;
        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}


// virtual
void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );

    if ( nIndex < m_aResults.size() )
        m_aResults[ nIndex ].xRow.clear();
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


OUString DataSupplier::assembleChildURL( const OUString& aName )
{
    OUString aURL;
    OUString aContURL
        = m_xContent->getIdentifier()->getContentIdentifier();
    sal_Int32 nParam = aContURL.indexOf( '?' );
    if ( nParam >= 0 )
    {
        aURL = aContURL.copy( 0, nParam );

        sal_Int32 nPackageUrlEnd = aURL.lastIndexOf( '/' );
        if ( nPackageUrlEnd != aURL.getLength() - 1 )
            aURL += "/";

        aURL += ::ucb_impl::urihelper::encodeSegment( aName ) +
            aContURL.subView( nParam );
    }
    else
    {
        aURL = aContURL;

        sal_Int32 nPackageUrlEnd = aURL.lastIndexOf( '/' );
        if ( nPackageUrlEnd != aURL.getLength() - 1 )
            aURL += "/";

        aURL += ::ucb_impl::urihelper::encodeSegment( aName );
    }
    return aURL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
