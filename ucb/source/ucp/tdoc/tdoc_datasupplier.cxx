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

#include "com/sun/star/ucb/IllegalIdentifierException.hpp"
#include "osl/diagnose.h"
#include "ucbhelper/contentidentifier.hxx"

#include "tdoc_datasupplier.hxx"
#include "tdoc_content.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;

namespace tdoc_ucp
{


// struct ResultListEntry.


struct ResultListEntry
{
    OUString                             aURL;
    uno::Reference< ucb::XContentIdentifier > xId;
    uno::Reference< ucb::XContent >           xContent;
    uno::Reference< sdbc::XRow >              xRow;

    explicit ResultListEntry( const OUString& rURL ) : aURL( rURL ) {}
};


// ResultList.


typedef std::vector< ResultListEntry* > ResultList;


// struct DataSupplier_Impl.


struct DataSupplier_Impl
{
    osl::Mutex                                   m_aMutex;
    ResultList                                   m_aResults;
    rtl::Reference< Content >                    m_xContent;
    uno::Reference< uno::XComponentContext >     m_xContext;
    uno::Sequence< OUString > *                  m_pNamesOfChildren;
    bool                                         m_bCountFinal;
    bool                                         m_bThrowException;

    DataSupplier_Impl(
            const uno::Reference< uno::XComponentContext >& rxContext,
            const rtl::Reference< Content >& rContent )
    : m_xContent( rContent ), m_xContext( rxContext ),
      m_pNamesOfChildren( nullptr ),
      m_bCountFinal( false ), m_bThrowException( false )
    {}
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

    delete m_pNamesOfChildren;
}

}

// DataSupplier Implementation.
ResultSetDataSupplier::ResultSetDataSupplier(
                const uno::Reference< uno::XComponentContext >& rxContext,
                const rtl::Reference< Content >& rContent )
: m_pImpl( new DataSupplier_Impl( rxContext, rContent ) )
{
}

// virtual
ResultSetDataSupplier::~ResultSetDataSupplier()
{
}

// virtual
OUString
ResultSetDataSupplier::queryContentIdentifierString( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
    {
        OUString aId = m_pImpl->m_aResults[ nIndex ]->aURL;
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
    return OUString();
}

// virtual
uno::Reference< ucb::XContentIdentifier >
ResultSetDataSupplier::queryContentIdentifier( sal_uInt32 nIndex )
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
ResultSetDataSupplier::queryContent( sal_uInt32 nIndex )
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
bool ResultSetDataSupplier::getResult( sal_uInt32 nIndex )
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

    if ( queryNamesOfChildren() )
    {
        for ( sal_uInt32 n = nOldCount;
              n < sal::static_int_cast<sal_uInt32>(
                      m_pImpl->m_pNamesOfChildren->getLength());
              ++n )
        {
            const OUString & rName
                = m_pImpl->m_pNamesOfChildren->getConstArray()[ n ];

            if ( rName.isEmpty() )
            {
                OSL_FAIL( "ResultDataSupplier::getResult - Empty name!" );
                break;
            }

            // Assemble URL for child.
            OUString aURL = assembleChildURL( rName );

            m_pImpl->m_aResults.push_back( new ResultListEntry( aURL ) );

            if ( n == nIndex )
            {
                // Result obtained.
                bFound = true;
                break;
            }
        }
    }

    if ( !bFound )
        m_pImpl->m_bCountFinal = true;

    rtl::Reference< ::ucbhelper::ResultSet > xResultSet = getResultSet().get();
    if ( xResultSet.is() )
    {
        // Callbacks follow!
        aGuard.clear();

        if ( nOldCount < m_pImpl->m_aResults.size() )
            xResultSet->rowCountChanged( nOldCount, m_pImpl->m_aResults.size() );

        if ( m_pImpl->m_bCountFinal )
            xResultSet->rowCountFinal();
    }

    return bFound;
}

// virtual
sal_uInt32 ResultSetDataSupplier::totalCount()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_bCountFinal )
        return m_pImpl->m_aResults.size();

    sal_uInt32 nOldCount = m_pImpl->m_aResults.size();

    if ( queryNamesOfChildren() )
    {
        for ( sal_uInt32 n = nOldCount;
              n < sal::static_int_cast<sal_uInt32>(
                      m_pImpl->m_pNamesOfChildren->getLength());
              ++n )
        {
            const OUString & rName
                = m_pImpl->m_pNamesOfChildren->getConstArray()[ n ];

            if ( rName.isEmpty() )
            {
                OSL_FAIL( "ResultDataSupplier::getResult - Empty name!" );
                break;
            }

            // Assemble URL for child.
            OUString aURL = assembleChildURL( rName );

            m_pImpl->m_aResults.push_back( new ResultListEntry( aURL ) );
        }
    }

    m_pImpl->m_bCountFinal = true;

    rtl::Reference< ::ucbhelper::ResultSet > xResultSet = getResultSet().get();
    if ( xResultSet.is() )
    {
        // Callbacks follow!
        aGuard.clear();

        if ( nOldCount < m_pImpl->m_aResults.size() )
            xResultSet->rowCountChanged( nOldCount, m_pImpl->m_aResults.size() );

        xResultSet->rowCountFinal();
    }

    return m_pImpl->m_aResults.size();
}

// virtual
sal_uInt32 ResultSetDataSupplier::currentCount()
{
    return m_pImpl->m_aResults.size();
}

// virtual
bool ResultSetDataSupplier::isCountFinal()
{
    return m_pImpl->m_bCountFinal;
}

// virtual
uno::Reference< sdbc::XRow >
ResultSetDataSupplier::queryPropertyValues( sal_uInt32 nIndex  )
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
                        m_pImpl->m_xContext,
                        getResultSet()->getProperties(),
                        m_pImpl->m_xContent->getContentProvider().get(),
                        queryContentIdentifierString( nIndex ) );
        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;
        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}

// virtual
void ResultSetDataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < m_pImpl->m_aResults.size() )
        m_pImpl->m_aResults[ nIndex ]->xRow.clear();
}

// virtual
void ResultSetDataSupplier::close()
{
}

// virtual
void ResultSetDataSupplier::validate()
{
    if ( m_pImpl->m_bThrowException )
        throw ucb::ResultSetException();
}

bool ResultSetDataSupplier::queryNamesOfChildren()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_pNamesOfChildren == nullptr )
    {
        uno::Sequence< OUString > * pNamesOfChildren
            = new uno::Sequence< OUString >();

        if ( !m_pImpl->m_xContent->getContentProvider()->queryNamesOfChildren(
                m_pImpl->m_xContent->getIdentifier()->getContentIdentifier(),
                *pNamesOfChildren ) )
        {
            OSL_FAIL( "Got no list of children!" );
            delete pNamesOfChildren;
            m_pImpl->m_bThrowException = true;
            return false;
        }
        else
        {
            m_pImpl->m_pNamesOfChildren = pNamesOfChildren;
        }
    }
    return true;
}

OUString
ResultSetDataSupplier::assembleChildURL( const OUString& aName )
{
    OUString aContURL
        = m_pImpl->m_xContent->getIdentifier()->getContentIdentifier();
    OUString aURL( aContURL );

    sal_Int32 nUrlEnd = aURL.lastIndexOf( '/' );
    if ( nUrlEnd != aURL.getLength() - 1 )
        aURL += "/";

    aURL += aName;
    return aURL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
