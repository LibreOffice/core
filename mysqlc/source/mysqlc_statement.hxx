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

#ifndef MYSQLC_STATEMENT_HXX
#define MYSQLC_STATEMENT_HXX

#include "mysqlc_connection.hxx"
#include "mysqlc_subcomponent.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/util/XCancellable.hpp>

#include <cppconn/statement.h>
#include <cppuhelper/compbase5.hxx>
#include <list>

namespace connectivity
{
    namespace mysqlc
    {
        using ::com::sun::star::sdbc::SQLWarning;
        using ::com::sun::star::sdbc::SQLException;
        using ::com::sun::star::uno::Any;
        using ::com::sun::star::uno::RuntimeException;

        typedef ::cppu::WeakComponentImplHelper5<   ::com::sun::star::sdbc::XStatement,
                                                    ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::util::XCancellable,
                                                    ::com::sun::star::sdbc::XCloseable,
                                                    ::com::sun::star::sdbc::XMultipleResults> OCommonStatement_IBase;

        class OCommonStatement;
        typedef OSubComponent< OCommonStatement, OCommonStatement_IBase >   OStatement_CBase;


        //************ Class: OCommonStatement
        // is a base class for the normal statement and for the prepared statement

        class OCommonStatement  :public OBase_Mutex
                                ,public OCommonStatement_IBase
                                ,public ::cppu::OPropertySetHelper
                                ,public OPropertyArrayUsageHelper<OCommonStatement>
                                ,public OStatement_CBase

        {
            friend class OSubComponent< OCommonStatement, OCommonStatement_IBase >;

        private:
            SQLWarning m_aLastWarning;

        protected:
            ::std::list< OUString>  m_aBatchList;

            OConnection*            m_pConnection;  // The owning Connection object

            sql::Statement          *cppStatement;

        protected:
            void disposeResultSet();

            // OPropertyArrayUsageHelper
            ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

            // OPropertySetHelper
            ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() SAL_OVERRIDE;
            sal_Bool SAL_CALL convertFastPropertyValue(Any & rConvertedValue, Any & rOldValue,
                                                               sal_Int32 nHandle, const Any& rValue)
                                                throw (::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;

            void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
                                                throw(::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

            void SAL_CALL getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const SAL_OVERRIDE;
            virtual ~OCommonStatement();

        protected:
            OCommonStatement(OConnection* _pConnection, sql::Statement *_cppStatement);

        public:
            ::cppu::OBroadcastHelper& rBHelper;
            using OCommonStatement_IBase::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

            // OComponentHelper
            void SAL_CALL disposing(void) SAL_OVERRIDE;

            // XInterface
            void SAL_CALL release()             throw() SAL_OVERRIDE;

            void SAL_CALL acquire()             throw() SAL_OVERRIDE;

            // XInterface
            Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type & rType)
                                                throw(RuntimeException, std::exception) SAL_OVERRIDE;

            //XTypeProvider
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                                throw(RuntimeException, std::exception) SAL_OVERRIDE;

            // XPropertySet
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
                                                throw(RuntimeException, std::exception) SAL_OVERRIDE;

            // XStatement
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery(const OUString& sql)
                                                throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            sal_Int32 SAL_CALL executeUpdate(const OUString& sql)
                                                throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            sal_Bool SAL_CALL execute( const OUString& sql )
                                                throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection()
                                                throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            // XWarningsSupplier
            Any SAL_CALL getWarnings()      throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL clearWarnings()       throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            // XCancellable
            void SAL_CALL cancel()              throw(RuntimeException, std::exception) SAL_OVERRIDE;

            // XCloseable
            void SAL_CALL close()               throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            // XMultipleResults
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSet()
                                                throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            sal_Int32 SAL_CALL getUpdateCount() throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            sal_Bool SAL_CALL getMoreResults()  throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            // other methods
            OConnection* getOwnConnection() const { return m_pConnection;}

        private:
            using ::cppu::OPropertySetHelper::getFastPropertyValue;
        };


        class OStatement :  public OCommonStatement,
                            public ::com::sun::star::sdbc::XBatchExecution,
                            public ::com::sun::star::lang::XServiceInfo

        {
        protected:
            virtual ~OStatement(){}

        public:
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OStatement(OConnection* _pConnection, sql::Statement *_cppStatement) : OCommonStatement(_pConnection, _cppStatement) {}

            virtual OUString SAL_CALL getImplementationName()
                throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            virtual sal_Bool SAL_CALL supportsService(
                OUString const & ServiceName) throw (css::uno::RuntimeException, std::exception)
                SAL_OVERRIDE;

            virtual css::uno::Sequence<OUString> SAL_CALL
            getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception)
                SAL_OVERRIDE;

            Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
                                                throw(RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL acquire()             throw() SAL_OVERRIDE;
            void SAL_CALL release()             throw() SAL_OVERRIDE;

            // XBatchExecution
            void SAL_CALL addBatch(const OUString& sql)
                                                throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            void SAL_CALL clearBatch()          throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

            ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch()
                                                throw(SQLException, RuntimeException, std::exception) SAL_OVERRIDE;

        };
    }
}
#endif // MYSQLC_STATEMENT_HXX

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
