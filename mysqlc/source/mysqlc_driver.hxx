/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef MYSQLC_SDRIVER_HXX
#define MYSQLC_SDRIVER_HXX

#include <config_lgpl.h>

#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/compbase2.hxx>
#include <cppconn/driver.h>
#include <osl/module.h>

namespace connectivity
{
    namespace mysqlc
    {
        using ::com::sun::star::sdbc::SQLException;
        using ::com::sun::star::uno::RuntimeException;
        using ::com::sun::star::uno::Exception;
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::Sequence;
        Reference< ::com::sun::star::uno::XInterface > SAL_CALL MysqlCDriver_CreateInstance(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw(Exception);

        typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::sdbc::XDriver,
                                                    ::com::sun::star::lang::XServiceInfo > ODriver_BASE;

        typedef void* (SAL_CALL * OMysqlCConnection_CreateInstanceFunction)(void* _pDriver);

        class MysqlCDriver : public ODriver_BASE
        {
        protected:
            Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
            ::osl::Mutex    m_aMutex;       // mutex is need to control member access
            OWeakRefArray   m_xConnections; // vector containing a list
                                            // of all the Connection objects
                                            // for this Driver
#ifndef SYSTEM_MYSQL_CPPCONN
#ifdef BUNDLE_MARIADB
            oslModule       m_hCConnModule;
            bool            m_bAttemptedLoadCConn;
#endif
            oslModule       m_hCppConnModule;
            bool            m_bAttemptedLoadCppConn;
#endif

            sql::Driver * cppDriver;

        public:

            MysqlCDriver(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // OComponentHelper
            void SAL_CALL disposing(void);
            // XInterface
            static OUString getImplementationName_Static()                  throw(RuntimeException);
            static Sequence< OUString > getSupportedServiceNames_Static()   throw(RuntimeException);

            // XServiceInfo
            OUString SAL_CALL getImplementationName()                       throw(RuntimeException);
            sal_Bool SAL_CALL supportsService(const OUString& ServiceName)  throw(RuntimeException);
            Sequence< OUString > SAL_CALL getSupportedServiceNames()        throw(RuntimeException);

            // XDriver
            Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect(const OUString& url, const Sequence< ::com::sun::star::beans::PropertyValue >& info)
                                                                            throw(SQLException, RuntimeException);

            sal_Bool SAL_CALL acceptsURL(const OUString& url) throw(SQLException, RuntimeException);
            Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo(const OUString& url, const Sequence< ::com::sun::star::beans::PropertyValue >& info)
                                                                            throw(SQLException, RuntimeException);

            sal_Int32 SAL_CALL getMajorVersion()                            throw(RuntimeException);
            sal_Int32 SAL_CALL getMinorVersion()                            throw(RuntimeException);

            inline Reference< ::com::sun::star::lang::XMultiServiceFactory > getFactory() const { return m_xFactory; }

            rtl_TextEncoding getDefaultEncoding() { return RTL_TEXTENCODING_UTF8; }

        private:
            void    impl_initCppConn_lck_throw();
        };
    } /* mysqlc */
} /* connectivity */

#endif // MYSQLC_SDRIVER_HXX
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
