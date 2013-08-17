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

#ifndef DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX
#define DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX

#include "connectiondependent.hxx"

#include <com/sun/star/sdb/tools/XTableName.hpp>

#include <cppuhelper/implbase1.hxx>

#include <memory>

namespace sdbtools
{

    // TableName
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdb::tools::XTableName
                                    >   TableName_Base;
    struct TableName_Impl;
    /** default implementation for XTableName
    */
    class TableName :public TableName_Base
                    ,public ConnectionDependentComponent
    {
    private:
        ::std::auto_ptr< TableName_Impl >   m_pImpl;

    public:
        /** constructs the instance

            @param _rContext
                the component's context
            @param  _rxConnection
                the connection to work with. Will be held weak. Must not be <NULL/>.

            @throws ::com::sun::star::lang::NullPointerException
                if _rxConnection is <NULL/>
        */
        TableName(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
        );

        // XTableName
        virtual OUString SAL_CALL getCatalogName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCatalogName( const OUString& _catalogname ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getSchemaName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setSchemaName( const OUString& _schemaname ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getTableName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTableName( const OUString& _tablename ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getNameForSelect() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getTable() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTable( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _table ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getComposedName( ::sal_Int32 Type, ::sal_Bool _Quote ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setComposedName( const OUString& ComposedName, ::sal_Int32 Type ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~TableName();

    private:
        TableName();                                // never implemented
        TableName( const TableName& );              // never implemented
        TableName& operator=( const TableName& );   // never implemented
    };

} // namespace sdbtools

#endif // DBACCESS_SOURCE_SDBTOOLS_CONNECTION_TABLENAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
