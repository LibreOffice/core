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

#include <sal/config.h>

#include <string_view>

#include "NConnection.hxx"
#include <TDatabaseMetaDataBase.hxx>
#include <FDatabaseMetaDataResultSet.hxx>


namespace connectivity::evoab
{

    //************ Class: OEvoabDatabaseMetaData

    typedef struct{
        gboolean bIsSplittedValue;
        GParamSpec *pField;
    }ColumnProperty;

    typedef enum {
        DEFAULT_ADDR_LINE1=1,DEFAULT_ADDR_LINE2,DEFAULT_CITY,DEFAULT_STATE,DEFAULT_COUNTRY,DEFAULT_ZIP,
        WORK_ADDR_LINE1,WORK_ADDR_LINE2,WORK_CITY,WORK_STATE,WORK_COUNTRY,WORK_ZIP,
        HOME_ADDR_LINE1,HOME_ADDR_LINE2,HOME_CITY,HOME_STATE,HOME_COUNTRY,HOME_ZIP,
        OTHER_ADDR_LINE1,OTHER_ADDR_LINE2,OTHER_CITY,OTHER_STATE,OTHER_COUNTRY,OTHER_ZIP
    }ColumnNumber;

    typedef struct {
        const gchar *pColumnName;
        ColumnNumber value;
    }SplitEvoColumns;

    const SplitEvoColumns* get_evo_addr();

    const ColumnProperty *getField(guint n);
    GType                 getGFieldType(guint nCol) ;
    sal_Int32             getFieldType(guint nCol) ;
    OUString         getFieldTypeName(guint nCol) ;
    OUString         getFieldName(guint nCol) ;
    guint                 findEvoabField(std::u16string_view aColName);

    void free_column_resources();

    class OEvoabDatabaseMetaData : public ODatabaseMetaDataBase
    {
        OEvoabConnection*                      m_pConnection;

    protected:
        virtual cpo::uno::Reference< css::sdbc::XResultSet > impl_getTypeInfo_throw() override;
        // cached database information
        virtual OUString    impl_getIdentifierQuoteString_throw(  ) override;
        virtual bool        impl_isCatalogAtStart_throw(  ) override;
        virtual OUString    impl_getCatalogSeparator_throw(  ) override;
        virtual bool        impl_supportsCatalogsInTableDefinitions_throw(  ) override;
        virtual bool        impl_supportsSchemasInTableDefinitions_throw(  ) override ;
        virtual bool        impl_supportsCatalogsInDataManipulation_throw(  ) override;
        virtual bool        impl_supportsSchemasInDataManipulation_throw(  ) override ;
        virtual bool        impl_supportsMixedCaseQuotedIdentifiers_throw(  ) override ;
        virtual bool        impl_supportsAlterTableWithAddColumn_throw(  ) override;
        virtual bool        impl_supportsAlterTableWithDropColumn_throw(  ) override;
        virtual sal_Int32   impl_getMaxStatements_throw(  ) override;
        virtual sal_Int32   impl_getMaxTablesInSelect_throw(  ) override;
        virtual bool        impl_storesMixedCaseQuotedIdentifiers_throw(  ) override;

        virtual ~OEvoabDatabaseMetaData() override;
    public:
        explicit OEvoabDatabaseMetaData(OEvoabConnection* _pCon);

        // as I mentioned before this interface is really BIG
        // XDatabaseMetaData
        virtual OUString getURL(  ) override;
        virtual OUString getUserName(  ) override;
        virtual bool isReadOnly(  ) override;
        virtual OUString getDatabaseProductName(  ) override;
        virtual bool usesLocalFiles(  ) override;
        virtual bool usesLocalFilePerTable(  ) override;

        virtual OUString getExtraNameCharacters(  ) override;
        virtual bool supportsOrderByUnrelated(  ) override;
        virtual bool supportsGroupByUnrelated(  ) override;
        virtual bool supportsMultipleResultSets(  ) override;
        virtual bool supportsNonNullableColumns(  ) override;
        virtual bool supportsCoreSQLGrammar(  ) override;
        virtual bool supportsANSI92EntryLevelSQL(  ) override;
        virtual bool supportsIntegrityEnhancementFacility(  ) override;
        virtual bool supportsFullOuterJoins(  ) override;
        virtual bool supportsSchemasInProcedureCalls(  ) override;
        virtual bool supportsSchemasInIndexDefinitions(  ) override;
        virtual bool supportsSchemasInPrivilegeDefinitions(  ) override;
        virtual bool supportsCatalogsInProcedureCalls(  ) override;
        virtual bool supportsCatalogsInIndexDefinitions(  ) override;
        virtual bool supportsCatalogsInPrivilegeDefinitions(  ) override;
        virtual sal_Int32 getMaxColumnNameLength(  ) override;
        virtual sal_Int32 getMaxTableNameLength(  ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getTables( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const cpo::uno::Sequence< OUString >& types ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getTableTypes(  ) override;
        virtual cpo::uno::Reference< css::sdbc::XResultSet > getColumns( const cpo::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern ) override;
        virtual bool supportsResultSetType( sal_Int32 setType ) override;
        virtual bool supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 concurrency ) override;
        virtual bool supportsBatchUpdates(  ) override;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
