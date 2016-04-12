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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_PREPAREDSTATEMENT_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_PREPAREDSTATEMENT_HXX
#include "mysqlc_statement.hxx"
#include "mysqlc_resultset.hxx"

#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedBatchExecution.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <cppconn/prepared_statement.h>

namespace connectivity
{
    namespace mysqlc
    {
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::Any;
        using ::com::sun::star::uno::Type;
        using ::com::sun::star::sdbc::SQLException;
        using ::com::sun::star::uno::RuntimeException;
        using ::com::sun::star::sdbc::XResultSetMetaData;

        typedef ::cppu::ImplHelper<    css::sdbc::XPreparedStatement,
                                        css::sdbc::XParameters,
                                        css::sdbc::XPreparedBatchExecution,
                                        css::sdbc::XResultSetMetaDataSupplier,
                                        css::lang::XServiceInfo> OPreparedStatement_BASE;

        class OPreparedStatement :  public OCommonStatement,
                                    public OPreparedStatement_BASE
        {
        protected:
            unsigned int        m_paramCount;   // number of placeholders
            Reference< XResultSetMetaData > m_xMetaData;

            void checkParameterIndex(sal_Int32 parameter);

        protected:
            void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
                                                                                    throw (css::uno::Exception, std::exception) SAL_OVERRIDE;
            virtual ~OPreparedStatement();
        public:
            virtual rtl::OUString SAL_CALL getImplementationName()
                throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            virtual sal_Bool SAL_CALL supportsService(
                rtl::OUString const & ServiceName) throw (css::uno::RuntimeException, std::exception)
                SAL_OVERRIDE;

            virtual css::uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception)
                SAL_OVERRIDE;

            OPreparedStatement(OConnection* _pConnection, sql::PreparedStatement * cppPrepStmt);

            //XInterface
            Any SAL_CALL queryInterface(const Type & rType) throw(RuntimeException, std::exception) SAL_OVERRIDE;
            void SAL_CALL acquire() throw() SAL_OVERRIDE;
            void SAL_CALL release() throw() SAL_OVERRIDE;

            //XTypeProvider
            css::uno::Sequence< Type > SAL_CALL getTypes()
                                                                                    throw(RuntimeException, std::exception) SAL_OVERRIDE;

            // XPreparedStatement
            Reference< css::sdbc::XResultSet > SAL_CALL executeQuery() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;
            sal_Int32 SAL_CALL executeUpdate() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;
            sal_Bool SAL_CALL execute() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;
            Reference< css::sdbc::XConnection > SAL_CALL getConnection() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            // XStatement
            using OCommonStatement::executeQuery;
            using OCommonStatement::executeUpdate;
            using OCommonStatement::execute;

            // XParameters
            void SAL_CALL setNull(sal_Int32 parameter, sal_Int32 sqlType)           throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setObjectNull(sal_Int32 parameter, sal_Int32 sqlType, const rtl::OUString& typeName)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setBoolean(sal_Int32 parameter, sal_Bool x)               throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setByte(sal_Int32 parameter, sal_Int8 x)                  throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setShort(sal_Int32 parameter, sal_Int16 x)                throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setInt(sal_Int32 parameter, sal_Int32 x)                  throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setLong(sal_Int32 parameter, sal_Int64 x)                 throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setFloat(sal_Int32 parameter, float x)                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setDouble(sal_Int32 parameter, double x)                  throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setString(sal_Int32 parameter, const rtl::OUString& x)         throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setBytes(sal_Int32 parameter, const css::uno::Sequence< sal_Int8 >& x)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setDate(sal_Int32 parameter, const css::util::Date& x)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setTime(sal_Int32 parameter, const css::util::Time& x)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;
            void SAL_CALL setTimestamp(sal_Int32 parameter, const css::util::DateTime& x)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setBinaryStream(sal_Int32 parameter, const Reference< css::io::XInputStream >& x, sal_Int32 length)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setCharacterStream(sal_Int32 parameter, const Reference< css::io::XInputStream >& x, sal_Int32 length)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setObject(sal_Int32 parameter, const Any& x)              throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setObjectWithInfo(sal_Int32 parameter, const Any& x, sal_Int32 targetSqlType, sal_Int32 scale)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setRef(sal_Int32 parameter, const Reference< css::sdbc::XRef >& x)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setBlob(sal_Int32 parameter, const Reference< css::sdbc::XBlob >& x)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setClob(sal_Int32 parameter, const Reference< css::sdbc::XClob >& x)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL setArray(sal_Int32 parameter, const Reference< css::sdbc::XArray >& x)
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL clearParameters() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            // XPreparedBatchExecution
            void SAL_CALL addBatch() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;
            void SAL_CALL clearBatch() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;
            css::uno::Sequence< sal_Int32 > SAL_CALL executeBatch()    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            // XCloseable
            void SAL_CALL close() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            // XResultSetMetaDataSupplier
            Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData()
                                                                                    throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;
        };
    } /* mysqlc */
} /* connectivity */
#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_PREPAREDSTATEMENT_HXX

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
