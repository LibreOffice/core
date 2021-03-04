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
#include <ado/AStatement.hxx>

namespace connectivity::ado
{
        /*
        **  java_sql_ResultSet
        */
        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XResultSet,
                                                   css::sdbc::XRow,
                                                   css::sdbc::XResultSetMetaDataSupplier,
                                                   css::util::XCancellable,
                                                   css::sdbc::XWarningsSupplier,
                                                   css::sdbc::XCloseable,
                                                   css::sdbc::XColumnLocate> ODatabaseMetaDataResultSet_BASE;

        class ODatabaseMetaDataResultSet :  public cppu::BaseMutex,
                                    public  ODatabaseMetaDataResultSet_BASE,
                                    public  ::cppu::OPropertySetHelper,
                                    public  ::comphelper::OPropertyArrayUsageHelper<ODatabaseMetaDataResultSet>
        {
            std::vector<sal_Int32>        m_aColMapping; // pos 0 is unused so we don't have to decrement 1 every time

            std::map<sal_Int32, ::std::map<sal_Int32,sal_Int32> >            m_aValueRange;
            std::map<sal_Int32, ::std::map<sal_Int32,sal_Int32> >::iterator  m_aValueRangeIter;

            std::map<sal_Int32, std::map< OUString,sal_Int32> >              m_aStrValueRange;
            std::map<sal_Int32, std::map< OUString,sal_Int32> >::iterator    m_aStrValueRangeIter;

            std::map<sal_Int32, std::map< sal_Int32,OUString> >              m_aIntValueRange;
            std::map<sal_Int32, std::map< sal_Int32,OUString> >::iterator    m_aIntValueRangeIter;

            ADORecordset*                   m_pRecordSet;
            css::uno::WeakReferenceHelper   m_aStatement;
            css::uno::Reference< css::sdbc::XResultSetMetaData>  m_xMetaData;
            OLEVariant                      m_aValue;
            sal_Int32                       m_nRowPos;
            bool                            m_bWasNull;
            bool                            m_bEOF;
            bool                            m_bOnFirstAfterOpen;

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static sal_Int32 getResultSetConcurrency();
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static sal_Int32 getResultSetType();
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static sal_Int32 getFetchDirection();
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            sal_Int32 getFetchSize()            const;
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static OUString getCursorName();

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void setFetchDirection(sal_Int32 _par0);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void setFetchSize(sal_Int32 _par0);


            inline sal_Int32 mapColumn (sal_Int32   column);
            /// @throws css::sdbc::SQLException
            void checkRecordSet();
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            OLEVariant getValue(sal_Int32 columnIndex );

        protected:

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
        public:
            // a Constructor, that is needed for when Returning the Object is needed:
            ODatabaseMetaDataResultSet( ADORecordset* _pRecordSet);
            ~ODatabaseMetaDataResultSet() override;

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

            const std::vector<sal_Int32>& getColumnMapping() { return m_aColMapping; }

            void setCatalogsMap();
            void setSchemasMap();
            void setColumnPrivilegesMap();
            void setColumnsMap();
            void setTablesMap();
            void setProcedureColumnsMap();
            void setProceduresMap();
            void setExportedKeysMap();
            void setImportedKeysMap();
            void setPrimaryKeysMap();
            void setIndexInfoMap();
            void setTablePrivilegesMap();
            void setCrossReferenceMap();
            void setTypeInfoMap(bool _bJetEngine);
        };

        inline sal_Int32 ODatabaseMetaDataResultSet::mapColumn (sal_Int32   column)
        {
            sal_Int32   map = column;

            if (!m_aColMapping.empty())
            {
                // Validate column number
                map = m_aColMapping[column];
            }

            return map;
        }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
