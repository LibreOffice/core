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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ODATABASEMETADATARESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ODATABASEMETADATARESULTSET_HXX

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
#include <comphelper/proparrhlp.hxx>
#include "odbc/OStatement.hxx"
#include "odbc/ODatabaseMetaData.hxx"
#include "odbc/odbcbasedllapi.hxx"
#include <comphelper/broadcasthelper.hxx>
#include <connectivity/StdTypeDefs.hxx>

namespace connectivity
{
    namespace odbc
    {
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

        class OOO_DLLPUBLIC_ODBCBASE ODatabaseMetaDataResultSet :
                                    public comphelper::OBaseMutex,
                                    public  ODatabaseMetaDataResultSet_BASE,
                                    public  ::cppu::OPropertySetHelper,
                                    public  ::comphelper::OPropertyArrayUsageHelper<ODatabaseMetaDataResultSet>
        {
            ::connectivity::TIntVector                  m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time

            ::std::map<sal_Int32, ::connectivity::TInt2IntMap >
                                                        m_aValueRange;

            ::std::map<sal_Int32,SWORD>                 m_aODBCColumnTypes;

            SQLHANDLE                                   m_aStatementHandle;   // ... until freed
            css::uno::WeakReferenceHelper               m_aStatement;
            css::uno::Reference< css::sdbc::XResultSetMetaData>
                                                        m_xMetaData;
            SQLUSMALLINT*                               m_pRowStatusArray;
            OConnection*                                m_pConnection;
            rtl_TextEncoding                            m_nTextEncoding;
            sal_Int32                                   m_nRowPos;
            sal_Int32                                   m_nDriverColumnCount;   // column count of the driver which can sometimes be less than the metadata count
            SQLRETURN                                   m_nCurrentFetchState;
            bool                                        m_bWasNull;
            bool                                        m_bEOF;                 // after last record

            // set the columncount of the driver
            void checkColumnCount();
            static sal_Int32 getResultSetConcurrency() { return css::sdbc::ResultSetConcurrency::READ_ONLY; }
            static sal_Int32 getResultSetType()        { return css::sdbc::ResultSetType::FORWARD_ONLY; }
            static sal_Int32 getFetchDirection()       { return css::sdbc::FetchDirection::FORWARD; }
            static sal_Int32 getFetchSize()     throw(css::sdbc::SQLException, css::uno::RuntimeException);
            static OUString getCursorName()     throw(css::sdbc::SQLException, css::uno::RuntimeException);
            SWORD                               impl_getColumnType_nothrow(sal_Int32 columnIndex);

            sal_Int32 mapColumn (sal_Int32  column);

        protected:

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                                                css::uno::Any & rConvertedValue,
                                                                css::uno::Any & rOldValue,
                                                                sal_Int32 nHandle,
                                                                const css::uno::Any& rValue )
                                    throw (css::lang::IllegalArgumentException) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue )
                                    throw (css::uno::Exception, std::exception) override;
            virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;
            virtual ~ODatabaseMetaDataResultSet();
            template < typename T, SQLSMALLINT sqlTypeId > T getInteger ( sal_Int32 columnIndex );

        public:
            // A ctor needed for returning the object
            ODatabaseMetaDataResultSet(OConnection* _pConnection);


            inline oslGenericFunction getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const
            {
                return m_pConnection->getOdbcFunction(_nIndex);
            }
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
            css::uno::Reference< css::uno::XInterface > operator *()
            {
                return css::uno::Reference< css::uno::XInterface >(*static_cast<ODatabaseMetaDataResultSet_BASE*>(this));
            }
            // XResultSet
            virtual sal_Bool SAL_CALL next(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isAfterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL beforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL afterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL first(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL last(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL previous(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL refreshRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowUpdated(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowInserted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowDeleted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XResultSetMetaDataSupplier
            virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            void openTablesTypes( ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openTypeInfo() throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openCatalogs() throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openSchemas() throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openTables(const css::uno::Any& catalog, const OUString& schemaPattern,
                                            const OUString& tableNamePattern, const css::uno::Sequence< OUString >& types )
                                            throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openColumnPrivileges(      const css::uno::Any& catalog,    const OUString& schema,
                                                                    const OUString& table,   const OUString& columnNamePattern )throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openColumns(       const css::uno::Any& catalog,                            const OUString& schemaPattern,
                                                    const OUString& tableNamePattern,        const OUString& columnNamePattern )throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openProcedureColumns(      const css::uno::Any& catalog,            const OUString& schemaPattern,
                                                            const OUString& procedureNamePattern,const OUString& columnNamePattern )throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openProcedures(    const css::uno::Any& catalog,            const OUString& schemaPattern,
                                                            const OUString& procedureNamePattern)throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openVersionColumns(const css::uno::Any& catalog, const OUString& schema,
                                                            const OUString& table)throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openBestRowIdentifier( const css::uno::Any& catalog, const OUString& schema,
                                                                    const OUString& table,sal_Int32 scope, bool nullable )throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openForeignKeys( const css::uno::Any& catalog, const OUString* schema,const OUString* table,
                                                              const css::uno::Any& catalog2, const OUString* schema2,const OUString* table2)throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openExportedKeys(const css::uno::Any& catalog, const OUString& schema,const OUString& table)throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openImportedKeys(const css::uno::Any& catalog, const OUString& schema,const OUString& table)throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openPrimaryKeys(const css::uno::Any& catalog, const OUString& schema,const OUString& table)throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openTablePrivileges(const css::uno::Any& catalog, const OUString& schemaPattern,
                                                              const OUString& tableNamePattern)throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openSpecialColumns(bool _bRowVer,const css::uno::Any& catalog, const OUString& schema,
                                                                    const OUString& table,sal_Int32 scope,   bool nullable )throw(css::sdbc::SQLException, css::uno::RuntimeException);
            void openIndexInfo( const css::uno::Any& catalog, const OUString& schema,
                                                    const OUString& table,bool unique,bool approximate )throw(css::sdbc::SQLException, css::uno::RuntimeException);

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };
    }

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ODATABASEMETADATARESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
