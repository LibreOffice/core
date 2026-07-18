/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/sdbc/XDatabaseMetaData2.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include "FDatabaseMetaDataResultSet.hxx"
#include <functional>
#include <connectivity/dbtoolsdllapi.hxx>

namespace comphelper { class OEventListenerHelper; }

namespace connectivity
{

    class OOO_DLLPUBLIC_DBTOOLS ODatabaseMetaDataBase :
                                    public  cppu::BaseMutex,
                                    public ::cppu::WeakImplHelper< css::sdbc::XDatabaseMetaData2,
                                                                   css::lang::XEventListener>
    {
    private:
        cpo::uno::Sequence< css::beans::PropertyValue >   m_aConnectionInfo;
        ::connectivity::ODatabaseMetaDataResultSet::ORows m_aTypeInfoRows;

        // cached database information
        std::pair<bool,bool>              m_isCatalogAtStart;
        std::pair<bool,OUString>          m_sCatalogSeparator;
        std::pair<bool,OUString>          m_sIdentifierQuoteString;
        std::pair<bool,bool>              m_supportsCatalogsInTableDefinitions;
        std::pair<bool,bool>              m_supportsSchemasInTableDefinitions;
        std::pair<bool,bool>              m_supportsCatalogsInDataManipulation;
        std::pair<bool,bool>              m_supportsSchemasInDataManipulation;
        std::pair<bool,bool>              m_supportsMixedCaseQuotedIdentifiers;
        std::pair<bool,bool>              m_supportsAlterTableWithAddColumn;
        std::pair<bool,bool>              m_supportsAlterTableWithDropColumn;
        std::pair<bool,sal_Int32>         m_MaxStatements;
        std::pair<bool,sal_Int32>         m_MaxTablesInSelect;
        std::pair<bool,bool>              m_storesMixedCaseQuotedIdentifiers;

        template <typename T> T callImplMethod(std::pair<bool,T>& _rCache,const std::function<T(ODatabaseMetaDataBase *)>& _pImplMethod)
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
        rtl::Reference< comphelper::OEventListenerHelper> m_xListenerHelper; // forward the calls from the connection to me

        virtual ~ODatabaseMetaDataBase() override;

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

        ODatabaseMetaDataBase(const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,const cpo::uno::Sequence< css::beans::PropertyValue >& _rInfo);

        // XDatabaseMetaData2
        virtual cpo::uno::Sequence< css::beans::PropertyValue > getConnectionInfo(  ) override;

        // XEventListener
        virtual void disposing( const css::lang::EventObject& Source ) override;

        virtual css::uno::Reference< css::sdbc::XResultSet > getTypeInfo(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getProcedures( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getProcedureColumns( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& procedureNamePattern, const OUString& columnNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getSchemas(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getCatalogs(  ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getColumnPrivileges( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getTablePrivileges( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getBestRowIdentifier( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table, sal_Int32 scope, bool nullable ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getVersionColumns( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getPrimaryKeys( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getImportedKeys( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getExportedKeys( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getCrossReference( const cpo::uno::Any& primaryCatalog, const OUString& primarySchema, const OUString& primaryTable, const cpo::uno::Any& foreignCatalog, const OUString& foreignSchema, const OUString& foreignTable ) override;
        virtual css::uno::Reference< css::sdbc::XResultSet > getIndexInfo( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table, bool unique, bool approximate ) override;

        virtual css::uno::Reference< css::sdbc::XConnection > getConnection(  ) override;
        // cached database information
        virtual OUString getIdentifierQuoteString(  ) override;
        virtual bool isCatalogAtStart(  ) override;
        virtual OUString getCatalogSeparator(  ) override;
        virtual bool supportsCatalogsInTableDefinitions(  ) override;
        virtual bool supportsSchemasInTableDefinitions(  ) override;
        virtual bool supportsCatalogsInDataManipulation(  ) override;
        virtual bool supportsSchemasInDataManipulation(  ) override;
        virtual bool supportsMixedCaseQuotedIdentifiers(  ) override;
        virtual bool supportsAlterTableWithAddColumn(  ) override;
        virtual bool supportsAlterTableWithDropColumn(  ) override;
        virtual sal_Int32 getMaxStatements(  ) override;
        virtual sal_Int32 getMaxTablesInSelect(  ) override;
        virtual bool storesMixedCaseQuotedIdentifiers(  ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
