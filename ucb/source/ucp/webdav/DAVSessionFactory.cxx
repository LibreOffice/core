/*************************************************************************
 *
 *  $RCSfile: DAVSessionFactory.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kso $ $Date: 2002-10-28 16:20:01 $
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
            ( aURI.GetScheme().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "http" ) ) ||
             aURI.GetScheme().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "https" ) ) )
            ? new NeonSession(
                this, inUri, m_xProxyDecider->getProxy( aURI.GetScheme(),
                                                        aURI.GetHost(),
                                                        aURI.GetPort() ) )
            : new NeonSession(
                this, inUri, m_xProxyDecider->getProxy( aURI.GetScheme(),
                                                        rtl::OUString() /* not used */,
                                                        -1 /* not used */ ) ) );
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

        if ( aURI.GetScheme().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "http" ) ) ||
             aURI.GetScheme().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "https" ) ) )
            aIt->second = new NeonSession( this,
                                           inUri,
                                           m_xProxyDecider->getProxy(
                                            aURI.GetScheme(),
                                            aURI.GetHost(),
                                            aURI.GetPort() ) );
        else
            aIt->second = new NeonSession( this,
                                           inUri,
                                           m_xProxyDecider->getProxy(
                                            aURI.GetScheme(),
                                            rtl::OUString() /* not used */,
                                            -1 /* not used */ ) );
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

