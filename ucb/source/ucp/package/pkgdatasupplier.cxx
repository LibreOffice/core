/*************************************************************************
 *
 *  $RCSfile: pkgdatasupplier.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2000-11-27 13:05:27 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <vector>

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif

#ifndef _PKGDATASUPPLIER_HXX
#include "pkgdatasupplier.hxx"
#endif
#ifndef _PKGCONTENT_HXX
#include "pkgcontent.hxx"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace rtl;
using namespace ucb;

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
    OUString                        aURL;
    Reference< XContentIdentifier > xId;
    Reference< XContent >           xContent;
    Reference< XRow >               xRow;

    ResultListEntry( const OUString& rURL ) : aURL( rURL ) {}
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
    Reference< XEnumeration >         m_xFolderEnum;
      sal_Int32                       m_nOpenMode;
      sal_Bool                        m_bCountFinal;

    DataSupplier_Impl( const Reference< XMultiServiceFactory >& rxSMgr,
                       const vos::ORef< Content >& rContent,
                       sal_Int32 nOpenMode )
    : m_xContent( rContent ), m_xSMgr( rxSMgr ),
      m_xFolderEnum( rContent->getIterator() ),
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

DataSupplier::DataSupplier( const Reference< XMultiServiceFactory >& rxSMgr,
                            const vos::ORef< Content >& rContent,
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
OUString DataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        OUString aId = m_pImpl->m_aResults[ nIndex ]->aURL;
        if ( aId.getLength() )
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
    return OUString();
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

    while ( m_pImpl->m_xFolderEnum->hasMoreElements() )
    {
        try
        {
            Reference< XNamed > xNamed;
            m_pImpl->m_xFolderEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                VOS_ENSURE( sal_False,
                            "DataSupplier::getResult - Got no XNamed!" );
                break;
            }

            OUString aName = xNamed->getName();

            if ( !aName.getLength() )
            {
                VOS_ENSURE( sal_False,
                            "DataSupplier::getResult - Empty name!" );
                break;
            }

            // Assemble URL for child.
            OUString aURL
                = m_pImpl->m_xContent->getIdentifier()->getContentIdentifier();
            aURL += OUString::createFromAscii( "/" );
            aURL += aName;

            m_pImpl->m_aResults.push_back( new ResultListEntry( aURL ) );

            if ( nPos == nIndex )
            {
                // Result obtained.
                bFound = sal_True;
                break;
            }

            nPos++;
        }
        catch ( NoSuchElementException & )
        {
            break;
        }
        catch ( WrappedTargetException & )
        {
            break;
        }
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
            Reference< XNamed > xNamed;
            m_pImpl->m_xFolderEnum->nextElement() >>= xNamed;

            if ( !xNamed.is() )
            {
                VOS_ENSURE( sal_False,
                            "DataSupplier::getResult - Got no XNamed!" );
                break;
            }

            OUString aName = xNamed->getName();

            if ( !aName.getLength() )
            {
                VOS_ENSURE( sal_False,
                            "DataSupplier::getResult - Empty name!" );
                break;
            }

            // Assemble URL for child.
            OUString aURL
                = m_pImpl->m_xContent->getIdentifier()->getContentIdentifier();
            aURL += OUString::createFromAscii( "/" );
            aURL += aName;

            m_pImpl->m_aResults.push_back( new ResultListEntry( aURL ) );
        }
        catch ( NoSuchElementException & )
        {
            break;
        }
        catch ( WrappedTargetException & )
        {
            break;
        }
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

