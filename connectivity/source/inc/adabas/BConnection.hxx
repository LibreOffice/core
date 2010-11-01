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
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#define _CONNECTIVITY_ADABAS_BCONNECTION_HXX_

#include <cppuhelper/compbase2.hxx>
#include "odbc/OConnection.hxx"
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>

namespace connectivity
{
    namespace odbc
    {
        class ODBCDriver;
    }
    namespace adabas
    {
        typedef connectivity::odbc::OConnection OConnection_BASE2;

        // we must use the name "OAdabasConnection" because of a compiler bug

        class OAdabasConnection :    public OConnection_BASE2
        {
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbcx::XTablesSupplier>      m_xCatalog;

        protected:
            virtual SQLRETURN                           openConnectionWithAuth(const ::rtl::OUString& aConnectStr,sal_Int32 nTimeOut, const ::rtl::OUString& _uid,const ::rtl::OUString& _pwd);
            virtual connectivity::odbc::OConnection*    cloneConnection(); // creates a new connection
        public:
            virtual SQLRETURN   Construct( const ::rtl::OUString& url,const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info) throw(::com::sun::star::sdbc::SQLException);

           OAdabasConnection(const SQLHANDLE _pDriverHandle,connectivity::odbc::ODBCDriver*      _pDriver);

            // OComponentHelper;
            virtual void SAL_CALL disposing();

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > createCatalog();
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            //XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            ::rtl::Reference<OSQLColumns> createSelectColumns(const ::rtl::OUString& _rSql);
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_BCONNECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
