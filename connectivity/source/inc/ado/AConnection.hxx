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
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <map>
#include <connectivity/CommonTools.hxx>
#include <OTypeInfo.hxx>
#include <TConnection.hxx>
#include <ado/Awrapado.hxx>

namespace connectivity::ado
{
        struct OExtendedTypeInfo
        {
            ::connectivity::OTypeInfo       aSimpleType;    // the general type info
            DataTypeEnum                    eType;

            OUString getDBName() const { return aSimpleType.aTypeName; }
        };

        class WpADOConnection;
        class ODriver;
        class OCatalog;
        typedef std::multimap<DataTypeEnum, OExtendedTypeInfo*>       OTypeInfoMap;
        typedef connectivity::OMetaConnection                           OConnection_BASE;


        class OConnection : public OConnection_BASE
        {
        protected:

            // Data attributes

            OTypeInfoMap                m_aTypeInfo;    //  vector containing an entry
                                                                                //  for each row returned by
                                                                                //  DatabaseMetaData.getTypeInfo.
            css::uno::WeakReference< css::sdbcx::XTablesSupplier>      m_xCatalog;
            ODriver*                    m_pDriver;
        private:
            WpADOConnection*            m_pAdoConnection;
            OCatalog*                   m_pCatalog;
            sal_Int32                   m_nEngineType;
            bool                        m_bClosed;
            bool                        m_bAutocommit;

        protected:
            /// @throws css::sdbc::SQLException
            void buildTypeInfo();
        public:
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            OConnection(ODriver*        _pDriver);
            //  OConnection(const SQLHANDLE _pConnectionHandle);
            ~OConnection() override;
            void construct(const OUString& url,const css::uno::Sequence< css::beans::PropertyValue >& info);

            //XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();
            // XServiceInfo
            DECLARE_SERVICE_INFO();
            // OComponentHelper
            virtual void SAL_CALL disposing() override;

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

            WpADOConnection* getConnection() { return m_pAdoConnection; }
            void setCatalog(const css::uno::WeakReference< css::sdbcx::XTablesSupplier>& _xCat) { m_xCatalog = _xCat; }
            void setCatalog(OCatalog* _pCatalog) { m_pCatalog = _pCatalog; }

            const OTypeInfoMap* getTypeInfo() const { return &m_aTypeInfo;}
            OCatalog* getAdoCatalog() const
            {
                if ( m_xCatalog.get().is() )
                    return m_pCatalog;
                return nullptr;
            }

            sal_Int32 getEngineType()   const { return m_nEngineType; }
            ODriver*  getDriver()       const { return m_pDriver; }

            static const OExtendedTypeInfo* getTypeInfoFromType(const OTypeInfoMap& _rTypeInfo,
                               DataTypeEnum _nType,
                               const OUString& _sTypeName,
                               sal_Int32 _nPrecision,
                               sal_Int32 _nScale,
                               bool& _brForceToType);
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
