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
#ifndef _DBA_CORE_CONNECTION_HXX_
#define _DBA_CORE_CONNECTION_HXX_

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

#ifndef INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_14
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_14
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 14
#include <comphelper/implbase_var.hxx>
#endif
#include <comphelper/stl_types.hxx>
#include <connectivity/ConnectionWrapper.hxx>
#include <connectivity/warningscontainer.hxx>

namespace dbaccess
{

typedef ::comphelper::ImplHelper14  <   ::com::sun::star::container::XChild
                                    ,   ::com::sun::star::sdbcx::XTablesSupplier
                                    ,   ::com::sun::star::sdbcx::XViewsSupplier
                                    ,   ::com::sun::star::sdbc::XConnection
                                    ,   ::com::sun::star::sdbc::XWarningsSupplier
                                    ,   ::com::sun::star::sdb::XQueriesSupplier
                                    ,   ::com::sun::star::sdb::XSQLQueryComposerFactory
                                    ,   ::com::sun::star::sdb::XCommandPreparation
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::lang::XMultiServiceFactory
                                    ,   ::com::sun::star::sdbcx::XUsersSupplier
                                    ,   ::com::sun::star::sdbcx::XGroupsSupplier
                                    ,   ::com::sun::star::sdb::tools::XConnectionTools
                                    ,   ::com::sun::star::sdb::application::XTableUIProvider
                                    >   OConnection_Base;

class ODatabaseSource;
// OConnection
class OConnection           :public ::comphelper::OBaseMutex
                            ,public OSubComponent
                            ,public ::connectivity::OConnectionWrapper
                            ,public OConnection_Base
                            ,public IRefreshListener
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier >
                            m_xMasterTables; // just to avoid the recreation of the catalog
    OWeakRefArray           m_aStatements;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                            m_xQueries;
    OWeakRefArray           m_aComposers;

    // the filter as set on the parent data link at construction of the connection
    ::com::sun::star::uno::Sequence< OUString >  m_aTableFilter;
    ::com::sun::star::uno::Sequence< OUString >  m_aTableTypeFilter;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >                m_aContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >                     m_xMasterConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XConnectionTools >          m_xConnectionTools;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XTableUIProvider >    m_xTableUIProvider;

    // defines the helper services for example to query the command of a view
    // @ see com.sun.star.sdb.tools.XViewAccess
    DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>, TSupportServices);
    TSupportServices                m_aSupportServices;

    OTableContainer*                m_pTables;
    OViewContainer*                 m_pViews;
    ::dbtools::WarningsContainer    m_aWarnings;
    oslInterlockedCount             m_nInAppend;
    sal_Bool                        m_bSupportsViews;       // true when the getTableTypes return "VIEW" as type
    sal_Bool                        m_bSupportsUsers;
    sal_Bool                        m_bSupportsGroups;

protected:
    virtual ~OConnection();
public:
    OConnection(ODatabaseSource& _rDB
                ,::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxMaster
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB);

// com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

// com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw( );
    virtual void SAL_CALL release() throw( );

// OComponentHelper
    virtual void SAL_CALL disposing(void);

// ::com::sun::star::container::XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbcx::XTablesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);
// ::com::sun::star::sdbcx::XViewsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getViews(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdb::XQueriesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getQueries(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdb::XSQLQueryComposerFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer > SAL_CALL createQueryComposer(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdb::XCommandPreparation
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCommand( const OUString& command, sal_Int32 commandType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XWarningsSupplier
    virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// XConnection
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCatalog( const OUString& catalog ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XCloseable
    virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // XMultiServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XUsersSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getUsers(  ) throw(::com::sun::star::uno::RuntimeException);
    // XGroupsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getGroups(  ) throw(::com::sun::star::uno::RuntimeException);

    // XConnectionTools
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XTableName > SAL_CALL createTableName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XObjectNames > SAL_CALL getObjectNames(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::tools::XDataSourceMetaData > SAL_CALL getDataSourceMetaData(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getFieldsByCommandDescriptor( ::sal_Int32 commandType, const OUString& command, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& keepFieldsAlive ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer > SAL_CALL getComposer( ::sal_Int32 commandType, const OUString& command ) throw (::com::sun::star::uno::RuntimeException);

    // XTableUIProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL getTableIcon( const OUString& TableName, ::sal_Int32 ColorMode ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getTableEditor( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& DocumentUI, const OUString& TableName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // IRefreshListener
    virtual void refresh(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rToBeRefreshed);

protected:
    inline  void checkDisposed() throw (::com::sun::star::lang::DisposedException)
    {
        if ( rBHelper.bDisposed || !m_xConnection.is() )
            throw ::com::sun::star::lang::DisposedException();
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > getMasterTables();

private:
    /** checks whether or not there are naming conflicts between tables and queries
    */
    void    impl_checkTableQueryNames_nothrow();

    /** loads the XConnectionTools implementation which we forward the respective functionality to

        @throws ::com::sun::star::uno::RuntimeException
            if the implementation cannot be loaded

        @postcond
            m_xConnectionTools is nol <NULL/>
    */
    void    impl_loadConnectionTools_throw();

    /** reads the table filter and table type filter from the datasourfce
    */
    void    impl_fillTableFilter();
};

}   // namespace dbaccess

#endif // _DBA_CORE_CONNECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
