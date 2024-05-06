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

#include <java/sql/Driver.hxx>
#include <java/sql/Connection.hxx>
#include <sal/log.hxx>
#include <connectivity/dbexception.hxx>
#include <jvmfwk/framework.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <strings.hxx>

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

OUString SAL_CALL java_sql_Driver::getImplementationName(  )
{
    return u"com.sun.star.comp.sdbc.JDBCDriver"_ustr;
        // this name is referenced in the configuration and in the jdbc.xml
        // Please take care when changing it.
}

sal_Bool SAL_CALL java_sql_Driver::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}


Sequence< OUString > SAL_CALL java_sql_Driver::getSupportedServiceNames(  )
{
    return { u"com.sun.star.sdbc.Driver"_ustr };
}

Reference< XConnection > SAL_CALL java_sql_Driver::connect( const OUString& url, const
                                                         Sequence< PropertyValue >& info )
{
    m_aLogger.log( LogLevel::INFO, STR_LOG_DRIVER_CONNECTING_URL, url );

    Reference< XConnection > xOut;
    if ( acceptsURL(url ) )
    {
        rtl::Reference<java_sql_Connection> pConnection = new java_sql_Connection( *this );
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
        Sequence< OUString > aBooleanValues{ u"false"_ustr, u"true"_ustr };

        return
        {
            {
                u"JavaDriverClass"_ustr
                ,u"The JDBC driver class name."_ustr
                ,true
                ,OUString()
                ,Sequence< OUString >()
            },
            {
                u"JavaDriverClassPath"_ustr
                ,u"The class path where to look for the JDBC driver."_ustr
                ,true
                , u""_ustr
                ,Sequence< OUString >()
            },
            {
                u"SystemProperties"_ustr
                ,u"Additional properties to set at java.lang.System before loading the driver."_ustr
                ,true
                , u""_ustr
                ,Sequence< OUString >()
            },
            {
                u"ParameterNameSubstitution"_ustr
                ,u"Change named parameters with '?'."_ustr
                ,false
                ,u"false"_ustr
                ,aBooleanValues
            },
            {
                u"IgnoreDriverPrivileges"_ustr
                ,u"Ignore the privileges from the database driver."_ustr
                ,false
                , u"false"_ustr
                ,aBooleanValues
            },
            {
                u"IsAutoRetrievingEnabled"_ustr
                ,u"Retrieve generated values."_ustr
                ,false
                ,u"false"_ustr
                ,aBooleanValues
            },
            {
                u"AutoRetrievingStatement"_ustr
                ,u"Auto-increment statement."_ustr
                ,false
                ,OUString()
                ,Sequence< OUString >()
            },
            {
                u"GenerateASBeforeCorrelationName"_ustr
                ,u"Generate AS before table correlation names."_ustr
                ,false
                ,u"false"_ustr
                ,aBooleanValues
            },
            {
                u"IgnoreCurrency"_ustr
                ,u"Ignore the currency field from the ResultsetMetaData."_ustr
                ,false
                ,u"false"_ustr
                ,aBooleanValues
            },
            {
                u"EscapeDateTime"_ustr
                ,u"Escape date time format."_ustr
                ,false
                ,u"true"_ustr
                ,aBooleanValues
            },
            {
                u"TypeInfoSettings"_ustr
                ,u"Defines how the type info of the database metadata should be manipulated."_ustr
                ,false
                ,OUString()
                ,Sequence< OUString > ()
            },
            {
                u"ImplicitCatalogRestriction"_ustr
                ,u"The catalog which should be used in getTables calls, when the caller passed NULL."_ustr
                ,false
                ,OUString( )
                ,Sequence< OUString > ()
            },
            {
                u"ImplicitSchemaRestriction"_ustr
                ,u"The schema which should be used in getTables calls, when the caller passed NULL."_ustr
                ,false
                ,OUString( )
                ,Sequence< OUString > ()
            }
        };
    }
    ::connectivity::SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
}

sal_Int32 SAL_CALL java_sql_Driver::getMajorVersion(  )
{
    return 1;
}

sal_Int32 SAL_CALL java_sql_Driver::getMinorVersion(  )
{
    return 0;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_java_sql_Driver_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new java_sql_Driver(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
