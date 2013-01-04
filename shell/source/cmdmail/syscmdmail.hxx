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

#ifndef _SYSTEMCMDMAIL_HXX_
#define _SYSTEMCMDMAIL_HXX_

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/system/XSystemMailProvider.hpp>

namespace shell
{
    typedef cppu::WeakComponentImplHelper3<
            com::sun::star::system::XSystemMailProvider,
            com::sun::star::system::XMailClient,
            com::sun::star::lang::XServiceInfo > SystemCommandMail_Base;

    class SystemCommandMail : protected cppu::BaseMutex,
                              public SystemCommandMail_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xConfigurationProvider;

    public:
        SystemCommandMail( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
        ~SystemCommandMail();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::system::XMailClient > SAL_CALL queryMailClient(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::system::XMailMessage > SAL_CALL createMailMessage(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL sendMailMessage( const ::com::sun::star::uno::Reference< ::com::sun::star::system::XMailMessage >& xMailMessage, sal_Int32 aFlag )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString SAL_CALL getImplementationName()
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw(::com::sun::star::uno::RuntimeException);

        static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > Create( const com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext);
        static rtl::OUString getImplementationName_static();
        static com::sun::star::uno::Sequence< rtl::OUString > getSupportedServiceNames_static();
    };
}

#endif
