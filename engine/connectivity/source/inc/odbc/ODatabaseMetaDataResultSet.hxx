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

#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/proparrhlp.hxx>
#include <odbc/OStatement.hxx>
#include <odbc/ODatabaseMetaData.hxx>
#include <odbc/odbcbasedllapi.hxx>
#include <memory>
#include <string_view>

namespace connectivity::odbc
{
    class OResultSetMetaData;
    /*
    **  java_sql_ResultSet
    */
    typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XResultSet,
                                               css::sdbc::XRow,
                                               css::sdbc::XResultSetMetaDataSupplier,
                                               css::util::XCancellable,
                                               css::sdbc::XWarningsSupplier,
                                               css::sdbc::XCloseable,
                                               css::sdbc::XColumnLocate> ODatabaseMetaDataResultSet_BASE;

    class ODatabaseMetaDataResultSet :
                                public cppu::BaseMutex,
                                public  ODatabaseMetaDataResultSet_BASE,
                                public  ::cppu::OPropertySetHelper,
                                public  ::comphelper::OPropertyArrayUsageHelper<ODatabaseMetaDataResultSet>
    {
        std::vector< sal_Int32>                     m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time

        std::map<sal_Int32, ::std::map<sal_Int32,sal_Int32> >
                                                    m_aValueRange;

        std::map<sal_Int32,SWORD>                   m_aODBCColumnTypes;

        SQLHANDLE                                   m_aStatementHandle;   // ... until freed
        rtl::Reference<OResultSetMetaData>          m_xMetaData;
        std::unique_ptr<SQLUSMALLINT[]>             m_pRowStatusArray;
        rtl::Reference<OConnection>                 m_pConnection;
        rtl_TextEncoding                            m_nTextEncoding;
        sal_Int32                                   m_nRowPos;
        sal_Int32                                   m_nDriverColumnCount;   // column count of the driver which can sometimes be less than the metadata count
        SQLRETURN                                   m_nCurrentFetchState;
        bool                                        m_bWasNull;
        bool                                        m_bEOF;                 // after last record

        // set the columncount of the driver
        void checkColumnCount();
        static sal_Int32 getFetchDirection()       { return css::sdbc::FetchDirection::FORWARD; }
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        static sal_Int32 getFetchSize();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        static const OUString & getCursorName();
        SWORD                               impl_getColumnType_nothrow(sal_Int32 columnIndex);

        sal_Int32 mapColumn (sal_Int32  column);

    protected:

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & getInfoHelper() override;

        virtual bool convertFastPropertyValue(
                                                            cpo::uno::Any & rConvertedValue,
                                                            cpo::uno::Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const cpo::uno::Any& rValue ) override;
        virtual void setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;
        virtual void getFastPropertyValue( cpo::uno::Any& rValue, sal_Int32 nHandle ) const override;
        virtual ~ODatabaseMetaDataResultSet() override;
        template < typename T, SQLSMALLINT sqlTypeId > T getInteger ( sal_Int32 columnIndex );

    public:
        // A ctor needed for returning the object
        ODatabaseMetaDataResultSet(OConnection* _pConnection);


        const Functions& functions() const { return m_pConnection->functions(); }
        // ::cppu::OComponentHelper
        virtual void disposing() override;
        // XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;
        //XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
        css::uno::Reference< css::uno::XInterface > operator *()
        {
            return css::uno::Reference< css::uno::XInterface >(*static_cast<ODatabaseMetaDataResultSet_BASE*>(this));
        }
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

        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openTablesTypes( );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openTypeInfo();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openCatalogs();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openSchemas();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openTables(const cpo::uno::Any& catalog, const OUString& schemaPattern,
                                        const OUString& tableNamePattern, const cpo::uno::Sequence< OUString >& types );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openColumnPrivileges(      const cpo::uno::Any& catalog,    const OUString& schema,
                                                                const OUString& table,   const OUString& columnNamePattern );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openColumns(       const cpo::uno::Any& catalog,                            const OUString& schemaPattern,
                                                const OUString& tableNamePattern,        const OUString& columnNamePattern );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openProcedureColumns(      const cpo::uno::Any& catalog,            const OUString& schemaPattern,
                                                        const OUString& procedureNamePattern,const OUString& columnNamePattern );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openProcedures(    const cpo::uno::Any& catalog,            const OUString& schemaPattern,
                                                        const OUString& procedureNamePattern);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openVersionColumns(const cpo::uno::Any& catalog, const OUString& schema,
                                                        const OUString& table);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openBestRowIdentifier( const cpo::uno::Any& catalog, const OUString& schema,
                                                                const OUString& table,sal_Int32 scope, bool nullable );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openForeignKeys( const cpo::uno::Any& catalog, const OUString* schema,const OUString* table,
                                                          const cpo::uno::Any& catalog2, const OUString* schema2,const OUString* table2);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openExportedKeys(const cpo::uno::Any& catalog, const OUString& schema,const OUString& table);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openImportedKeys(const cpo::uno::Any& catalog, const OUString& schema,const OUString& table);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openPrimaryKeys(const cpo::uno::Any& catalog, const OUString& schema,const OUString& table);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openTablePrivileges(const cpo::uno::Any& catalog, const OUString& schemaPattern,
                                                          const OUString& tableNamePattern);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openSpecialColumns(bool _bRowVer,const cpo::uno::Any& catalog, const OUString& schema,
                                                                const OUString& table,sal_Int32 scope,   bool nullable );
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void openIndexInfo( const cpo::uno::Any& catalog, const OUString& schema,
                                                const OUString& table,bool unique,bool approximate );

    protected:
        using OPropertySetHelper::getFastPropertyValue;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
