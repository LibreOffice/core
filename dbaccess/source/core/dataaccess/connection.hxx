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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_CONNECTION_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_CONNECTION_HXX

#include <sal/config.h>

#include <atomic>
#include <cstddef>
#include <map>

#include <apitools.hxx>
#include <querycontainer.hxx>
#include <tablecontainer.hxx>
#include <viewcontainer.hxx>
#include <RefreshListener.hxx>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdb/XCommandPreparation.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XGroupsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/tools/XConnectionTools.hpp>
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>

#include <cppuhelper/implbase13.hxx>
#include <connectivity/ConnectionWrapper.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/warningscontainer.hxx>

namespace dbaccess
{

typedef cppu::ImplHelper13  <   css::container::XChild
                                    ,   css::sdbcx::XTablesSupplier
                                    ,   css::sdbcx::XViewsSupplier
                                    ,   css::sdbc::XConnection
                                    ,   css::sdbc::XWarningsSupplier
                                    ,   css::sdb::XQueriesSupplier
                                    ,   css::sdb::XSQLQueryComposerFactory
                                    ,   css::sdb::XCommandPreparation
                                    ,   css::lang::XMultiServiceFactory
                                    ,   css::sdbcx::XUsersSupplier
                                    ,   css::sdbcx::XGroupsSupplier
                                    ,   css::sdb::tools::XConnectionTools
                                    ,   css::sdb::application::XTableUIProvider
                                    >   OConnection_Base;

class ODatabaseSource;
// OConnection
class OConnection final     :public ::cppu::BaseMutex
                            ,public OSubComponent
                            ,public ::connectivity::OConnectionWrapper
                            ,public OConnection_Base
                            ,public IRefreshListener
{
    css::uno::Reference< css::sdbcx::XTablesSupplier >
                                          m_xMasterTables; // just to avoid the recreation of the catalog
    connectivity::OWeakRefArray           m_aStatements;
    css::uno::Reference< css::container::XNameAccess >
                                          m_xQueries;
    connectivity::OWeakRefArray           m_aComposers;

    // the filter as set on the parent data link at construction of the connection
    css::uno::Sequence< OUString >                                    m_aTableFilter;
    css::uno::Sequence< OUString >                                    m_aTableTypeFilter;
    css::uno::Reference< css::uno::XComponentContext >                m_aContext;
    css::uno::Reference< css::sdbc::XConnection >                     m_xMasterConnection;
    css::uno::Reference< css::sdb::tools::XConnectionTools >          m_xConnectionTools;
    css::uno::Reference< css::sdb::application::XTableUIProvider >    m_xTableUIProvider;

    // defines the helper services for example to query the command of a view
    // @ see com.sun.star.sdb.tools.XViewAccess
    typedef std::map< OUString, css::uno::Reference< css::uno::XInterface> > TSupportServices;
    TSupportServices                m_aSupportServices;

    std::unique_ptr<OTableContainer> m_pTables;
    std::unique_ptr<OViewContainer>  m_pViews;
    ::dbtools::WarningsContainer    m_aWarnings;
    std::atomic<std::size_t>        m_nInAppend;
    bool                            m_bSupportsViews;       // true when the getTableTypes return "VIEW" as type
    bool                            m_bSupportsUsers;
    bool                            m_bSupportsGroups;

    virtual ~OConnection() override;
public:
    OConnection(ODatabaseSource& _rDB
                ,css::uno::Reference< css::sdbc::XConnection > const & _rxMaster
                ,const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

// css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

// css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
        virtual void SAL_CALL acquire() throw( ) override;
    virtual void SAL_CALL release() throw( ) override;

// OComponentHelper
    virtual void SAL_CALL disposing() override;

// css::container::XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

// css::sdbcx::XTablesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTables(  ) override;
// css::sdbcx::XViewsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getViews(  ) override;

// css::sdb::XQueriesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getQueries(  ) override;

// css::sdb::XSQLQueryComposerFactory
    virtual css::uno::Reference< css::sdb::XSQLQueryComposer > SAL_CALL createQueryComposer(  ) override;

// css::sdb::XCommandPreparation
    virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCommand( const OUString& command, sal_Int32 commandType ) override;

// css::sdbc::XWarningsSupplier
    virtual css::uno::Any SAL_CALL getWarnings(  ) override;
    virtual void SAL_CALL clearWarnings(  ) override;

// css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

// XConnection
    virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) override;
    virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) override;
    virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) override;
    virtual OUString SAL_CALL nativeSQL( const OUString& sql ) override;
    virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) override;
    virtual sal_Bool SAL_CALL getAutoCommit(  ) override;
    virtual void SAL_CALL commit(  ) override;
    virtual void SAL_CALL rollback(  ) override;
    virtual sal_Bool SAL_CALL isClosed(  ) override;
    virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) override;
    virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) override;
    virtual sal_Bool SAL_CALL isReadOnly(  ) override;
    virtual void SAL_CALL setCatalog( const OUString& catalog ) override;
    virtual OUString SAL_CALL getCatalog(  ) override;
    virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) override;
    virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) override;
    virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;

// css::sdbc::XCloseable
    virtual void SAL_CALL close(  ) override;

    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;

    // XUsersSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getUsers(  ) override;
    // XGroupsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getGroups(  ) override;

    // XConnectionTools
    virtual css::uno::Reference< css::sdb::tools::XTableName > SAL_CALL createTableName(  ) override;
    virtual css::uno::Reference< css::sdb::tools::XObjectNames > SAL_CALL getObjectNames(  ) override;
    virtual css::uno::Reference< css::sdb::tools::XDataSourceMetaData > SAL_CALL getDataSourceMetaData(  ) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getFieldsByCommandDescriptor( ::sal_Int32 commandType, const OUString& command, css::uno::Reference< css::lang::XComponent >& keepFieldsAlive ) override;
    virtual css::uno::Reference< css::sdb::XSingleSelectQueryComposer > SAL_CALL getComposer( ::sal_Int32 commandType, const OUString& command ) override;

    // XTableUIProvider
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL getTableIcon( const OUString& TableName, ::sal_Int32 ColorMode ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getTableEditor( const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& DocumentUI, const OUString& TableName ) override;

    // IRefreshListener
    virtual void refresh(const css::uno::Reference< css::container::XNameAccess >& _rToBeRefreshed) override;

private:
    /// @throws css::lang::DisposedException
    void checkDisposed()
    {
        if ( rBHelper.bDisposed || !m_xConnection.is() )
            throw css::lang::DisposedException();
    }

    css::uno::Reference< css::sdbcx::XTablesSupplier > const & getMasterTables();

    /** checks whether or not there are naming conflicts between tables and queries
    */
    void    impl_checkTableQueryNames_nothrow();

    /** loads the XConnectionTools implementation which we forward the respective functionality to

        @throws css::uno::RuntimeException
            if the implementation cannot be loaded

        @postcond
            m_xConnectionTools is nol <NULL/>
    */
    void    impl_loadConnectionTools_throw();

    /** reads the table filter and table type filter from the datasource
    */
    void    impl_fillTableFilter();
};

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_CONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
