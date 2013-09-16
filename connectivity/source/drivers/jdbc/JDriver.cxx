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
#include "connectivity/dbexception.hxx"
#include <jvmfwk/framework.h>
#include "diagnose_ex.h"
#include "resource/jdbc_log.hrc"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"
#include <comphelper/processfactory.hxx>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
java_sql_Driver::java_sql_Driver(const Reference< ::com::sun::star::uno::XComponentContext >& _rxContext)
    :m_aContext( _rxContext )
    ,m_aLogger( _rxContext, "sdbcl", "org.openoffice.sdbc.jdbcBridge" )
{
}
// --------------------------------------------------------------------------------
java_sql_Driver::~java_sql_Driver()
{
}

// static ServiceInfo
//------------------------------------------------------------------------------
OUString java_sql_Driver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.sdbc.JDBCDriver");
        // this name is referenced in the configuration and in the jdbc.xml
        // Please take care when changing it.
}
//------------------------------------------------------------------------------
Sequence< OUString > java_sql_Driver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = OUString("com.sun.star.sdbc.Driver");
    return aSNS;
}
//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL connectivity::java_sql_Driver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new java_sql_Driver( comphelper::getComponentContext(_rxFactory)));
}
// --------------------------------------------------------------------------------
OUString SAL_CALL java_sql_Driver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Driver::supportsService( const OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

// --------------------------------------------------------------------------------
Sequence< OUString > SAL_CALL java_sql_Driver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL java_sql_Driver::connect( const OUString& url, const
                                                         Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Driver::acceptsURL( const OUString& url ) throw(SQLException, RuntimeException)
{
    // don't ask the real driver for the url
    // I feel responsible for all jdbc url's
    sal_Bool bEnabled = sal_False;
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
// -------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL java_sql_Driver::getPropertyInfo( const OUString& url,
                                                                         const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< OUString > aBooleanValues(2);
        aBooleanValues[0] = OUString(  "false"  );
        aBooleanValues[1] = OUString(  "true"  );

        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("JavaDriverClass")
                ,OUString("The JDBC driver class name.")
                ,sal_True
                ,OUString()
                ,Sequence< OUString >())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("JavaDriverClassPath")
                ,OUString("The class path where to look for the JDBC driver.")
                ,sal_True
                ,OUString(  ""  )
                ,Sequence< OUString >())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("SystemProperties")
                ,OUString("Additional properties to set at java.lang.System before loading the driver.")
                ,sal_True
                ,OUString(  ""  )
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
                ,OUString(  "true"  )
                ,aBooleanValues)
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("IgnoreCurrency")
                ,OUString("Ignore the currency field from the ResultsetMetaData.")
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
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("TypeInfoSettings")
                ,OUString("Defines how the type info of the database metadata should be manipulated.")
                ,sal_False
                ,OUString( )
                ,Sequence< OUString > ())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("ImplicitCatalogRestriction")
                ,OUString("The catalog which should be used in getTables calls, when the caller passed NULL.")
                ,sal_False
                ,OUString( )
                ,Sequence< OUString > ())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("ImplicitSchemaRestriction")
                ,OUString("The schema which should be used in getTables calls, when the caller passed NULL.")
                ,sal_False
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
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Driver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Driver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
