/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FDatabaseMetaDataResultSet.hxx,v $
 * $Revision: 1.15 $
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

#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#define _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_

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
#include "connectivity/CommonTools.hxx"
#include <comphelper/propertycontainer.hxx>
#ifndef _CONNECTIVITY_VALUE_HXX_
#include "connectivity/FValue.hxx"
#endif
#include <comphelper/broadcasthelper.hxx>
#include <vos/refernce.hxx>

namespace connectivity
{
    class ODatabaseMetaDataResultSetMetaData;
    typedef ::cppu::WeakComponentImplHelper7<   ::com::sun::star::sdbc::XResultSet,
                                                ::com::sun::star::sdbc::XRow,
                                                ::com::sun::star::sdbc::XResultSetMetaDataSupplier,
                                                ::com::sun::star::util::XCancellable,
                                                ::com::sun::star::sdbc::XWarningsSupplier,
                                                ::com::sun::star::sdbc::XCloseable,
                                                ::com::sun::star::sdbc::XColumnLocate> ODatabaseMetaDataResultSet_BASE;


    //  typedef ORefVector<ORowSetValue>    ORow;
    //  typedef ORefVector<ORow>            ORows;

    class ODatabaseMetaDataResultSet :  public comphelper::OBaseMutex,
                                        public  ODatabaseMetaDataResultSet_BASE,
                                        public  ::comphelper::OPropertyContainer,
                                        public  ::comphelper::OPropertyArrayUsageHelper<ODatabaseMetaDataResultSet>
    {

    public:
        DECLARE_STL_VECTOR(ORowSetValueDecoratorRef,ORow);
        DECLARE_STL_VECTOR(ORow, ORows);

    private:
        ORowSetValue                    m_aEmptyValue;
        ::com::sun::star::uno::WeakReferenceHelper    m_aStatement;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData>        m_xMetaData;
        sal_Int32                       m_nColPos;

        sal_Int32                       m_nFetchSize;
        sal_Int32                       m_nResultSetType;
        sal_Int32                       m_nFetchDirection;
        sal_Int32                       m_nResultSetConcurrency;

        void construct();
        void checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException);

    protected:
        ORows                           m_aRows;
        ORows::iterator                 m_aRowsIter;
        sal_Bool                        m_bBOF;
        sal_Bool                        m_bEOF;

        virtual const ORowSetValue& getValue(sal_Int32 columnIndex);

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        virtual ~ODatabaseMetaDataResultSet();
    public:

        virtual void    SAL_CALL acquire() throw();
        virtual void    SAL_CALL release() throw();

        enum MetaDataResultSetType
        {
            /// describes a result set as expected by XDatabaseMetaData::getCatalogs
            eCatalogs,
            /// describes a result set as expected by XDatabaseMetaData::getSchemas
            eSchemas,
            /// describes a result set as expected by XDatabaseMetaData::getColumnPrivileges
            eColumnPrivileges,
            /// describes a result set as expected by XDatabaseMetaData::getColumns
            eColumns,
            /// describes a result set as expected by XDatabaseMetaData::getTables
            eTables,
            /// describes a result set as expected by XDatabaseMetaData::getTableTypes
            eTableTypes,
            /// describes a result set as expected by XDatabaseMetaData::getProcedureColumns
            eProcedureColumns,
            /// describes a result set as expected by XDatabaseMetaData::getProcedures
            eProcedures,
            /// describes a result set as expected by XDatabaseMetaData::getExportedKeys
            eExportedKeys,
            /// describes a result set as expected by XDatabaseMetaData::getImportedKeys
            eImportedKeys,
            /// describes a result set as expected by XDatabaseMetaData::getPrimaryKeys
            ePrimaryKeys,
            /// describes a result set as expected by XDatabaseMetaData::getIndexInfo
            eIndexInfo,
            /// describes a result set as expected by XDatabaseMetaData::getTablePrivileges
            eTablePrivileges,
            /// describes a result set as expected by XDatabaseMetaData::getCrossReference
            eCrossReference,
            /// describes a result set as expected by XDatabaseMetaData::getTypeInfo
            eTypeInfo,
            /// describes a result set as expected by XDatabaseMetaData::getBestRowIdentifier
            eBestRowIdentifier,
            /// describes a result set as expected by XDatabaseMetaData::getVersionColumns
            eVersionColumns
        };
        /// default construction
        ODatabaseMetaDataResultSet();
        /// construction of a pre-defined result set type
        ODatabaseMetaDataResultSet( MetaDataResultSetType _eType );

        void setRows(const ORows& _rRows);

    protected:
        // ::cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);
        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
        //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
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
    public:
        // some methods to get already defined ORowSetValues
        // this increase the reuse of ORowSetValues
        /// return an empty ORowSetValueDecorator
        static ORowSetValueDecoratorRef getEmptyValue();
        /// return an ORowSetValueDecorator with 0 as value
        static ORowSetValueDecoratorRef get0Value();
        /// return an ORowSetValueDecorator with 1 as value
        static ORowSetValueDecoratorRef get1Value();
        /// return an ORowSetValueDecorator with ColumnSearch::BASIC as value
        static ORowSetValueDecoratorRef getBasicValue();
        /// return an ORowSetValueDecorator with string SELECT as value
        static ORowSetValueDecoratorRef getSelectValue();
        /// return an ORowSetValueDecorator with string INSERT as value
        static ORowSetValueDecoratorRef getInsertValue();
        /// return an ORowSetValueDecorator with string DELETE as value
        static ORowSetValueDecoratorRef getDeleteValue();
        /// return an ORowSetValueDecorator with string UPDATE as value
        static ORowSetValueDecoratorRef getUpdateValue();
        /// return an ORowSetValueDecorator with string CREATE as value
        static ORowSetValueDecoratorRef getCreateValue();
        /// return an ORowSetValueDecorator with string READ as value
        static ORowSetValueDecoratorRef getReadValue();
        /// return an ORowSetValueDecorator with string ALTER as value
        static ORowSetValueDecoratorRef getAlterValue();
        /// return an ORowSetValueDecorator with string DROP as value
        static ORowSetValueDecoratorRef getDropValue();
        /// return an ORowSetValueDecorator with string ' as value
        static ORowSetValueDecoratorRef getQuoteValue();

    };
}
#endif // _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_

