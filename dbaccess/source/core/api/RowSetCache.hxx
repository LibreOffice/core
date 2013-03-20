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
#pragma once
#if 1

#include <connectivity/CommonTools.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <cppuhelper/compbase11.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/componentcontext.hxx>
#include <cppuhelper/implbase5.hxx>
#include <comphelper/proparrhlp.hxx>
#include "RowSetRow.hxx"
#include "RowSetCacheIterator.hxx"

namespace connectivity
{
    class OSQLParseNode;
}
namespace dbaccess
{
    class OCacheSet;

    class ORowSetCache
    {
        friend class ORowSetBase;
        friend class ORowSet;
        friend class ORowSetClone;
        friend class ORowSetCacheIterator;

        typedef ::std::vector< TORowSetOldRowHelperRef >    TOldRowSetRows;

        ::std::map<sal_Int32,sal_Int32> m_aKeyColumns;
        //the set can be static, bookmarkable or keyset
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>       m_xSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >  m_xMetaData; // must be before m_aInsertRow
        ::comphelper::ComponentContext                                                  m_aContext;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>                 m_xCacheSet;

        OCacheSet*                      m_pCacheSet;            // is a bookmarkable, keyset or static resultset
        ORowSetMatrix*                  m_pMatrix;              // represent the table struct
        ORowSetMatrix::iterator         m_aMatrixIter;          // represent a row of the table
        ORowSetMatrix::iterator         m_aMatrixEnd;           // present the row behind the last row of the table
        ORowSetCacheMap                 m_aCacheIterators;
        TOldRowSetRows                  m_aOldRows;

        ORowSetMatrix*                  m_pInsertMatrix;        // represent the rows which should be inserted normally this is only one
        ORowSetMatrix::iterator         m_aInsertRow;           // represent a insert row

        sal_Int32                       m_nLastColumnIndex;     // the last column ask for, used for wasNull()

        connectivity::OSQLTable         m_aUpdateTable;         // used for updates/deletes and inserts

        sal_Int32                   m_nFetchSize;
        sal_Int32                   m_nRowCount;
        sal_Int32                   m_nPrivileges;
        sal_Int32                   m_nPosition;                // 0 means before first (i.e. 1-based)

        sal_Int32                   m_nStartPos;                // start pos of the window zero based (inclusive)
        sal_Int32                   m_nEndPos;                  // end   pos of the window zero based (exclusive)

        sal_Bool                    m_bRowCountFinal ;
        sal_Bool                    m_bBeforeFirst ;
        sal_Bool                    m_bAfterLast ;
        sal_Bool                    m_bUpdated ;
        sal_Bool&                   m_bModified ;           // points to the rowset member m_bModified
        sal_Bool&                   m_bNew ;                // points to the rowset member m_bNew

        sal_Bool fill(ORowSetMatrix::iterator& _aIter,const ORowSetMatrix::const_iterator& _aEnd,sal_Int32& _nPos,sal_Bool _bCheck);
        sal_Bool reFillMatrix(sal_Int32 _nNewStartPos,sal_Int32 nNewEndPos);
        sal_Bool fillMatrix(sal_Int32 &_nNewStartPos,sal_Int32 &_nNewEndPos);
        sal_Bool moveWindow();
        // returns true when a keyset needs to be created.
        sal_Bool impl_createBookmarkSet_nothrow(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& _xRs);

        void firePropertyChange(sal_Int32 _nColumnIndex,const ::connectivity::ORowSetValue& _rOldValue);

        void rotateCacheIterator(ORowSetMatrix::difference_type _nDist);
        void updateValue(sal_Int32 columnIndex
                        ,const connectivity::ORowSetValue& x
                        ,ORowSetValueVector::Vector& io_aRow
                        ,::std::vector<sal_Int32>& o_ChangedColumns
                        );

        void impl_updateRowFromCache_throw(ORowSetValueVector::Vector& io_aRow
                                   ,::std::vector<sal_Int32>& o_ChangedColumns
                                   );
        // checks and set the flags isAfterLast isLast and position when afterlast is true
        void checkPositionFlags();
        void checkUpdateConditions(sal_Int32 columnIndex);
        sal_Bool checkJoin( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >& _xComposer,
                            const ::rtl::OUString& _sUpdateTableName);
        sal_Bool checkInnerJoin(const ::connectivity::OSQLParseNode *pNode
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection
                            ,const ::rtl::OUString& _sUpdateTableName);

