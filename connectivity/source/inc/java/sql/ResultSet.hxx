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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_RESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_RESULTSET_HXX

#include "java/lang/Object.hxx"
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
#include <comphelper/proparrhlp.hxx>
#include <connectivity/CommonTools.hxx>
#include "java/sql/JStatement.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "java/sql/ConnectionLog.hxx"

namespace connectivity
{

    /*
    **  java_sql_ResultSet
    */
    typedef ::cppu::WeakComponentImplHelper<      css::sdbc::XResultSet,
                                                  css::sdbc::XRow,
                                                  css::sdbc::XResultSetMetaDataSupplier,
                                                  css::util::XCancellable,
                                                  css::sdbc::XWarningsSupplier,
                                                  css::sdbc::XResultSetUpdate,
                                                  css::sdbc::XRowUpdate,
                                                  css::sdbc::XCloseable,
                                                  css::sdbc::XColumnLocate,
                                                  css::lang::XServiceInfo> java_sql_ResultSet_BASE;

    class java_sql_Connection;
    class java_sql_ResultSet :  public cppu::BaseMutex,
                                public  java_sql_ResultSet_BASE,
                                public  java_lang_Object,
                                public  ::cppu::OPropertySetHelper,
                                public  ::comphelper::OPropertyArrayUsageHelper<java_sql_ResultSet>
    {
        css::uno::Reference< css::uno::XInterface>            m_xStatement;
        java::sql::ConnectionLog                              m_aLogger;
        java_sql_Connection*                                  m_pConnection;

        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getResultSetConcurrency() const;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getResultSetType()        const;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getFetchDirection()       const;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getFetchSize()            const;
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        OUString getCursorName()     const;

        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setFetchDirection(sal_Int32 _par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setFetchSize(sal_Int32 _par0);
    protected:
        // Static data for the class
        static jclass theClass;

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
        virtual ~java_sql_ResultSet() override;
    public:
        DECLARE_SERVICE_INFO();
        virtual jclass getMyClass() const override;
        // A ctor that is needed for returning the object
        java_sql_ResultSet( JNIEnv * pEnv, jobject myObj, const java::sql::ConnectionLog& _rParentLogger,java_sql_Connection& _rConnection,
            java_sql_Statement_Base* pStmt = nullptr );

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

    public:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_RESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
