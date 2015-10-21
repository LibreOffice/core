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
                                        public ::cppu::WeakImplHelper< ::com::sun::star::sdbc::XDatabaseMetaData2,
                                                                       ::com::sun::star::lang::XEventListener>
        {
        private:
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aConnectionInfo;
            ::connectivity::ODatabaseMetaDataResultSet::ORows                           m_aTypeInfoRows;

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
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >     m_xConnection;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>   m_xListenerHelper; // forward the calls from the connection to me

            virtual ~ODatabaseMetaDataBase();

        protected:
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > impl_getTypeInfo_throw() = 0;
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

            ODatabaseMetaDataBase(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo);

            // XDatabaseMetaData2
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getConnectionInfo(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getTypeInfo(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getProcedures( const ::com::sun::star::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getProcedureColumns( const ::com::sun::star::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern, const OUString& columnNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getSchemas(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getCatalogs(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getColumnPrivileges( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getTablePrivileges( const ::com::sun::star::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getBestRowIdentifier( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope, sal_Bool nullable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getVersionColumns( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getPrimaryKeys( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getImportedKeys( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getExportedKeys( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getCrossReference( const ::com::sun::star::uno::Any& primaryCatalog, const OUString& primarySchema, const OUString& primaryTable, const ::com::sun::star::uno::Any& foreignCatalog, const OUString& foreignSchema, const OUString& foreignTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getIndexInfo( const ::com::sun::star::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Bool unique, sal_Bool approximate ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // cached database information
            virtual OUString SAL_CALL getIdentifierQuoteString(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isCatalogAtStart(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getCatalogSeparator(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsCatalogsInTableDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsSchemasInTableDefinitions(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsCatalogsInDataManipulation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsSchemasInDataManipulation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsMixedCaseQuotedIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsAlterTableWithAddColumn(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsAlterTableWithDropColumn(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMaxStatements(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMaxTablesInSelect(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL storesMixedCaseQuotedIdentifiers(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_TDATABASEMETADATABASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
