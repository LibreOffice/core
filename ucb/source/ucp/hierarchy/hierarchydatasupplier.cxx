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

#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include "hierarchydatasupplier.hxx"
#include "hierarchyprovider.hxx"
#include "hierarchycontent.hxx"

using namespace com::sun::star;
using namespace hierarchy_ucp;



HierarchyResultSetDataSupplier::HierarchyResultSetDataSupplier(
                const uno::Reference< uno::XComponentContext >& rxContext,
                const rtl::Reference< HierarchyContent >& rContent,
                sal_Int32 nOpenMode )
: m_xContent( rContent ), m_xContext( rxContext ),
  m_aFolder( rxContext,
             static_cast< HierarchyContentProvider * >(
                 rContent->getProvider().get() ),
             rContent->getIdentifier()->getContentIdentifier() ),
  m_nOpenMode( nOpenMode ), m_bCountFinal( false )
{
}


// virtual
HierarchyResultSetDataSupplier::~HierarchyResultSetDataSupplier()
{
}


// virtual
OUString HierarchyResultSetDataSupplier::queryContentIdentifierString(
                                                        sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );
    return queryContentIdentifierStringImpl(aGuard, nIndex);
}

OUString HierarchyResultSetDataSupplier::queryContentIdentifierStringImpl(
                                            std::unique_lock<std::mutex>& rGuard,
                                            sal_uInt32 nIndex )
{
    if ( nIndex < m_aResults.size() )
    {
        OUString aId = m_aResults[ nIndex ]->aId;
        if ( !aId.isEmpty() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResultImpl( rGuard, nIndex ) )
    {
        OUString aId
            = m_xContent->getIdentifier()->getContentIdentifier();

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += "/";

        aId += m_aResults[ nIndex ]->aData.getName();

        m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return OUString();
}


// virtual
uno::Reference< ucb::XContentIdentifier >
HierarchyResultSetDataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );

    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< ucb::XContentIdentifier > xId
                                = m_aResults[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    OUString aId = queryContentIdentifierStringImpl( aGuard, nIndex );
    if ( !aId.isEmpty() )
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = new ::ucbhelper::ContentIdentifier( aId );
        m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return uno::Reference< ucb::XContentIdentifier >();
}


// virtual
uno::Reference< ucb::XContent >
HierarchyResultSetDataSupplier::queryContent( sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );

    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< ucb::XContent > xContent
                                = m_aResults[ nIndex ]->xContent;
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
            m_aResults[ nIndex ]->xContent = xContent;
            return xContent;

        }
        catch ( ucb::IllegalIdentifierException const & )
        {
        }
    }
    return uno::Reference< ucb::XContent >();
}


// virtual
bool HierarchyResultSetDataSupplier::getResult( sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );
    return getResultImpl(aGuard, nIndex);
}

bool HierarchyResultSetDataSupplier::getResultImpl( std::unique_lock<std::mutex>& rGuard, sal_uInt32 nIndex )
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

    while ( m_aFolder.next( m_aIterator ) )
    {
        const HierarchyEntryData& rResult = *m_aIterator;
        if ( checkResult( rResult ) )
        {
            m_aResults.emplace_back( new ResultListEntry( rResult ) );

            if ( nPos == nIndex )
            {
                // Result obtained.
                bFound = true;
                break;
            }
        }
        nPos++;
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
sal_uInt32 HierarchyResultSetDataSupplier::totalCount()
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_bCountFinal )
        return m_aResults.size();

    sal_uInt32 nOldCount = m_aResults.size();

    while ( m_aFolder.next( m_aIterator ) )
    {
        const HierarchyEntryData& rResult = *m_aIterator;
        if ( checkResult( rResult ) )
            m_aResults.emplace_back( new ResultListEntry( rResult ) );
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
sal_uInt32 HierarchyResultSetDataSupplier::currentCount()
{
    return m_aResults.size();
}


// virtual
bool HierarchyResultSetDataSupplier::isCountFinal()
{
    return m_bCountFinal;
}


// virtual
uno::Reference< sdbc::XRow >
HierarchyResultSetDataSupplier::queryPropertyValues( sal_uInt32 nIndex  )
{
    std::unique_lock aGuard( m_aMutex );

    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< sdbc::XRow > xRow
            = m_aResults[ nIndex ]->xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( getResultImpl( aGuard, nIndex ) )
    {
        HierarchyContentProperties aData(
            m_aResults[ nIndex ]->aData );

        uno::Reference< sdbc::XRow > xRow
            = HierarchyContent::getPropertyValues(
                m_xContext,
                getResultSet()->getProperties(),
                aData,
                static_cast< HierarchyContentProvider * >(
                    m_xContent->getProvider().get() ),
                queryContentIdentifierStringImpl( aGuard, nIndex ) );
        m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}


// virtual
void HierarchyResultSetDataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    std::unique_lock aGuard( m_aMutex );

    if ( nIndex < m_aResults.size() )
        m_aResults[ nIndex ]->xRow.clear();
}


// virtual
void HierarchyResultSetDataSupplier::close()
{
}


// virtual
void HierarchyResultSetDataSupplier::validate()
{
}


bool HierarchyResultSetDataSupplier::checkResult(
                                    const HierarchyEntryData& rResult )
{
    switch ( m_nOpenMode )
    {
        case ucb::OpenMode::FOLDERS:
            if ( rResult.getType() == HierarchyEntryData::LINK )
            {
                // Entry is a link.
                return false;
            }
            break;

        case ucb::OpenMode::DOCUMENTS:
            if ( rResult.getType() == HierarchyEntryData::FOLDER )
            {
                // Entry is a folder.
                return false;
            }
            break;

        case ucb::OpenMode::ALL:
        default:
            break;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
