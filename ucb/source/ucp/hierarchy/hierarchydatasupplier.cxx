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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <vector>
#include <ucbhelper/contentidentifier.hxx>
#include "hierarchydatasupplier.hxx"
#include "hierarchyprovider.hxx"
#include "hierarchycontent.hxx"

using namespace com::sun::star;
using namespace hierarchy_ucp;

namespace hierarchy_ucp
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
    HierarchyEntryData                        aData;

    ResultListEntry( const HierarchyEntryData& rEntry ) : aData( rEntry ) {}
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
    osl::Mutex                                      m_aMutex;
    ResultList                                      m_aResults;
    rtl::Reference< HierarchyContent >              m_xContent;
    uno::Reference< lang::XMultiServiceFactory >    m_xSMgr;
    HierarchyEntry                                  m_aFolder;
    HierarchyEntry::iterator                        m_aIterator;
    sal_Int32                                       m_nOpenMode;
    sal_Bool                                        m_bCountFinal;

    DataSupplier_Impl(
        const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
        const rtl::Reference< HierarchyContent >& rContent,
        sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xSMgr( rxSMgr ),
      m_aFolder( rxSMgr,
                 static_cast< HierarchyContentProvider * >(
                     rContent->getProvider().get() ),
                 rContent->getIdentifier()->getContentIdentifier() ),
      m_nOpenMode( nOpenMode ), m_bCountFinal( sal_False ) {}
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
// HierarchyResultSetDataSupplier Implementation.
//
//=========================================================================
//=========================================================================

HierarchyResultSetDataSupplier::HierarchyResultSetDataSupplier(
                const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
                const rtl::Reference< HierarchyContent >& rContent,
                sal_Int32 nOpenMode )
: m_pImpl( new DataSupplier_Impl( rxSMgr, rContent, nOpenMode ) )
{
}

//=========================================================================
// virtual
HierarchyResultSetDataSupplier::~HierarchyResultSetDataSupplier()
{
    delete m_pImpl;
}

//=========================================================================
// virtual
rtl::OUString HierarchyResultSetDataSupplier::queryContentIdentifierString(
                                                        sal_uInt32 nIndex )
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
        rtl::OUString aId
            = m_pImpl->m_xContent->getIdentifier()->getContentIdentifier();

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

        aId += m_pImpl->m_aResults[ nIndex ]->aData.getName();

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return rtl::OUString();
}

//=========================================================================
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

//=========================================================================
// virtual
sal_Bool HierarchyResultSetDataSupplier::getResult( sal_uInt32 nIndex )
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

    while ( m_pImpl->m_aFolder.next( m_pImpl->m_aIterator ) )
    {
        const HierarchyEntryData& rResult = *m_pImpl->m_aIterator;
        if ( checkResult( rResult ) )
        {
            m_pImpl->m_aResults.push_back( new ResultListEntry( rResult ) );

            if ( nPos == nIndex )
            {
                // Result obtained.
                bFound = sal_True;
                break;
            }
        }
        nPos++;
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
sal_uInt32 HierarchyResultSetDataSupplier::currentCount()
{
    return m_pImpl->m_aResults.size();
}

//=========================================================================
// virtual
sal_Bool HierarchyResultSetDataSupplier::isCountFinal()
{
    return m_pImpl->m_bCountFinal;
}

//=========================================================================
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
        static rtl::OUString aFolderType(
            RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_FOLDER_CONTENT_TYPE ) );
        static rtl::OUString aLinkType(
            RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_LINK_CONTENT_TYPE ) );

        HierarchyContentProperties aData(
            m_pImpl->m_aResults[ nIndex ]->aData );

        uno::Reference< sdbc::XRow > xRow
            = HierarchyContent::getPropertyValues(
                m_pImpl->m_xSMgr,
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

//=========================================================================
// virtual
void HierarchyResultSetDataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
        m_pImpl->m_aResults[ nIndex ]->xRow = uno::Reference< sdbc::XRow >();
}

//=========================================================================
// virtual
void HierarchyResultSetDataSupplier::close()
{
}

//=========================================================================
// virtual
void HierarchyResultSetDataSupplier::validate()
    throw( ucb::ResultSetException )
{
}

//=========================================================================
sal_Bool HierarchyResultSetDataSupplier::checkResult(
                                    const HierarchyEntryData& rResult )
{
    switch ( m_pImpl->m_nOpenMode )
    {
        case ucb::OpenMode::FOLDERS:
            if ( rResult.getType() == HierarchyEntryData::LINK )
            {
                // Entry is a link.
                return sal_False;
            }
            break;

        case ucb::OpenMode::DOCUMENTS:
            if ( rResult.getType() == HierarchyEntryData::FOLDER )
            {
                // Entry is a folder.
                return sal_False;
            }
            break;

        case ucb::OpenMode::ALL:
        default:
            break;
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
