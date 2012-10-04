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


#ifdef WNT
#include <windows.h>
#endif
#include <vector>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include "odma_datasupplier.hxx"
#include "odma_content.hxx"
#include "odma_contentprops.hxx"
#include "odma_provider.hxx"

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;

using namespace odma;

namespace odma
{

//=========================================================================
//
// struct ResultListEntry.
//
//=========================================================================

struct ResultListEntry
{
    ::rtl::OUString                     aId;
    Reference< XContentIdentifier >     xId;
    Reference< XContent >               xContent;
    Reference< XRow >                   xRow;
    ::rtl::Reference<ContentProperties> rData;

    ResultListEntry( const ::rtl::Reference<ContentProperties>& rEntry ) : rData( rEntry ) {}
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
    osl::Mutex                        m_aMutex;
    ResultList                        m_aResults;
    rtl::Reference< Content >         m_xContent;
    Reference< XMultiServiceFactory > m_xSMgr;
// @@@ The data source and an iterator for it
//  Entry                             m_aFolder;
//  Entry::iterator                   m_aIterator;
      sal_Int32                       m_nOpenMode;
      sal_Bool                        m_bCountFinal;

    DataSupplier_Impl( const Reference< XMultiServiceFactory >& rxSMgr,
                       const rtl::Reference< Content >& rContent,
                       sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xSMgr( rxSMgr ),
//    m_aFolder( rxSMgr, rContent->getIdentifier()->getContentIdentifier() ),
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
// DataSupplier Implementation.
//
//=========================================================================
//=========================================================================

DataSupplier::DataSupplier( const Reference<XMultiServiceFactory >& rxSMgr,
                           const rtl::Reference< ::odma::Content >& rContent,
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
::rtl::OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        ::rtl::OUString aId = m_pImpl->m_aResults[ nIndex ]->aId;
        if ( aId.getLength() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( nIndex ) )
    {
        ::rtl::OUString aId
            = m_pImpl->m_xContent->getIdentifier()->getContentIdentifier();

        aId += m_pImpl->m_aResults[ nIndex ]->rData->m_sTitle;

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return ::rtl::OUString();
}

//=========================================================================
// virtual
Reference< XContentIdentifier > DataSupplier::queryContentIdentifier(
                                                        sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        Reference< XContentIdentifier > xId
                                = m_pImpl->m_aResults[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    ::rtl::OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() )
    {
        Reference< XContentIdentifier > xId
            = new ucbhelper::ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return Reference< XContentIdentifier >();
}

//=========================================================================
// virtual
Reference< XContent > DataSupplier::queryContent( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        Reference< XContent > xContent
                                = m_pImpl->m_aResults[ nIndex ]->xContent;
        if ( xContent.is() )
        {
            // Already cached.
            return xContent;
        }
    }

    Reference< XContentIdentifier > xId = queryContentIdentifier( nIndex );
    if ( xId.is() )
    {
        try
        {
            Reference< XContent > xContent
                = m_pImpl->m_xContent->getProvider()->queryContent( xId );
            m_pImpl->m_aResults[ nIndex ]->xContent = xContent;
            return xContent;

        }
        catch ( IllegalIdentifierException& )
        {
        }
    }
    return Reference< XContent >();
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
//  sal_uInt32 nPos = nOldCount;

    // @@@ Obtain data and put it into result list...
/*
    while ( m_pImpl->m_aFolder.next( m_pImpl->m_aIterator ) )
    {
        m_pImpl->m_aResults.push_back(
                        new ResultListEntry( *m_pImpl->m_aIterator ) );

        if ( nPos == nIndex )
        {
            // Result obtained.
            bFound = sal_True;
            break;
        }

        nPos++;
    }
*/
    // now query for all documents in the DMS
    OSL_ENSURE(ContentProvider::getHandle(),"No Handle!");
    sal_Char* pQueryId      = new sal_Char[ODM_QUERYID_MAX];
    sal_Char* lpszDMSList   = new sal_Char[ODM_DMSID_MAX];

    ODMSTATUS odm = NODMGetDMS(ODMA_ODMA_REGNAME, lpszDMSList);
    lpszDMSList[strlen(lpszDMSList)+1] = '\0';

    const char sQuery[] = "SELECT ODM_DOCID_LATEST, ODM_NAME";

    DWORD dwFlags = ODM_SPECIFIC;
    odm = NODMQueryExecute(ContentProvider::getHandle(), sQuery, dwFlags, lpszDMSList, pQueryId );
    if(odm != ODM_SUCCESS) {
        delete[] pQueryId;
        delete[] lpszDMSList;
        return sal_False;
    }

    sal_uInt16 nCount       = 10;
    sal_uInt16 nMaxCount    = 10;
    sal_Char* lpszDocId     = new sal_Char[ODM_DOCID_MAX * nMaxCount];
    sal_Char* lpszDocName   = new sal_Char[ODM_NAME_MAX * nMaxCount];


    ::rtl::OUString sContentType(RTL_CONSTASCII_USTRINGPARAM(ODMA_CONTENT_TYPE));
    sal_uInt32 nCurrentCount = 0;
    do
    {
        if(nCount >= nMaxCount)
        {
            nCount = nMaxCount;
            odm = NODMQueryGetResults(ContentProvider::getHandle(), pQueryId,lpszDocId, lpszDocName, ODM_NAME_MAX, (WORD*)&nCount);
            nCurrentCount += nCount;
        }
        if(odm == ODM_SUCCESS && nIndex < nCurrentCount)
        {
            bFound = sal_True;
            for(sal_uInt16 i = 0; i < nCount; ++i)
            {
                ::rtl::Reference<ContentProperties> rProps = new ContentProperties();
                rProps->m_sDocumentId   = ::rtl::OString(&lpszDocId[ODM_DOCID_MAX*i]);
                rProps->m_sContentType  = sContentType;
                m_pImpl->m_xContent->getContentProvider()->append(rProps);
                m_pImpl->m_aResults.push_back( new ResultListEntry(rProps));
            }
        }
    }
    while(nCount > nMaxCount);


    // now close the query
    odm = NODMQueryClose(ContentProvider::getHandle(), pQueryId);

    delete [] lpszDMSList;
    delete [] pQueryId;
    delete [] lpszDocId;
    delete [] lpszDocName;

    if ( !bFound )
        m_pImpl->m_bCountFinal = sal_True;

    rtl::Reference< ucbhelper::ResultSet > xResultSet = getResultSet();
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

    // @@@ Obtain data and put it into result list...
/*
    while ( m_pImpl->m_aFolder.next( m_pImpl->m_aIterator ) )
        m_pImpl->m_aResults.push_back(
                        new ResultListEntry( *m_pImpl->m_aIterator ) );
*/
    m_pImpl->m_bCountFinal = sal_True;

    rtl::Reference< ucbhelper::ResultSet > xResultSet = getResultSet();
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
Reference< XRow > DataSupplier::queryPropertyValues( sal_uInt32 nIndex  )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
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
        Reference< XRow > xRow = Content::getPropertyValues(
                                    m_pImpl->m_xSMgr,
                                    getResultSet()->getProperties(),
                                    m_pImpl->m_aResults[ nIndex ]->rData,
                                    m_pImpl->m_xContent->getProvider(),
                                    queryContentIdentifierString( nIndex ) );
        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return Reference< XRow >();
}

//=========================================================================
// virtual
void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
        m_pImpl->m_aResults[ nIndex ]->xRow = Reference< XRow >();
}

//=========================================================================
// virtual
void DataSupplier::close()
{
}

//=========================================================================
// virtual
void DataSupplier::validate()
    throw( ResultSetException )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
