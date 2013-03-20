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
#if 1

#include <connectivity/CommonTools.hxx>
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <cppuhelper/implbase5.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include "apitools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>



namespace dbaccess
{
    typedef ::cppu::ImplHelper5<    ::com::sun::star::sdb::XSQLQueryComposer,
                                    ::com::sun::star::sdb::XParametersSupplier,
                                    ::com::sun::star::sdbcx::XTablesSupplier,
                                    ::com::sun::star::sdbcx::XColumnsSupplier,
                                    ::com::sun::star::lang::XServiceInfo    > OQueryComposer_BASE;

    class OQueryComposer :  public ::comphelper::OBaseMutex,
                            public OSubComponent,
                            public OQueryComposer_BASE
    {
        ::std::vector< ::rtl::OUString>                                                      m_aFilters;
        ::std::vector< ::rtl::OUString>                                                      m_aOrders;
        ::rtl::OUString m_sOrgFilter;
        ::rtl::OUString m_sOrgOrder;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer> m_xComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer> m_xComposerHelper;

    protected:
        virtual void SAL_CALL disposing();
        virtual ~OQueryComposer();
    public:

        OQueryComposer( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection );

        // ::com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);
        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::uno::XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
                throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
        // XSQLQueryComposer
        virtual ::rtl::OUString SAL_CALL getQuery(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setQuery( const ::rtl::OUString& command ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getComposedQuery(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getFilter(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > SAL_CALL getStructuredFilter(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getOrder(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL appendFilterByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL appendOrderByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column, sal_Bool ascending ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFilter( const ::rtl::OUString& filter ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setOrder( const ::rtl::OUString& order ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // XTablesSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);
        // XColumnsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);
        // XParametersSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getParameters(  ) throw(::com::sun::star::uno::RuntimeException);
    };
}
#endif // DBACCESS_CORE_API_QUERYCOMPOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
