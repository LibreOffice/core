/*************************************************************************
 *
 *  $RCSfile: hierarchydatasupplier.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:54:18 $
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

#ifndef __VECTOR__
#include <stl/vector>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _HIERARCHYDATASUPPLIER_HXX
#include "hierarchydatasupplier.hxx"
#endif
#ifndef _HIERARCHYPROVIDER_HXX
#include "hierarchyprovider.hxx"
#endif
#ifndef _HIERARCHYCONTENT_HXX
#include "hierarchycontent.hxx"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace rtl;
using namespace ucb;

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
    OUString                        aId;
    Reference< XContentIdentifier > xId;
    Reference< XContent >           xContent;
    Reference< XRow >               xRow;
        HierarchyEntryData      rData;

    ResultListEntry( const HierarchyEntryData& rEntry ) : rData( rEntry ) {}
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
    vos::ORef< HierarchyContent >     m_xContent;
    Reference< XMultiServiceFactory > m_xSMgr;
    HierarchyEntry                    m_aFolder;
    HierarchyEntry::iterator          m_aIterator;
      sal_Int32                       m_nOpenMode;
      sal_Bool                        m_bCountFinal;

    DataSupplier_Impl( const Reference< XMultiServiceFactory >& rxSMgr,
                       const vos::ORef< HierarchyContent >& rContent,
                       sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xSMgr( rxSMgr ),
      m_aFolder( rxSMgr, rContent->getIdentifier()->getContentIdentifier() ),
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
// HierarchyResultSetDataSupplier Implementation.
//
//=========================================================================
//=========================================================================

HierarchyResultSetDataSupplier::HierarchyResultSetDataSupplier(
                            const Reference< XMultiServiceFactory >& rxSMgr,
                            const vos::ORef< HierarchyContent >& rContent,
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
OUString HierarchyResultSetDataSupplier::queryContentIdentifierString(
                                                        sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        OUString aId = m_pImpl->m_aResults[ nIndex ]->aId;
        if ( aId.getLength() )
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
            aId += OUString::createFromAscii( "/" );

        aId += HierarchyContentProvider::encodeSegment(
                            m_pImpl->m_aResults[ nIndex ]->rData.aTitle );

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }
    return OUString();
}

//=========================================================================
// virtual
Reference< XContentIdentifier >
HierarchyResultSetDataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
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

    OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() )
    {
        Reference< XContentIdentifier > xId = new ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }
    return Reference< XContentIdentifier >();
}

//=========================================================================
// virtual
Reference< XContent > HierarchyResultSetDataSupplier::queryContent(
                                                        sal_uInt32 nIndex )
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
Reference< XRow > HierarchyResultSetDataSupplier::queryPropertyValues(
                                                        sal_uInt32 nIndex  )
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
        HierarchyContentProperties aData;

        aData.aTitle       = m_pImpl->m_aResults[ nIndex ]->rData.aTitle;
        aData.aTargetURL   = m_pImpl->m_aResults[ nIndex ]->rData.aTargetURL;
        aData.bIsDocument  = ( aData.aTargetURL.getLength() > 0 );
        aData.bIsFolder    = !aData.bIsDocument;
        aData.aContentType = aData.bIsFolder
                           ? OUString::createFromAscii(
                                           HIERARCHY_FOLDER_CONTENT_TYPE )
                           : OUString::createFromAscii(
                                           HIERARCHY_LINK_CONTENT_TYPE );

        Reference< XRow > xRow = HierarchyContent::getPropertyValues(
                                    m_pImpl->m_xSMgr,
                                    getResultSet()->getProperties(),
                                    aData,
                                    m_pImpl->m_xContent->getProvider(),
                                    queryContentIdentifierString( nIndex ) );
        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return Reference< XRow >();
}

//=========================================================================
// virtual
void HierarchyResultSetDataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
        m_pImpl->m_aResults[ nIndex ]->xRow = Reference< XRow >();
}

//=========================================================================
// virtual
void HierarchyResultSetDataSupplier::close()
{
}

//=========================================================================
// virtual
void HierarchyResultSetDataSupplier::validate()
    throw( ResultSetException )
{
}

//=========================================================================
sal_Bool HierarchyResultSetDataSupplier::checkResult(
                                    const HierarchyEntryData& rResult )
{
    switch ( m_pImpl->m_nOpenMode )
    {
        case OpenMode::FOLDERS:
            if ( rResult.aTargetURL.getLength() > 0 )
            {
                // Entry is a link.
                return sal_False;
            }
            break;

        case OpenMode::DOCUMENTS:
            if ( rResult.aTargetURL.getLength() == 0 )
            {
                // Entry is a folder.
                return sal_False;
            }
            break;

        case OpenMode::ALL:
        default:
            break;
    }

    return sal_True;
}

