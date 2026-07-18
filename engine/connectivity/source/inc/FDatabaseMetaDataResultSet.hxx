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

#include <vector>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>

#include <comphelper/propcontainerimplhelper.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
    class ODatabaseMetaDataResultSetMetaData;

    class SAL_DLLPUBLIC_RTTI ODatabaseMetaDataResultSet
        : public comphelper::OPropertyContainerImplHelper<
              comphelper::WeakComponentImplHelper<
                  css::sdbc::XResultSet,
                  css::sdbc::XRow,
                  css::sdbc::XResultSetMetaDataSupplier,
                  css::util::XCancellable,
                  css::sdbc::XWarningsSupplier,
                  css::sdbc::XCloseable,
                  css::lang::XInitialization,
                  css::lang::XServiceInfo,
                  css::sdbc::XColumnLocate>,
              ODatabaseMetaDataResultSet>
    {

    public:
        typedef std::vector<ORowSetValueDecoratorRef> ORow;
        typedef std::vector<ORow> ORows;

        enum MetaDataResultSetType
        {
            /// describes a result set as expected by XDatabaseMetaData::getCatalogs
            eCatalogs = 0,
            /// describes a result set as expected by XDatabaseMetaData::getSchemas
            eSchemas = 1,
            /// describes a result set as expected by XDatabaseMetaData::getColumnPrivileges
            eColumnPrivileges = 2,
            /// describes a result set as expected by XDatabaseMetaData::getColumns
            eColumns = 3,
            /// describes a result set as expected by XDatabaseMetaData::getTables
            eTables = 4,
            /// describes a result set as expected by XDatabaseMetaData::getTableTypes
            eTableTypes = 5,
            /// describes a result set as expected by XDatabaseMetaData::getProcedureColumns
            eProcedureColumns = 6,
            /// describes a result set as expected by XDatabaseMetaData::getProcedures
            eProcedures = 7,
            /// describes a result set as expected by XDatabaseMetaData::getExportedKeys
            eExportedKeys = 8,
            /// describes a result set as expected by XDatabaseMetaData::getImportedKeys
            eImportedKeys = 9,
            /// describes a result set as expected by XDatabaseMetaData::getPrimaryKeys
            ePrimaryKeys = 10,
            /// describes a result set as expected by XDatabaseMetaData::getIndexInfo
            eIndexInfo = 11,
            /// describes a result set as expected by XDatabaseMetaData::getTablePrivileges
            eTablePrivileges = 12,
            /// describes a result set as expected by XDatabaseMetaData::getCrossReference
            eCrossReference = 13,
            /// describes a result set as expected by XDatabaseMetaData::getTypeInfo
            eTypeInfo = 14,
            /// describes a result set as expected by XDatabaseMetaData::getBestRowIdentifier
            eBestRowIdentifier = 15,
            /// describes a result set as expected by XDatabaseMetaData::getVersionColumns
            eVersionColumns = 16,
            /// describes a result set as expected by XDatabaseMetaData::getUDTs
            eUDTs = 17
        };

    private:
        ORowSetValue                    m_aEmptyValue;
        rtl::Reference< ODatabaseMetaDataResultSetMetaData>        m_xMetaData;
        sal_Int32                       m_nColPos;

        sal_Int32                       m_nFetchSize;
        sal_Int32                       m_nResultSetType;
        sal_Int32                       m_nFetchDirection;
        sal_Int32                       m_nResultSetConcurrency;

        void construct();
        /// @throws css::sdbc::SQLException
        void checkIndex(std::unique_lock<std::mutex>& rGuard, sal_Int32 columnIndex );
        void setType(MetaDataResultSetType _eType);
        css::uno::Reference< css::sdbc::XResultSetMetaData > getMetaData( std::unique_lock<std::mutex>& );

    protected:
        ORows                           m_aRows;
        ORows::iterator                 m_aRowsIter;
        bool                        m_bBOF;
        bool                        m_bEOF;

        virtual const ORowSetValue& getValue(sal_Int32 columnIndex);

        // OPropertyArrayUsageHelper
        OOO_DLLPUBLIC_DBTOOLS virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        bool next(std::unique_lock<std::mutex>& );
        bool isBeforeFirst(std::unique_lock<std::mutex>& );
        bool isAfterLast(std::unique_lock<std::mutex>& );

        virtual ~ODatabaseMetaDataResultSet() override;
    public:

        /// default construction
        ODatabaseMetaDataResultSet();
        /// construction of a pre-defined result set type
        OOO_DLLPUBLIC_DBTOOLS ODatabaseMetaDataResultSet( MetaDataResultSetType _eType );

        OOO_DLLPUBLIC_DBTOOLS void setRows(ORows&& _rRows);

        // XServiceInfo

    protected:
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;
        // ::comphelper::WeakComponentImplHelper
        virtual void disposing(std::unique_lock<std::mutex>&) override;
        // XResultSet
        virtual bool next(  ) override;
        virtual bool isBeforeFirst(  ) override;
        virtual bool isAfterLast(  ) override;
        virtual bool isFirst(  ) override;
        virtual bool isLast(  ) override;
        virtual void beforeFirst(  ) override;
        virtual void afterLast(  ) override;
        virtual bool first(  ) override;
        virtual bool last(  ) override;
        virtual sal_Int32 getRow(  ) override;
        virtual bool absolute( sal_Int32 row ) override;
        virtual bool relative( sal_Int32 rows ) override;
        virtual bool previous(  ) override;
        virtual void refreshRow(  ) override;
        virtual bool rowUpdated(  ) override;
        virtual bool rowInserted(  ) override;
        virtual bool rowDeleted(  ) override;
        virtual css::uno::Reference< css::uno::XInterface > getStatement(  ) override;
        // XRow
        virtual bool wasNull(  ) override;
        virtual OUString getString( sal_Int32 columnIndex ) override;
        virtual bool getBoolean( sal_Int32 columnIndex ) override;
        virtual sal_Int8 getByte( sal_Int32 columnIndex ) override;
        virtual sal_Int16 getShort( sal_Int32 columnIndex ) override;
        virtual sal_Int32 getInt( sal_Int32 columnIndex ) override;
        virtual sal_Int64 getLong( sal_Int32 columnIndex ) override;
        virtual float getFloat( sal_Int32 columnIndex ) override;
        virtual double getDouble( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Sequence< sal_Int8 > getBytes( sal_Int32 columnIndex ) override;
        virtual css::util::Date getDate( sal_Int32 columnIndex ) override;
        virtual css::util::Time getTime( sal_Int32 columnIndex ) override;
        virtual css::util::DateTime getTimestamp( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getBinaryStream( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getCharacterStream( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Any getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        virtual css::uno::Reference< css::sdbc::XRef > getRef( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XBlob > getBlob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XClob > getClob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XArray > getArray( sal_Int32 columnIndex ) override;
        // XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > getMetaData(  ) override;
        // XCancellable
        virtual void cancel(  ) override;
        // XCloseable
        virtual void close(  ) override;
        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) override;
        virtual void clearWarnings(  ) override;
        // XColumnLocate
        virtual sal_Int32 findColumn( const OUString& columnName ) override;

        // XInitialization
        virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

        void setCatalogsMap();
        void setSchemasMap();
        void setColumnPrivilegesMap();
        void setColumnsMap();
        void setTablesMap();
        void setTableTypes();
        void setProcedureColumnsMap();
        void setProceduresMap();
        void setExportedKeysMap();
        void setImportedKeysMap();
        void setPrimaryKeysMap();
        void setIndexInfoMap();
        void setTablePrivilegesMap();
        void setCrossReferenceMap();
        void setTypeInfoMap();
        void setBestRowIdentifierMap();
        void setVersionColumnsMap();
        void setUDTsMap();
    public:
        // some methods to get already defined ORowSetValues
        // this increase the reuse of ORowSetValues
        /// return an empty ORowSetValueDecorator
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getEmptyValue();
        /// return an ORowSetValueDecorator with 0 as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & get0Value();
        /// return an ORowSetValueDecorator with 1 as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & get1Value();
        /// return an ORowSetValueDecorator with ColumnSearch::BASIC as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getBasicValue();
        /// return an ORowSetValueDecorator with string SELECT as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getSelectValue();
        /// return an ORowSetValueDecorator with string INSERT as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getInsertValue();
        /// return an ORowSetValueDecorator with string DELETE as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getDeleteValue();
        /// return an ORowSetValueDecorator with string UPDATE as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getUpdateValue();
        /// return an ORowSetValueDecorator with string CREATE as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getCreateValue();
        /// return an ORowSetValueDecorator with string READ as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getReadValue();
        /// return an ORowSetValueDecorator with string ALTER as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getAlterValue();
        /// return an ORowSetValueDecorator with string DROP as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getDropValue();
        /// return an ORowSetValueDecorator with string ' as value
        OOO_DLLPUBLIC_DBTOOLS static ORowSetValueDecoratorRef const & getQuoteValue();

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
