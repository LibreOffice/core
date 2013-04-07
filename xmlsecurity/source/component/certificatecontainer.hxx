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

#ifndef _XCERTIFICATECONTAINER_HXX_
#define _XCERTIFICATECONTAINER_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/security/XCertificateContainer.hpp>

#include <com/sun/star/security/CertificateContainerStatus.hpp>


#include <vector>
#include <map>

using namespace com::sun::star;
using namespace cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class CertificateContainer : public ::cppu::WeakImplHelper2< ::com::sun::star::lang::XServiceInfo, ::com::sun::star::security::XCertificateContainer >
{
    private:
        typedef std::map< OUString, OUString > Map;
        Map certMap;
        Map certTrustMap;

        ::sal_Bool SAL_CALL searchMap( const OUString & url, const OUString & certificate_name, Map &_certMap  );
        virtual ::sal_Bool SAL_CALL isTemporaryCertificate( const OUString & url, const OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isCertificateTrust( const OUString & url, const OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);

    public:

        CertificateContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ) {};
        virtual ~CertificateContainer(){};

        virtual ::sal_Bool SAL_CALL addCertificate( const OUString & url, const OUString & certificate_name, ::sal_Bool trust ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::security::CertificateContainerStatus SAL_CALL hasCertificate( const OUString & url, const OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);
        // provide factory
    static OUString SAL_CALL
                                        impl_getStaticImplementationName( ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                    impl_getStaticSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                    impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL
                                        getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL
                                        supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                                        getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
