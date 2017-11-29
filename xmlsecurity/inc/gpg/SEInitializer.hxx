/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_XMLSECURITY_SOURCE_GPG_SEINITIALIZER_HXX
#define INCLUDED_XMLSECURITY_SOURCE_GPG_SEINITIALIZER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>

#include <xsecgpgdllapi.h>
#include <cppuhelper/implbase.hxx>

class XSECGPG_DLLPUBLIC SEInitializerGpg : public cppu::WeakImplHelper
<
    css::xml::crypto::XSEInitializer,
    css::lang::XServiceInfo
>
{
public:
    SEInitializerGpg();
    virtual ~SEInitializerGpg() override;

    /* XSEInitializer */
    virtual css::uno::Reference< css::xml::crypto::XXMLSecurityContext >
        SAL_CALL createSecurityContext( const OUString& ) override;

    virtual void SAL_CALL freeSecurityContext( const css::uno::Reference<
        css::xml::crypto::XXMLSecurityContext >& securityContext ) override;

    static css::uno::Sequence< OUString > impl_getSupportedServiceNames();

    static OUString impl_getImplementationName();

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) ;
    static css::uno::Reference< css::lang::XSingleServiceFactory > impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) ;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
