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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABPREPAREDSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABPREPAREDSTATEMENT_HXX

#include "MacabStatement.hxx"
#include "MacabResultSetMetaData.hxx"
#include <connectivity/FValue.hxx>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <cppuhelper/implbase.hxx>

namespace connectivity
{
    namespace macab
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

            void checkAndResizeParameters(sal_Int32 nParams) throw(css::sdbc::SQLException);
            void setMacabFields() const throw(css::sdbc::SQLException);

        protected:
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const css::uno::Any& rValue) throw (css::uno::Exception) override;

            virtual void resetParameters() const throw(css::sdbc::SQLException) override;
            virtual void getNextParameter(OUString &rParameter) const throw(css::sdbc::SQLException) override;
            virtual ~MacabPreparedStatement();

        public:
            DECLARE_SERVICE_INFO();
            MacabPreparedStatement(MacabConnection* _pConnection, const OUString& sql);

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XPreparedStatement
            using MacabCommonStatement::executeQuery;
            using MacabCommonStatement::executeUpdate;
            using MacabCommonStatement::execute;

            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL executeUpdate(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL execute(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

            // XParameters
            virtual void SAL_CALL setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setFloat( sal_Int32 parameterIndex, float x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setDouble( sal_Int32 parameterIndex, double x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setString( sal_Int32 parameterIndex, const OUString& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setBytes( sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setDate( sal_Int32 parameterIndex, const css::util::Date& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setTime( sal_Int32 parameterIndex, const css::util::Time& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setBinaryStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setCharacterStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setObject( sal_Int32 parameterIndex, const css::uno::Any& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setObjectWithInfo( sal_Int32 parameterIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setRef( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setBlob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setClob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL setArray( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL clearParameters(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

            // XCloseable
            virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

            // XResultSetMetaDataSupplier
            virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABPREPAREDSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
