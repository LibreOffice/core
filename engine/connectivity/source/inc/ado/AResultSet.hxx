/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/proparrhlp.hxx>
#include <ado/AStatement.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity::ado
{
    /*
    **  java_sql_ResultSet
    */
    typedef ::cppu::WeakComponentImplHelper<  css::sdbc::XResultSet,
                                              css::sdbc::XRow,
                                              css::sdbc::XResultSetMetaDataSupplier,
                                              css::util::XCancellable,
                                              css::sdbc::XWarningsSupplier,
                                              css::sdbc::XResultSetUpdate,
                                              css::sdbc::XRowUpdate,
                                              css::sdbc::XCloseable,
                                              css::sdbc::XColumnLocate,
                                              css::sdbcx::XRowLocate,
                                              css::sdbcx::XDeleteRows,
                                              css::lang::XServiceInfo> OResultSet_BASE;

    class OResultSet :  public cppu::BaseMutex,
                        public  OResultSet_BASE,
                        public  ::cppu::OPropertySetHelper,
                        public  ::comphelper::OPropertyArrayUsageHelper<OResultSet>
    {

        ADORecordset*                   m_pRecordSet;
        OStatement_Base*                m_pStmt;
        css::uno::Reference< css::uno::XInterface>    m_xStatement;
        css::uno::Reference< css::sdbc::XResultSetMetaData>        m_xMetaData;
        std::vector<OLEVariant>       m_aBookmarks;
        OLEVariant                      m_aValue;
        long m_nRowPos;
        bool                            m_bEOF;
        bool                            m_bOnFirstAfterOpen;

        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getResultSetConcurrency() const;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getResultSetType()        const;
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
        void updateValue(sal_Int32 columnIndex,const OLEVariant& x);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        OLEVariant getValue(sal_Int32 columnIndex );

    protected:

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & getInfoHelper() override;
        virtual bool convertFastPropertyValue(
                                                            cpo::uno::Any & rConvertedValue,
                                                            cpo::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                                                            const cpo::uno::Any& rValue ) override;
        virtual void setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const cpo::uno::Any& rValue
                                                 ) override;
        virtual void getFastPropertyValue(
                                cpo::uno::Any& rValue,
                                sal_Int32 nHandle
                                     ) const override;
        virtual ~OResultSet() override;
    public:
        // a Constructor, that is needed for when Returning the Object is needed:
        OResultSet( ADORecordset* _pRecordSet,OStatement_Base* pStmt);
        OResultSet( ADORecordset* _pRecordSet);

        // late constructor
        void construct();
        void setMetaData(const css::uno::Reference< css::sdbc::XResultSetMetaData>& _xMetaData) { m_xMetaData = _xMetaData;}

        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

        // ::cppu::OComponentHelper
        virtual void disposing() override;
        // XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;
        //XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
        // XResultSet
        virtual bool next(  ) override;
        virtual bool isBeforeFirst(  ) override;
        virtual bool isAfterLast(  ) override;
        virtual bool isFirst(  ) override;
        virtual bool isLast(  ) override;
        virtual void beforeFirst(  ) override;
        virtual void afterLast(  ) override;
        virtual bool first(  ) override;
        virtual bool last(  ) override;
        virtual sal_Int32 getRow(  ) override;
        virtual bool absolute( sal_Int32 row ) override;
        virtual bool relative( sal_Int32 rows ) override;
        virtual bool previous(  ) override;
        virtual void refreshRow(  ) override;
        virtual bool rowUpdated(  ) override;
        virtual bool rowInserted(  ) override;
        virtual bool rowDeleted(  ) override;
        virtual css::uno::Reference< css::uno::XInterface > getStatement(  ) override;
        // XRow
        virtual bool wasNull(  ) override;
        virtual OUString getString( sal_Int32 columnIndex ) override;
        virtual bool getBoolean( sal_Int32 columnIndex ) override;
        virtual sal_Int8 getByte( sal_Int32 columnIndex ) override;
        virtual sal_Int16 getShort( sal_Int32 columnIndex ) override;
        virtual sal_Int32 getInt( sal_Int32 columnIndex ) override;
        virtual sal_Int64 getLong( sal_Int32 columnIndex ) override;
        virtual float getFloat( sal_Int32 columnIndex ) override;
        virtual double getDouble( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Sequence< sal_Int8 > getBytes( sal_Int32 columnIndex ) override;
        virtual css::util::Date getDate( sal_Int32 columnIndex ) override;
        virtual css::util::Time getTime( sal_Int32 columnIndex ) override;
        virtual css::util::DateTime getTimestamp( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getBinaryStream( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getCharacterStream( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Any getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        virtual css::uno::Reference< css::sdbc::XRef > getRef( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XBlob > getBlob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XClob > getClob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XArray > getArray( sal_Int32 columnIndex ) override;
        // XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > getMetaData(  ) override;
        // XCancellable
        virtual void cancel(  ) override;
        // XCloseable
        virtual void close(  ) override;
        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) override;
        virtual void clearWarnings(  ) override;
        // XResultSetUpdate
        virtual void insertRow(  ) override;
        virtual void updateRow(  ) override;
        virtual void deleteRow(  ) override;
        virtual void cancelRowUpdates(  ) override;
        virtual void moveToInsertRow(  ) override;
        virtual void moveToCurrentRow(  ) override;
        // XRowUpdate
        virtual void updateNull( sal_Int32 columnIndex ) override;
        virtual void updateBoolean( sal_Int32 columnIndex, bool x ) override;
        virtual void updateByte( sal_Int32 columnIndex, sal_Int8 x ) override;
        virtual void updateShort( sal_Int32 columnIndex, sal_Int16 x ) override;
        virtual void updateInt( sal_Int32 columnIndex, sal_Int32 x ) override;
        virtual void updateLong( sal_Int32 columnIndex, sal_Int64 x ) override;
        virtual void updateFloat( sal_Int32 columnIndex, float x ) override;
        virtual void updateDouble( sal_Int32 columnIndex, double x ) override;
        virtual void updateString( sal_Int32 columnIndex, const OUString& x ) override;
        virtual void updateBytes( sal_Int32 columnIndex, const cpo::uno::Sequence< sal_Int8 >& x ) override;
        virtual void updateDate( sal_Int32 columnIndex, const css::util::Date& x ) override;
        virtual void updateTime( sal_Int32 columnIndex, const css::util::Time& x ) override;
        virtual void updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x ) override;
        virtual void updateBinaryStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void updateObject( sal_Int32 columnIndex, const cpo::uno::Any& x ) override;
        virtual void updateNumericObject( sal_Int32 columnIndex, const cpo::uno::Any& x, sal_Int32 scale ) override;
        // XColumnLocate
        virtual sal_Int32 findColumn( const OUString& columnName ) override;
        // XRowLocate
        virtual cpo::uno::Any getBookmark(  ) override;
        virtual bool moveToBookmark( const cpo::uno::Any& bookmark ) override;
        virtual bool moveRelativeToBookmark( const cpo::uno::Any& bookmark, sal_Int32 rows ) override;
        virtual sal_Int32 compareBookmarks( const cpo::uno::Any& first, const cpo::uno::Any& second ) override;
        virtual bool hasOrderedBookmarks(  ) override;
        virtual sal_Int32 hashBookmark( const cpo::uno::Any& bookmark ) override;
        // XDeleteRows
        virtual cpo::uno::Sequence< sal_Int32 > deleteRows( const cpo::uno::Sequence< cpo::uno::Any >& rows ) override;

    };

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
