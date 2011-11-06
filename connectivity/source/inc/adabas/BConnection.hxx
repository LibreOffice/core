/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#define _CONNECTIVITY_ADABAS_BCONNECTION_HXX_

#include <cppuhelper/compbase2.hxx>
#include "odbc/OConnection.hxx"
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <cppuhelper/weakref.hxx>
#include <vos/ref.hxx>

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

            ::vos::ORef<OSQLColumns> createSelectColumns(const ::rtl::OUString& _rSql);
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_BCONNECTION_HXX_

