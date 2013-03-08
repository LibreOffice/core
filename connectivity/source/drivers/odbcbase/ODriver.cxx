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

#include "odbc/ODriver.hxx"
#include "odbc/OConnection.hxx"
#include "odbc/OFunctions.hxx"
#include "odbc/OTools.hxx"
#include "connectivity/dbexception.hxx"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
ODBCDriver::ODBCDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    :ODriver_BASE(m_aMutex)
    ,m_xORB(_rxFactory)
    ,m_pDriverHandle(SQL_NULL_HANDLE)
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
    return rtl::OUString("com.sun.star.comp.sdbc.ODBCDriver");
        // this name is referenced in the configuration and in the odbc.xml
        // Please take care when changing it.
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODBCDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString("com.sun.star.sdbc.Driver");
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
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODBCDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODBCDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        return NULL;

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
    return url.startsWith("sdbc:odbc:");
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL ODBCDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< ::rtl::OUString > aBooleanValues(2);
        aBooleanValues[0] = ::rtl::OUString(  "false"  );
        aBooleanValues[1] = ::rtl::OUString(  "true"  );

        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("CharSet")
                ,::rtl::OUString("CharSet of the database.")
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("UseCatalog")
                ,::rtl::OUString("Use catalog for file-based databases.")
                ,sal_False
                ,::rtl::OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("SystemDriverSettings")
                ,::rtl::OUString("Driver settings.")
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("ParameterNameSubstitution")
                ,::rtl::OUString("Change named parameters with '?'.")
                ,sal_False
                ,::rtl::OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("IgnoreDriverPrivileges")
                ,::rtl::OUString("Ignore the privileges from the database driver.")
                ,sal_False
                ,::rtl::OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("IsAutoRetrievingEnabled")
                ,::rtl::OUString("Retrieve generated values.")
                ,sal_False
                ,::rtl::OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("AutoRetrievingStatement")
                ,::rtl::OUString("Auto-increment statement.")
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("GenerateASBeforeCorrelationName")
                ,::rtl::OUString("Generate AS before table correlation names.")
                ,sal_False
                ,::rtl::OUString(  "true"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString("EscapeDateTime")
                ,::rtl::OUString("Escape date time format.")
                ,sal_False
                ,::rtl::OUString(  "true"  )
                ,aBooleanValues)
                );

        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }
    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
