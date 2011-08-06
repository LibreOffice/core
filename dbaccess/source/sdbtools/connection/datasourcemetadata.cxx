/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "datasourcemetadata.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/NullPointerException.hpp>
/** === end UNO includes === **/

#include <connectivity/dbmetadata.hxx>

//........................................................................
namespace sdbtools
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::uno::RuntimeException;
    /** === end UNO using === **/

    //====================================================================
    //= DataSourceMetaData_Impl
    //====================================================================
    struct DataSourceMetaData_Impl
    {
    };

    //====================================================================
    //= DataSourceMetaData
    //====================================================================
    //--------------------------------------------------------------------
    DataSourceMetaData::DataSourceMetaData( const ::comphelper::ComponentContext& _rContext, const Reference< XConnection >& _rxConnection )
        :ConnectionDependentComponent( _rContext )
        ,m_pImpl( new DataSourceMetaData_Impl )
    {
        if ( !_rxConnection.is() )
            throw NullPointerException();
        setWeakConnection( _rxConnection );
    }

    //--------------------------------------------------------------------
    DataSourceMetaData::~DataSourceMetaData()
    {
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL DataSourceMetaData::supportsQueriesInFrom(  ) throw (RuntimeException)
    {
        EntryGuard aGuard( *this );
        ::dbtools::DatabaseMetaData aMeta( getConnection() );
        return aMeta.supportsSubqueriesInFrom();
    }

//........................................................................
} // namespace sdbtools
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
