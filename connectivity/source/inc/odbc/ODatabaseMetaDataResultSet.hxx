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
        typedef ::cppu::WeakComponentImplHelper<   ::com::sun::star::sdbc::XResultSet,
                                                   ::com::sun::star::sdbc::XRow,
                                                   ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                                   ::com::sun::star::util::XCancellable,
                                                   ::com::sun::star::sdbc::XWarningsSupplier,
                                                   ::com::sun::star::sdbc::XCloseable,
                                                   ::com::sun::star::sdbc::XColumnLocate> ODatabaseMetaDataResultSet_BASE;

        class OOO_DLLPUBLIC_ODBCBASE ODatabaseMetaDataResultSet :
                                    public comphelper::OBaseMutex,
                                    public  ODatabaseMetaDataResultSet_BASE,
                                    public  ::cppu::OPropertySetHelper,
                                    public  ::comphelper::OPropertyArrayUsageHelper<ODatabaseMetaDataResultSet>
        {
            ::connectivity::TIntVector                      m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time

            ::std::map<sal_Int32, ::connectivity::TInt2IntMap >                 m_aValueRange;
            ::std::map<sal_Int32, ::connectivity::TString2IntMap >              m_aStrValueRange;

            ::std::map<sal_Int32, ::connectivity::TInt2StringMap >              m_aIntValueRange;
            ::std::map<sal_Int32,SWORD>                                         m_aODBCColumnTypes;

            SQLHANDLE                                                                   m_aStatementHandle;   // ... until freed
            ::com::sun::star::uno::WeakReferenceHelper                                  m_aStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>        m_xMetaData;
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
            static sal_Int32 getFetchSize()     throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            static OUString getCursorName()     throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            SWORD                               impl_getColumnType_nothrow(sal_Int32 columnIndex);

            sal_Int32 mapColumn (sal_Int32  column);

        protected:

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                                                ::com::sun::star::uno::Any & rConvertedValue,
                                                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                                                        const ::com::sun::star::uno::Any& rValue
                                                     )
                                                                                                         throw (::com::sun::star::uno::Exception, std::exception) override;
            virtual void SAL_CALL getFastPropertyValue(
                                                                        ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const override;
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
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > operator *()
            {
                return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*static_cast<ODatabaseMetaDataResultSet_BASE*>(this));
            }
            // XResultSet
            virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isAfterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL beforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL afterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL first(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL last(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL previous(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            void openTablesTypes( ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTypeInfo() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openCatalogs() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openSchemas() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTables(const ::com::sun::star::uno::Any& catalog, const OUString& schemaPattern,
                                            const OUString& tableNamePattern, const ::com::sun::star::uno::Sequence< OUString >& types )
                                            throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openColumnPrivileges(      const ::com::sun::star::uno::Any& catalog,    const OUString& schema,
                                                                    const OUString& table,   const OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openColumns(       const ::com::sun::star::uno::Any& catalog,                            const OUString& schemaPattern,
                                                    const OUString& tableNamePattern,        const OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openProcedureColumns(      const ::com::sun::star::uno::Any& catalog,            const OUString& schemaPattern,
                                                            const OUString& procedureNamePattern,const OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openProcedures(    const ::com::sun::star::uno::Any& catalog,            const OUString& schemaPattern,
                                                            const OUString& procedureNamePattern)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openVersionColumns(const ::com::sun::star::uno::Any& catalog, const OUString& schema,
                                                            const OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openBestRowIdentifier( const ::com::sun::star::uno::Any& catalog, const OUString& schema,
                                                                    const OUString& table,sal_Int32 scope, bool nullable )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openForeignKeys( const ::com::sun::star::uno::Any& catalog, const OUString* schema,const OUString* table,
                                                              const ::com::sun::star::uno::Any& catalog2, const OUString* schema2,const OUString* table2)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openExportedKeys(const ::com::sun::star::uno::Any& catalog, const OUString& schema,const OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openImportedKeys(const ::com::sun::star::uno::Any& catalog, const OUString& schema,const OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openPrimaryKeys(const ::com::sun::star::uno::Any& catalog, const OUString& schema,const OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTablePrivileges(const ::com::sun::star::uno::Any& catalog, const OUString& schemaPattern,
                                                              const OUString& tableNamePattern)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openSpecialColumns(bool _bRowVer,const ::com::sun::star::uno::Any& catalog, const OUString& schema,
                                                                    const OUString& table,sal_Int32 scope,   bool nullable )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openIndexInfo( const ::com::sun::star::uno::Any& catalog, const OUString& schema,
                                                    const OUString& table,bool unique,bool approximate )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };
    }

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ODATABASEMETADATARESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
