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
#include <rtl/ustring.hxx>
#include <framework/xmlsignaturetemplateimpl.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star::uno ;
using ::com::sun::star::lang::XMultiServiceFactory ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::crypto::XXMLSignatureTemplate ;

XMLSignatureTemplateImpl::XMLSignatureTemplateImpl()
    :m_nStatus ( css::xml::crypto::SecurityOperationStatus_UNKNOWN )
{
}

XMLSignatureTemplateImpl::~XMLSignatureTemplateImpl() {
}

/* XXMLSignatureTemplate */
void SAL_CALL XMLSignatureTemplateImpl::setTemplate( const Reference< XXMLElementWrapper >& aTemplate )
{
    m_xTemplate = aTemplate ;
}

/* XXMLSignatureTemplate */
Reference< XXMLElementWrapper > SAL_CALL XMLSignatureTemplateImpl::getTemplate()
{
    return m_xTemplate ;
}

void SAL_CALL XMLSignatureTemplateImpl::setTarget( const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& aXmlElement )
{
    targets.push_back( aXmlElement );
}

css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > > SAL_CALL XMLSignatureTemplateImpl::getTargets()
{
    return comphelper::containerToSequence(targets);
}

void SAL_CALL XMLSignatureTemplateImpl::setBinding(
    const css::uno::Reference< css::xml::crypto::XUriBinding >& aUriBinding )
{
    m_xUriBinding = aUriBinding;
}

css::uno::Reference< css::xml::crypto::XUriBinding > SAL_CALL XMLSignatureTemplateImpl::getBinding()
{
    return m_xUriBinding;
}

void SAL_CALL XMLSignatureTemplateImpl::setStatus(
    css::xml::crypto::SecurityOperationStatus status )
{
    m_nStatus = status;
}

css::xml::crypto::SecurityOperationStatus SAL_CALL XMLSignatureTemplateImpl::getStatus(  )
{
    return m_nStatus;
}

/* XServiceInfo */
OUString SAL_CALL XMLSignatureTemplateImpl::getImplementationName() {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignatureTemplateImpl::supportsService( const OUString& serviceName) {
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSignatureTemplateImpl::getSupportedServiceNames() {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSignatureTemplateImpl::impl_getSupportedServiceNames() {
    Sequence<OUString> seqServiceNames { u"com.sun.star.xml.crypto.XMLSignatureTemplate"_ustr };
    return seqServiceNames ;
}

OUString XMLSignatureTemplateImpl::impl_getImplementationName() {
    return u"com.sun.star.xml.security.framework.XMLSignatureTemplateImpl"_ustr ;
}

//Helper for registry
Reference< XInterface > XMLSignatureTemplateImpl::impl_createInstance( const Reference< XMultiServiceFactory >&  ) {
    return Reference< XInterface >( *new XMLSignatureTemplateImpl ) ;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
