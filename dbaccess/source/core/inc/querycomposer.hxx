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

#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/basemutex.hxx>
#include <connectivity/sqliterator.hxx>
#include <apitools.hxx>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>


namespace dbaccess
{
    typedef ::cppu::ImplHelper5<    css::sdb::XSQLQueryComposer,
                                    css::sdb::XParametersSupplier,
                                    css::sdbcx::XTablesSupplier,
                                    css::sdbcx::XColumnsSupplier,
                                    css::lang::XServiceInfo    > OQueryComposer_BASE;

    class OQueryComposer :  public ::cppu::BaseMutex,
                            public OSubComponent,
                            public OQueryComposer_BASE
    {
        std::vector< OUString>                                   m_aFilters;
        std::vector< OUString>                                   m_aOrders;
        OUString m_sOrgFilter;
        OUString m_sOrgOrder;
        css::uno::Reference< css::sdb::XSingleSelectQueryComposer> m_xComposer;
        css::uno::Reference< css::sdb::XSingleSelectQueryComposer> m_xComposerHelper;

    protected:
        virtual void SAL_CALL disposing() override;
        virtual ~OQueryComposer() override;
    public:

        OQueryComposer( const css::uno::Reference< css::sdbc::XConnection>& _xConnection );

        // css::lang::XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

        // css::uno::XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
        virtual void SAL_CALL acquire() noexcept override;
        virtual void SAL_CALL release() noexcept override;
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
        // XSQLQueryComposer
        virtual OUString SAL_CALL getQuery(  ) override;
        virtual void SAL_CALL setQuery( const OUString& command ) override;
        virtual OUString SAL_CALL getComposedQuery(  ) override;
        virtual OUString SAL_CALL getFilter(  ) override;
        virtual css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > SAL_CALL getStructuredFilter(  ) override;
        virtual OUString SAL_CALL getOrder(  ) override;
        virtual void SAL_CALL appendFilterByColumn( const css::uno::Reference< css::beans::XPropertySet >& column ) override;
        virtual void SAL_CALL appendOrderByColumn( const css::uno::Reference< css::beans::XPropertySet >& column, sal_Bool ascending ) override;
        virtual void SAL_CALL setFilter( const OUString& filter ) override;
        virtual void SAL_CALL setOrder( const OUString& order ) override;
        // XTablesSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTables(  ) override;
        // XColumnsSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getColumns(  ) override;
        // XParametersSupplier
        virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getParameters(  ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
