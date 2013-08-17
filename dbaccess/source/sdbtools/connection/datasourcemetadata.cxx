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

#include "datasourcemetadata.hxx"

#include <com/sun/star/lang/NullPointerException.hpp>

#include <connectivity/dbmetadata.hxx>

namespace sdbtools
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::XComponentContext;

    // DataSourceMetaData_Impl
    struct DataSourceMetaData_Impl
    {
    };

    // DataSourceMetaData
    DataSourceMetaData::DataSourceMetaData( const Reference<XComponentContext>& _rContext, const Reference< XConnection >& _rxConnection )
        :ConnectionDependentComponent( _rContext )
        ,m_pImpl( new DataSourceMetaData_Impl )
    {
        if ( !_rxConnection.is() )
            throw NullPointerException();
        setWeakConnection( _rxConnection );
    }

    DataSourceMetaData::~DataSourceMetaData()
    {
    }

    ::sal_Bool SAL_CALL DataSourceMetaData::supportsQueriesInFrom(  ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        ::dbtools::DatabaseMetaData aMeta( getConnection() );
        return aMeta.supportsSubqueriesInFrom();
    }

} // namespace sdbtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
