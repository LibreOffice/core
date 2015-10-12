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

#ifndef INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_DATASOURCEMETADATA_HXX
#define INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_DATASOURCEMETADATA_HXX

#include "connectiondependent.hxx"

#include <com/sun/star/sdb/tools/XDataSourceMetaData.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>

namespace sdbtools
{

    // DataSourceMetaData
    typedef ::cppu::WeakImplHelper<   css::sdb::tools::XDataSourceMetaData
                                  >   DataSourceMetaData_Base;
    struct DataSourceMetaData_Impl;
    /** default implementation for XDataSourceMetaData
    */
    class DataSourceMetaData    :public DataSourceMetaData_Base
                                ,public ConnectionDependentComponent
    {
    private:
        ::std::unique_ptr< DataSourceMetaData_Impl >   m_pImpl;

    public:
        /** constructs the instance
            @param _rContext
                the component's context
            @param  _rxConnection
                the connection to work with. Will be held weak. Must not be <NULL/>.
            @throws css::lang::NullPointerException
                if _rxConnection is <NULL/>
        */
        DataSourceMetaData(
            const css::uno::Reference< css::uno::XComponentContext >& _rContext,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
        );

        // XDataSourceMetaData
        virtual sal_Bool SAL_CALL supportsQueriesInFrom() throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    protected:
        virtual ~DataSourceMetaData();

    private:
        DataSourceMetaData( const DataSourceMetaData& ) = delete;
        DataSourceMetaData& operator=( const DataSourceMetaData& ) = delete;
    };

} // namespace sdbtools

#endif // INCLUDED_DBACCESS_SOURCE_SDBTOOLS_CONNECTION_DATASOURCEMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
