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

#include "KeySet.hxx"
#include <cppuhelper/implbase1.hxx>

#include <connectivity/sqlparse.hxx>
#include <connectivity/sqliterator.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <comphelper/stl_types.hxx>
#include <comphelper/componentcontext.hxx>

namespace dbaccess
{
     // is used when the source supports keys
    class OptimisticSet : public OKeySet
    {
        ::connectivity::OSQLParser                              m_aSqlParser;
        ::connectivity::OSQLParseTreeIterator                   m_aSqlIterator;

        ::std::map<sal_Int32,sal_Int32>                         m_aJoinedColumns;
        ::std::map<sal_Int32,sal_Int32>                         m_aJoinedKeyColumns;


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

        void impl_convertValue_throw(const ORowSetRow& _rInsertRow,const SelectColumnDescription& i_aMetaData);

        void executeDelete(const ORowSetRow& _rDeleteRow,const ::rtl::OUString& i_sSQL,const ::rtl::OUString& i_sTableName);
        void fillJoinedColumns_throw(const ::std::vector< ::connectivity::TNodePair>& i_aJoinColumns);
        void fillJoinedColumns_throw(const ::rtl::OUString& i_sLeftColumn,const ::rtl::OUString& i_sRightColumn);
    protected:
        virtual ~OptimisticSet();
    public:
        OptimisticSet(const ::comphelper::ComponentContext& _rContext,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& i_xConnection,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >& _xComposer,
                      const ORowSetValueVector& _aParameterValueForCache,
                      sal_Int32 i_nMaxRows,
                      sal_Int32& o_nRowCount);

        // late ctor which can throw exceptions
        virtual void construct(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet,const ::rtl::OUString& i_sRowSetFilter);

        // -------------------------------------------------------------------------
        // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ,const connectivity::OSQLTable& _xTable) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

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

