/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: myucp_datasupplier.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:45:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef DBA_DATASUPPLIER_HXX
#include "myucp_datasupplier.hxx"
#endif
#ifndef DBA_CONTENTHELPER_HXX
#include "ContentHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
DBG_NAME(DataSupplier)

DataSupplier::DataSupplier( const Reference< XMultiServiceFactory >& rxSMgr,
                           const rtl::Reference< ODocumentContainer >& rContent,
                            sal_Int32 nOpenMode )
: m_pImpl( new DataSupplier_Impl( rxSMgr, rContent,nOpenMode ) )
{
    DBG_CTOR(DataSupplier,NULL);

}

//=========================================================================
// virtual
DataSupplier::~DataSupplier()
{

    DBG_DTOR(DataSupplier,NULL);
}

//=========================================================================
// virtual
rtl::OUString DataSupplier::queryContentIdentifierString( sal_Int32 nIndex )
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

//=========================================================================
// virtual
Reference< XContentIdentifier >
DataSupplier::queryContentIdentifier( sal_Int32 nIndex )
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
        Reference< XContentIdentifier > xId = new ::ucb::ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return Reference< XContentIdentifier >();
}

//=========================================================================
// virtual
Reference< XContent >
DataSupplier::queryContent( sal_Int32 _nIndex )
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

//=========================================================================
// virtual
sal_Bool DataSupplier::getResult( sal_Int32 nIndex )
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

    sal_Int32 nOldCount = m_pImpl->m_aResults.size();
    sal_Bool bFound = sal_False;
    sal_Int32 nPos = nOldCount;

    // @@@ Obtain data and put it into result list...
    Sequence< ::rtl::OUString> aSeq = m_pImpl->m_xContent->getElementNames();
    if ( nIndex < aSeq.getLength() )
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

    rtl::Reference< ::ucb::ResultSet > xResultSet = getResultSet().getBodyPtr();
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

//=========================================================================
// virtual
sal_Int32 DataSupplier::totalCount()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bCountFinal )
        return m_pImpl->m_aResults.size();

    sal_Int32 nOldCount = m_pImpl->m_aResults.size();

    // @@@ Obtain data and put it into result list...
    Sequence< ::rtl::OUString> aSeq = m_pImpl->m_xContent->getElementNames();
    const ::rtl::OUString* pIter = aSeq.getConstArray();
    const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
    for(;pIter != pEnd;++pIter)
        m_pImpl->m_aResults.push_back(
                        new ResultListEntry( m_pImpl->m_xContent->getContent(*pIter)->getContentProperties() ) );

    m_pImpl->m_bCountFinal = sal_True;

    rtl::Reference< ::ucb::ResultSet > xResultSet = getResultSet().getBodyPtr();
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

//=========================================================================
// virtual
sal_Int32 DataSupplier::currentCount()
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
Reference< XRow >
DataSupplier::queryPropertyValues( sal_Int32 nIndex  )
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

//=========================================================================
// virtual
void DataSupplier::releasePropertyValues( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( (size_t)nIndex < m_pImpl->m_aResults.size() )
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

