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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_STATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_STATEMENT_HXX

#include "StatementCommonBase.hxx"

#include <cppuhelper/implbase.hxx>

namespace connectivity
{
    namespace firebird
    {

        typedef ::cppu::ImplHelper < css::sdbc::XStatement >
            OStatement_Base;

        class OStatement :  public OStatementCommonBase,
                            public OStatement_Base,
                            public css::sdbc::XBatchExecution,
                            public css::lang::XServiceInfo
        {
        protected:
            virtual ~OStatement() override {}

            XSQLDA* m_pSqlda;

        public:
            // a constructor, which is required for returning objects:
            explicit OStatement( Connection* _pConnection)
                : OStatementCommonBase( _pConnection),
                  m_pSqlda(nullptr)
            {}

            virtual void disposeResultSet() override;

            DECLARE_SERVICE_INFO();

            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;

            // XStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
                executeQuery(const ::rtl::OUString& sql)
                throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL executeUpdate(const ::rtl::OUString& sqlIn)
                throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL
                execute(const ::rtl::OUString& sql)
                throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL
                getConnection()
                throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            // XBatchExecution - UNSUPPORTED
            virtual void SAL_CALL addBatch( const ::rtl::OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearBatch(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

            // XInterface
            virtual css::uno::Any SAL_CALL
                queryInterface(const css::uno::Type & rType)
                throw(css::uno::RuntimeException, std::exception) override;

            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL
                getTypes()
                throw(css::uno::RuntimeException, std::exception) override;
            // OComponentHelper
            virtual void SAL_CALL disposing() override;


        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_STATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
