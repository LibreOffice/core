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

#include "DAVSessionFactory.hxx"
#include "SerfSession.hxx"
#include "SerfUri.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace http_dav_ucp;
using namespace com::sun::star;

DAVSessionFactory::~DAVSessionFactory()
{
}

rtl::Reference< DAVSession > DAVSessionFactory::createDAVSession(
                const OUString & inUri,
                const uno::Reference< uno::XComponentContext > & rxContext )
    throw( DAVException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xProxyDecider.get() )
        m_xProxyDecider.reset( new ucbhelper::InternetProxyDecider( rxContext ) );

    Map::iterator aIt( m_aMap.begin() );
    Map::iterator aEnd( m_aMap.end() );

    while ( aIt != aEnd )
    {
        if ( (*aIt).second->CanUse( inUri ) )
            break;

        ++aIt;
    }

    if ( aIt == aEnd )
    {
        SerfUri aURI( inUri );

        std::unique_ptr< DAVSession > xElement(
            new SerfSession( this, inUri, *m_xProxyDecider.get() ) );

        aIt = m_aMap.insert( Map::value_type( inUri, xElement.get() ) ).first;
        aIt->second->m_aContainerIt = aIt;
        xElement.release();
        return aIt->second;
    }
    else if ( osl_incrementInterlockedCount( &aIt->second->m_nRefCount ) > 1 )
    {
        rtl::Reference< DAVSession > xElement( aIt->second );
        osl_decrementInterlockedCount( &aIt->second->m_nRefCount );
        return xElement;
    }
    else
    {
        osl_decrementInterlockedCount( &aIt->second->m_nRefCount );
        aIt->second->m_aContainerIt = m_aMap.end();

        // If URL scheme is different from http or https we definitely
        // have to use a proxy and therefore can optimize the getProxy
        // call a little:
        SerfUri aURI( inUri );

        aIt->second = new SerfSession( this, inUri, *m_xProxyDecider.get() );
        aIt->second->m_aContainerIt = aIt;
        return aIt->second;
    }
}

void DAVSessionFactory::releaseElement( DAVSession * pElement )
{
    assert( pElement );
    osl::MutexGuard aGuard( m_aMutex );
    if ( pElement->m_aContainerIt != m_aMap.end() )
        m_aMap.erase( pElement->m_aContainerIt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
