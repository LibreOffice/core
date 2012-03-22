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

#ifndef MYSQLC_SDRIVER_HXX
#define MYSQLC_SDRIVER_HXX

#include "mysqlc_connection.hxx"

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/compbase2.hxx>
#include <preextstl.h>
#include <cppconn/driver.h>
#include <postextstl.h>
#include <osl/module.h>

namespace connectivity
{
    namespace mysqlc
    {
        using ::rtl::OUString;
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
