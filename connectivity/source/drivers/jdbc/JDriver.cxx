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

#include "java/sql/Driver.hxx"
#include "java/lang/Object.hxx"
#include "java/lang/Class.hxx"
#include "java/sql/DriverPropertyInfo.hxx"
#include "java/sql/Connection.hxx"
#include "java/util/Property.hxx"
#include "java/tools.hxx"
#include <connectivity/dbexception.hxx>
#include <jvmfwk/framework.hxx>
#include "resource/conn_shared_res.hrc"
#include "strings.hrc"
#include "resource/sharedresources.hxx"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "strings.hxx"

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


java_sql_Driver::java_sql_Driver(const Reference< css::uno::XComponentContext >& _rxContext)
    :m_aContext( _rxContext )
    ,m_aLogger( _rxContext, "org.openoffice.sdbc.jdbcBridge" )
{
}

java_sql_Driver::~java_sql_Driver()
{
}

// static ServiceInfo

OUString java_sql_Driver::getImplementationName_Static(  )
{
    return OUString("com.sun.star.comp.sdbc.JDBCDriver");
        // this name is referenced in the configuration and in the jdbc.xml
        // Please take care when changing it.
}

Sequence< OUString > java_sql_Driver::getSupportedServiceNames_Static(  )
{
    Sequence<OUString> aSNS { "com.sun.star.sdbc.Driver" };
    return aSNS;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL connectivity::java_sql_Driver_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory)
{
    return *(new java_sql_Driver( comphelper::getComponentContext(_rxFactory)));
}

OUString SAL_CALL java_sql_Driver::getImplementationName(  )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL java_sql_Driver::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}


Sequence< OUString > SAL_CALL java_sql_Driver::getSupportedServiceNames(  )
{
    return getSupportedServiceNames_Static();
}

Reference< XConnection > SAL_CALL java_sql_Driver::connect( const OUString& url, const
                                                         Sequence< PropertyValue >& info )
{
    m_aLogger.log( LogLevel::INFO, STR_LOG_DRIVER_CONNECTING_URL, url );

    Reference< XConnection > xOut;
    if ( acceptsURL(url ) )
    {
        java_sql_Connection* pConnection = new java_sql_Connection( *this );
        xOut = pConnection;
        if ( !pConnection->construct(url,info) )
            xOut.clear(); // an error occurred and the java driver didn't throw an exception
        else
            m_aLogger.log( LogLevel::INFO, STR_LOG_DRIVER_SUCCESS );
    }
    return xOut;
}

sal_Bool SAL_CALL java_sql_Driver::acceptsURL( const OUString& url )
{
    // don't ask the real driver for the url
    // I feel responsible for all jdbc url's
    bool bEnabled = false;
    javaFrameworkError e = jfw_getEnabled(&bEnabled);
    switch (e) {
    case JFW_E_NONE:
        break;
    case JFW_E_DIRECT_MODE:
        SAL_INFO(
            "connectivity.jdbc",
            "jfw_getEnabled: JFW_E_DIRECT_MODE, assuming true");
        bEnabled = true;
        break;
    default:
        SAL_WARN("connectivity.jdbc", "jfw_getEnabled: error code " << +e);
        break;
    }
    return bEnabled && url.startsWith("jdbc:");
}

Sequence< DriverPropertyInfo > SAL_CALL java_sql_Driver::getPropertyInfo( const OUString& url,
                                                                         const Sequence< PropertyValue >& /*info*/ )
{
    if ( acceptsURL(url) )
    {
        std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< OUString > aBooleanValues(2);
        aBooleanValues[0] = "false";
        aBooleanValues[1] = "true";

        aDriverInfo.push_back(DriverPropertyInfo(
                "JavaDriverClass"
                ,"The JDBC driver class name."
                ,true
                ,OUString()
                ,Sequence< OUString >())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                "JavaDriverClassPath"
                ,"The class path where to look for the JDBC driver."
                ,true
                , ""
                ,Sequence< OUString >())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                "SystemProperties"
                ,"Additional properties to set at java.lang.System before loading the driver."
                ,true
                , ""
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
                , "false"
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
                "IgnoreCurrency"
                ,"Ignore the currency field from the ResultsetMetaData."
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
        aDriverInfo.push_back(DriverPropertyInfo(
                "TypeInfoSettings"
                ,"Defines how the type info of the database metadata should be manipulated."
                ,false
                ,OUString( )
                ,Sequence< OUString > ())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                "ImplicitCatalogRestriction"
                ,"The catalog which should be used in getTables calls, when the caller passed NULL."
                ,false
                ,OUString( )
                ,Sequence< OUString > ())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                "ImplicitSchemaRestriction"
                ,"The schema which should be used in getTables calls, when the caller passed NULL."
                ,false
                ,OUString( )
                ,Sequence< OUString > ())
        );
        return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
    }
    ::connectivity::SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL java_sql_Driver::getMajorVersion(  )
{
    return 1;
}

sal_Int32 SAL_CALL java_sql_Driver::getMinorVersion(  )
{
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
