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

#ifndef INCLUDED_MYSQLC_SOURCE_MYSQLC_STATEMENT_HXX
#define INCLUDED_MYSQLC_SOURCE_MYSQLC_STATEMENT_HXX

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
#include <rtl/ref.hxx>

namespace connectivity
{
    namespace mysqlc
    {
        using ::com::sun::star::sdbc::SQLWarning;
        using ::com::sun::star::sdbc::SQLException;
        using ::com::sun::star::uno::Any;
        using ::com::sun::star::uno::RuntimeException;

        typedef ::cppu::WeakComponentImplHelper5<   css::sdbc::XStatement,
                                                    css::sdbc::XWarningsSupplier,
                                                    css::util::XCancellable,
                                                    css::sdbc::XCloseable,
                                                    css::sdbc::XMultipleResults> OCommonStatement_IBase;

        //************ Class: OCommonStatement
        // is a base class for the normal statement and for the prepared statement

        class OCommonStatement  :public OBase_Mutex
                                ,public OCommonStatement_IBase
                                ,public ::cppu::OPropertySetHelper
                                ,public OPropertyArrayUsageHelper<OCommonStatement>

        {
        private:
            SQLWarning m_aLastWarning;

        protected:
            rtl::Reference<OConnection> m_xConnection;  // The owning Connection object

            sql::Statement          *cppStatement;

        protected:
            void disposeResultSet();

            // OPropertyArrayUsageHelper
            ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

            // OPropertySetHelper
            ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() SAL_OVERRIDE;
            sal_Bool SAL_CALL convertFastPropertyValue(Any & rConvertedValue, Any & rOldValue,
                                                               sal_Int32 nHandle, const Any& rValue) SAL_OVERRIDE;

            void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) SAL_OVERRIDE;

            void SAL_CALL getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const SAL_OVERRIDE;
            virtual ~OCommonStatement();

        protected:
            OCommonStatement(OConnection* _pConnection, sql::Statement *_cppStatement);

        public:
            using OCommonStatement_IBase::rBHelper;
            using OCommonStatement_IBase::operator css::uno::Reference< css::uno::XInterface >;

            // OComponentHelper
            void SAL_CALL disposing() SAL_OVERRIDE;

            // XInterface
            void SAL_CALL release()             throw() SAL_OVERRIDE;

            void SAL_CALL acquire()             throw() SAL_OVERRIDE;

            // XInterface
            Any SAL_CALL queryInterface(const css::uno::Type & rType) SAL_OVERRIDE;

            //XTypeProvider
            css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() SAL_OVERRIDE;

            // XPropertySet
            css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() SAL_OVERRIDE;

            // XStatement
            css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery(const rtl::OUString& sql) SAL_OVERRIDE;

            sal_Int32 SAL_CALL executeUpdate(const rtl::OUString& sql) SAL_OVERRIDE;

            sal_Bool SAL_CALL execute( const rtl::OUString& sql ) SAL_OVERRIDE;

            css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection() SAL_OVERRIDE;

            // XWarningsSupplier
            Any SAL_CALL getWarnings() SAL_OVERRIDE;

            void SAL_CALL clearWarnings() SAL_OVERRIDE;

            // XCancellable
            void SAL_CALL cancel() SAL_OVERRIDE;

            // XCloseable
            void SAL_CALL close() SAL_OVERRIDE;

            // XMultipleResults
            css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getResultSet() SAL_OVERRIDE;

            sal_Int32 SAL_CALL getUpdateCount() SAL_OVERRIDE;

            sal_Bool SAL_CALL getMoreResults() SAL_OVERRIDE;

            // other methods
            OConnection* getOwnConnection() const { return m_xConnection.get();}

        private:
            using ::cppu::OPropertySetHelper::getFastPropertyValue;
        };


        class OStatement final : public OCommonStatement,
                            public css::sdbc::XBatchExecution,
                            public css::lang::XServiceInfo

        {
            virtual ~OStatement(){}

        public:
            // A constructor which is required for the return of the objects
            OStatement(OConnection* _pConnection, sql::Statement *_cppStatement) :
                OCommonStatement(_pConnection, _cppStatement) {}

            virtual rtl::OUString SAL_CALL getImplementationName() SAL_OVERRIDE;

            virtual sal_Bool SAL_CALL supportsService(
                rtl::OUString const & ServiceName)
                SAL_OVERRIDE;

            virtual css::uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames()
                SAL_OVERRIDE;

            Any SAL_CALL queryInterface( const css::uno::Type & rType ) SAL_OVERRIDE;

            void SAL_CALL acquire()             throw() SAL_OVERRIDE;
            void SAL_CALL release()             throw() SAL_OVERRIDE;

            // XBatchExecution
            void SAL_CALL addBatch(const rtl::OUString& sql) SAL_OVERRIDE;

            void SAL_CALL clearBatch() SAL_OVERRIDE;

            css::uno::Sequence< sal_Int32 > SAL_CALL executeBatch() SAL_OVERRIDE;

        };
    }
}
#endif // INCLUDED_MYSQLC_SOURCE_MYSQLC_STATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
