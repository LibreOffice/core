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

#include <sal/config.h>

#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include "securityenvironment_mscryptimpl.hxx"

#include <xmlsec/xmlstreamio.hxx>

#include "akmngr.hxx"

#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;

namespace {

class XMLSecurityContext_MSCryptImpl : public ::cppu::WeakImplHelper<
    css::xml::crypto::XXMLSecurityContext ,
    css::lang::XServiceInfo >
{
    private:
        //xmlSecKeysMngrPtr m_pKeysMngr ;
        css::uno::Reference< css::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment ;

    public:
        XMLSecurityContext_MSCryptImpl();

        //Methods from XXMLSecurityContext
        virtual sal_Int32 SAL_CALL addSecurityEnvironment(
            const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment
            ) override;

        virtual ::sal_Int32 SAL_CALL getSecurityEnvironmentNumber(  ) override;

        virtual css::uno::Reference<
            css::xml::crypto::XSecurityEnvironment > SAL_CALL
            getSecurityEnvironmentByIndex( ::sal_Int32 index ) override;

        virtual css::uno::Reference<
            css::xml::crypto::XSecurityEnvironment > SAL_CALL
            getSecurityEnvironment(  ) override;

        virtual ::sal_Int32 SAL_CALL getDefaultSecurityEnvironmentIndex(  ) override;

        virtual void SAL_CALL setDefaultSecurityEnvironmentIndex( sal_Int32 nDefaultEnvIndex ) override;


        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

}

XMLSecurityContext_MSCryptImpl::XMLSecurityContext_MSCryptImpl()
{
}

sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::addSecurityEnvironment(
    const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment)
{
    if( !aSecurityEnvironment.is() )
    {
        throw uno::RuntimeException() ;
    }

    m_xSecurityEnvironment = aSecurityEnvironment;

    return 0;
}


sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::getSecurityEnvironmentNumber(  )
{
    return 1;
}

css::uno::Reference< css::xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_MSCryptImpl::getSecurityEnvironmentByIndex( sal_Int32 index )
{
    if (index != 0)
    {
        throw uno::RuntimeException() ;
    }
    return m_xSecurityEnvironment;
}

css::uno::Reference< css::xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_MSCryptImpl::getSecurityEnvironment(  )
{
    return m_xSecurityEnvironment;
}

sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::getDefaultSecurityEnvironmentIndex(  )
{
    return 0;
}

void SAL_CALL XMLSecurityContext_MSCryptImpl::setDefaultSecurityEnvironmentIndex( sal_Int32 /*nDefaultEnvIndex*/ )
{
    //dummy
}

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContext_MSCryptImpl::getImplementationName() {
    return "com.sun.star.xml.crypto.XMLSecurityContext" ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContext_MSCryptImpl::supportsService( const OUString& serviceName) {
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
uno::Sequence< OUString > SAL_CALL XMLSecurityContext_MSCryptImpl::getSupportedServiceNames() {
    return { "com.sun.star.xml.crypto.XMLSecurityContext" };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_XMLSecurityContext_get_implementation(
    uno::XComponentContext* /*pCtx*/, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new XMLSecurityContext_MSCryptImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
