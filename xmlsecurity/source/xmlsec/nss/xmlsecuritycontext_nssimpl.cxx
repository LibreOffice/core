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
#include <vector>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <o3tl/safeint.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;

namespace {

class XMLSecurityContext_NssImpl
    : public ::cppu::WeakImplHelper<xml::crypto::XXMLSecurityContext, lang::XServiceInfo>
{
private:
    std::vector<uno::Reference<xml::crypto::XSecurityEnvironment>> m_vSecurityEnvironments;

    sal_Int32 m_nDefaultEnvIndex;

public:
    XMLSecurityContext_NssImpl();

    //XXMLSecurityContext
    virtual sal_Int32 SAL_CALL addSecurityEnvironment(
        const uno::Reference<xml::crypto::XSecurityEnvironment>& aSecurityEnvironment) override;

    virtual ::sal_Int32 SAL_CALL getSecurityEnvironmentNumber() override;

    virtual uno::Reference<xml::crypto::XSecurityEnvironment>
        SAL_CALL getSecurityEnvironmentByIndex(::sal_Int32 index) override;

    virtual uno::Reference<xml::crypto::XSecurityEnvironment>
        SAL_CALL getSecurityEnvironment() override;

    virtual ::sal_Int32 SAL_CALL getDefaultSecurityEnvironmentIndex() override;

    virtual void SAL_CALL setDefaultSecurityEnvironmentIndex(sal_Int32 nDefaultEnvIndex) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;

    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

}

XMLSecurityContext_NssImpl::XMLSecurityContext_NssImpl()
    : m_nDefaultEnvIndex(-1)
{
}

sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::addSecurityEnvironment(
    const uno::Reference< xml::crypto::XSecurityEnvironment >& aSecurityEnvironment)
{
    if( !aSecurityEnvironment.is() )
    {
        throw uno::RuntimeException() ;
    }

    m_vSecurityEnvironments.push_back( aSecurityEnvironment );

    return m_vSecurityEnvironments.size() - 1 ;
}


sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::getSecurityEnvironmentNumber(  )
{
    return m_vSecurityEnvironments.size();
}

uno::Reference< xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_NssImpl::getSecurityEnvironmentByIndex( sal_Int32 index )
{
    if (index < 0 || o3tl::make_unsigned(index) >= m_vSecurityEnvironments.size())
        throw uno::RuntimeException();

    uno::Reference< xml::crypto::XSecurityEnvironment > xSecurityEnvironment = m_vSecurityEnvironments[index];
    return xSecurityEnvironment;
}

uno::Reference< xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_NssImpl::getSecurityEnvironment(  )
{
    if (m_nDefaultEnvIndex < 0 || o3tl::make_unsigned(m_nDefaultEnvIndex) >= m_vSecurityEnvironments.size())
        throw uno::RuntimeException();

    return getSecurityEnvironmentByIndex(m_nDefaultEnvIndex);
}

sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::getDefaultSecurityEnvironmentIndex(  )
{
    return m_nDefaultEnvIndex ;
}

void SAL_CALL XMLSecurityContext_NssImpl::setDefaultSecurityEnvironmentIndex( sal_Int32 nDefaultEnvIndex )
{
    m_nDefaultEnvIndex = nDefaultEnvIndex;
}

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContext_NssImpl::getImplementationName() {
    return u"com.sun.star.xml.crypto.XMLSecurityContext"_ustr;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContext_NssImpl::supportsService( const OUString& serviceName) {
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
uno::Sequence< OUString > SAL_CALL XMLSecurityContext_NssImpl::getSupportedServiceNames() {
    return { u"com.sun.star.xml.crypto.XMLSecurityContext"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_XMLSecurityContext_get_implementation(
    uno::XComponentContext* /*pCtx*/, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new XMLSecurityContext_NssImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
