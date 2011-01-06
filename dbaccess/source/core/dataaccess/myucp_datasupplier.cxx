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
#include "precompiled_dbaccess.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <vector>

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>

#include "myucp_datasupplier.hxx"
#include "ContentHelper.hxx"
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <tools/debug.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;

// @@@ Adjust namespace name.
using namespace dbaccess;

// @@@ Adjust namespace name.
namespace dbaccess
{

//=========================================================================
//
// struct ResultListEntry.
//
//=========================================================================

struct ResultListEntry
{
    rtl::OUString                       aId;
    Reference< XContentIdentifier >     xId;
    ::rtl::Reference< OContentHelper >  xContent;
    Reference< XRow >                   xRow;
    const ContentProperties&            rData;

    ResultListEntry( const ContentProperties& rEntry ) : rData( rEntry ) {}
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
    rtl::Reference< ODocumentContainer >             m_xContent;
    Reference< XMultiServiceFactory >            m_xSMgr;
      sal_Int32                                  m_nOpenMode;
      sal_Bool                                   m_bCountFinal;

    DataSupplier_Impl( const Reference< XMultiServiceFactory >& rxSMgr,
                        const rtl::Reference< ODocumentContainer >& rContent,
                       sal_Int32 nOpenMode )
    : m_xContent(rContent)
    , m_xSMgr( rxSMgr )
    , m_nOpenMode( nOpenMode )
    , m_bCountFinal( sal_False ) {}
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
//
// DataSupplier Implementation.
//
//=========================================================================
DBG_NAME(DataSupplier)

DataSupplier::DataSupplier( const Reference< XMultiServiceFactory >& rxSMgr,
                           const rtl::Reference< ODocumentContainer >& rContent,
                            sal_Int32 nOpenMode )
: m_pImpl( new DataSupplier_Impl( rxSMgr, rContent,nOpenMode ) )
{
    DBG_CTOR(DataSupplier,NULL);

}

DataSupplier::~DataSupplier()
{

    DBG_DTOR(DataSupplier,NULL);
}

rtl::OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( (size_t)nIndex < m_pImpl->m_aResults.size() )
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

        if ( aId.getLength() )
            aId += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

        aId += m_pImpl->m_aResults[ nIndex ]->rData.aTitle;

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return rtl::OUString();
}

Reference< XContentIdentifier >
DataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( (size_t)nIndex < m_pImpl->m_aResults.size() )
    {
        Reference< XContentIdentifier > xId = m_pImpl->m_aResults[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    rtl::OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() )
    {
        Reference< XContentIdentifier > xId = new ::ucbhelper::ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return Reference< XContentIdentifier >();
}

Reference< XContent >
DataSupplier::queryContent( sal_uInt32 _nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( (size_t)_nIndex < m_pImpl->m_aResults.size() )
    {
        Reference< XContent > xContent = m_pImpl->m_aResults[ _nIndex ]->xContent.get();
        if ( xContent.is() )
        {
            // Already cached.
            return xContent;
        }
    }

    Reference< XContentIdentifier > xId = queryContentIdentifier( _nIndex );
    if ( xId.is() )
    {
        try
        {
            Reference< XContent > xContent;
            ::rtl::OUString sName = xId->getContentIdentifier();
            sal_Int32 nIndex = sName.lastIndexOf('/') + 1;
            sName = sName.getToken(0,'/',nIndex);

            m_pImpl->m_aResults[ _nIndex ]->xContent = m_pImpl->m_xContent->getContent(sName);

            xContent = m_pImpl->m_aResults[ _nIndex ]->xContent.get();
            return xContent;

        }
        catch ( IllegalIdentifierException& )
        {
        }
    }
    return Reference< XContent >();
}

sal_Bool DataSupplier::getResult( sal_uInt32 nIndex )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( (size_t)nIndex < m_pImpl->m_aResults.size() )
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

    // @@@ Obtain data and put it into result list...
    Sequence< ::rtl::OUString> aSeq = m_pImpl->m_xContent->getElementNames();
    if ( nIndex < sal::static_int_cast< sal_uInt32 >( aSeq.getLength() ) )
    {
        const ::rtl::OUString* pIter = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
        for(pIter = pIter + nPos;pIter != pEnd;++pIter,++nPos)
        {
            m_pImpl->m_aResults.push_back(
                            new ResultListEntry( m_pImpl->m_xContent->getContent(*pIter)->getContentProperties() ) );

            if ( nPos == nIndex )
            {
                // Result obtained.
                bFound = sal_True;
                break;
            }
        }
    }

    if ( !bFound )
        m_pImpl->m_bCountFinal = sal_True;

    rtl::Reference< ::ucbhelper::ResultSet > xResultSet = getResultSet().get();
    if ( xResultSet.is() )
    {
        // Callbacks follow!
        aGuard.clear();

        if ( (size_t)nOldCount < m_pImpl->m_aResults.size() )
            xResultSet->rowCountChanged(
                                    nOldCount, m_pImpl->m_aResults.size() );

        if ( m_pImpl->m_bCountFinal )
            xResultSet->rowCountFinal();
    }

    return bFound;
}

sal_uInt32 DataSupplier::totalCount()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bCountFinal )
        return m_pImpl->m_aResults.size();

    sal_uInt32 nOldCount = m_pImpl->m_aResults.size();

    // @@@ Obtain data and put it into result list...
    Sequence< ::rtl::OUString> aSeq = m_pImpl->m_xContent->getElementNames();
    const ::rtl::OUString* pIter = aSeq.getConstArray();
    const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
    for(;pIter != pEnd;++pIter)
        m_pImpl->m_aResults.push_back(
                        new ResultListEntry( m_pImpl->m_xContent->getContent(*pIter)->getContentProperties() ) );

    m_pImpl->m_bCountFinal = sal_True;

    rtl::Reference< ::ucbhelper::ResultSet > xResultSet = getResultSet().get();
    if ( xResultSet.is() )
    {
        // Callbacks follow!
        aGuard.clear();

        if ( (size_t)nOldCount < m_pImpl->m_aResults.size() )
            xResultSet->rowCountChanged(
                                    nOldCount, m_pImpl->m_aResults.size() );

        xResultSet->rowCountFinal();
    }

    return m_pImpl->m_aResults.size();
}

sal_uInt32 DataSupplier::currentCount()
{
    return m_pImpl->m_aResults.size();
}

sal_Bool DataSupplier::isCountFinal()
{
    return m_pImpl->m_bCountFinal;
}

Reference< XRow >
DataSupplier::queryPropertyValues( sal_uInt32 nIndex  )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( (size_t)nIndex < m_pImpl->m_aResults.size() )
    {
        Reference< XRow > xRow = m_pImpl->m_aResults[ nIndex ]->xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( getResult( nIndex ) )
    {
        if ( !m_pImpl->m_aResults[ nIndex ]->xContent.is() )
            queryContent(nIndex);

        Reference< XRow > xRow = m_pImpl->m_aResults[ nIndex ]->xContent->getPropertyValues(getResultSet()->getProperties());
        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return Reference< XRow >();
}

void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( (size_t)nIndex < m_pImpl->m_aResults.size() )
        m_pImpl->m_aResults[ nIndex ]->xRow = Reference< XRow >();
}

void DataSupplier::close()
{
}

void DataSupplier::validate()
    throw( ResultSetException )
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
