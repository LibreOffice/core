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

#include <odbc/ODriver.hxx>
#include <odbc/OConnection.hxx>
#include <odbc/OTools.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

ODBCDriver::ODBCDriver(const css::uno::Reference< css::uno::XComponentContext >& _rxContext)
    :ODriver_BASE(m_aMutex)
    ,m_xContext(_rxContext)
    ,m_pDriverHandle(SQL_NULL_HANDLE)
{
}

void ODBCDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);


    for (auto const& connection : m_xConnections)
    {
        Reference< XComponent > xComp(connection.get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}

// static ServiceInfo

OUString ODBCDriver::getImplementationName(  )
{
    return "com.sun.star.comp.sdbc.ODBCDriver";
        // this name is referenced in the configuration and in the odbc.xml
        // Please take care when changing it.
}


Sequence< OUString > ODBCDriver::getSupportedServiceNames(  )
{
    return { "com.sun.star.sdbc.Driver" };
}


sal_Bool SAL_CALL ODBCDriver::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}


Reference< XConnection > SAL_CALL ODBCDriver::connect( const OUString& url, const Sequence< PropertyValue >& info )
{
    if ( ! acceptsURL(url) )
        return nullptr;

    if(!m_pDriverHandle)
    {
        OUString aPath;
        if(!EnvironmentHandle(aPath))
            throw SQLException(aPath,*this,OUString(),1000,Any());
    }
    rtl::Reference<OConnection> pCon = new OConnection(m_pDriverHandle,this);
    pCon->Construct(url,info);
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return pCon;
}

sal_Bool SAL_CALL ODBCDriver::acceptsURL( const OUString& url )
{
    return url.startsWith("sdbc:odbc:");
}

Sequence< DriverPropertyInfo > SAL_CALL ODBCDriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ )
{
    if ( acceptsURL(url) )
    {
        std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< OUString > aBooleanValues(2);
        aBooleanValues[0] = "false";
        aBooleanValues[1] = "true";

        aDriverInfo.push_back(DriverPropertyInfo(
                "CharSet"
                ,"CharSet of the database."
                ,false
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "UseCatalog"
                ,"Use catalog for file-based databases."
                ,false
                ,"false"
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "SystemDriverSettings"
                ,"Driver settings."
                ,false
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "ParameterNameSubstitution"
                ,"Change named parameters with '?'."
                ,false
                ,"false"
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "IgnoreDriverPrivileges"
                ,"Ignore the privileges from the database driver."
                ,false
                ,"false"
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "IsAutoRetrievingEnabled"
                ,"Retrieve generated values."
                ,false
                ,"false"
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "AutoRetrievingStatement"
                ,"Auto-increment statement."
                ,false
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "GenerateASBeforeCorrelationName"
                ,"Generate AS before table correlation names."
                ,false
                ,"false"
                ,aBooleanValues)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "EscapeDateTime"
                ,"Escape date time format."
                ,false
                ,"true"
                ,aBooleanValues)
                );

        return Sequence< DriverPropertyInfo >(aDriverInfo.data(),aDriverInfo.size());
    }
    ::connectivity::SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL ODBCDriver::getMajorVersion(  )
{
    return 1;
}

sal_Int32 SAL_CALL ODBCDriver::getMinorVersion(  )
{
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
