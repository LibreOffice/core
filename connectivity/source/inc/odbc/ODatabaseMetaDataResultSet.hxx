/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ODBC_ADATABASEMETADATARESULTSET_HXX_
#define _CONNECTIVITY_ODBC_ADATABASEMETADATARESULTSET_HXX_

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <cppuhelper/compbase7.hxx>
#include <comphelper/proparrhlp.hxx>
#include "odbc/OStatement.hxx"
#include "odbc/ODatabaseMetaData.hxx"
#include "odbc/odbcbasedllapi.hxx"
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/StdTypeDefs.hxx"

namespace connectivity
{
    namespace odbc
    {
        class ODatabaseMetaDataResultSetMetaData;
        /*
        **  java_sql_ResultSet
        */
        typedef ::cppu::WeakComponentImplHelper7<   ::com::sun::star::sdbc::XResultSet,
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
            ::connectivity::TIntVector                      m_aColMapping; // pos 0 is unused so we don't have to decrement 1 everytime

            ::std::map<sal_Int32, ::connectivity::TInt2IntMap >                 m_aValueRange;
            ::std::map<sal_Int32, ::connectivity::TString2IntMap >              m_aStrValueRange;

            ::std::map<sal_Int32, ::connectivity::TInt2StringMap >              m_aIntValueRange;
            ::std::map<sal_Int32,SWORD>                                         m_aODBCColumnTypes;

            SQLHANDLE                                                                   m_aStatementHandle;
            SQLHANDLE                                                                   m_aConnectionHandle;
            ::com::sun::star::uno::WeakReferenceHelper                                  m_aStatement;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>        m_xMetaData;
            SQLUSMALLINT*                               m_pRowStatusArray;
            OConnection*                                m_pConnection;
            rtl_TextEncoding                            m_nTextEncoding;
            sal_Int32                                   m_nRowPos;
            sal_Int32                                   m_nLastColumnPos;       // used for m_aRow just to know where we are
            sal_Int32                                   m_nDriverColumnCount;   // column count of the driver wich can sometimes be less than the metadata count
            SQLRETURN                                   m_nCurrentFetchState;
            sal_Bool                                    m_bWasNull;
            sal_Bool                                    m_bEOF;                 // after last record
            sal_Bool                                    m_bFreeHandle;

            // set the columncount of the driver
            void checkColumnCount();
            sal_Int32 getResultSetConcurrency() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getResultSetType()        const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getFetchDirection()       const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getFetchSize()            const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            ::rtl::OUString getCursorName()     const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            SWORD                               impl_getColumnType_nothrow(sal_Int32 columnIndex);

            sal_Int32 mapColumn (sal_Int32  column);

        protected:

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                                                ::com::sun::star::uno::Any & rConvertedValue,
                                                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                                                        const ::com::sun::star::uno::Any& rValue
                                                     )
                                                                                                         throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                                                        ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
            ~ODatabaseMetaDataResultSet();
            template < typename T, SQLSMALLINT sqlTypeId > T getInteger ( sal_Int32 columnIndex );

        public:
            // A ctor needed for returning the object
            ODatabaseMetaDataResultSet(OConnection* _pConnection);


            inline oslGenericFunction getOdbcFunction(sal_Int32 _nIndex)  const
            {
                return m_pConnection->getOdbcFunction(_nIndex);
            }
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > operator *()
            {
                return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(*(ODatabaseMetaDataResultSet_BASE*)this);
            }
            // XResultSet
            virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isAfterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL beforeFirst(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL afterLast(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL first(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL last(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL previous(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL refreshRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XResultSetMetaDataSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException);
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const ::rtl::OUString& columnName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            const ::connectivity::TIntVector& getColumnMapping() { return m_aColMapping; }

            void openTablesTypes( ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTypeInfo() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openCatalogs() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openSchemas() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTables(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern,
                                            const ::rtl::OUString& tableNamePattern, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& types )
                                            throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openColumnPrivileges(      const ::com::sun::star::uno::Any& catalog,    const ::rtl::OUString& schema,
                                                                    const ::rtl::OUString& table,   const ::rtl::OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openColumns(       const ::com::sun::star::uno::Any& catalog,                            const ::rtl::OUString& schemaPattern,
                                                    const ::rtl::OUString& tableNamePattern,        const ::rtl::OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openProcedureColumns(      const ::com::sun::star::uno::Any& catalog,            const ::rtl::OUString& schemaPattern,
                                                            const ::rtl::OUString& procedureNamePattern,const ::rtl::OUString& columnNamePattern )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openProcedures(    const ::com::sun::star::uno::Any& catalog,            const ::rtl::OUString& schemaPattern,
                                                            const ::rtl::OUString& procedureNamePattern)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openVersionColumns(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,
                                                            const ::rtl::OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openBestRowIdentifier( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,
                                                                    const ::rtl::OUString& table,sal_Int32 scope,sal_Bool nullable )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openForeignKeys( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString* schema,const ::rtl::OUString* table,
                                                              const ::com::sun::star::uno::Any& catalog2, const ::rtl::OUString* schema2,const ::rtl::OUString* table2)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openExportedKeys(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,const ::rtl::OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openImportedKeys(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,const ::rtl::OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openPrimaryKeys(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,const ::rtl::OUString& table)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openTablePrivileges(const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern,
                                                              const ::rtl::OUString& tableNamePattern)throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openSpecialColumns(sal_Bool _bRowVer,const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,
                                                                    const ::rtl::OUString& table,sal_Int32 scope,   sal_Bool nullable )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void openIndexInfo( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schema,
                                                    const ::rtl::OUString& table,sal_Bool unique,sal_Bool approximate )throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };
    }

}
#endif // _CONNECTIVITY_ADO_ADATABASEMETADATARESULTSET_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
