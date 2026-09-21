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

#include "MacabConnection.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <cppuhelper/implbase.hxx>

namespace connectivity::macab
{

    class MacabDatabaseMetaData : public ::cppu::WeakImplHelper< css::sdbc::XDatabaseMetaData>
    {
        rtl::Reference< MacabConnection > m_xConnection;
        bool                                   m_bUseCatalog;

    public:

        MacabConnection* getOwnConnection() const { return m_xConnection.get(); }

        explicit MacabDatabaseMetaData(MacabConnection* _pCon);
        virtual ~MacabDatabaseMetaData() override;

        // this interface is really BIG
        // XDatabaseMetaData
        virtual OUString getURL(  ) override;
        virtual OUString getUserName(  ) override;
        virtual bool isReadOnly(  ) override;
        virtual OUString getDatabaseProductName(  ) override;
        virtual bool usesLocalFiles(  ) override;
        virtual bool usesLocalFilePerTable(  ) override;
        virtual bool supportsMixedCaseQuotedIdentifiers(  ) override;
        virtual bool storesMixedCaseQuotedIdentifiers(  ) override;
        virtual OUString getIdentifierQuoteString(  ) override;
        virtual OUString getExtraNameCharacters(  ) override;
        virtual bool supportsAlterTableWithAddColumn(  ) override;
        virtual bool supportsAlterTableWithDropColumn(  ) override;
        virtual bool supportsOrderByUnrelated(  ) override;
        virtual bool supportsGroupByUnrelated(  ) override;
        virtual bool supportsMultipleResultSets(  ) override;
        virtual bool supportsNonNullableColumns(  ) override;
        virtual bool supportsCoreSQLGrammar(  ) override;
        virtual bool supportsANSI92EntryLevelSQL(  ) override;
        virtual bool supportsIntegrityEnhancementFacility(  ) override;
        virtual bool supportsFullOuterJoins(  ) override;
        virtual bool isCatalogAtStart(  ) override;
        virtual OUString getCatalogSeparator(  ) override;
        virtual bool supportsSchemasInDataManipulation(  ) override;
        virtual bool supportsSchemasInProcedureCalls(  ) override;
        virtual bool supportsSchemasInTableDefinitions(  ) override;
        virtual bool supportsSchemasInIndexDefinitions(  ) override;
        virtual bool supportsSchemasInPrivilegeDefinitions(  ) override;
        virtual bool supportsCatalogsInDataManipulation(  ) override;
        virtual bool supportsCatalogsInProcedureCalls(  ) override;
        virtual bool supportsCatalogsInTableDefinitions(  ) override;
        virtual bool supportsCatalogsInIndexDefinitions(  ) override;
        virtual bool supportsCatalogsInPrivilegeDefinitions(  ) override;
        virtual sal_Int32 getMaxColumnNameLength(  ) override;
        virtual sal_Int32 getMaxStatements(  ) override;
        virtual sal_Int32 getMaxTableNameLength(  ) override;
        virtual sal_Int32 getMaxTablesInSelect(  ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getTables( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const cpo::uno::Sequence< OUString >& types ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getTableTypes(  ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getColumns( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getColumnPrivileges( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table, const OUString& columnNamePattern ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getTablePrivileges( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getVersionColumns( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getPrimaryKeys( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getImportedKeys( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getTypeInfo(  ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getIndexInfo( const cpo::uno::Any& catalog, const OUString& schema, const OUString& table, bool unique, bool approximate ) override;
        virtual bool supportsResultSetType( sal_Int32 setType ) override;
        virtual bool supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) override;
        virtual bool supportsBatchUpdates(  ) override;
        virtual cpo::uno::Reference< css::sdbc::XConnection > getConnection(  ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
