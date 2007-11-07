/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DAVSessionFactory.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:03:15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#ifndef _DAVSESSIONFACTORY_HXX_
#include "DAVSessionFactory.hxx"
#endif
#ifndef _NEONSESSION_HXX_
#include "NeonSession.hxx"
#endif
#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

using namespace webdav_ucp;
using namespace com::sun::star;

DAVSessionFactory::~DAVSessionFactory()
{
}

rtl::Reference< DAVSession > DAVSessionFactory::createDAVSession(
                const ::rtl::OUString & inUri,
                const uno::Reference< lang::XMultiServiceFactory > & rxSMgr )
    throw( DAVException )
{
    m_xMSF = rxSMgr;

    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xProxyDecider.get() )
        m_xProxyDecider.reset( new ucbhelper::InternetProxyDecider( rxSMgr ) );

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
        NeonUri aURI( inUri );

        std::auto_ptr< DAVSession > xElement(
            new NeonSession( this, inUri, *m_xProxyDecider.get() ) );

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
        NeonUri aURI( inUri );

        aIt->second = new NeonSession( this, inUri, *m_xProxyDecider.get() );
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

