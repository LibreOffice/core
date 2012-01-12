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



#ifndef _NSSINITIALIZER_HXX
#define _NSSINITIALIZER_HXX

#include <com/sun/star/xml/crypto/XDigestContextSupplier.hpp>
#include <com/sun/star/xml/crypto/XCipherContextSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/implbase3.hxx>

#define NSS_SERVICE_NAME "com.sun.star.xml.crypto.NSSInitializer"

class ONSSInitializer : public cppu::WeakImplHelper3
<
    ::com::sun::star::xml::crypto::XDigestContextSupplier,
    ::com::sun::star::xml::crypto::XCipherContextSupplier,
    ::com::sun::star::lang::XServiceInfo
>
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;

    ONSSInitializer()
    {}

public:
    ONSSInitializer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &rxMSF );
    virtual ~ONSSInitializer();

    bool initNSS( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &xMSF );

    /* XDigestContextSupplier */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XDigestContext > SAL_CALL getDigestContext( ::sal_Int32 nDigestID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aParams ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    /* XCipherContextSupplier */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > SAL_CALL getCipherContext( ::sal_Int32 nCipherID, const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aKey, const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aInitializationVector, ::sal_Bool bEncryption, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aParams ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    /* XServiceInfo */
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);
};

rtl::OUString ONSSInitializer_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL ONSSInitializer_supportsService( const rtl::OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL ONSSInitializer_getSupportedServiceNames()
    throw ( ::com::sun::star::uno::RuntimeException );

com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL ONSSInitializer_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
    throw ( ::com::sun::star::uno::Exception );

#endif

