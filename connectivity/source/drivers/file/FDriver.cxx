/*************************************************************************
 *
 *  $RCSfile: FDriver.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:21 $
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

#ifndef _CONNECTIVITY_FILE_ODRIVER_HXX_
#include "file/FDriver.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_OCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

using namespace connectivity::file;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
// --------------------------------------------------------------------------------
OFileDriver::OFileDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex)
    ,m_xFactory(_rxFactory)
{
}
// --------------------------------------------------------------------------------
void OFileDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);


    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OFileDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdbc.driver.file.Driver");
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OFileDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
        Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
    aSNS[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Driver");
    return aSNS;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL OFileDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL OFileDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
        Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OFileDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OFileDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
                throw DisposedException();

    OConnection* pCon = new OConnection(this);
    pCon->construct(url,info);
        Reference< XConnection > xCon = pCon;
        m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OFileDriver::acceptsURL( const ::rtl::OUString& url )
                throw(SQLException, RuntimeException)
{
    if(!url.compareTo(::rtl::OUString::createFromAscii("sdbc:file:"),10))
    {
        return sal_True;
    }
    return sal_False;
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL OFileDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
        return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OFileDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OFileDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// XDataDefinitionSupplier
Reference< XTablesSupplier > SAL_CALL OFileDriver::getDataDefinitionByConnection( const Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
                throw DisposedException();

    OConnection* pConnection = NULL;
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
                if ((OConnection*) Reference< XConnection >::query(i->get().get()).get() == (OConnection*)connection.get())
        {
            pConnection = (OConnection*)connection.get();
            break;
        }
    }

        Reference< XTablesSupplier > xTab = NULL;
    if(pConnection)
        xTab = pConnection->createCatalog();
    return xTab;
}

//------------------------------------------------------------------
Any SAL_CALL OFileDriver::queryInterface( const Type & rType ) throw(RuntimeException)
{
        Any aRet = ::cppu::queryInterface(rType, static_cast<XDataDefinitionSupplier*>(this));
    if(aRet.hasValue())
       return aRet;
    return ODriver_BASE::queryInterface(rType);
}
// --------------------------------------------------------------------------------
Reference< XTablesSupplier > SAL_CALL OFileDriver::getDataDefinitionByURL( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    return getDataDefinitionByConnection(connect(url,info));
}


