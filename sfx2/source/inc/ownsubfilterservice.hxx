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

#ifndef _OWNSUBFILTERSERVICE_HXX_
#define _OWNSUBFILTERSERVICE_HXX_

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <cppuhelper/implbase3.hxx>

class SfxObjectShell;

namespace sfx2 {

class OwnSubFilterService : public cppu::WeakImplHelper3 < ::com::sun::star::document::XFilter
                                                        ,::com::sun::star::lang::XInitialization
                                                        ,::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream;
    SfxObjectShell* m_pObjectShell;

public:

    OwnSubFilterService(const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory);
    virtual ~OwnSubFilterService();

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL
            impl_getStaticSupportedServiceNames();

    static OUString SAL_CALL impl_getStaticImplementationName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
        impl_createFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );


    // XFilter
    virtual ::sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel() throw (::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};

} // namespace sfx2

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
