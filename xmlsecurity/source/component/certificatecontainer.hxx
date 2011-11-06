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



#ifndef _XCERTIFICATECONTAINER_HXX_
#define _XCERTIFICATECONTAINER_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>

#ifndef _XCERTIFICATECONTAINER_HPP_
#include <com/sun/star/security/XCertificateContainer.hpp>
#endif

#ifndef _CERTIFICATECONTAINERSTATUS_HPP_
#include <com/sun/star/security/CertificateContainerStatus.hpp>
#endif


#include <vector>
#include <map>

using namespace com::sun::star;
using namespace cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class CertificateContainer : public ::cppu::WeakImplHelper2< ::com::sun::star::lang::XServiceInfo, ::com::sun::star::security::XCertificateContainer >
{
    private:
        typedef std::map< ::rtl::OUString, ::rtl::OUString > Map;
        Map certMap;
        Map certTrustMap;

        ::sal_Bool SAL_CALL searchMap( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name, Map &_certMap  );
        virtual ::sal_Bool SAL_CALL isTemporaryCertificate( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isCertificateTrust( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);

    public:

        CertificateContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ) {};
        virtual ~CertificateContainer(){};

        virtual ::sal_Bool SAL_CALL addCertificate( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name, ::sal_Bool trust ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::security::CertificateContainerStatus SAL_CALL hasCertificate( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);
        // provide factory
    static ::rtl::OUString SAL_CALL
                                        impl_getStaticImplementationName( ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                    impl_getStaticSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
                    impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ServiceManager ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                    impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
                                        getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL
                                        supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                        getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

};



#endif

