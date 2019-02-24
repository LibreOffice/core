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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MRESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MRESULTSET_HXX

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/proparrhlp.hxx>
#include <tools/gen.hxx>
#include <rtl/ref.hxx>
#include "MStatement.hxx"
#include "MQueryHelper.hxx"
#include <connectivity/CommonTools.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/sqliterator.hxx>
#include <TSortIndex.hxx>

namespace connectivity
{
    namespace mork
    {

        /*
        **  java_sql_ResultSet
        */
        typedef ::cppu::WeakComponentImplHelper<      css::sdbc::XResultSet,
                                                      css::sdbc::XRow,
                                                      css::sdbc::XResultSetMetaDataSupplier,
                                                      css::util::XCancellable,
                                                      css::sdbc::XWarningsSupplier,
                                                      css::sdbc::XCloseable,
                                                      css::sdbc::XColumnLocate,
                                                      css::sdbc::XResultSetUpdate,
                                                      css::sdbc::XRowUpdate,
                                                      css::sdbcx::XRowLocate,
                                                      css::sdbcx::XDeleteRows,
                                                      css::lang::XServiceInfo> OResultSet_BASE;


        typedef sal_Int64 TVoidPtr;
        typedef std::allocator< TVoidPtr >    TVoidAlloc;
        typedef std::vector<TVoidPtr>         TVoidVector;

        class OResultSet :  public  cppu::BaseMutex,
                            public  OResultSet_BASE,
                            public  ::cppu::OPropertySetHelper,
                            public  ::comphelper::OPropertyArrayUsageHelper<OResultSet>
        {
        protected:
            OCommonStatement*                           m_pStatement;
            css::uno::Reference< css::uno::XInterface>            m_xStatement;
            css::uno::Reference< css::sdbc::XResultSetMetaData>   m_xMetaData;
            sal_uInt32                                  m_nRowPos;
            bool                                        m_bWasNull;
            sal_Int32                                   m_nResultSetType;
            sal_Int32                                   m_nFetchDirection;


            std::shared_ptr< ::connectivity::OSQLParseTreeIterator >
                                                        m_pSQLIterator;
            const connectivity::OSQLParseNode*          m_pParseTree;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                css::uno::Any & rConvertedValue,
                                css::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const css::uno::Any& rValue ) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const css::uno::Any& rValue
                                     ) override;
            virtual void SAL_CALL getFastPropertyValue(
                                    css::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const override;

            // you can't delete objects of this type
            virtual ~OResultSet() override;
        public:
            DECLARE_SERVICE_INFO();

            OResultSet(OCommonStatement* pStmt, const std::shared_ptr< ::connectivity::OSQLParseTreeIterator >& _pSQLIterator );

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
            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) override;

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
            // XRowLocate
            virtual css::uno::Any SAL_CALL getBookmark(  ) override;
            virtual sal_Bool SAL_CALL moveToBookmark( const css::uno::Any& bookmark ) override;
            virtual sal_Bool SAL_CALL moveRelativeToBookmark( const css::uno::Any& bookmark, sal_Int32 rows ) override;
            virtual sal_Int32 SAL_CALL compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) override;
            virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) override;
            virtual sal_Int32 SAL_CALL hashBookmark( const css::uno::Any& bookmark ) override;
            // XDeleteRows
            virtual css::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const css::uno::Sequence< css::uno::Any >& rows ) override;

