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


#include <connectivity/dbmetadata.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/DriversConfig.hxx>
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/sdb/BooleanComparisonMode.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData2.hpp>
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>

#include <tools/diagnose_ex.h>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <sal/macros.h>

#include <boost/optional.hpp>


namespace dbtools
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::sdbc::XDatabaseMetaData2;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::sdbcx::XUsersSupplier;
    using ::com::sun::star::sdbcx::XDataDefinitionSupplier;
    using ::com::sun::star::sdbc::XDriverAccess;
    using ::com::sun::star::sdbc::DriverManager;
    using ::com::sun::star::sdbc::XDriverManager2;
    using ::com::sun::star::uno::UNO_SET_THROW;

    namespace BooleanComparisonMode = ::com::sun::star::sdb::BooleanComparisonMode;

    struct DatabaseMetaData_Impl
    {
        Reference< XConnection >        xConnection;
        Reference< XDatabaseMetaData >  xConnectionMetaData;
        ::connectivity::DriversConfig   aDriverConfig;

        ::boost::optional< OUString >    sCachedIdentifierQuoteString;
        ::boost::optional< OUString >    sCachedCatalogSeparator;

        DatabaseMetaData_Impl()
            :xConnection()
            ,xConnectionMetaData()
            ,aDriverConfig( ::comphelper::getProcessComponentContext() )
            ,sCachedIdentifierQuoteString()
            ,sCachedCatalogSeparator()
        {
        }
    };


    namespace
    {

        static void lcl_construct( DatabaseMetaData_Impl& _metaDataImpl, const Reference< XConnection >& _connection )
        {
            _metaDataImpl.xConnection = _connection;
            if ( !_metaDataImpl.xConnection.is() )
                return;

            _metaDataImpl.xConnectionMetaData = _connection->getMetaData();
            if ( !_metaDataImpl.xConnectionMetaData.is() )
                throw IllegalArgumentException();
        }


        static void lcl_checkConnected( const DatabaseMetaData_Impl& _metaDataImpl )
        {
            if ( !_metaDataImpl.xConnection.is() || !_metaDataImpl.xConnectionMetaData.is() )
            {
                ::connectivity::SharedResources aResources;
                const OUString sError( aResources.getResourceString(STR_NO_CONNECTION_GIVEN));
                throwSQLException( sError, SQL_CONNECTION_DOES_NOT_EXIST, nullptr );
            }
        }


        static bool lcl_getDriverSetting( const sal_Char* _asciiName, const DatabaseMetaData_Impl& _metaData, Any& _out_setting )
        {
            lcl_checkConnected( _metaData );
            const ::comphelper::NamedValueCollection& rDriverMetaData = _metaData.aDriverConfig.getMetaData( _metaData.xConnectionMetaData->getURL() );
            if ( !rDriverMetaData.has( _asciiName ) )
                return false;
            _out_setting = rDriverMetaData.get( _asciiName );
            return true;
        }


        static bool lcl_getConnectionSetting( const sal_Char* _asciiName, const DatabaseMetaData_Impl& _metaData, Any& _out_setting )
        {
            try
            {
                Reference< XChild > xConnectionAsChild( _metaData.xConnection, UNO_QUERY );
                if ( xConnectionAsChild.is() )
                {
                    Reference< XPropertySet > xDataSource( xConnectionAsChild->getParent(), UNO_QUERY_THROW );
                    Reference< XPropertySet > xDataSourceSettings(
                        xDataSource->getPropertyValue("Settings"),
                        UNO_QUERY_THROW );

                    _out_setting = xDataSourceSettings->getPropertyValue( OUString::createFromAscii( _asciiName ) );
                }
                else
                {
                    Reference< XDatabaseMetaData2 > xExtendedMetaData( _metaData.xConnectionMetaData, UNO_QUERY_THROW );
                    ::comphelper::NamedValueCollection aSettings( xExtendedMetaData->getConnectionInfo() );
                    _out_setting = aSettings.get( _asciiName );
                    return _out_setting.hasValue();
                }
                return true;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return false;
        }


        static const OUString& lcl_getConnectionStringSetting(
            const DatabaseMetaData_Impl& _metaData, ::boost::optional< OUString >& _cachedSetting,
            OUString (SAL_CALL XDatabaseMetaData::*_getter)() )
        {
            if ( !_cachedSetting )
            {
                lcl_checkConnected( _metaData );
                try
                {
                    _cachedSetting.reset( (_metaData.xConnectionMetaData.get()->*_getter)() );
                }
                catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
            }
            return *_cachedSetting;
        }
    }

    DatabaseMetaData::DatabaseMetaData()
        :m_pImpl( new DatabaseMetaData_Impl )
    {
    }


    DatabaseMetaData::DatabaseMetaData( const Reference< XConnection >& _connection )
        :m_pImpl( new DatabaseMetaData_Impl )
    {
        lcl_construct( *m_pImpl, _connection );
    }


    DatabaseMetaData::DatabaseMetaData( const DatabaseMetaData& _copyFrom )
        :m_pImpl( new DatabaseMetaData_Impl( *_copyFrom.m_pImpl ) )
    {
    }


    DatabaseMetaData& DatabaseMetaData::operator=( const DatabaseMetaData& _copyFrom )
    {
        if ( this == &_copyFrom )
            return *this;

        m_pImpl.reset( new DatabaseMetaData_Impl( *_copyFrom.m_pImpl ) );
        return *this;
    }


    DatabaseMetaData::~DatabaseMetaData()
    {
    }


    bool DatabaseMetaData::isConnected() const
    {
        return m_pImpl->xConnection.is();
    }


    bool DatabaseMetaData::supportsSubqueriesInFrom() const
    {
        lcl_checkConnected( *m_pImpl );

        bool bSupportsSubQueries = false;
        try
        {
            sal_Int32 maxTablesInselect = m_pImpl->xConnectionMetaData->getMaxTablesInSelect();
            bSupportsSubQueries = ( maxTablesInselect > 1 ) || ( maxTablesInselect == 0 );
            // TODO: is there a better way to determine this? The above is not really true. More precise,
            // it's a *very* generous heuristics ...
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bSupportsSubQueries;
    }


    bool DatabaseMetaData::supportsPrimaryKeys() const
    {
        lcl_checkConnected( *m_pImpl );

        bool bDoesSupportPrimaryKeys = false;
        try
        {
            Any setting;
            if  (   !( lcl_getConnectionSetting( "PrimaryKeySupport", *m_pImpl, setting ) )
                ||  !( setting >>= bDoesSupportPrimaryKeys )
                )
                bDoesSupportPrimaryKeys = m_pImpl->xConnectionMetaData->supportsCoreSQLGrammar()
                    || m_pImpl->xConnectionMetaData->supportsANSI92EntryLevelSQL();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bDoesSupportPrimaryKeys;
    }


    const OUString&  DatabaseMetaData::getIdentifierQuoteString() const
    {
        return lcl_getConnectionStringSetting( *m_pImpl, m_pImpl->sCachedIdentifierQuoteString, &XDatabaseMetaData::getIdentifierQuoteString );
    }


    const OUString&  DatabaseMetaData::getCatalogSeparator() const
    {
        return lcl_getConnectionStringSetting( *m_pImpl, m_pImpl->sCachedCatalogSeparator, &XDatabaseMetaData::getCatalogSeparator );
    }


    bool DatabaseMetaData::restrictIdentifiersToSQL92() const
    {
        lcl_checkConnected( *m_pImpl );

        bool restrict( false );
        Any setting;
        if ( lcl_getConnectionSetting( "EnableSQL92Check", *m_pImpl, setting ) )
            OSL_VERIFY( setting >>= restrict );
        return restrict;
    }


    bool DatabaseMetaData::generateASBeforeCorrelationName() const
    {
        bool doGenerate( false );
        Any setting;
        if ( lcl_getConnectionSetting( "GenerateASBeforeCorrelationName", *m_pImpl, setting ) )
            OSL_VERIFY( setting >>= doGenerate );
        return doGenerate;
    }

    bool DatabaseMetaData::shouldEscapeDateTime() const
    {
        bool doGenerate( true );
        Any setting;
        if ( lcl_getConnectionSetting( "EscapeDateTime", *m_pImpl, setting ) )
            OSL_VERIFY( setting >>= doGenerate );
        return doGenerate;
    }

    bool DatabaseMetaData::isAutoIncrementPrimaryKey() const
    {
        bool is( true );
        Any setting;
        if ( lcl_getDriverSetting( "AutoIncrementIsPrimaryKey", *m_pImpl, setting ) )
            OSL_VERIFY( setting >>= is );
        return is;
    }

    sal_Int32 DatabaseMetaData::getBooleanComparisonMode() const
    {
        sal_Int32 mode( BooleanComparisonMode::EQUAL_INTEGER );
        Any setting;
        if ( lcl_getConnectionSetting( "BooleanComparisonMode", *m_pImpl, setting ) )
            OSL_VERIFY( setting >>= mode );
        return mode;
    }

    bool DatabaseMetaData::supportsRelations() const
    {
        lcl_checkConnected( *m_pImpl );
        bool bSupport = false;
        try
        {
            bSupport = m_pImpl->xConnectionMetaData->supportsIntegrityEnhancementFacility();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        try
        {
            if ( !bSupport )
            {
                const OUString url = m_pImpl->xConnectionMetaData->getURL();
                char pMySQL[] = "sdbc:mysql";
                bSupport = url.matchAsciiL(pMySQL,(sizeof(pMySQL)/sizeof(pMySQL[0]))-1);
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bSupport;
    }


    bool DatabaseMetaData::supportsColumnAliasInOrderBy() const
    {
        bool doGenerate( true );
        Any setting;
        if ( lcl_getConnectionSetting( "ColumnAliasInOrderBy", *m_pImpl, setting ) )
            OSL_VERIFY( setting >>= doGenerate );
        return doGenerate;
    }


    bool DatabaseMetaData::supportsUserAdministration( const Reference<XComponentContext>& _rContext ) const
    {
        lcl_checkConnected( *m_pImpl  );

        bool isSupported( false );
        try
        {
            // find the XUsersSupplier interface
            // - either directly at the connection
            Reference< XUsersSupplier > xUsersSupp( m_pImpl->xConnection, UNO_QUERY );
            if ( !xUsersSupp.is() )
            {
                // - or at the driver manager
                Reference< XDriverManager2 > xDriverManager = DriverManager::create( _rContext );
                Reference< XDataDefinitionSupplier > xDriver( xDriverManager->getDriverByURL( m_pImpl->xConnectionMetaData->getURL() ), UNO_QUERY );
                if ( xDriver.is() )
                    xUsersSupp.set( xDriver->getDataDefinitionByConnection( m_pImpl->xConnection ), UNO_QUERY );
            }

            isSupported = ( xUsersSupp.is() && xUsersSupp->getUsers().is() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return isSupported;
    }


    bool DatabaseMetaData::displayEmptyTableFolders() const
    {
        bool doDisplay( true );
#ifdef IMPLEMENTED_LATER
        Any setting;
        if ( lcl_getConnectionSetting( "DisplayEmptyTableFolders", *m_pImpl, setting ) )
            OSL_VERIFY( setting >>= doDisplay );
#else
        try
        {
            Reference< XDatabaseMetaData > xMeta( m_pImpl->xConnectionMetaData, UNO_SET_THROW );
            OUString sConnectionURL( xMeta->getURL() );
            doDisplay = sConnectionURL.startsWith( "sdbc:mysql:mysqlc" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
#endif
        return doDisplay;
    }

    bool DatabaseMetaData::supportsThreads() const
    {
        bool bSupported( true );
        try
        {
            Reference< XDatabaseMetaData > xMeta( m_pImpl->xConnectionMetaData, UNO_SET_THROW );
            OUString sConnectionURL( xMeta->getURL() );
            bSupported = !sConnectionURL.startsWith( "sdbc:mysql:mysqlc" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bSupported;
    }


} // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
