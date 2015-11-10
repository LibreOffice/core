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
#include <connectivity/dbexception.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

ODBCDriver::ODBCDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    :ODriver_BASE(m_aMutex)
    ,m_xORB(_rxFactory)
    ,m_pDriverHandle(SQL_NULL_HANDLE)
{
}

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

OUString ODBCDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.sdbc.ODBCDriver");
        // this name is referenced in the configuration and in the odbc.xml
        // Please take care when changing it.
}


Sequence< OUString > ODBCDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = "com.sun.star.sdbc.Driver";
    return aSNS;
}


OUString SAL_CALL ODBCDriver::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ODBCDriver::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}


Sequence< OUString > SAL_CALL ODBCDriver::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}


Reference< XConnection > SAL_CALL ODBCDriver::connect( const OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException, std::exception)
{
    if ( ! acceptsURL(url) )
        return nullptr;

    if(!m_pDriverHandle)
    {
        OUString aPath;
        if(!EnvironmentHandle(aPath))
            throw SQLException(aPath,*this,OUString(),1000,Any());
    }
    OConnection* pCon = new OConnection(m_pDriverHandle,this);
    Reference< XConnection > xCon = pCon;
    pCon->Construct(url,info);
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}

sal_Bool SAL_CALL ODBCDriver::acceptsURL( const OUString& url )
        throw(SQLException, RuntimeException, std::exception)
{
    return url.startsWith("sdbc:odbc:");
}

Sequence< DriverPropertyInfo > SAL_CALL ODBCDriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException, std::exception)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< OUString > aBooleanValues(2);
        aBooleanValues[0] = "false";
        aBooleanValues[1] = "true";

        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("CharSet")
                ,OUString("CharSet of the database.")
                ,sal_False
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("UseCatalog")
                ,OUString("Use catalog for file-based databases.")
                ,sal_False
                ,OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("SystemDriverSettings")
                ,OUString("Driver settings.")
                ,sal_False
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("ParameterNameSubstitution")
                ,OUString("Change named parameters with '?'.")
                ,sal_False
                ,OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("IgnoreDriverPrivileges")
                ,OUString("Ignore the privileges from the database driver.")
                ,sal_False
                ,OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("IsAutoRetrievingEnabled")
                ,OUString("Retrieve generated values.")
                ,sal_False
                ,OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("AutoRetrievingStatement")
                ,OUString("Auto-increment statement.")
                ,sal_False
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("GenerateASBeforeCorrelationName")
                ,OUString("Generate AS before table correlation names.")
                ,sal_False
                ,OUString(  "false"  )
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("EscapeDateTime")
                ,OUString("Escape date time format.")
                ,sal_False
                ,OUString(  "true"  )
                ,aBooleanValues)
                );

        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }
    ::connectivity::SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL ODBCDriver::getMajorVersion(  ) throw(RuntimeException, std::exception)
{
    return 1;
}

sal_Int32 SAL_CALL ODBCDriver::getMinorVersion(  ) throw(RuntimeException, std::exception)
{
    return 0;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
