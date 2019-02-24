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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FRESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FRESULTSET_HXX

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
#include <file/FStatement.hxx>
#include <connectivity/CommonTools.hxx>
#include <comphelper/propertycontainer.hxx>
#include <file/fanalyzer.hxx>
#include <file/FTable.hxx>
#include <file/filedllapi.hxx>
#include <TSortIndex.hxx>
#include <TSkipDeletedSet.hxx>
#include <com/sun/star/lang/XEventListener.hpp>

namespace connectivity
{
    namespace file
    {
        typedef ::cppu::WeakComponentImplHelper<  css::sdbc::XResultSet,
                                                  css::sdbc::XRow,
                                                  css::sdbc::XResultSetMetaDataSupplier,
                                                  css::util::XCancellable,
                                                  css::sdbc::XWarningsSupplier,
                                                  css::sdbc::XResultSetUpdate,
                                                  css::sdbc::XRowUpdate,
                                                  css::sdbc::XCloseable,
                                                  css::sdbc::XColumnLocate,
                                                  css::lang::XServiceInfo,
                                                  css::lang::XEventListener,
                                                  css::lang::XUnoTunnel> OResultSet_BASE;

        class OOO_DLLPUBLIC_FILE OResultSet :
                            public  cppu::BaseMutex,
                            public  ::connectivity::IResultSetHelper,
                            public  OResultSet_BASE,
                            public  ::comphelper::OPropertyContainer,
                            public  ::comphelper::OPropertyArrayUsageHelper<OResultSet>
        {

        protected:
            std::vector<sal_Int32>                m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time

            std::vector<sal_Int32>                m_aOrderbyColumnNumber;
            std::vector<TAscendingOrder>          m_aOrderbyAscending;

            OValueRefRow                            m_aSelectRow;
            OValueRefRow                            m_aRow;
            OValueRefRow                            m_aEvaluateRow; // contains all values of a row
            OValueRefRow                            m_aInsertRow;   // needed for insert by cursor
            ORefAssignValues                        m_aAssignValues; // needed for insert,update and parameters
                                                                    // to compare with the restrictions
            OSkipDeletedSet                         m_aSkipDeletedSet;

            ::rtl::Reference<OKeySet>                   m_pFileSet;
            OKeySet::Vector::iterator               m_aFileSetIter;


            std::unique_ptr<OSortIndex>             m_pSortIndex;
            ::rtl::Reference<connectivity::OSQLColumns> m_xColumns; // this are the select columns
            rtl::Reference<OFileTable>              m_pTable;
            connectivity::OSQLParseNode*            m_pParseTree;

            OSQLAnalyzer*                           m_pSQLAnalyzer;
            connectivity::OSQLParseTreeIterator&    m_aSQLIterator;

            sal_Int32                               m_nFetchSize;
            sal_Int32                               m_nResultSetType;
            sal_Int32                               m_nFetchDirection;
            sal_Int32                               m_nResultSetConcurrency;

            css::uno::Reference< css::uno::XInterface>            m_xStatement;
            css::uno::Reference< css::sdbc::XResultSetMetaData>   m_xMetaData;
            css::uno::Reference< css::container::XNameAccess>     m_xColNames; // table columns
            css::uno::Reference< css::container::XIndexAccess>    m_xColsIdx; // table columns


            sal_Int32                               m_nRowPos;
            sal_Int32                               m_nFilePos;
            sal_Int32                               m_nLastVisitedPos;
            sal_Int32                               m_nRowCountResult;
            sal_Int32                               m_nColumnCount;
            bool                                m_bWasNull;
            bool                                m_bInserted;            // true when moveToInsertRow was called
                                                                            // set to false when cursor moved or cancel
            bool                                m_bRowUpdated;
            bool                                m_bRowInserted;
            bool                                m_bRowDeleted;
            bool                                m_bShowDeleted;
            bool                                m_bIsCount;

            static void initializeRow(OValueRefRow& _rRow,sal_Int32 _nColumnCount);
            void construct();
            //sal_Bool evaluate();

            bool ExecuteRow(IResultSetHelper::Movement eFirstCursorPosition,
                                sal_Int32 nOffset = 1,
                                bool bEvaluate = true,
                                bool bRetrieveData = true);

            std::unique_ptr<OKeyValue> GetOrderbyKeyValue(OValueRefRow const & _rRow);
            bool IsSorted() const { return !m_aOrderbyColumnNumber.empty() && m_aOrderbyColumnNumber[0] >= 0;}

            // return true when the select statement is "select count(*) from table"
            bool isCount() const { return m_bIsCount; }
            /// @throws css::sdbc::SQLException
            void checkIndex(sal_Int32 columnIndex );

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            const ORowSetValue& getValue(sal_Int32 columnIndex);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void updateValue(sal_Int32 columnIndex,const ORowSetValue& x );
            // clear insert row
            void clearInsertRow();
            void sortRows();
        protected:

            using OResultSet_BASE::rBHelper;

            bool Move(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, bool bRetrieveData);
            virtual bool fillIndexValues(const css::uno::Reference< css::sdbcx::XColumnsSupplier> &_xIndex);

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

            virtual ~OResultSet() override;
        public:
            DECLARE_SERVICE_INFO();
            // a Constructor, that is needed for when Returning the Object is needed:
            OResultSet( OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator& _aSQLIterator);

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
            // XResultSet
            virtual sal_Bool SAL_CALL next(  ) override;
            virtual sal_Bool SAL_CALL isBeforeFirst(  ) override;
            virtual sal_Bool SAL_CALL isAfterLast(  ) override;
            virtual sal_Bool SAL_CALL isFirst(  ) override;
            virtual sal_Bool SAL_CALL isLast(  ) override;
            virtual void SAL_CALL beforeFirst(  ) override;
            virtual void SAL_CALL afterLast(  ) override;
            virtual sal_Bool SAL_CALL first(  ) override;
            virtual sal_Bool SAL_CALL last(  ) override;
            virtual sal_Int32 SAL_CALL getRow(  ) override;
            virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) override;
            virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) override;
            virtual sal_Bool SAL_CALL previous(  ) override;
            virtual void SAL_CALL refreshRow(  ) override;
            virtual sal_Bool SAL_CALL rowUpdated(  ) override;
            virtual sal_Bool SAL_CALL rowInserted(  ) override;
            virtual sal_Bool SAL_CALL rowDeleted(  ) override;
            virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getStatement(  ) override;
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  ) override;
            virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) override;
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) override;
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) override;
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) override;
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) override;
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) override;
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) override;
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) override;
            virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) override;
            virtual css::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) override;
            virtual css::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) override;
            virtual css::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) override;
            virtual css::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
            virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) override;
            // XResultSetMetaDataSupplier
            virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;
            // XCancellable
            virtual void SAL_CALL cancel(  ) override;
            // XCloseable
            virtual void SAL_CALL close(  ) override;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;
            // XResultSetUpdate
            virtual void SAL_CALL insertRow(  ) override;
            virtual void SAL_CALL updateRow(  ) override;
            virtual void SAL_CALL deleteRow(  ) override;
            virtual void SAL_CALL cancelRowUpdates(  ) override;
            virtual void SAL_CALL moveToInsertRow(  ) override;
            virtual void SAL_CALL moveToCurrentRow(  ) override;
            // XRowUpdate
            virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) override;
            virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) override;
            virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) override;
            virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) override;
            virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) override;
            virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) override;
            virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) override;
            virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) override;
            virtual void SAL_CALL updateString( sal_Int32 columnIndex, const OUString& x ) override;
            virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const css::uno::Sequence< sal_Int8 >& x ) override;
            virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const css::util::Date& x ) override;
            virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const css::util::Time& x ) override;
            virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x ) override;
            virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
            virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
            virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const css::uno::Any& x ) override;
            virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const css::uno::Any& x, sal_Int32 scale ) override;
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) override;
            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            //XEventlistener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

            // special methods
            inline sal_Int32 mapColumn(sal_Int32    column);
            void OpenImpl();
            void doTableSpecials(const OSQLTable& _xTable);

            sal_Int32 getRowCountResult() const { return m_nRowCountResult; }
            void setEvaluationRow(const OValueRefRow& _aRow)                     { m_aEvaluateRow = _aRow; }
            void setAssignValues(const ORefAssignValues& _aAssignValues)         { m_aAssignValues = _aAssignValues; }
            void setBindingRow(const OValueRefRow& _aRow)                        { m_aRow = _aRow; }
            void setSelectRow(const OValueRefRow& _rRow)
            {
                m_aSelectRow = _rRow;
                m_nColumnCount = m_aSelectRow->get().size();
            }
            void setColumnMapping(const std::vector<sal_Int32>& _aColumnMapping)   { m_aColMapping = _aColumnMapping; }
            void setSqlAnalyzer(OSQLAnalyzer* _pSQLAnalyzer)                     { m_pSQLAnalyzer = _pSQLAnalyzer; }

            void setOrderByColumns(const std::vector<sal_Int32>& _aColumnOrderBy)  { m_aOrderbyColumnNumber = _aColumnOrderBy; }
            void setOrderByAscending(const std::vector<TAscendingOrder>& _aOrderbyAsc)    { m_aOrderbyAscending = _aOrderbyAsc; }
            void setMetaData(const css::uno::Reference< css::sdbc::XResultSetMetaData>& _xMetaData) { m_xMetaData = _xMetaData;}

            static void setBoundedColumns(const OValueRefRow& _rRow,
                                    const OValueRefRow& _rSelectRow,
                                    const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,
                                    const css::uno::Reference< css::container::XIndexAccess>& _xNames,
                                    bool _bSetColumnMapping,
                                    const css::uno::Reference< css::sdbc::XDatabaseMetaData>& _xMetaData,
                                    std::vector<sal_Int32>& _rColMapping);

            // IResultSetHelper
            virtual bool move(IResultSetHelper::Movement _eCursorPosition, sal_Int32 _nOffset, bool _bRetrieveData) override;
            virtual sal_Int32 getDriverPos() const override;
            virtual bool isRowDeleted() const override;
        };

        inline sal_Int32 OResultSet::mapColumn(sal_Int32 column)
        {
            sal_Int32   map = column;

            OSL_ENSURE(column > 0, "file::OResultSet::mapColumn: invalid column index!");
            // the first column (index 0) is for convenience only. The first real select column is number 1.
            if ((column > 0) && (column < static_cast<sal_Int32>(m_aColMapping.size())))
                map = m_aColMapping[column];

            return map;
        }
    }
}
#endif // _CONNECTIVITY_FILE_ORESULTSET_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
