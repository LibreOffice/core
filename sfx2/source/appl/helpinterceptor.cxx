/*************************************************************************
 *
 *  $RCSfile: helpinterceptor.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: pb $ $Date: 2000-11-20 12:54:12 $
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

#include "helpinterceptor.hxx"
#include "sfxuno.hxx"

using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

HelpInterceptor::HelpInterceptor() :

    m_pHistory( NULL )

{
}

// -----------------------------------------------------------------------

HelpInterceptor::~HelpInterceptor()
{
    for ( USHORT i = 0; i < m_pHistory->Count(); ++i )
        delete m_pHistory->GetObject(i);
    delete m_pHistory;
}

// XDispatchProvider

Reference< XDispatch > SAL_CALL HelpInterceptor::queryDispatch(

    const URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags )

    throw( RuntimeException )

{
    Reference< XDispatch > xResult;
    return xResult;
}

Sequence < Reference < XDispatch > > SAL_CALL HelpInterceptor::queryDispatches(

    const Sequence< DispatchDescriptor >& aDescripts )

    throw( RuntimeException )

{
    Sequence< Reference< XDispatch > > aReturn( aDescripts.getLength() );
    Reference< XDispatch >* pReturn = aReturn.getArray();
    const DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for ( sal_Int16 i = 0; i < aDescripts.getLength(); ++i, ++pReturn, ++pDescripts )
    {
        *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
    }
    return aReturn;
}

// XDispatchProviderInterceptor

Reference< XDispatchProvider > SAL_CALL HelpInterceptor::getSlaveDispatchProvider()

    throw( RuntimeException )

{
    return m_xSlaveDispatcher;
}

void SAL_CALL HelpInterceptor::setSlaveDispatchProvider( const Reference< XDispatchProvider >& xNewSlave )

    throw( RuntimeException )

{
    m_xSlaveDispatcher = xNewSlave;
}

Reference< XDispatchProvider > SAL_CALL HelpInterceptor::getMasterDispatchProvider()

    throw( RuntimeException )

{
    return m_xMasterDispatcher;
}

void SAL_CALL HelpInterceptor::setMasterDispatchProvider( const Reference< XDispatchProvider >& xNewMaster )

    throw( RuntimeException )

{
    m_xMasterDispatcher = xNewMaster;
}

// XInterceptorInfo

Sequence< ::rtl::OUString > SAL_CALL HelpInterceptor::getInterceptedURLs()

    throw( RuntimeException )

{
    Sequence< ::rtl::OUString > aURLList( 1 );
    aURLList[0] = ::rtl::OUString::createFromAscii( "vnd.sun.star.help://*" );
    return aURLList;;
}

// XDispatch

void SAL_CALL HelpInterceptor::dispatch( const URL& aURL,
                                         const Sequence< ::com::sun::star::beans::PropertyValue >& aArgs )

    throw( RuntimeException )

{
    if ( String( DEFINE_CONST_UNICODE(".uno:") ) == String( aURL.Protocol ) )
    {
        String aCommand( aURL.Path );

        if ( aCommand == String( DEFINE_CONST_UNICODE("backward" ) ) )
        {
        }
    }
    else
    {
    }
}

void SAL_CALL HelpInterceptor::addStatusListener( const Reference< XStatusListener >& xControl,
                                                  const URL& aURL )

    throw( RuntimeException )

{
}

void SAL_CALL HelpInterceptor::removeStatusListener( const Reference< XStatusListener >& xControl,
                                                     const URL& aURL )

    throw( RuntimeException )

{
}

