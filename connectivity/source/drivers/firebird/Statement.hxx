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

        typedef ::cppu::ImplHelper < ::com::sun::star::sdbc::XStatement >
            OStatement_Base;

        class OStatement :  public OStatementCommonBase,
                            public OStatement_Base,
                            public ::com::sun::star::sdbc::XBatchExecution,
                            public ::com::sun::star::lang::XServiceInfo
        {
        protected:
            virtual ~OStatement(){}

            XSQLDA* m_pSqlda;

        public:
            // a constructor, which is required for returning objects:
            explicit OStatement( Connection* _pConnection)
                : OStatementCommonBase( _pConnection),
                  m_pSqlda(0)
            {}

            virtual void disposeResultSet() override;

            DECLARE_SERVICE_INFO();

            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;

            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL
                executeQuery(const ::rtl::OUString& sql)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL executeUpdate(const ::rtl::OUString& sqlIn)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL
                execute(const ::rtl::OUString& sql)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL
                getConnection()
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XBatchExecution - UNSUPPORTED
            virtual void SAL_CALL addBatch( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL
                queryInterface(const ::com::sun::star::uno::Type & rType)
                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
                getTypes()
                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // XCloseable
            virtual void SAL_CALL close()
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException, std::exception) override;
            // OComponentHelper
            virtual void SAL_CALL disposing() override;


        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_STATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
