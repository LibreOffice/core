/*************************************************************************
 *
 *  $RCSfile: ODriver.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2002-08-01 06:58:56 $
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

#ifndef _CONNECTIVITY_ODBC_ODRIVER_HXX_
#include "odbc/ODriver.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OCONNECTION_HXX_
#include "odbc/OConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_OFUNCTIONS_HXX_
#include "odbc/OFunctions.hxx"
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
ODBCDriver::ODBCDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex)
    ,m_pDriverHandle(SQL_NULL_HANDLE)
    ,m_xORB(_rxFactory)
{
}
// --------------------------------------------------------------------------------
void ODBCDriver::disposing()
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
rtl::OUString ODBCDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.sdbc.ODBCDriver");
        // this name is referenced in the configuration and in the odbc.xml
        // Please take care when changing it.
}

typedef Sequence< ::rtl::OUString > SS;
//------------------------------------------------------------------------------
SS ODBCDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    SS aSNS( 1 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
    return aSNS;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL ODBCDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL ODBCDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    SS aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//------------------------------------------------------------------
SS SAL_CALL ODBCDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODBCDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);

    if(!m_pDriverHandle)
    {
        ::rtl::OUString aPath;
        if(!EnvironmentHandle(aPath))
            throw SQLException(aPath,*this,::rtl::OUString(),1000,Any());
    }
    OConnection* pCon = new OConnection(m_pDriverHandle,this);
    Reference< XConnection > xCon = pCon;
    pCon->Construct(url,info);
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL ODBCDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    return (!url.compareTo(::rtl::OUString::createFromAscii("sdbc:odbc:"),10));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL ODBCDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODBCDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODBCDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// --------------------------------------------------------------------------------
//-----------------------------------------------------------------------------


