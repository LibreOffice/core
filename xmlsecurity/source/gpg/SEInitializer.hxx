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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>

class SEInitializer : public cppu::WeakImplHelper< css::xml::crypto::XSEInitializer,
                                                   css::lang::XServiceInfo >
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

public:
    explicit SEInitializer(const css::uno::Reference<css::uno::XComponentContext > &rxContext);
    virtual ~SEInitializer() override;

    /* XSEInitializer */
    virtual css::uno::Reference< css::xml::crypto::XXMLSecurityContext >
        SAL_CALL createSecurityContext( const OUString& )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL freeSecurityContext( const css::uno::Reference<
        css::xml::crypto::XXMLSecurityContext >& securityContext )
        throw (css::uno::RuntimeException, std::exception) override;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw (css::uno::RuntimeException, std::exception) override;
};

/// @throws css::uno::RuntimeException
OUString SEInitializer_getImplementationName()
    throw ( css::uno::RuntimeException );

/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL SEInitializer_getSupportedServiceNames()
    throw ( css::uno::RuntimeException );

/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL SEInitializer_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rxMSF)
    throw ( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
