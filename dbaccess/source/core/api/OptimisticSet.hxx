/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: KeySet.hxx,v $
 * $Revision: 1.30 $
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

#ifndef DBACCESS_CORE_API_OPTIMISTICSET_HXX
#define DBACCESS_CORE_API_OPTIMISTICSET_HXX

#include "CacheSet.hxx"
#include "KeySet.hxx"
#include <cppuhelper/implbase1.hxx>
#include <map>
#include <memory>
#include <connectivity/sqlparse.hxx>
#include <connectivity/sqliterator.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <comphelper/stl_types.hxx>
#include <comphelper/componentcontext.hxx>

namespace dbaccess
{
     // is used when the source supports keys
    class OptimisticSet : public OCacheSet
    {
        ::connectivity::OSQLParser                              m_aSqlParser;
        ::connectivity::OSQLParseTreeIterator                   m_aSqlIterator;
        OKeySetMatrix                                           m_aKeyMap;
        OKeySetMatrix::iterator                                 m_aKeyIter;

        ::std::vector< ::rtl::OUString >                        m_aAutoColumns;  // contains all columns which are autoincrement ones
        ::std::map<sal_Int32,sal_Int32>                         m_aJoinedColumns;
        ::std::map<sal_Int32,sal_Int32>                         m_aJoinedKeyColumns;

        OUpdatedParameter                                       m_aUpdatedParameter;    // contains all parameter which have been updated and are needed for refetching
        ORowSetValueVector                                      m_aParameterValueForCache;
        ::std::auto_ptr<SelectColumnsMetaData>                  m_pKeyColumnNames;      // contains all key column names
        ::std::auto_ptr<SelectColumnsMetaData>                  m_pColumnNames;         // contains all column names
        ::std::auto_ptr<SelectColumnsMetaData>                  m_pParameterNames;      // contains all parameter names
        connectivity::OSQLTable                                 m_xTable; // reference to our table
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement>   m_xStatement;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>           m_xSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>                 m_xRow;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >   m_xComposer;

        sal_Bool m_bRowCountFinal;
        mutable bool m_bResultSetChanged;

        /**
            getComposedTableName return the composed table name for the query
            @param _sCatalog    the catalogname may be empty
            @param _sSchema     the schemaname may be empty
            @param _sTable      the tablename

            @return the composed name
        */
        ::rtl::OUString getComposedTableName( const ::rtl::OUString& _sCatalog,
                                              const ::rtl::OUString& _sSchema,
                                              const ::rtl::OUString& _sTable);

        /** copies the values from the insert row into the key row
        *
        * \param _rInsertRow the row which was inserted
        * \param _rKeyRow The current key row of the row set.
        + \param i_nBookmark The bookmark is used to update the parameter
        */
        void copyRowValue(const ORowSetRow& _rInsertRow,ORowSetRow& _rKeyRow,sal_Int32 i_nBookmark);

        void fillAllRows();
        sal_Bool fetchRow();

        void impl_convertValue_throw(const ORowSetRow& _rInsertRow,const SelectColumnDescription& i_aMetaData);
        void findTableColumnsMatching_throw( const ::com::sun::star::uno::Any& i_aTable
                                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& i_xMeta
                                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& i_xQueryColumns);
        void executeUpdate(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName);
        void executeInsert( const ORowSetRow& _rInsertRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName);
        void executeDelete(const ORowSetRow& _rDeleteRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName);
        void fillJoinedColumns_throw(const ::std::vector< ::connectivity::TNodePair>& i_aJoinColumns);
        void fillJoinedColumns_throw(const ::rtl::OUString& i_sLeftColumn,const ::rtl::OUString& i_sRightColumn);
    protected:
        virtual ~OptimisticSet();
    public:
        OptimisticSet(const ::comphelper::ComponentContext& _rContext,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& i_xConnection,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >& _xComposer,
                      const ORowSetValueVector& _aParameterValueForCache);

        // late ctor which can throw exceptions
        virtual void construct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet,const ::rtl::OUString& i_sRowSetFilter);

        // ::com::sun::star::sdbc::XRow
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


        virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbc::XResultSet
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
        // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ,const connectivity::OSQLTable& _xTable) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        // CacheSet
        virtual bool isResultSetChanged() const;
        virtual void reset(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet);
        virtual void mergeColumnValues(sal_Int32 i_nColumnIndex,ORowSetValueVector::Vector& io_aInsertRow,ORowSetValueVector::Vector& io_aRow,::std::vector<sal_Int32>& o_aChangedColumns);
        virtual bool columnValuesUpdated(ORowSetValueVector::Vector& o_aCachedRow,const ORowSetValueVector::Vector& i_aRow);
        virtual bool updateColumnValues(const ORowSetValueVector::Vector& io_aCachedRow,ORowSetValueVector::Vector& io_aRow,const ::std::vector<sal_Int32>& i_aChangedColumns);
        virtual void fillMissingValues(ORowSetValueVector::Vector& io_aRow) const;

        bool isReadOnly() const { return m_aJoinedKeyColumns.empty(); }
        const ::std::map<sal_Int32,sal_Int32>& getJoinedColumns() const { return m_aJoinedColumns; }
        const ::std::map<sal_Int32,sal_Int32>& getJoinedKeyColumns() const { return m_aJoinedKeyColumns; }
    };
}
#endif // DBACCESS_CORE_API_OPTIMISTICSET_HXX

