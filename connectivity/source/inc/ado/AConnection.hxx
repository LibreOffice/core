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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ACONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ACONNECTION_HXX

#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <connectivity/OSubComponent.hxx>
#include <map>
#include <connectivity/CommonTools.hxx>
#include "OTypeInfo.hxx"
#include "TConnection.hxx"
#include "ado/Awrapado.hxx"

namespace connectivity
{
    namespace ado
    {
        struct OExtendedTypeInfo
        {
            ::connectivity::OTypeInfo       aSimpleType;    // the general type info
            DataTypeEnum                    eType;

            inline OUString getDBName() const { return aSimpleType.aTypeName; }
        };

        class WpADOConnection;
        class ODriver;
        class OCatalog;
        typedef std::multimap<DataTypeEnum, OExtendedTypeInfo*>         OTypeInfoMap;
        typedef connectivity::OMetaConnection                           OConnection_BASE;


        class OConnection : public OConnection_BASE,
                            public connectivity::OSubComponent<OConnection, OConnection_BASE>
        {
            friend class connectivity::OSubComponent<OConnection, OConnection_BASE>;

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
            sal_Bool                    m_bClosed;
            sal_Bool                    m_bAutocommit;

        protected:
            void buildTypeInfo() throw( css::sdbc::SQLException);
        public:

            OConnection(ODriver*        _pDriver) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            //  OConnection(const SQLHANDLE _pConnectionHandle);
            ~OConnection();
            void construct(const OUString& url,const css::uno::Sequence< css::beans::PropertyValue >& info);

            void closeAllStatements () throw( css::sdbc::SQLException);

            //XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw (css::uno::RuntimeException) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            // XServiceInfo
            DECLARE_SERVICE_INFO();
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;

            // XConnection
            virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL commit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL rollback(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isClosed(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setCatalog( const OUString& catalog ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual OUString SAL_CALL getCatalog(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

            WpADOConnection* getConnection() { return m_pAdoConnection; }
            void setCatalog(const css::uno::WeakReference< css::sdbcx::XTablesSupplier>& _xCat) { m_xCatalog = _xCat; }
            void setCatalog(OCatalog* _pCatalog) { m_pCatalog = _pCatalog; }

            const OTypeInfoMap* getTypeInfo() const { return &m_aTypeInfo;}
            OCatalog* getAdoCatalog() const
            {
                if ( m_xCatalog.get().is() )
                    return m_pCatalog;
                return NULL;
            }

            sal_Int32 getEngineType()   const { return m_nEngineType; }
            ODriver*  getDriver()       const { return m_pDriver; }

            static const OExtendedTypeInfo* getTypeInfoFromType(const OTypeInfoMap& _rTypeInfo,
                               DataTypeEnum _nType,
                               const OUString& _sTypeName,
                               sal_Int32 _nPrecision,
                               sal_Int32 _nScale,
                               sal_Bool& _brForceToType);
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ACONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
