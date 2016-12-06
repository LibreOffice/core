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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_ROWSETCACHE_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_ROWSETCACHE_HXX

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
#include <comphelper/propertycontainer.hxx>
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

        ::std::map<sal_Int32,sal_Int32>                       m_aKeyColumns;
        //the set can be static, bookmarkable or keyset
        css::uno::WeakReference< css::sdbc::XResultSet>       m_xSet;
        css::uno::Reference< css::sdbc::XResultSetMetaData >  m_xMetaData; // must be before m_aInsertRow
        css::uno::Reference< css::uno::XComponentContext>     m_aContext;

        css::uno::Reference< css::sdbc::XRow>                 m_xCacheSet;

        OCacheSet*                      m_pCacheSet;            // is a bookmarkable, keyset or static resultset
        ORowSetMatrix*                  m_pMatrix;              // represent the table struct
        ORowSetMatrix::iterator         m_aMatrixIter;          // represent a row of the table
        ORowSetMatrix::iterator         m_aMatrixEnd;           // present the row behind the last row of the table
        ORowSetCacheMap                 m_aCacheIterators;
        TOldRowSetRows                  m_aOldRows;

        ORowSetMatrix*                  m_pInsertMatrix;        // represent the rows which should be inserted normally this is only one
        ORowSetMatrix::iterator         m_aInsertRow;           // represent a insert row

        connectivity::OSQLTable         m_aUpdateTable;         // used for updates/deletes and inserts

        sal_Int32                   m_nFetchSize;
        sal_Int32                   m_nRowCount;
        sal_Int32                   m_nPrivileges;
        sal_Int32                   m_nPosition;                // 0 means before first (i.e. 1-based)

        sal_Int32                   m_nStartPos;                // start pos of the window zero based (inclusive)
        sal_Int32                   m_nEndPos;                  // end   pos of the window zero based (exclusive)

        bool                    m_bRowCountFinal ;
        bool                    m_bBeforeFirst ;
        bool                    m_bAfterLast ;
        bool                    m_bUpdated ;
        bool&                   m_bModified ;           // points to the rowset member m_bModified
        bool&                   m_bNew ;                // points to the rowset member m_bNew

        bool fill(ORowSetMatrix::iterator& _aIter, const ORowSetMatrix::const_iterator& _aEnd, sal_Int32& _nPos, bool _bCheck);
        bool reFillMatrix(sal_Int32 _nNewStartPos,sal_Int32 nNewEndPos);
        bool fillMatrix(sal_Int32 &_nNewStartPos,sal_Int32 &_nNewEndPos);
        void moveWindow();

        void rotateCacheIterator(ORowSetMatrix::difference_type _nDist);
        void rotateAllCacheIterators();
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
        bool checkJoin( const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                            const css::uno::Reference< css::sdb::XSingleSelectQueryAnalyzer >& _xComposer,
                            const OUString& _sUpdateTableName);
        bool checkInnerJoin(const ::connectivity::OSQLParseNode *pNode
                            ,const css::uno::Reference< css::sdbc::XConnection>& _xConnection
                            ,const OUString& _sUpdateTableName);

        // clears the insert row
        void                    clearInsertRow();
        ORowSetMatrix::iterator calcPosition() const;

    protected:
        const ORowSetMatrix::iterator& getEnd() const { return m_aMatrixEnd;}
        // is called when after a moveToInsertRow a movement (next, etc) was called
        void cancelRowModification();
    public:
        ORowSetCache(const css::uno::Reference< css::sdbc::XResultSet >& _xRs,
                     const css::uno::Reference< css::sdb::XSingleSelectQueryAnalyzer >& _xAnalyzer,
                     const css::uno::Reference< css::uno::XComponentContext >& _rContext,
                     const OUString& _rUpdateTableName,
                     bool&  _bModified,
                     bool&  _bNew,
                     const ORowSetValueVector& _aParameterValueForCache,
                     const OUString& i_sRowSetFilter,
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

    // css::sdbc::XResultSetMetaDataSupplier
        const css::uno::Reference< css::sdbc::XResultSetMetaData >& getMetaData(  ) { return m_xMetaData;}

    // css::sdbcx::XRowLocate
        css::uno::Any getBookmark(  );
        bool moveToBookmark( const css::uno::Any& bookmark );
        bool moveRelativeToBookmark( const css::uno::Any& bookmark, sal_Int32 rows );
        sal_Int32 compareBookmarks( const css::uno::Any& first, const css::uno::Any& second );
        bool hasOrderedBookmarks(  );
        sal_Int32 hashBookmark( const css::uno::Any& bookmark );

    // css::sdbc::XRowUpdate
        void updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length,ORowSetValueVector::Vector& io_aRow,::std::vector<sal_Int32>& o_ChangedColumns
             );
        void updateObject( sal_Int32 columnIndex, const css::uno::Any& x,ORowSetValueVector::Vector& io_aRow ,::std::vector<sal_Int32>& o_ChangedColumns);
        void updateNumericObject( sal_Int32 columnIndex, const css::uno::Any& x, sal_Int32 scale,ORowSetValueVector::Vector& io_aRow ,::std::vector<sal_Int32>& o_ChangedColumns);
        void updateNull(sal_Int32 columnIndex
                        ,ORowSetValueVector::Vector& io_aRow
                        ,::std::vector<sal_Int32>& o_ChangedColumns
                        );

    // css::sdbc::XResultSet
        bool next(  );
        bool isBeforeFirst(  ) const { return m_bBeforeFirst;}
        bool isAfterLast(  ) const { return m_bAfterLast;}
        bool isFirst(  );
        bool isLast(  );
        bool beforeFirst(  );
        bool afterLast(  );
        bool first(  );
        bool last(  );
        sal_Int32 getRow(  );
        bool absolute( sal_Int32 row );
        bool relative( sal_Int32 rows );
        bool previous(  );
        void refreshRow(  );
        bool rowUpdated(  );
        bool rowInserted(  );

    // css::sdbc::XResultSetUpdate
        bool insertRow(::std::vector< css::uno::Any >& o_aBookmarks);
        void resetInsertRow(bool _bClearInsertRow);

        void updateRow( ORowSetMatrix::iterator& _rUpdateRow,::std::vector< css::uno::Any >& o_aBookmarks );
        bool deleteRow();
        void cancelRowUpdates(  );
        void moveToInsertRow(  );

        const ::std::map<sal_Int32,sal_Int32>& getKeyColumns() const { return m_aKeyColumns; }
        bool isResultSetChanged() const;
        void reset(const css::uno::Reference< css::sdbc::XResultSet>& _xDriverSet);
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