        // clears the insert row
        void                    clearInsertRow();
        ORowSetMatrix::iterator calcPosition() const;

    protected:
        const ORowSetMatrix::iterator& getEnd() const { return m_aMatrixEnd;}
        // is called when after a moveToInsertRow a movement (next, etc) was called
        void cancelRowModification();
    public:
        ORowSetCache(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& _xRs,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >& _xAnalyzer,
                     const ::comphelper::ComponentContext& _rContext,
                     const ::rtl::OUString& _rUpdateTableName,
                     sal_Bool&  _bModified,
                     sal_Bool&  _bNew,
                     const ORowSetValueVector& _aParameterValueForCache,
                     const ::rtl::OUString& i_sRowSetFilter,
                     sal_Int32 i_nMaxRows);
        ~ORowSetCache();


        // called from the rowset when a updateXXX was called for the first time
        void setUpdateIterator(const ORowSetMatrix::iterator& _rOriginalRow);
        ORowSetCacheIterator createIterator(ORowSetBase* _pRowSet);
        void deleteIterator(const ORowSetBase* _pRowSet);
        // sets the size of the matrix
        void setFetchSize(sal_Int32 _nSize);

        TORowSetOldRowHelperRef registerOldRow();
        void deregisterOldRow(const TORowSetOldRowHelperRef& _rRow);

    // ::com::sun::star::sdbc::XResultSetMetaDataSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > getMetaData(  );

    // ::com::sun::star::sdbcx::XRowLocate
        ::com::sun::star::uno::Any getBookmark(  );
        sal_Bool moveToBookmark( const ::com::sun::star::uno::Any& bookmark );
        sal_Bool moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows );
        sal_Int32 compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second );
        sal_Bool hasOrderedBookmarks(  );
        sal_Int32 hashBookmark( const ::com::sun::star::uno::Any& bookmark );

    // ::com::sun::star::sdbc::XRowUpdate
        void updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length,ORowSetValueVector::Vector& io_aRow,::std::vector<sal_Int32>& o_ChangedColumns
             );
        void updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x,ORowSetValueVector::Vector& io_aRow ,::std::vector<sal_Int32>& o_ChangedColumns);
        void updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale,ORowSetValueVector::Vector& io_aRow ,::std::vector<sal_Int32>& o_ChangedColumns);
        void updateNull(sal_Int32 columnIndex
                        ,ORowSetValueVector::Vector& io_aRow
                        ,::std::vector<sal_Int32>& o_ChangedColumns
                        );

    // ::com::sun::star::sdbc::XResultSet
        sal_Bool next(  );
        sal_Bool isBeforeFirst(  );
        sal_Bool isAfterLast(  );
        sal_Bool isFirst(  );
        sal_Bool isLast(  );
        sal_Bool beforeFirst(  );
        sal_Bool afterLast(  );
        sal_Bool first(  );
        sal_Bool last(  );
        sal_Int32 getRow(  );
        sal_Bool absolute( sal_Int32 row );
        sal_Bool relative( sal_Int32 rows );
        sal_Bool previous(  );
        void refreshRow(  );
        sal_Bool rowUpdated(  );
        sal_Bool rowInserted(  );

    // ::com::sun::star::sdbc::XResultSetUpdate
        sal_Bool insertRow(::std::vector< ::com::sun::star::uno::Any >& o_aBookmarks);
        void resetInsertRow(sal_Bool _bClearInsertRow);

        void updateRow( ORowSetMatrix::iterator& _rUpdateRow,::std::vector< ::com::sun::star::uno::Any >& o_aBookmarks );
        bool deleteRow();
        void cancelRowUpdates(  );
        void moveToInsertRow(  );

        const ::std::map<sal_Int32,sal_Int32>& getKeyColumns() const { return m_aKeyColumns; }
        bool isResultSetChanged() const;
        void reset(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet);
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
