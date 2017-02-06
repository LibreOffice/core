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

#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include "hierarchydatasupplier.hxx"
#include "hierarchyprovider.hxx"
#include "hierarchycontent.hxx"

using namespace com::sun::star;
using namespace hierarchy_ucp;

namespace hierarchy_ucp
{


// struct ResultListEntry.


struct ResultListEntry
{
    OUString                             aId;
    uno::Reference< ucb::XContentIdentifier > xId;
    uno::Reference< ucb::XContent >           xContent;
    uno::Reference< sdbc::XRow >              xRow;
    HierarchyEntryData                        aData;

    explicit ResultListEntry( const HierarchyEntryData& rEntry ) : aData( rEntry ) {}
};


// ResultList.


typedef std::vector< ResultListEntry* > ResultList;


// struct DataSupplier_Impl.


struct DataSupplier_Impl
{
    osl::Mutex                                      m_aMutex;
    ResultList                                      m_aResults;
    rtl::Reference< HierarchyContent >              m_xContent;
    uno::Reference< uno::XComponentContext >        m_xContext;
    HierarchyEntry                                  m_aFolder;
    HierarchyEntry::iterator                        m_aIterator;
    sal_Int32                                       m_nOpenMode;
    bool                                        m_bCountFinal;

    DataSupplier_Impl(
        const uno::Reference< uno::XComponentContext >& rxContext,
        const rtl::Reference< HierarchyContent >& rContent,
        sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xContext( rxContext ),
      m_aFolder( rxContext,
                 static_cast< HierarchyContentProvider * >(
                     rContent->getProvider().get() ),
                 rContent->getIdentifier()->getContentIdentifier() ),
      m_nOpenMode( nOpenMode ), m_bCountFinal( false ) {}
    ~DataSupplier_Impl();
};


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


// HierarchyResultSetDataSupplier Implementation.


HierarchyResultSetDataSupplier::HierarchyResultSetDataSupplier(
                const uno::Reference< uno::XComponentContext >& rxContext,
                const rtl::Reference< HierarchyContent >& rContent,
                sal_Int32 nOpenMode )
: m_pImpl( new DataSupplier_Impl( rxContext, rContent, nOpenMode ) )
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
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        OUString aId = m_pImpl->m_aResults[ nIndex ]->aId;
        if ( !aId.isEmpty() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( nIndex ) )
    {
        OUString aId
            = m_pImpl->m_xContent->getIdentifier()->getContentIdentifier();

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += "/";

        aId += m_pImpl->m_aResults[ nIndex ]->aData.getName();

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return OUString();
}


// virtual
uno::Reference< ucb::XContentIdentifier >
HierarchyResultSetDataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
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

    OUString aId = queryContentIdentifierString( nIndex );
    if ( !aId.isEmpty() )
    {
        uno::Reference< ucb::XContentIdentifier > xId
            = new ::ucbhelper::ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return uno::Reference< ucb::XContentIdentifier >();
}


// virtual
uno::Reference< ucb::XContent >
HierarchyResultSetDataSupplier::queryContent( sal_uInt32 nIndex )
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


// virtual
bool HierarchyResultSetDataSupplier::getResult( sal_uInt32 nIndex )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_aResults.size() > nIndex )
    {
        // Result already present.
        return true;
    }

    // Result not (yet) present.

    if ( m_pImpl->m_bCountFinal )
        return false;

    // Try to obtain result...

    sal_uInt32 nOldCount = m_pImpl->m_aResults.size();
    bool bFound = false;
    sal_uInt32 nPos = nOldCount;

    while ( m_pImpl->m_aFolder.next( m_pImpl->m_aIterator ) )
    {
        const HierarchyEntryData& rResult = *m_pImpl->m_aIterator;
        if ( checkResult( rResult ) )
        {
            m_pImpl->m_aResults.push_back( new ResultListEntry( rResult ) );

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
        m_pImpl->m_bCountFinal = true;

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


// virtual
sal_uInt32 HierarchyResultSetDataSupplier::totalCount()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bCountFinal )
        return m_pImpl->m_aResults.size();

    sal_uInt32 nOldCount = m_pImpl->m_aResults.size();

    while ( m_pImpl->m_aFolder.next( m_pImpl->m_aIterator ) )
    {
        const HierarchyEntryData& rResult = *m_pImpl->m_aIterator;
        if ( checkResult( rResult ) )
            m_pImpl->m_aResults.push_back( new ResultListEntry( rResult ) );
    }

    m_pImpl->m_bCountFinal = true;

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


// virtual
sal_uInt32 HierarchyResultSetDataSupplier::currentCount()
{
    return m_pImpl->m_aResults.size();
}


// virtual
bool HierarchyResultSetDataSupplier::isCountFinal()
{
    return m_pImpl->m_bCountFinal;
}


// virtual
uno::Reference< sdbc::XRow >
HierarchyResultSetDataSupplier::queryPropertyValues( sal_uInt32 nIndex  )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        uno::Reference< sdbc::XRow > xRow
            = m_pImpl->m_aResults[ nIndex ]->xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( getResult( nIndex ) )
    {
        HierarchyContentProperties aData(
            m_pImpl->m_aResults[ nIndex ]->aData );

        uno::Reference< sdbc::XRow > xRow
            = HierarchyContent::getPropertyValues(
                m_pImpl->m_xContext,
                getResultSet()->getProperties(),
                aData,
                static_cast< HierarchyContentProvider * >(
                    m_pImpl->m_xContent->getProvider().get() ),
                queryContentIdentifierString( nIndex ) );
        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}


// virtual
void HierarchyResultSetDataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
        m_pImpl->m_aResults[ nIndex ]->xRow.clear();
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
    switch ( m_pImpl->m_nOpenMode )
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
