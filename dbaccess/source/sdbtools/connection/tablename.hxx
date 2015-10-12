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

#ifndef INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX
#define INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX

#include "connectiondependent.hxx"

#include <com/sun/star/sdb/tools/XTableName.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>

namespace sdbtools
{

    // TableName
    typedef ::cppu::WeakImplHelper<   css::sdb::tools::XTableName
                                  >   TableName_Base;
    struct TableName_Impl;
    /** default implementation for XTableName
    */
    class TableName :public TableName_Base
                    ,public ConnectionDependentComponent
    {
    private:
        ::std::unique_ptr< TableName_Impl >   m_pImpl;

    public:
        /** constructs the instance

            @param _rContext
                the component's context
            @param  _rxConnection
                the connection to work with. Will be held weak. Must not be <NULL/>.

            @throws css::lang::NullPointerException
                if _rxConnection is <NULL/>
        */
        TableName(
            const css::uno::Reference< css::uno::XComponentContext >& _rContext,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
        );

        // XTableName
        virtual OUString SAL_CALL getCatalogName() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setCatalogName( const OUString& _catalogname ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getSchemaName() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setSchemaName( const OUString& _schemaname ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getTableName() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setTableName( const OUString& _tablename ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getNameForSelect() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getTable() throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setTable( const css::uno::Reference< css::beans::XPropertySet >& _table ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getComposedName( ::sal_Int32 Type, sal_Bool _Quote ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setComposedName( const OUString& ComposedName, ::sal_Int32 Type ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        virtual ~TableName();

    private:
        TableName( const TableName& ) = delete;
        TableName& operator=( const TableName& ) = delete;
    };

} // namespace sdbtools

#endif // INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
