/*************************************************************************
 *
 *  $RCSfile: odma_datasupplier.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:00:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <vector>

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif

#ifndef ODMA_DATASUPPLIER_HXX
#include "odma_datasupplier.hxx"
#endif
#ifndef ODMA_CONTENT_HXX
#include "odma_content.hxx"
#endif
#ifndef ODMA_CONTENTPROPS_HXX
#include "odma_contentprops.hxx"
#endif
#ifndef ODMA_PROVIDER_HXX
#include "odma_provider.hxx"
#endif
#ifndef ODMA_LIB_HXX
#include "odma_lib.hxx"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace ucb;

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
    ::rtl::OUString                 aId;
    Reference< XContentIdentifier > xId;
    Reference< XContent >           xContent;
    Reference< XRow >               xRow;
    ::vos::ORef<ContentProperties>  rData;

    ResultListEntry( const ::vos::ORef<ContentProperties>& rEntry ) : rData( rEntry ) {}
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
    vos::ORef< Content >              m_xContent;
    Reference< XMultiServiceFactory > m_xSMgr;
// @@@ The data source and an iterator for it
//  Entry                             m_aFolder;
//  Entry::iterator                   m_aIterator;
      sal_Int32                       m_nOpenMode;
      sal_Bool                        m_bCountFinal;

    DataSupplier_Impl( const Reference< XMultiServiceFactory >& rxSMgr,
                       const vos::ORef< Content >& rContent,
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
        it++;
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
                           const vos::ORef< ::odma::Content >& rContent,
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
        Reference< XContentIdentifier > xId = new ucb::ContentIdentifier( aId );
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
    sal_uInt32 nPos = nOldCount;

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

    ::rtl::OString sQuery("SELECT ODM_DOCID, ODM_NAME");

    DWORD dwFlags = ODM_SPECIFIC;
    odm = NODMQueryExecute(ContentProvider::getHandle(), sQuery,dwFlags, lpszDMSList, pQueryId );
    if(odm != ODM_SUCCESS)
        return sal_False;

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
                ::vos::ORef<ContentProperties> rProps = new ContentProperties();
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

    delete lpszDMSList;
    delete pQueryId;
    delete lpszDocId;
    delete lpszDocName;

    if ( !bFound )
        m_pImpl->m_bCountFinal = sal_True;

    vos::ORef< ResultSet > xResultSet = getResultSet();
    if ( xResultSet.isValid() )
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

    vos::ORef< ResultSet > xResultSet = getResultSet();
    if ( xResultSet.isValid() )
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
