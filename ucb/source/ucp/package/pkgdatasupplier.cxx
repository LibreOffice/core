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

#include <vector>
#include <osl/diagnose.h>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include "pkgdatasupplier.hxx"
#include "pkgcontent.hxx"
#include "pkgprovider.hxx"

#include "../inc/urihelper.hxx"

using namespace com::sun::star;
using namespace package_ucp;

namespace package_ucp
{

//=========================================================================
//
// struct ResultListEntry.
//
//=========================================================================

struct ResultListEntry
{
    rtl::OUString                             aURL;
    uno::Reference< ucb::XContentIdentifier > xId;
    uno::Reference< ucb::XContent >           xContent;
    uno::Reference< sdbc::XRow >              xRow;

    ResultListEntry( const rtl::OUString& rURL ) : aURL( rURL ) {}
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
    uno::Reference< container::XEnumeration >    m_xFolderEnum;
    sal_Int32                                    m_nOpenMode;
    sal_Bool                                     m_bCountFinal;
    sal_Bool                                     m_bThrowException;

    DataSupplier_Impl(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            const rtl::Reference< Content >& rContent,
            sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xSMgr( rxSMgr ),
      m_xFolderEnum( rContent->getIterator() ), m_nOpenMode( nOpenMode ),
      m_bCountFinal( !m_xFolderEnum.is() ), m_bThrowException( m_bCountFinal )
    {}
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
        rtl::OUString aId = m_pImpl->m_aResults[ nIndex ]->aURL;
        if ( !aId.isEmpty() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( nIndex ) )
    {
        // Note: getResult fills m_pImpl->m_aResults[ nIndex ]->aURL.
        return m_pImpl->m_aResults[ nIndex ]->aURL;
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
    if ( !aId.isEmpty() )
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
uno::Reference< ucb::XContent > DataSupplier::queryContent(
                                                        sal_uInt32 nIndex )
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
        catch ( ucb::IllegalIdentifierException const & )
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

    // Result not (yet) present.

    if ( m_pImpl->m_bCountFinal )
        return sal_False;

    // Try to obtain result...

    sal_uInt32 nOldCount = m_pImpl->m_aResults.size();
    sal_Bool bFound = sal_False;
    sal_uInt32 nPos = nOldCount;

    while ( m_pImpl->m_xFolderEnum->hasMoreElements() )
    {
        try
        {
            uno::Reference< container::XNamed > xNamed;
            m_pImpl->m_xFolderEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                OSL_FAIL( "DataSupplier::getResult - Got no XNamed!" );
                break;
            }

            rtl::OUString aName = xNamed->getName();

            if ( aName.isEmpty() )
            {
                OSL_FAIL( "DataSupplier::getResult - Empty name!" );
                break;
            }

            // Assemble URL for child.
            rtl::OUString aURL = assembleChildURL( aName );

            m_pImpl->m_aResults.push_back( new ResultListEntry( aURL ) );

            if ( nPos == nIndex )
            {
                // Result obtained.
                bFound = sal_True;
                break;
            }

            nPos++;
        }
        catch ( container::NoSuchElementException const & )
        {
            m_pImpl->m_bThrowException = sal_True;
            break;
        }
        catch ( lang::WrappedTargetException const & )
        {
            m_pImpl->m_bThrowException = sal_True;
            break;
        }
    }

    if ( !bFound )
        m_pImpl->m_bCountFinal = sal_True;

    rtl::Reference< ::ucbhelper::ResultSet > xResultSet = getResultSet().get();
    if ( xResultSet.is() )
    {
        // Callbacks follow!
        aGuard.clear();

        if ( nOldCount < m_pImpl->m_aResults.size() )
            xResultSet->rowCountChanged(
                                    nOldCount, m_pImpl->m_aResults.size() );

        if ( m_pImpl->m_bCountFinal )
            xResultSet->rowCountFinal();
    }

    return bFound;
}

//=========================================================================
// virtual
sal_uInt32 DataSupplier::totalCount()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bCountFinal )
        return m_pImpl->m_aResults.size();

    sal_uInt32 nOldCount = m_pImpl->m_aResults.size();

    while ( m_pImpl->m_xFolderEnum->hasMoreElements() )
    {
        try
        {
            uno::Reference< container::XNamed > xNamed;
            m_pImpl->m_xFolderEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                OSL_FAIL( "DataSupplier::getResult - Got no XNamed!" );
                break;
            }

            rtl::OUString aName = xNamed->getName();

            if ( aName.isEmpty() )
            {
                OSL_FAIL( "DataSupplier::getResult - Empty name!" );
                break;
            }

            // Assemble URL for child.
            rtl::OUString aURL = assembleChildURL( aName );

            m_pImpl->m_aResults.push_back( new ResultListEntry( aURL ) );
        }
        catch ( container::NoSuchElementException const & )
        {
            m_pImpl->m_bThrowException = sal_True;
            break;
        }
        catch ( lang::WrappedTargetException const & )
        {
            m_pImpl->m_bThrowException = sal_True;
            break;
        }
    }

    m_pImpl->m_bCountFinal = sal_True;

    rtl::Reference< ::ucbhelper::ResultSet > xResultSet = getResultSet().get();
    if ( xResultSet.is() )
    {
        // Callbacks follow!
        aGuard.clear();

        if ( nOldCount < m_pImpl->m_aResults.size() )
            xResultSet->rowCountChanged(
                                    nOldCount, m_pImpl->m_aResults.size() );

        xResultSet->rowCountFinal();
    }

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
        uno::Reference< sdbc::XRow > xRow = Content::getPropertyValues(
                        m_pImpl->m_xSMgr,
                        getResultSet()->getProperties(),
                        static_cast< ContentProvider * >(
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
::rtl::OUString DataSupplier::assembleChildURL( const ::rtl::OUString& aName )
{
    rtl::OUString aURL;
    rtl::OUString aContURL
        = m_pImpl->m_xContent->getIdentifier()->getContentIdentifier();
    sal_Int32 nParam = aContURL.indexOf( '?' );
    if ( nParam >= 0 )
    {
        aURL = aContURL.copy( 0, nParam );

        sal_Int32 nPackageUrlEnd = aURL.lastIndexOf( '/' );
        if ( nPackageUrlEnd != aURL.getLength() - 1 )
            aURL += rtl::OUString("/");

        aURL += ::ucb_impl::urihelper::encodeSegment( aName );
        aURL += aContURL.copy( nParam );
    }
    else
    {
        aURL = aContURL;

        sal_Int32 nPackageUrlEnd = aURL.lastIndexOf( '/' );
        if ( nPackageUrlEnd != aURL.getLength() - 1 )
            aURL += rtl::OUString("/");

        aURL += ::ucb_impl::urihelper::encodeSegment( aName );
    }
    return aURL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
