/*************************************************************************
 *
 *  $RCSfile: ODriver.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:23 $
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

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
ODBCDriver::ODBCDriver() : ODriver_BASE(m_aMutex),m_pDriverHandle(SQL_NULL_HANDLE)
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
    return rtl::OUString::createFromAscii("com.sun.star.sdbc.ODBCDriver");
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODBCDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
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
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODBCDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL connectivity::odbc::ODBCDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new ODBCDriver());
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODBCDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if(!m_pDriverHandle)
    {
        ::rtl::OUString aPath;
        if(!EnvironmentHandle(aPath))
            throw SQLException(aPath,*this,::rtl::OUString(),1000,Any());
    }
    OConnection* pCon = new OConnection(m_pDriverHandle,this);
    pCon->Construct(url,info);
    Reference< XConnection > xCon = pCon;
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL ODBCDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    if(!url.compareTo(::rtl::OUString::createFromAscii("sdbc:odbc:"),10))
    {
        return sal_True;
    }
    return sal_False;
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
// ODBC Environment (gemeinsam fuer alle Connections):
SQLHANDLE ODBCDriver::EnvironmentHandle(::rtl::OUString &_rPath)
{
    // Ist (fuer diese Instanz) bereits ein Environment erzeugt worden?
    if (!m_pDriverHandle)
    {
        SQLHANDLE h = SQL_NULL_HANDLE;
        // Environment allozieren

        // ODBC-DLL jetzt laden:
        if (!LoadLibrary_ODBC3(_rPath))
            return SQL_NULL_HANDLE;

        if (N3SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&h) != SQL_SUCCESS)
            return SQL_NULL_HANDLE;

        // In globaler Struktur merken ...
        m_pDriverHandle = (void *) h;
        SQLRETURN nError = N3SQLSetEnvAttr(h, SQL_ATTR_ODBC_VERSION,(SQLPOINTER) SQL_OV_ODBC3, SQL_IS_UINTEGER);
        //N3SQLSetEnvAttr(h, SQL_ATTR_CONNECTION_POOLING,(SQLPOINTER) SQL_CP_ONE_PER_HENV, SQL_IS_INTEGER);
    }

    return m_pDriverHandle;
}


