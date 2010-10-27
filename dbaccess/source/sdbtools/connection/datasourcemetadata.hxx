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

#ifndef DBACCESS_DATASOURCEMETADATA_HXX
#define DBACCESS_DATASOURCEMETADATA_HXX

#include "connectiondependent.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/tools/XDataSourceMetaData.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <memory>

//........................................................................
namespace sdbtools
{
//........................................................................

    //====================================================================
    //= DataSourceMetaData
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdb::tools::XDataSourceMetaData
                                    >   DataSourceMetaData_Base;
    struct DataSourceMetaData_Impl;
    /** default implementation for XDataSourceMetaData
    */
    class DataSourceMetaData    :public DataSourceMetaData_Base
                                ,public ConnectionDependentComponent
    {
    private:
        ::std::auto_ptr< DataSourceMetaData_Impl >   m_pImpl;

    public:
        /** constructs the instance
            @param _rContext
                the component's context
            @param  _rxConnection
                the connection to work with. Will be held weak. Must not be <NULL/>.
            @throws ::com::sun::star::lang::NullPointerException
                if _rxConnection is <NULL/>
        */
        DataSourceMetaData(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
        );

        // XDataSourceMetaData
        virtual ::sal_Bool SAL_CALL supportsQueriesInFrom(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~DataSourceMetaData();

    private:
        DataSourceMetaData();                                  // never implemented
        DataSourceMetaData( const DataSourceMetaData& );              // never implemented
        DataSourceMetaData& operator=( const DataSourceMetaData& );   // never implemented
    };


//........................................................................
} // namespace sdbtools
//........................................................................

#endif // DBACCESS_DATASOURCEMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