protected:
            //MQuery                   m_aQuery;
            MQueryHelper             m_aQueryHelper;
            rtl::Reference<OTable>   m_xTable;
            sal_Int32                   m_CurrentRowCount;
            css::uno::Reference< css::container::XNameAccess >
                                     m_xTableColumns;

            std::vector<sal_Int32> m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time
            std::vector<sal_Int32> m_aOrderbyColumnNumber;
            std::vector<TAscendingOrder>  m_aOrderbyAscending;
            css::uno::Sequence< OUString> m_aColumnNames;
            OValueRow                m_aRow;
            OValueRow                m_aParameterRow;
            sal_Int32                m_nParamIndex;
            bool                 m_bIsAlwaysFalseQuery;
            ::rtl::Reference<OKeySet>     m_pKeySet;
            TriState                      m_bIsReadOnly;
            void resetParameters() { m_nParamIndex = 0; }

            ::rtl::Reference<connectivity::OSQLColumns>  m_xColumns; // this are the select columns

            void parseParameter( const OSQLParseNode* pNode, OUString& rMatchString );
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void fillRowData();
            void analyseWhereClause( const OSQLParseNode*                 parseTree,
                                     MQueryExpression                    &queryExpression);

            bool isCount() const;

            bool IsSorted() const { return !m_aOrderbyColumnNumber.empty(); }

            enum eRowPosition {
                NEXT_POS, PRIOR_POS, FIRST_POS, LAST_POS, ABSOLUTE_POS, RELATIVE_POS
            };

            sal_uInt32  currentRowCount();

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            bool fetchRow(sal_Int32 rowIndex,bool bForceReload=false);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            bool fetchCurrentRow();
            bool validRow( sal_uInt32 nRow );
            bool seekRow( eRowPosition pos, sal_Int32 nOffset = 0 );
            sal_Int32 deletedCount();
            void fillKeySet(sal_Int32 nMaxCardNumber);  //When we get new rows, fill the m_pKeySet items for them
            sal_Int32 getRowForCardNumber(sal_Int32 nCardNum);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            const ORowSetValue& getValue(sal_Int32 rowIndex, sal_Int32 columnIndex);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void updateValue(sal_Int32 columnIndex,const ORowSetValue& x );
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static void checkPendingUpdate();
            sal_Int32 getCurrentCardNumber();

public:
             bool determineReadOnly();
            // MozAddressbook Specific methods
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void executeQuery();

            void setTable(OTable* _rTable);

            void setParameterRow(const OValueRow& _rParaRow)
                      { m_aParameterRow = _rParaRow; }

            void setBindingRow(const OValueRow& _aRow)
                      { m_aRow = _aRow; }

            void setColumnMapping(const std::vector<sal_Int32>& _aColumnMapping);

            void setOrderByColumns(const std::vector<sal_Int32>& _aColumnOrderBy);

            void setOrderByAscending(const std::vector<TAscendingOrder>& _aOrderbyAsc);

            inline sal_Int32 mapColumn(sal_Int32 column);

            /// @throws css::sdbc::SQLException
            void checkIndex(sal_Int32 columnIndex );

            static void setBoundedColumns(
                const OValueRow& _rRow,
                const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,
                const css::uno::Reference< css::container::XIndexAccess>& _xNames,
                bool _bSetColumnMapping,
                const css::uno::Reference< css::sdbc::XDatabaseMetaData>& _xMetaData,
                std::vector<sal_Int32>& _rColMapping);

            ::osl::Mutex&   getMutex() { return m_aMutex; }
            void            methodEntry();

            private:
                void impl_ensureKeySet()
                {
                    if ( !m_pKeySet.is() )
                        m_pKeySet = new OKeySet();
                }

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };

        inline sal_Int32 OResultSet::mapColumn(sal_Int32 column)
        {
            sal_Int32   map = column;

            OSL_ENSURE(column > 0, "OResultSet::mapColumn: invalid column index!");
                // the first column (index 0) is for convenience only. The
                // first real select column is no 1.
            if ((column > 0) && (column < static_cast<sal_Int32>(m_aColMapping.size())))
                map = m_aColMapping[column];

            return map;
        }

        class ResultSetEntryGuard : public ::osl::MutexGuard
        {
        public:
            explicit ResultSetEntryGuard( OResultSet& _rRS ) : ::osl::MutexGuard( _rRS.getMutex() )
            {
                _rRS.methodEntry();
            }
        };

    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MRESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
