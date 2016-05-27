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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OCONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OCONNECTION_HXX

#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <connectivity/odbc.hxx>
#include "odbc/odbcbasedllapi.hxx"
#include <connectivity/OSubComponent.hxx>
#include <connectivity/CommonTools.hxx>
#include "TConnection.hxx"
#include "OTypeInfo.hxx"
#include "OTools.hxx"
#include <cppuhelper/weakref.hxx>
#include "AutoRetrievingBase.hxx"
#include <osl/module.h>


#include <map>

namespace connectivity
{
    namespace odbc
    {
        class ODBCDriver;

        typedef connectivity::OMetaConnection OConnection_BASE;
        typedef ::std::vector< ::connectivity::OTypeInfo>   TTypeInfoVector;

        class OOO_DLLPUBLIC_ODBCBASE OConnection :
                            public OConnection_BASE,
                            public connectivity::OSubComponent<OConnection, OConnection_BASE>,
                            public OAutoRetrievingBase
        {
            friend class connectivity::OSubComponent<OConnection, OConnection_BASE>;

        protected:

            // Data attributes

            ::std::map< SQLHANDLE,OConnection*> m_aConnections; // holds all connections which are need for serveral statements


            OUString        m_sUser;        //  the user name
            ODBCDriver*     m_pDriver;      //  Pointer to the owning
                                            //  driver object

            SQLHANDLE       m_aConnectionHandle;
            SQLHANDLE       m_pDriverHandleCopy;    // performance reason
            sal_Int32       m_nStatementCount;
            bool            m_bClosed;
            bool            m_bUseCatalog;  // should we use the catalog on filebased databases
            bool            m_bUseOldDateFormat;
            bool            m_bParameterSubstitution;
            bool            m_bIgnoreDriverPrivileges;
            bool            m_bPreventGetVersionColumns;    // #i60273#
            bool            m_bReadOnly;


            SQLRETURN       OpenConnection(const OUString& aConnectStr,sal_Int32 nTimeOut, bool bSilent);

            OConnection*    cloneConnection(); // creates a new connection

        public:
            oslGenericFunction  getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const;
            SQLRETURN       Construct( const OUString& url,const css::uno::Sequence< css::beans::PropertyValue >& info) throw(css::sdbc::SQLException, std::exception);

            OConnection(const SQLHANDLE _pDriverHandle,ODBCDriver*  _pDriver);
            //  OConnection(const SQLHANDLE _pConnectionHandle);
            virtual ~OConnection();

            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();
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
            // XCloseable
            virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            SQLHANDLE       getConnection() { return m_aConnectionHandle; }

            // should we use the catalog on filebased databases
            inline bool     isCatalogUsed()                     const { return m_bUseCatalog; }
            inline bool     isParameterSubstitutionEnabled()    const { return m_bParameterSubstitution; }
            inline bool     isIgnoreDriverPrivilegesEnabled()   const { return m_bIgnoreDriverPrivileges; }
            inline bool     preventGetVersionColumns()          const { return m_bPreventGetVersionColumns; }
            inline bool     useOldDateFormat()                  const { return m_bUseOldDateFormat; }
            inline ODBCDriver*      getDriver()                     const { return m_pDriver;}

            SQLHANDLE       createStatementHandle();
            // close and free the handle and set it to SQL_NULLHANDLE
            void            freeStatementHandle(SQLHANDLE& _pHandle);
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
