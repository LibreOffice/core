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

#include "StatementCommonBase.hxx"

#include <cppuhelper/implbase5.hxx>

#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <ibase.h>

namespace connectivity::firebird
    {

        class OBoundParam;
        typedef ::cppu::ImplHelper5<    css::sdbc::XPreparedStatement,
                                        css::sdbc::XParameters,
                                        css::sdbc::XPreparedBatchExecution,
                                        css::sdbc::XResultSetMetaDataSupplier,
                                        css::lang::XServiceInfo> OPreparedStatement_Base;

        class OPreparedStatement :  public  OStatementCommonBase,
                                    public  OPreparedStatement_Base
        {
        protected:
            OUString                                              m_sSqlStatement;
            css::uno::Reference< css::sdbc::XResultSetMetaData >  m_xMetaData;

            XSQLDA*         m_pOutSqlda;
            XSQLDA*         m_pInSqlda;
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void checkParameterIndex(sal_Int32 nParameterIndex);

            /**
             * Set a numeric value in the input SQLDA. If the destination
             * parameter is not of nType then an Exception will be thrown.
             *
             * @throws css::sdbc::SQLException
             * @throws css::uno::RuntimeException
             */
            template <typename T> void setValue(sal_Int32 nIndex, const T& nValue, ISC_SHORT nType);
            void setParameterNull(sal_Int32 nParameterIndex, bool bSetNull = true);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void ensurePrepared();
            /**
             * Assumes that all necessary mutexes have been taken.
             */
            void openBlobForWriting(isc_blob_handle& rBlobHandle, ISC_QUAD& rBlobId);
            /**
             * Assumes that all necessary mutexes have been taken.
             */
            void closeBlobAfterWriting(isc_blob_handle& rBlobHandle);
            void setClob(sal_Int32 nParamIndex, const OUString& rStr);

        protected:
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,
                                                                   const css::uno::Any& rValue) override;
            virtual ~OPreparedStatement() override;
        public:
            DECLARE_SERVICE_INFO();
            // a constructor, which is required for returning objects:
            OPreparedStatement( Connection* _pConnection,
                                const OUString& sql);

            //XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

            // XPreparedStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
                executeQuery() override;
            virtual sal_Int32 SAL_CALL
                executeUpdate() override;
            virtual sal_Bool SAL_CALL
                execute() override;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL
                getConnection() override;

            // XParameters
            virtual void SAL_CALL setNull(sal_Int32 nIndex, sal_Int32 nValue) override;
            virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) override;
            virtual void SAL_CALL setBoolean( sal_Int32 nIndex, sal_Bool nValue) override;
            virtual void SAL_CALL setByte(sal_Int32 nIndex, sal_Int8 nValue) override;
            virtual void SAL_CALL setShort(sal_Int32 nIndex, sal_Int16 nValue) override;
            virtual void SAL_CALL setInt(sal_Int32 nIndex, sal_Int32 nValue) override;
            virtual void SAL_CALL setLong(sal_Int32 nIndex, sal_Int64 nValue) override;
            virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) override;
            virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) override;
            virtual void SAL_CALL setString( sal_Int32 parameterIndex, const OUString& x ) override;
            virtual void SAL_CALL setBytes( sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x ) override;
            virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const css::util::Date& x ) override;
            virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const css::util::Time& x ) override;
            virtual void SAL_CALL setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x ) override;
            virtual void SAL_CALL setBinaryStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
            virtual void SAL_CALL setCharacterStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
            virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const css::uno::Any& x ) override;
            virtual void SAL_CALL setObjectWithInfo( sal_Int32 parameterIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) override;
            virtual void SAL_CALL setRef( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef >& x ) override;
            virtual void SAL_CALL setBlob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob >& x ) override;
            virtual void SAL_CALL setClob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob >& x ) override;
            virtual void SAL_CALL setArray( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray >& x ) override;
            virtual void SAL_CALL clearParameters(  ) override;

            // XPreparedBatchExecution -- UNSUPPORTED by firebird
            virtual void SAL_CALL
                addBatch() override;
            virtual void SAL_CALL
                clearBatch() override;
            virtual css::uno::Sequence< sal_Int32 > SAL_CALL
                executeBatch() override;

            // XCloseable
            virtual void SAL_CALL close() override;
            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XResultSetMetaDataSupplier
            virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;

        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
