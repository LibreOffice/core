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
#pragma once

#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <connectivity/odbc.hxx>
#include <odbc/odbcbasedllapi.hxx>
#include <connectivity/CommonTools.hxx>
#include <TConnection.hxx>
#include <OTypeInfo.hxx>
#include <odbc/OTools.hxx>
#include <cppuhelper/weakref.hxx>
#include <AutoRetrievingBase.hxx>
#include <osl/module.h>
#include <rtl/ref.hxx>


#include <map>

namespace connectivity::odbc
    {
        class ODBCDriver;

        typedef connectivity::OMetaConnection OConnection_BASE;
        typedef std::vector< ::connectivity::OTypeInfo>   TTypeInfoVector;

        class OOO_DLLPUBLIC_ODBCBASE OConnection final :
                            public OConnection_BASE,
                            public OAutoRetrievingBase
        {
            // Data attributes

            std::map< SQLHANDLE, rtl::Reference<OConnection>> m_aConnections; // holds all connections which are need for several statements


            OUString        m_sUser;        //  the user name
            rtl::Reference<ODBCDriver>
                            m_xDriver;      //  Pointer to the owning
                                            //  driver object

            SQLHANDLE       m_aConnectionHandle;
            SQLHANDLE       m_pDriverHandleCopy;    // performance reason
            sal_Int32       m_nStatementCount;
            bool            m_bClosed;
            bool            m_bUseCatalog;  // should we use the catalog on filebased databases
            bool            m_bUseOldDateFormat;
            bool            m_bIgnoreDriverPrivileges;
            bool            m_bPreventGetVersionColumns;    // #i60273#
            bool            m_bReadOnly;


            SQLRETURN       OpenConnection(const OUString& aConnectStr,sal_Int32 nTimeOut, bool bSilent);

        public:
            oslGenericFunction  getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const;
            /// @throws css::sdbc::SQLException
            SQLRETURN       Construct( const OUString& url,const css::uno::Sequence< css::beans::PropertyValue >& info);

            OConnection(const SQLHANDLE _pDriverHandle,ODBCDriver*  _pDriver);
            //  OConnection(const SQLHANDLE _pConnectionHandle);
            virtual ~OConnection() override;

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();
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
            // XCloseable
            virtual void SAL_CALL close(  ) override;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;

            SQLHANDLE       getConnection() { return m_aConnectionHandle; }

            // should we use the catalog on filebased databases
            bool     isCatalogUsed()                     const { return m_bUseCatalog; }
            bool     isIgnoreDriverPrivilegesEnabled()   const { return m_bIgnoreDriverPrivileges; }
            bool     preventGetVersionColumns()          const { return m_bPreventGetVersionColumns; }
            bool     useOldDateFormat()                  const { return m_bUseOldDateFormat; }
            ODBCDriver*      getDriver()                 const { return m_xDriver.get();}

            SQLHANDLE       createStatementHandle();
            // close and free the handle and set it to SQL_NULLHANDLE
            void            freeStatementHandle(SQLHANDLE& _pHandle);
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
