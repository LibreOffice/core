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

#include "DAVSessionFactory.hxx"
#include "NeonSession.hxx"
#include "NeonUri.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace webdav_ucp;
using namespace com::sun::star;

DAVSessionFactory::~DAVSessionFactory()
{
}

rtl::Reference< DAVSession > DAVSessionFactory::createDAVSession(
                const ::rtl::OUString & inUri,
                const uno::Sequence< beans::NamedValue >& rFlags,
                const uno::Reference< uno::XComponentContext > & rxContext )
    throw( DAVException )
{
    m_xContext = rxContext;

    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xProxyDecider.get() )
        m_xProxyDecider.reset( new ucbhelper::InternetProxyDecider( rxContext ) );

    Map::iterator aIt( m_aMap.begin() );
    Map::iterator aEnd( m_aMap.end() );

    while ( aIt != aEnd )
    {
        if ( (*aIt).second->CanUse( inUri, rFlags ) )
            break;

        ++aIt;
    }

    if ( aIt == aEnd )
    {
        NeonUri aURI( inUri );

        std::auto_ptr< DAVSession > xElement(
            new NeonSession( this, inUri, rFlags, *m_xProxyDecider.get() ) );

        aIt = m_aMap.insert( Map::value_type( inUri, xElement.get() ) ).first;
        aIt->second->m_aContainerIt = aIt;
        xElement.release();
        return aIt->second;
    }
    else if ( osl_atomic_increment( &aIt->second->m_nRefCount ) > 1 )
    {
        rtl::Reference< DAVSession > xElement( aIt->second );
        osl_atomic_decrement( &aIt->second->m_nRefCount );
        return xElement;
    }
    else
    {
        osl_atomic_decrement( &aIt->second->m_nRefCount );
        aIt->second->m_aContainerIt = m_aMap.end();

        // If URL scheme is different from http or https we definitely
        // have to use a proxy and therefore can optimize the getProxy
        // call a little:
        NeonUri aURI( inUri );

        aIt->second = new NeonSession( this, inUri, rFlags, *m_xProxyDecider.get() );
        aIt->second->m_aContainerIt = aIt;
        return aIt->second;
    }
}

void DAVSessionFactory::releaseElement( DAVSession * pElement ) SAL_THROW(())
{
    OSL_ASSERT( pElement );
    osl::MutexGuard aGuard( m_aMutex );
    if ( pElement->m_aContainerIt != m_aMap.end() )
        m_aMap.erase( pElement->m_aContainerIt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
