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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_QUERYCOMPOSER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_QUERYCOMPOSER_HXX

#include <connectivity/CommonTools.hxx>
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <cppuhelper/implbase.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include "apitools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>



namespace dbaccess
{
    typedef ::cppu::ImplHelper <    css::sdb::XSQLQueryComposer,
                                    css::sdb::XParametersSupplier,
                                    css::sdbcx::XTablesSupplier,
                                    css::sdbcx::XColumnsSupplier,
                                    css::lang::XServiceInfo    > OQueryComposer_BASE;

    class OQueryComposer :  public ::comphelper::OBaseMutex,
                            public OSubComponent,
                            public OQueryComposer_BASE
    {
        ::std::vector< OUString>                                   m_aFilters;
        ::std::vector< OUString>                                   m_aOrders;
        OUString m_sOrgFilter;
        OUString m_sOrgOrder;
        css::uno::Reference< css::sdb::XSingleSelectQueryComposer> m_xComposer;
        css::uno::Reference< css::sdb::XSingleSelectQueryComposer> m_xComposerHelper;

    protected:
        virtual void SAL_CALL disposing() override;
        virtual ~OQueryComposer();
    public:

        OQueryComposer( const css::uno::Reference< css::sdbc::XConnection>& _xConnection );

        // css::lang::XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;

        // css::uno::XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;
        // XSQLQueryComposer
        virtual OUString SAL_CALL getQuery(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setQuery( const OUString& command ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getComposedQuery(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getFilter(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > SAL_CALL getStructuredFilter(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getOrder(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL appendFilterByColumn( const css::uno::Reference< css::beans::XPropertySet >& column ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL appendOrderByColumn( const css::uno::Reference< css::beans::XPropertySet >& column, sal_Bool ascending ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFilter( const OUString& filter ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setOrder( const OUString& order ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        // XTablesSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTables(  ) throw(css::uno::RuntimeException, std::exception) override;
        // XColumnsSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getColumns(  ) throw(css::uno::RuntimeException, std::exception) override;
        // XParametersSupplier
        virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getParameters(  ) throw(css::uno::RuntimeException, std::exception) override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_QUERYCOMPOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
