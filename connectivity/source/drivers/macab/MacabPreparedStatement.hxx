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

#include "MacabStatement.hxx"
#include "MacabResultSetMetaData.hxx"
#include <connectivity/FValue.hxx>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <cppuhelper/implbase.hxx>

namespace connectivity::macab
{
        typedef ::cppu::ImplInheritanceHelper< MacabCommonStatement,
                                               css::sdbc::XPreparedStatement,
                                               css::sdbc::XParameters,
                                               css::sdbc::XResultSetMetaDataSupplier,
                                               css::lang::XServiceInfo> MacabPreparedStatement_BASE;

        class MacabPreparedStatement : public  MacabPreparedStatement_BASE
        {
        protected:
            OUString                        m_sSqlStatement;
            ::rtl::Reference< MacabResultSetMetaData >
                                            m_xMetaData;
            bool                            m_bPrepared;
            mutable sal_Int32               m_nParameterIndex;
            OValueRow                       m_aParameterRow;

            /// @throws css::sdbc::SQLException
            void checkAndResizeParameters(sal_Int32 nParams);
            /// @throws css::sdbc::SQLException
            void setMacabFields() const;

        protected:
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const css::uno::Any& rValue) override;

            virtual void resetParameters() const override;
            virtual void getNextParameter(OUString &rParameter) const override;
            virtual ~MacabPreparedStatement() override;

        public:
            DECLARE_SERVICE_INFO();
            MacabPreparedStatement(MacabConnection* _pConnection, const OUString& sql);

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XPreparedStatement
            using MacabCommonStatement::executeQuery;
            using MacabCommonStatement::executeUpdate;
            using MacabCommonStatement::execute;

            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery(  ) override;
            virtual sal_Int32 SAL_CALL executeUpdate(  ) override;
            virtual sal_Bool SAL_CALL execute(  ) override;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override;

            // XParameters
            virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) override;
            virtual void SAL_CALL setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) override;
            virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x ) override;
            virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x ) override;
            virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x ) override;
            virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x ) override;
            virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x ) override;
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

            // XCloseable
            virtual void SAL_CALL close(  ) override;

            // XResultSetMetaDataSupplier
            virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
