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

#include <map>

#include "apitools.hxx"
#include "querycontainer.hxx"
#include "tablecontainer.hxx"
#include "viewcontainer.hxx"
#include "RefreshListener.hxx"

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

#include <cppuhelper/implbase.hxx>
#include <comphelper/stl_types.hxx>
#include <connectivity/ConnectionWrapper.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/warningscontainer.hxx>

namespace dbaccess
{

typedef cppu::ImplHelper  <   css::container::XChild
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
class OConnection           :public ::cppu::BaseMutex
                            ,public OSubComponent
                            ,public ::connectivity::OConnectionWrapper
                            ,public OConnection_Base
                            ,public IRefreshListener
{
protected:
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

    OTableContainer*                m_pTables;
    OViewContainer*                 m_pViews;
    ::dbtools::WarningsContainer    m_aWarnings;
    oslInterlockedCount             m_nInAppend;
    bool                            m_bSupportsViews;       // true when the getTableTypes return "VIEW" as type
    bool                            m_bSupportsUsers;
    bool                            m_bSupportsGroups;

protected:
    virtual ~OConnection() override;
public:
    OConnection(ODatabaseSource& _rDB
                ,css::uno::Reference< css::sdbc::XConnection >& _rxMaster
                ,const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

// css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;

// css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw( ) override;
    virtual void SAL_CALL release() throw( ) override;

// OComponentHelper
    virtual void SAL_CALL disposing() override;

// css::container::XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw(css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

// css::sdbcx::XTablesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTables(  ) throw(css::uno::RuntimeException, std::exception) override;
// css::sdbcx::XViewsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getViews(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::sdb::XQueriesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getQueries(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::sdb::XSQLQueryComposerFactory
    virtual css::uno::Reference< css::sdb::XSQLQueryComposer > SAL_CALL createQueryComposer(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::sdb::XCommandPreparation
    virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCommand( const OUString& command, sal_Int32 commandType ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

// css::sdbc::XWarningsSupplier
    virtual css::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

// css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

// XConnection
    virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL commit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rollback(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isClosed(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCatalog( const OUString& catalog ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCatalog(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

// css::sdbc::XCloseable
    virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // XMultiServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XUsersSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getUsers(  ) throw(css::uno::RuntimeException, std::exception) override;
    // XGroupsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getGroups(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XConnectionTools
    virtual css::uno::Reference< css::sdb::tools::XTableName > SAL_CALL createTableName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdb::tools::XObjectNames > SAL_CALL getObjectNames(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdb::tools::XDataSourceMetaData > SAL_CALL getDataSourceMetaData(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getFieldsByCommandDescriptor( ::sal_Int32 commandType, const OUString& command, css::uno::Reference< css::lang::XComponent >& keepFieldsAlive ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdb::XSingleSelectQueryComposer > SAL_CALL getComposer( ::sal_Int32 commandType, const OUString& command ) throw (css::uno::RuntimeException, std::exception) override;

    // XTableUIProvider
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL getTableIcon( const OUString& TableName, ::sal_Int32 ColorMode ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getTableEditor( const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& DocumentUI, const OUString& TableName ) throw (css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // IRefreshListener
    virtual void refresh(const css::uno::Reference< css::container::XNameAccess >& _rToBeRefreshed) override;

protected:
    inline  void checkDisposed() throw (css::lang::DisposedException)
    {
        if ( rBHelper.bDisposed || !m_xConnection.is() )
            throw css::lang::DisposedException();
    }

    css::uno::Reference< css::sdbcx::XTablesSupplier > const & getMasterTables();

private:
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
