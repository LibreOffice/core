/*************************************************************************
 *
 *  $RCSfile: DAVSessionFactory.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kso $ $Date: 2001-06-25 08:51:54 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

using namespace webdav_ucp;
using namespace com::sun::star;

DAVSessionFactory::~DAVSessionFactory()
{
    if ( m_xProxySettings.is() )
    {
        m_xProxySettings->dispose();
        m_xProxySettings = 0;
    }
}

rtl::Reference< DAVSession > DAVSessionFactory::createDAVSession(
                const ::rtl::OUString & inUri,
                const uno::Reference< lang::XMultiServiceFactory > & rxSMgr )
    throw( DAVException )
{
     osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_xProxySettings.is() )
        m_xProxySettings = new ProxySettings( rxSMgr );

    DAVSession * theSession = GetExistingSession( inUri );
    if ( theSession == NULL )
    {
        theSession = new NeonSession( this,
                                      inUri,
                                      m_xProxySettings->getProxy( inUri ) );
        sActiveSessions.push_back( theSession );
    }

    return theSession;
}

void DAVSessionFactory::ReleaseDAVSession( DAVSession * inSession )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    std::vector< DAVSession * >::iterator theIterator =
                                            sActiveSessions.begin();
    while ( theIterator != sActiveSessions.end() )
    {
        if ( *theIterator == inSession )
        {
            sActiveSessions.erase( theIterator );
            break;
        }
        ++theIterator;
    }
}

DAVSession * DAVSessionFactory::GetExistingSession( const rtl::OUString & inUri )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    DAVSession * theSession = NULL;
    std::vector< DAVSession * >::iterator theIterator =
                                            sActiveSessions.begin();
    while ( theIterator != sActiveSessions.end() )
    {
        if ( (*theIterator)->CanUse( inUri ) )
        {
            theSession = *theIterator;
            break;
        }
        ++theIterator;
    }
    return theSession;
}
