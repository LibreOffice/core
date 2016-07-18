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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FDATABASEMETADATARESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FDATABASEMETADATARESULTSET_HXX

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
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/proparrhlp.hxx>
#include <connectivity/CommonTools.hxx>
#include <comphelper/propertycontainer.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace connectivity
{
    typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XResultSet,
                                               css::sdbc::XRow,
                                               css::sdbc::XResultSetMetaDataSupplier,
                                               css::util::XCancellable,
                                               css::sdbc::XWarningsSupplier,
                                               css::sdbc::XCloseable,
                                               css::lang::XInitialization,
                                               css::lang::XServiceInfo,
                                               css::sdbc::XColumnLocate> ODatabaseMetaDataResultSet_BASE;

    //  typedef ORefVector<ORowSetValue>    ORow;
    //  typedef ORefVector<ORow>            ORows;

    class OOO_DLLPUBLIC_DBTOOLS ODatabaseMetaDataResultSet :
                                        public cppu::BaseMutex,
                                        public  ODatabaseMetaDataResultSet_BASE,
                                        public  ::comphelper::OPropertyContainer,
                                        public  ::comphelper::OPropertyArrayUsageHelper<ODatabaseMetaDataResultSet>
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
        css::uno::WeakReferenceHelper    m_aStatement;
        css::uno::Reference< css::sdbc::XResultSetMetaData>        m_xMetaData;
        sal_Int32                       m_nColPos;

        sal_Int32                       m_nFetchSize;
        sal_Int32                       m_nResultSetType;
        sal_Int32                       m_nFetchDirection;
        sal_Int32                       m_nResultSetConcurrency;

        void construct();
        void checkIndex(sal_Int32 columnIndex ) throw(css::sdbc::SQLException);
        void setType(MetaDataResultSetType _eType);

    protected:
        ORows                           m_aRows;
        ORows::iterator                 m_aRowsIter;
        bool                        m_bBOF;
        bool                        m_bEOF;

        virtual const ORowSetValue& getValue(sal_Int32 columnIndex);

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

        virtual ~ODatabaseMetaDataResultSet();
    public:

        virtual void    SAL_CALL acquire() throw() override;
        virtual void    SAL_CALL release() throw() override;

        /// default construction
        ODatabaseMetaDataResultSet();
        /// construction of a pre-defined result set type
        ODatabaseMetaDataResultSet( MetaDataResultSetType _eType );

        void setRows(const ORows& _rRows);

        // XServiceInfo

        static OUString getImplementationName_Static(  ) throw(css::uno::RuntimeException);
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (css::uno::RuntimeException);

    protected:
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;
        // ::cppu::OComponentHelper
        virtual void SAL_CALL disposing() override;
        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
        //XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
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

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

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
        static ORowSetValueDecoratorRef const & getEmptyValue();
        /// return an ORowSetValueDecorator with 0 as value
        static ORowSetValueDecoratorRef const & get0Value();
        /// return an ORowSetValueDecorator with 1 as value
        static ORowSetValueDecoratorRef const & get1Value();
        /// return an ORowSetValueDecorator with ColumnSearch::BASIC as value
        static ORowSetValueDecoratorRef const & getBasicValue();
        /// return an ORowSetValueDecorator with string SELECT as value
        static ORowSetValueDecoratorRef const & getSelectValue();
        /// return an ORowSetValueDecorator with string INSERT as value
        static ORowSetValueDecoratorRef const & getInsertValue();
        /// return an ORowSetValueDecorator with string DELETE as value
        static ORowSetValueDecoratorRef const & getDeleteValue();
        /// return an ORowSetValueDecorator with string UPDATE as value
        static ORowSetValueDecoratorRef const & getUpdateValue();
        /// return an ORowSetValueDecorator with string CREATE as value
        static ORowSetValueDecoratorRef const & getCreateValue();
        /// return an ORowSetValueDecorator with string READ as value
        static ORowSetValueDecoratorRef const & getReadValue();
        /// return an ORowSetValueDecorator with string ALTER as value
        static ORowSetValueDecoratorRef const & getAlterValue();
        /// return an ORowSetValueDecorator with string DROP as value
        static ORowSetValueDecoratorRef const & getDropValue();
        /// return an ORowSetValueDecorator with string ' as value
        static ORowSetValueDecoratorRef const & getQuoteValue();

    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FDATABASEMETADATARESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
