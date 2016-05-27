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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_TDATABASEMETADATABASE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_TDATABASEMETADATABASE_HXX

#include <com/sun/star/sdbc/XDatabaseMetaData2.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include "FDatabaseMetaDataResultSet.hxx"
#include <functional>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
        class OOO_DLLPUBLIC_DBTOOLS ODatabaseMetaDataBase :
                                        public  comphelper::OBaseMutex,
                                        public ::cppu::WeakImplHelper< css::sdbc::XDatabaseMetaData2,
                                                                       css::lang::XEventListener>
        {
        private:
            css::uno::Sequence< css::beans::PropertyValue >   m_aConnectionInfo;
            ::connectivity::ODatabaseMetaDataResultSet::ORows m_aTypeInfoRows;

            // cached database information
            ::std::pair<bool,bool>              m_isCatalogAtStart;
            ::std::pair<bool,OUString>          m_sCatalogSeparator;
            ::std::pair<bool,OUString>          m_sIdentifierQuoteString;
            ::std::pair<bool,bool>              m_supportsCatalogsInTableDefinitions;
            ::std::pair<bool,bool>              m_supportsSchemasInTableDefinitions;
            ::std::pair<bool,bool>              m_supportsCatalogsInDataManipulation;
            ::std::pair<bool,bool>              m_supportsSchemasInDataManipulation;
            ::std::pair<bool,bool>              m_supportsMixedCaseQuotedIdentifiers;
            ::std::pair<bool,bool>              m_supportsAlterTableWithAddColumn;
            ::std::pair<bool,bool>              m_supportsAlterTableWithDropColumn;
            ::std::pair<bool,sal_Int32>         m_MaxStatements;
            ::std::pair<bool,sal_Int32>         m_MaxTablesInSelect;
            ::std::pair<bool,bool>              m_storesMixedCaseQuotedIdentifiers;

            template <typename T> T callImplMethod(::std::pair<bool,T>& _rCache,const ::std::mem_fun_t<T,ODatabaseMetaDataBase>& _pImplMethod)
            {
                ::osl::MutexGuard aGuard( m_aMutex );
                if ( !_rCache.first )
                {
                    _rCache.second = _pImplMethod(this);
                    _rCache.first = true;
                }
                return _rCache.second;
            }
        protected:
            css::uno::Reference< css::sdbc::XConnection >     m_xConnection;
            css::uno::Reference< css::lang::XEventListener>   m_xListenerHelper; // forward the calls from the connection to me

            virtual ~ODatabaseMetaDataBase();

        protected:
            virtual css::uno::Reference< css::sdbc::XResultSet > impl_getTypeInfo_throw() = 0;
            // cached database information
            virtual OUString    impl_getIdentifierQuoteString_throw(  )             = 0;
            virtual bool        impl_isCatalogAtStart_throw(  )                     = 0;
            virtual OUString    impl_getCatalogSeparator_throw(  )                  = 0;
            virtual bool        impl_supportsCatalogsInTableDefinitions_throw(  )   = 0;
            virtual bool        impl_supportsSchemasInTableDefinitions_throw(  )    = 0;
            virtual bool        impl_supportsCatalogsInDataManipulation_throw(  )   = 0;
            virtual bool        impl_supportsSchemasInDataManipulation_throw(  )    = 0;
            virtual bool        impl_supportsMixedCaseQuotedIdentifiers_throw(  )   = 0;
            virtual bool        impl_supportsAlterTableWithAddColumn_throw(  )      = 0;
            virtual bool        impl_supportsAlterTableWithDropColumn_throw(  )     = 0;
            virtual sal_Int32   impl_getMaxStatements_throw(  )                     = 0;
            virtual sal_Int32   impl_getMaxTablesInSelect_throw(  )                 = 0;
            virtual bool        impl_storesMixedCaseQuotedIdentifiers_throw(  )     = 0;


        public:

            ODatabaseMetaDataBase(const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,const css::uno::Sequence< css::beans::PropertyValue >& _rInfo);

            // XDatabaseMetaData2
            virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getConnectionInfo(  ) throw (css::uno::RuntimeException, std::exception) override;

            // XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTypeInfo(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getProcedures( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getProcedureColumns( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern, const OUString& columnNamePattern ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getSchemas(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getCatalogs(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumnPrivileges( const css::uno::Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTablePrivileges( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getBestRowIdentifier( const css::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope, sal_Bool nullable ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getVersionColumns( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getPrimaryKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getImportedKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getExportedKeys( const css::uno::Any& catalog, const OUString& schema, const OUString& table ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getCrossReference( const css::uno::Any& primaryCatalog, const OUString& primarySchema, const OUString& primaryTable, const css::uno::Any& foreignCatalog, const OUString& foreignSchema, const OUString& foreignTable ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getIndexInfo( const css::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Bool unique, sal_Bool approximate ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // cached database information
            virtual OUString SAL_CALL getIdentifierQuoteString(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isCatalogAtStart(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getCatalogSeparator(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsCatalogsInTableDefinitions(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsSchemasInTableDefinitions(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsCatalogsInDataManipulation(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsSchemasInDataManipulation(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsMixedCaseQuotedIdentifiers(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsAlterTableWithAddColumn(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsAlterTableWithDropColumn(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMaxStatements(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMaxTablesInSelect(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL storesMixedCaseQuotedIdentifiers(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_TDATABASEMETADATABASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
